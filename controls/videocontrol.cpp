#include "videocontrol.h"
#include "core/resourceview.h"
#include "core/resource.h"

#include <QGraphicsVideoItem>
#include <QMediaPlayer>

VideoControl::VideoControl(ResourceView * res)
    : Control(res, {WithSelectBar, LayoutScale})
{
}

QGraphicsItem * VideoControl::create(ResourceView * res)
{
    QGraphicsVideoItem * item = new QGraphicsVideoItem();
    item->setAspectRatioMode(Qt::KeepAspectRatio);
    QObject::connect(item, &QGraphicsVideoItem::nativeSizeChanged,
                     this, [this](QSizeF const & size) {
        QGraphicsVideoItem * item = static_cast<QGraphicsVideoItem *>(item_);
        resize(size);
        loadFinished(true);
        item->disconnect(this);
    });
    QMediaPlayer * player = new QMediaPlayer(this);
    player->setVideoOutput(item);
    player->setMedia(res->resource()->url());
    player->setVolume(50);
    player->play();
    player_ = player;
    return item;
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
