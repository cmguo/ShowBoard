#include "videocontrol.h"
#include "core/resourceview.h"
#include "core/resource.h"
#include "core/optiontoolbuttons.h"

#include <QGraphicsVideoItem>
#include <QMediaPlayer>

static constexpr char const * toolstr =
        "playRate||OptionsGroup,Popup,NeedUpdate|;";

VideoControl::VideoControl(ResourceView * res)
    : Control(res, {LayoutScale, Touchable})
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
        resize(size);
    });
    QMediaPlayer * player = new QMediaPlayer(this);
    QObject::connect(player, &QMediaPlayer::durationChanged, [this]() {
        loadFinished(true);
    });
    player->setVideoOutput(item);
    player->setMedia(res_->resource()->url());
    player->setVolume(50);
    player->setPlaybackRate(1);
    player->play();
    player_ = player;
}

void VideoControl::detached()
{
    //player_->setVideoOutput(static_cast<QGraphicsVideoItem*>(nullptr));
    player_->stop();
    delete player_;
    player_ = nullptr;
}

void VideoControl::resize(const QSizeF &size)
{
    QGraphicsVideoItem * item = static_cast<QGraphicsVideoItem *>(item_);
    item->setSize(size);
    item->setOffset({size.width() / -2.0, size.height() / -2.0});
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

