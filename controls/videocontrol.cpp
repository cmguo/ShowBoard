#include "videocontrol.h"
#include "core/resourceview.h"
#include "core/resource.h"
#include "core/optiontoolbuttons.h"

#include <core/resourcepackage.h>
#include <core/resourcepage.h>
#include <core/toolbutton.h>
#include <views/whitecanvas.h>
#include <views/whitecanvaswidget.h>

#include <QGraphicsVideoItem>
#include <QMediaPlayer>
#include <QWindow>

static constexpr char const * toolstr =
        "play()|播放|NeedUpdate,Checkable|;"
        "playRate||OptionsGroup,Popup,NeedUpdate|;"
        "fullScreen()|全屏|;"
        "stop()|停止|;";

VideoControl::VideoControl(ResourceView * res)
    : Control(res, {KeepAspectRatio, LayoutScale, Touchable, FixedOnCanvas})
    , player_(nullptr)
    , fullscreen_(nullptr)
{
    setToolsString(toolstr);
}

qreal VideoControl::playRate() const
{
    return player_->playbackRate();
}

void VideoControl::setPlayRate(qreal v)
{
    player_->setPlaybackRate(v);
}

void VideoControl::play()
{
    if (player_->state() != QMediaPlayer::PlayingState)
        player_->play();
    else
        player_->pause();
}

void VideoControl::fullScreen()
{
    if (player_->parent() != this) {
        QTimer::singleShot(0, this, [this] () {
            VideoControl * another = qobject_cast<VideoControl*>(player_->parent());
            res_->page()->removeFromPackage();
            another->fullscreen_->hide();
        });
        return;
    }
    WhiteCanvasWidget * widget = qobject_cast<WhiteCanvasWidget*>(fullscreen_);
    if (widget == nullptr) {
        widget = new WhiteCanvasWidget();
        widget->setAttribute(Qt::WA_NativeWindow);
        widget->windowHandle()->setSurfaceType(QSurface::RasterSurface);
        widget->setResourcePackage(new ResourcePackage(widget));
        widget->scene()->setBackgroundBrush(Qt::black);
        fullscreen_ = widget;
    }
    widget->show();
    widget->package()->newPage(QUrl("video:full"), {
                                   {"pageMode", ResourceView::Independent},
                                   {"deletable", false},
                                   {"scaleMode", FullLayout},
                                   {"player", QVariant::fromValue(player_)}
                               });
}

void VideoControl::stop()
{
    player_->stop();
}

QGraphicsItem * VideoControl::create(ResourceView *)
{
    QGraphicsVideoItem * item = new QGraphicsVideoItem();
    item->setAspectRatioMode(Qt::KeepAspectRatio);
    item->setCursor(Qt::SizeAllCursor);
    return item;
}

void VideoControl::attached()
{
    QGraphicsVideoItem * item = static_cast<QGraphicsVideoItem *>(item_);
    QObject::connect(item, &QGraphicsVideoItem::nativeSizeChanged,
                     this, [this](QSizeF const & size) {
        if (!flags_.testFlag(FullLayout))
            resize(size);
        loadFinished(true);
        sender()->disconnect(this);
    });
    player_ = res_->property("player").value<QMediaPlayer*>();
    if (player_ == nullptr) {
        QMediaPlayer * player = new QMediaPlayer(this);
        player->setMedia(res_->resource()->url());
        player->setVolume(50);
        player->setPlaybackRate(1);
        player_ = player;
    }
    player_->setVideoOutput(item);
    QObject::connect(player_, &QMediaPlayer::stateChanged, this, [this] () {
        raiseButtonsChanged();
    });
}

void VideoControl::detached()
{
    if (player_->parent() == this) {
        player_->stop();
#if QT_VERSION >= 0x050F00 // 5.15.0
        player_->setVideoOutput(static_cast<QGraphicsVideoItem*>(nullptr));
#endif
        delete player_;
        delete fullscreen_;
    } else {
        player_->disconnect(this);
        VideoControl * another = qobject_cast<VideoControl*>(player_->parent());
        player_->setVideoOutput(static_cast<QGraphicsVideoItem*>(another->item_));
    }
    player_ = nullptr;
}

void VideoControl::resize(const QSizeF &size)
{
    QGraphicsVideoItem * item = static_cast<QGraphicsVideoItem *>(item_);
    item->setSize(size);
    item->setOffset({size.width() / -2.0, size.height() / -2.0});
}

void VideoControl::updateToolButton(ToolButton *button)
{
    if (button->name() == "play()") {
        bool playing = player_->state() == QMediaPlayer::PlayingState;
        button->setChecked(playing);
        button->setText(playing ? "暂停" : "播放");
    }
    Control::updateToolButton(button);
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

