#include "imagecontrol.h"
#include "core/resourceview.h"
#include "core/resource.h"
#include "core/resourcetransform.h"
#include "core/imagecache.h"
#include "views/whitecanvas.h"

#include <QPixmap>
#include <QGraphicsPixmapItem>
#include <QCursor>

ImageControl::ImageControl(ResourceView * res, Flags flags, Flags clearFlags)
    : Control(res, flags | Flags{KeepAspectRatio, FullSelect, FixedOnCanvas}, clearFlags)
    , mipmap_(0)
{
    setMinSize({0.1, 0});
}

QGraphicsItem * ImageControl::create(ResourceView * res)
{
    (void)res;
#ifdef QT_DEBUG
    if (metaObject() == &staticMetaObject) {
        flags_ |= AutoPosition;
    }
#endif
    QGraphicsPixmapItem * item = new QGraphicsPixmapItem();
    item->setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
    item->setTransformationMode(Qt::SmoothTransformation);
    item->setCursor(Qt::SizeAllCursor);
    return item;
}

void ImageControl::attached()
{
    if (!qIsNull(mipmap_)) {
        QObject::connect(&res_->transform(), &ResourceTransform::changed,
                         this, [this] (int c) {
            if ((c & 4) && (flags_ & LoadFinished))
                adjustMipmap();
        });
    }
    data_ = ImageCache::instance().get(res_->url());
    if (data_) {
        adjustMipmap2(whiteCanvas()->rect().size());
        return;
    }
    QWeakPointer<int> l = life();
    res_->resource()->getData().then([this, l] (QByteArray data) {
        if (l.isNull()) return;
        QPixmap pixmap;
        pixmap.loadFromData(data);
        data_ = ImageCache::instance().put(res_->url(), pixmap, mipmap_);
        adjustMipmap2(whiteCanvas()->rect().size());
    }).fail([this, l](std::exception& e) {
        if (l.isNull()) return;
        loadFinished(false, e.what());
    });
}

void ImageControl::setPixmap(const QPixmap &pixmap)
{
    if (qIsNull(mipmap_)) {
        qreal scale = res_->transform().scale().m11();
        QSizeF size = item_->boundingRect().size() * scale;
        setMipMapPixmap(pixmap, size);
    } else {
        data_.reset(new ImageData(pixmap, mipmap_));
        if (!flags_.testFlag(LoadFinished)) {
            QObject::connect(&res_->transform(), &ResourceTransform::changed,
                             this, [this](int c) {
                if (c & 4) // scale
                    adjustMipmap();
            });
            adjustMipmap2(whiteCanvas()->rect().size());
        } else {
            qreal scale = res_->transform().scale().m11();
            adjustMipmap2(item_->boundingRect().size() * scale);
        }
    }
}

QPixmap ImageControl::pixmap() const
{
    return data_ ? data_->pixmap()
                 : static_cast<QGraphicsPixmapItem*>(item_)->pixmap();
}

void ImageControl::setMipMapPixmap(const QPixmap &pixmap, QSizeF const & sizeHint)
{
    QGraphicsPixmapItem * item = static_cast<QGraphicsPixmapItem*>(item_);
    if (pixmap == item->pixmap())
        return;
    qDebug() << "setMipMapPixmap" << sizeHint << pixmap.size();
    item->setPixmap(pixmap);
    if (!flags_.testFlag(LoadFinished)) {
        loadFinished(true, property("finishIcon").toString());
    } else {
        res_->transform().scaleTo(sizeHint.width() / pixmap.width());
        adjusting(true);
        sizeChanged();
        adjusting(false);
    }
}

void ImageControl::adjustMipmap()
{
    if (!data_)
        return;
    qreal scale = res_->transform().scale().m11();
    if (scale >= 1 || scale <= 1 / mipmap_) {
        adjustMipmap2(item_->boundingRect().size() * scale);
    }
}

void ImageControl::adjustMipmap2(const QSizeF &sizeHint)
{
    data_->load(sizeHint).then([this, sizeHint, l = life()] (QPixmap const &pixmap) {
        if (!l.isNull())
            setMipMapPixmap(pixmap, sizeHint);
    });
}
