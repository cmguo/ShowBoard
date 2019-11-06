#include "imagecontrol.h"
#include "core/resourceview.h"
#include "core/resource.h"
#include "views/stateitem.h"

#include <QPixmap>
#include <QGraphicsPixmapItem>

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
    stateItem()->setLoading();
    QWeakPointer<int> life(this->life());
    res_->resource()->getData().then([this, life](QByteArray data) {
        if (life.isNull())
            return;
        QPixmap pixmap;
        pixmap.loadFromData(data);
        QGraphicsPixmapItem * item = static_cast<QGraphicsPixmapItem *>(item_);
        item->setPixmap(pixmap);
        item->setOffset(pixmap.width() / -2, pixmap.height() / -2);
        clearStateItem();
        initScale(pixmap.size());
    }).fail([this, life](std::exception & e) {
        if (life.isNull())
            return;
        stateItem()->setFailed(e.what());
    });
}
