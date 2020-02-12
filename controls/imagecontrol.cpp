#include "imagecontrol.h"
#include "core/resourceview.h"
#include "core/resource.h"
#include "core/resourcetransform.h"
#include "core/workthread.h"
#include "views/whitecanvas.h"

#include <QPixmap>
#include <QGraphicsPixmapItem>
#include <QCursor>

class ImageData
{
public:
    static QSharedPointer<ImageData> get(QUrl const & url);

    static QSharedPointer<ImageData> put(QUrl const & url, QPixmap const & pixmap, qreal mipmap);

public:
    ImageData(QPixmap const pixmap, qreal mipmap);

    void load(ImageControl * context, QSizeF const & sizeHint);

private:
    static QMutex mutex_;
    static WorkThread thread_;
    static QMap<QUrl, QWeakPointer<ImageData>> cachedImages_;

private:
    QPixmap pixmap_;
    qreal mipmap_;
    QList<QPixmap> mipmaps_;
};

ImageControl::ImageControl(ResourceView * res, Flags flags, Flags clearFlags)
    : Control(res, flags | Flags{KeepAspectRatio, FullSelect}, clearFlags)
    , mipmap_(0)
{
    setMinSize({0.1, 0});
}

QGraphicsItem * ImageControl::create(ResourceView * res)
{
    (void)res;
    if (metaObject() == &staticMetaObject) {
        flags_ |= AutoPosition;
    }
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
                         this, &ImageControl::adjustMipmap);
    }
    data_ = ImageData::get(res_->url());
    if (data_) {
        data_->load(this, whiteCanvas()->rect().size());
        return;
    }
    QWeakPointer<int> l = life();
    res_->resource()->getData().then([this, l] (QByteArray data) {
        if (l.isNull()) return;
        QPixmap pixmap;
        pixmap.loadFromData(data);
        data_ = ImageData::put(res_->url(), pixmap, mipmap_);
        data_->load(this, whiteCanvas()->rect().size());
    }).fail([this, l](std::exception& e) {
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
                             this, &ImageControl::adjustMipmap);
            data_->load(this, whiteCanvas()->rect().size());
        } else {
            qreal scale = res_->transform().scale().m11();
            QSizeF size = item_->boundingRect().size() * scale;
            data_->load(this, size);
        }
    }
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
    qreal scale = res_->transform().scale().m11();
    if (scale >= 1 || scale <= 1 / mipmap_) {
        QSizeF size = item_->boundingRect().size() * scale;
        data_->load(this, size);
    }
}

QMutex ImageData::mutex_;
WorkThread ImageData::thread_("Image");
QMap<QUrl, QWeakPointer<ImageData>> ImageData::cachedImages_;

QSharedPointer<ImageData> ImageData::get(const QUrl &url)
{
    QWeakPointer<ImageData> data(cachedImages_.value(url));
    return data.toStrongRef();
}

QSharedPointer<ImageData> ImageData::put(const QUrl &url, const QPixmap &pixmap, qreal mipmap)
{
    QSharedPointer<ImageData> data(new ImageData(pixmap, mipmap));
    cachedImages_.insert(url, data.toWeakRef());
    return data;
}

ImageData::ImageData(const QPixmap pixmap, qreal mipmap)
    : pixmap_(pixmap)
    , mipmap_(mipmap)
{
}

void ImageData::load(ImageControl *context, const QSizeF &sizeHint)
{
    if (qIsNull(mipmap_)) {
        context->setMipMapPixmap(pixmap_, sizeHint);
        return;
    }
    QMutexLocker l(&mutex_);
    QPixmap pixmap(mipmaps_.isEmpty() ? pixmap_ : mipmaps_.back());
    QSizeF size = pixmap.size();
    if (size.width() < sizeHint.width() || size.height() < sizeHint.height()) {
        if (!mipmaps_.isEmpty()) {
            for (int i = mipmaps_.size() - 1; i >= -1; --i) {
                pixmap = i >= 0 ? mipmaps_[i] : pixmap_;
                size = pixmap.size();
                if (size.width() >= sizeHint.width() && size.height() >= sizeHint.height()) {
                    break;
                }
            }
        }
        l.unlock();
        context->setMipMapPixmap(pixmap, sizeHint);
        return;
    }
    if (QThread::currentThread() != &thread_) {
        thread_.postWork([this, context, sizeHint]() {
            load(context, sizeHint);
        });
        return;
    }
    size /= mipmap_;
    while (size.width() >= sizeHint.width() && size.height() >= sizeHint.height()) {
        pixmap = pixmap.scaledToWidth(qRound(size.width()), Qt::SmoothTransformation);
        size = pixmap.size();
        size /= mipmap_;
        mipmaps_.append(pixmap);
    }
    WorkThread::postWork(context, [context, pixmap, sizeHint]() {
        context->setMipMapPixmap(pixmap, sizeHint);
    });
}
