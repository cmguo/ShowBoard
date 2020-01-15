#include "imagecontrol.h"
#include "core/resourceview.h"
#include "core/resource.h"
#include "views/stateitem.h"

#include <QPixmap>
#include <QGraphicsPixmapItem>

static QMap<QUrl, QPixmap> cachedImages;

ImageControl::ImageControl(ResourceView * res)
    : Control(res, {KeepAspectRatio, FullSelect, AutoPosition})
{
}

QGraphicsItem * ImageControl::create(ResourceView * res)
{
    (void)res;
    QGraphicsPixmapItem * item = new QGraphicsPixmapItem();
    item->setTransformationMode(Qt::SmoothTransformation);
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
    loadData();
}

void ImageControl::onData(QByteArray data)
{
    QPixmap pixmap;
    pixmap.loadFromData(data);
    cachedImages[res_->url()] = pixmap;
    QGraphicsPixmapItem * item = static_cast<QGraphicsPixmapItem *>(item_);
    item->setPixmap(pixmap);
    item->setOffset(pixmap.width() / -2, pixmap.height() / -2);
}

void ImageControl::detached()
{
    cachedImages.remove(res_->url());
}
