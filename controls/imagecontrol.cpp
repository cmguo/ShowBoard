#include "imagecontrol.h"
#include "core/resourceview.h"
#include "core/resource.h"
#include "core/resourcetransform.h"
#include "data/imagecache.h"
#include "views/whitecanvas.h"

#include <QPixmap>
#include <QGraphicsPixmapItem>
#include <QPen>
#include <QMimeData>

ImageControl::ImageControl(ResourceView * res, Flags flags, Flags clearFlags)
    : Control(res, flags | Flags{KeepAspectRatio, FullSelect, FixedOnCanvas}, clearFlags)
    , borderSize_(0)
    , mipmap_(0)
    , mipScale_(1.0)
{
    setMinSize({0.1, 0});
}

void ImageControl::setBorderSize(qreal borderSize)
{
    borderSize_ = borderSize;
    QGraphicsRectItem * item = static_cast<QGraphicsRectItem*>(item_);
    if (qFuzzyIsNull(borderSize_))
        item->setPen(Qt::NoPen);
    else
        item->setPen(QPen(Qt::gray, borderSize_));
}

void ImageControl::setMipmap(qreal mipmap)
{
    mipmap_ = mipmap;
}

ControlView * ImageControl::create(ControlView * parent)
{
    (void)parent;
#ifdef QT_DEBUG
    if (metaObject() == &staticMetaObject) {
        flags_ |= AutoPosition;
    }
#endif
#ifdef SHOWBOARD_QUICK
    return nullptr;
#else
    QGraphicsRectItem * item = new QGraphicsRectItem;
    // TODO: fix not update on z-order changed
    //item->setFlag(QGraphicsItem::ItemHasNoContents);
    item->setPen(Qt::NoPen);
    image_ = new QGraphicsPixmapItem(item);
    image_->setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
    image_->setTransformationMode(Qt::SmoothTransformation);
    return item;
#endif
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
    if (auto * data = res_->mimeData()) {
        data_.reset(new ImageData(data->imageData().value<QPixmap>(), mipmap_));
        adjustMipmap2(whiteCanvas()->rect().size());
        return;
    }
    ImageCache::instance().getOrCreate(res_, res_->url(), mipmap_).then([this, l = life()] (QSharedPointer<ImageData> data) {
        if (l.isNull()) return;
        data_ = data;
        adjustMipmap2(whiteCanvas()->rect().size());
    }, [this, l = life()](std::exception &) {
        if (l.isNull()) return;
        loadFailed();
    });
}

void ImageControl::copy(QMimeData &data)
{
    Control::copy(data);
    if (data_)
        data.setImageData(data_->pixmap().toImage());
}

void ImageControl::setPixmap(const QPixmap &pixmap)
{
    if (qIsNull(mipmap_)) {
        qreal scale = res_->transform().zoom();
        QSizeF size = initImageSize_ * scale;
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
            qreal scale = res_->transform().zoom();
            adjustMipmap2(item_->boundingRect().size() * scale);
        }
    }
}

QPixmap ImageControl::pixmap() const
{
#ifdef SHOWBOARD_QUICK
    return QPixmap();
#else
    return data_ ? data_->pixmap() : image_->pixmap();
#endif
}

void ImageControl::setMipMapPixmap(const QPixmap &pixmap, QSizeF const & sizeHint)
{
    if (pixmap.cacheKey() == image_->pixmap().cacheKey())
        return;
    qDebug() << "ImageControl setMipMapPixmap" << sizeHint << pixmap.size();
    image_->setPixmap(pixmap);
    QSizeF size = pixmap.size();
    image_->setOffset(-size.width() / 2, -size.height() / 2);
    if (!flags_.testFlag(LoadFinished)) {
#ifdef SHOWBOARD_QUICK
#else
        QGraphicsRectItem * item = static_cast<QGraphicsRectItem*>(item_);
        initImageSize_ = size;
        if (pixmap.hasAlpha())
            setBorderSize(0);
        qDebug() << "ImageControl initImageSize" << initImageSize_;
        qreal adjust = borderSize_ / 2;
        item->setRect(image_->boundingRect().adjusted(-adjust, -adjust, adjust, adjust));
#endif
        loadFinished(true, property("finishIcon").toString());
        adjust /= res_->transform().zoom();
        QPen pen = item->pen();
        pen.setWidthF(borderSize_ / res_->transform().zoom());
        item->setPen(pen);
        // ignore size change notify
        item->setRect(image_->boundingRect().adjusted(-adjust, -adjust, adjust, adjust));
        adjustMipmap();
    } else {
        mipScale_ = initImageSize_.width() / size.width();
        image_->setTransform(QTransform::fromScale(mipScale_, mipScale_));
    }
}

void ImageControl::adjustMipmap()
{
    if (!data_)
        return;
    qreal scale = res_->transform().zoom() * mipScale_;
    if (scale >= 1 || scale <= 1 / mipmap_) {
        adjustMipmap2(image_->boundingRect().size() * scale);
    }
}

void ImageControl::adjustMipmap2(const QSizeF &sizeHint)
{
    data_->load(sizeHint).then([this, sizeHint, l = life()] (QPixmap const &pixmap) {
        if (!l.isNull())
            setMipMapPixmap(pixmap, sizeHint);
    }, [this, l = life()](std::exception & e) {
        if (!l.isNull())
            data_.reset();
        (void) e;
    });
}
