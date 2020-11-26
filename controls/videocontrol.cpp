#include "videocontrol.h"
#include "core/resourceview.h"
#include "core/resource.h"
#include "core/optiontoolbuttons.h"
#include <core/resourcepackage.h>
#include <core/resourcepage.h>
#include <core/toolbutton.h>
#include <media/avmediaplayerbridge.h>
#include <views/whitecanvas.h>
#include <views/whitecanvaswidget.h>

#include <QMediaService>
#include <QWindow>

#include <media/mediaplayer.h>

static constexpr char const * toolstr =
        "play()|播放|NeedUpdate,Checkable|;"
        "seek()|拖动||;"
        "playRate||OptionsGroup,Popup,NeedUpdate|;"
        "fullScreen()|全屏|;"
        "stop()|停止|;";

VideoControl::VideoControl(ResourceView * res)
    : WidgetControl(res, {
                    WithSelectBar,
                    ExpandScale,
                    LayoutScale,
                    Touchable,
                    FixedOnCanvas,
                    DelayApplySize}, {CanRotate})
    , playerBridge_(nullptr)
    , player_(nullptr)
    , fullScreenWidget_(nullptr)
{
#ifdef QT_DEBUG
    setToolsString(toolstr);
#else
    (void) toolstr;
#endif
    playerBridge_ = new AVMediaPlayerBridge(this);
}

VideoControl::~VideoControl()
{
    if (fullScreenWidget_)
        delete fullScreenWidget_;
}

bool VideoControl::isFullScreen() const
{
    return res_->url()==QString("video:full");
}


QObject *VideoControl::mediaPlayer() const
{
    return player_;
}

QWidget *VideoControl::createWidget(ControlView *parent)
{
    Q_UNUSED(parent)
    player_ = res_->property("player").value<QObject*>();
    if(!player_){
        player_ = playerBridge_->createMediaPlayer(res_->resource()->url().toString());
        player_->setParent(this);
    }
    QWidget * surfaceView = playerBridge_->createSurfaceView();
    surfaceView->resize(640, 360);
    player_->setProperty("surfaceView", QVariant::fromValue(surfaceView));
    connect(surfaceView,&QObject::destroyed,this,[this](){
        widget_ = nullptr;
    });
    return surfaceView;
}

void VideoControl::attached()
{
    if (!isFullScreen()) {
        player_->setProperty("position",
                             res_->property("position"));
        connect(player_, SIGNAL(videoStateChanged()),
                this, SLOT(loaded()));
    } else {
        loadFinished(true);
    }
}

void VideoControl::detached()
{   
    res_->setProperty("position", player_->property("position"));
    player_->setProperty("surfaceView", QVariant());
    if(!isFullScreen())
       player_->setProperty("videoState", static_cast<int>(MediaPlayer::StoppedState));
}

void VideoControl::fullScreen(bool)
{
    if (!flags_.testFlag(LoadFinished))
        return;
    if (player_->parent() != this) {
        QTimer::singleShot(0, this, [this] () {
            VideoControl * another = qobject_cast<VideoControl*>(player_->parent());
            ResourcePackage *pkgage =  qobject_cast<WhiteCanvasWidget*>(another->fullScreenWidget_)->package();
            pkgage->removePage(res_->page());
            another->player_->setProperty("surfaceView", QVariant::fromValue(another->widget()));
            another->fullScreenWidget_->hide();
        });
        return;
    }
    WhiteCanvasWidget * widget = qobject_cast<WhiteCanvasWidget*>(fullScreenWidget_);
    if (widget == nullptr) {
        widget = new WhiteCanvasWidget(WhiteCanvasWidget::mainInstance()->window());
        widget->setWindowFlag(Qt::Tool);
        widget->setAttribute(Qt::WA_NativeWindow);
        widget->windowHandle()->setSurfaceType(QSurface::RasterSurface);
        widget->setResourcePackage(new ResourcePackage(widget));
        widget->scene()->setBackgroundBrush(Qt::black);
        fullScreenWidget_ = widget;
    }
    widget->show();

    widget->package()->newPage(QUrl("video:full"), {
                                   {"deletable", false},
                                   {"pageMode", ResourceView::Independent},
                                   {"scaleMode", FullLayout},
                                   {"player", QVariant::fromValue(player_)}
                               });
}

void VideoControl::loaded()
{
    if (flags_.testFlag(LoadFinished)
            || player_->property("videoState").toInt() != MediaPlayer::State::PreparedState)
        return;
    QSizeF size = player_->property("videoSize").toSizeF();
    if(!size.isEmpty())
        resize(size);
    loadFinished(true);
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
