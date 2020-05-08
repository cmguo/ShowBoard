#include "imagecache.h"
#include "resource.h"
#include "workthread.h"

using namespace QtPromise;

static WorkThread& thread()
{
    static WorkThread th("Image");
    return th;
}

static QMutex& mutex()
{
    static QMutex mutex;
    return mutex;
}

ImageCache &ImageCache::instance()
{
    static ImageCache cache;
    return cache;
}

ImageCache::ImageCache(QObject *parent)
    : QObject(parent)
{
}

QSharedPointer<ImageData> ImageCache::get(const QUrl &url)
{
    QWeakPointer<ImageData> data(cachedImages_.value(url));
    return data.toStrongRef();
}

QtPromise::QPromise<QSharedPointer<ImageData>> ImageCache::getOrCreate(const QUrl &url, qreal mipmap)
{
    QSharedPointer<ImageData> image = get(url);
    if (image)
        return QtPromise::resolve(image);
    Resource * res = new Resource(nullptr, url);
    return res->getData().then([this, url, mipmap](QByteArray data) {
        QPixmap pixmap;
        pixmap.loadFromData(data);
        return QtPromise::resolve(put(url, pixmap, mipmap));
    }).finally([res] {
        delete res;
    });
}

QSharedPointer<ImageData> ImageCache::put(const QUrl &url, const QPixmap &pixmap, qreal mipmap)
{
    QSharedPointer<ImageData> data(new ImageData(pixmap, mipmap));
    cachedImages_.insert(url, data.toWeakRef());
    return data;
}

static void nopdel(int *) {}

ImageData::ImageData(const QPixmap pixmap, qreal mipmap)
    : pixmap_(pixmap)
    , mipmap_(mipmap)
    , life_(reinterpret_cast<int*>(1), nopdel)
{
}

ImageData::~ImageData()
{
    life_.reset();
    QMutexLocker l(&mutex());
}

QPromise<QPixmap> ImageData::load(const QSizeF &sizeHint)
{
    if (qIsNull(mipmap_)) {
        return QPromise<QPixmap>::resolve(pixmap_);
    }
    QMutexLocker l(&mutex());
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
        return QPromise<QPixmap>::resolve(pixmap);
    }
    if (QThread::currentThread() != &thread()) {
        return QPromise<QPixmap>([thiz = sharedFromThis(), sizeHint](
                                 const QPromiseResolve<QPixmap>& resolve,
                                 const QPromiseReject<QPixmap>& reject) {
            thread().postWork([=] {
                thiz->load(sizeHint).then([resolve](QPixmap const &p){resolve(p);}, reject);
            });
        });
    }
    size /= mipmap_;
    while (size.width() >= sizeHint.width() && size.height() >= sizeHint.height()) {
        pixmap = pixmap.scaledToWidth(qRound(size.width()), Qt::SmoothTransformation);
        size = pixmap.size();
        size /= mipmap_;
        mipmaps_.append(pixmap);
    }
    return QPromise<QPixmap>::resolve(pixmap);
}
