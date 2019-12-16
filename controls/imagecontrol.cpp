#include "imagecontrol.h"
#include "core/resourceview.h"
#include "core/resource.h"
#include "views/stateitem.h"

#include <QPixmap>
#include <QGraphicsPixmapItem>

static QMap<QUrl, QPixmap> cachedImages;

ImageControl::ImageControl(ResourceView * res)
    : Control(res, {KeepAspectRatio, FullSelect})
{
}

QGraphicsItem * ImageControl::create(ResourceView * res)
{
    (void)res;
    QGraphicsPixmapItem * item = new QGraphicsPixmapItem();
    return item;
}

void ImageControl::attached()
{
    if (cachedImages.contains(res_->url())) {
        QPixmap& pixmap(cachedImages[res_->url()]);
        QGraphicsPixmapItem * item = static_cast<QGraphicsPixmapItem *>(item_);
        item->setPixmap(pixmap);
        item->setOffset(pixmap.width() / -2, pixmap.height() / -2);
        loadFinished(true);
        return;
    }
    stateItem()->setLoading();
    QWeakPointer<int> life(this->life());
    res_->resource()->getData().then([this, life](QByteArray data) {
        if (life.isNull())
            return;
        QPixmap pixmap;
        pixmap.loadFromData(data);
        cachedImages[res_->url()] = pixmap;
        QGraphicsPixmapItem * item = static_cast<QGraphicsPixmapItem *>(item_);
        item->setPixmap(pixmap);
        item->setOffset(pixmap.width() / -2, pixmap.height() / -2);
        loadFinished(true);
    }).fail([this, life](std::exception & e) {
        if (life.isNull())
            return;
        loadFinished(false, e.what());
    });
}

void ImageControl::detached()
{
    cachedImages.remove(res_->url());
}
