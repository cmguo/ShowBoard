#include "imagecontrol.h"
#include "resourceview.h"

#include <QPixmap>
#include <QGraphicsPixmapItem>

ImageControl::ImageControl(ResourceView * res)
    : Control(res, KeepAspectRatio)
{
}

QGraphicsItem * ImageControl::create(ResourceView * res)
{
    QGraphicsPixmapItem * item = new QGraphicsPixmapItem();
    res->getData().then([this, item](QByteArray data) {
        QPixmap pixmap;
        pixmap.loadFromData(data);
        item->setPixmap(pixmap);
        item->setOffset(pixmap.width() / -2, pixmap.height() / -2);
        sizeChanged(pixmap.size());
    });
    return item;
}
