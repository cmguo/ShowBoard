#include "videocontrol.h"
#include "core/resourceview.h"
#include "core/resource.h"
#include "core/optiontoolbuttons.h"
#include "media/mediaplayer.h"

#include <core/resourcepackage.h>
#include <core/resourcepage.h>
#include <core/toolbutton.h>
#include <media/avmediaplayer.h>
#include <views/whitecanvas.h>
#include <views/whitecanvaswidget.h>

#include <QMediaService>
#include <QWindow>

static constexpr char const * toolstr =
        "play()|播放|NeedUpdate,Checkable|;"
        "seek()|拖动||;"
        "playRate||OptionsGroup,Popup,NeedUpdate|;"
        "fullScreen()|全屏|;"
        "stop()|停止|;";

VideoControl::VideoControl(ResourceView * res)
    : WidgetControl(res, {WithSelectBar, ExpandScale, LayoutScale, Touchable, FixedOnCanvas}, {CanRotate})
    , player_(nullptr)
    , fullScreenWidget_(nullptr)
{
    //setToolsString(toolstr);

}

VideoControl::~VideoControl()
{
    if(fullScreenWidget_)
        delete fullScreenWidget_;
}

bool VideoControl::isFullScreen() const
{
    return flags_.testFlag(FullLayout);
}


QObject *VideoControl::mediaPlayer() const
{
    return player_;
}

void VideoControl::fullScreen(bool)
{
    if(!flags_.testFlag(LoadFinished))
        return;
    if (player_->parent() != this) {
          QTimer::singleShot(0, this, [this] () {
              VideoControl * another = qobject_cast<VideoControl*>(player_->parent());
              ResourcePackage *pkgage =  qobject_cast<WhiteCanvasWidget*>(another->fullScreenWidget_)->package();
              pkgage->removePage(res_->page());
              another->fullScreenWidget_->hide();
          });
          return;
      }
    WhiteCanvasWidget * widget = qobject_cast<WhiteCanvasWidget*>(fullScreenWidget_);
    if (widget == nullptr) {
        widget = new WhiteCanvasWidget();
        widget->setAttribute(Qt::WA_NativeWindow);
        widget->windowHandle()->setSurfaceType(QSurface::RasterSurface);
        widget->setResourcePackage(new ResourcePackage(widget));
        widget->scene()->setBackgroundBrush(Qt::black);
        fullScreenWidget_ = widget;
    }
    widget->show();

    widget->package()->newPage(QUrl("video:full"), {
                                   {"pageMode", ResourceView::Independent},
                                   {"deletable", false},
                                   {"scaleMode", FullLayout},
                                   {"player", QVariant::fromValue(player_)}
                               });
}


QWidget *VideoControl::createWidget(ControlView *parent)
{
    Q_UNUSED(parent)
    player_ = res_->property("player").value<MediaPlayer*>();
    if(!player_){
      player_ = new AVMediaPlayer(this);
      player_->setUrl(res_->resource()->url().toString());
    }
    QWidget * vo = player_->createRenderer();
    connect(vo,&QObject::destroyed,this,[this](){
        widget_ = nullptr;
    });
    return vo;
}

void VideoControl::attached()
{
    loadFinished(true);
    player_->showNextFrame();
    if(!flags_.testFlag(FullLayout))
      player_->setProperty("position",res_->property("position"));
}

void VideoControl::detached()
{   
    res_->setProperty("position",player_->property("position"));
    player_->removeRenderer(widget_);
    if(flags_.testFlag(FullLayout)) return;
       player_->pause();
}


class PlayRateOptionButtons : public OptionToolButtons
{
public:
    PlayRateOptionButtons()
        : OptionToolButtons(QList<qreal>{2, 1.5, 1.25, 1, 0.75, 0.5}, 1)
    {
    }

protected:
    QString buttonTitle(const QVariant &value) override
    {
        return QString("X ") + value.toString();
    }
};

static PlayRateOptionButtons playRateButtons;

REGISTER_OPTION_BUTTONS(VideoControl, playRate, playRateButtons)
