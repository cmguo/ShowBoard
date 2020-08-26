#include "imagecache.h"
#include "core/oomhandler.h"
#include "core/resource.h"
#include "core/workthread.h"

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
    oomHandler.addHandler(0, std::bind(&ImageCache::dropOneImage, this));
}

QSharedPointer<ImageData> ImageCache::get(const QUrl &url)
{
    QWeakPointer<ImageData> data(cachedImages_.value(url));
    if (data.isNull())
        cachedImages_.remove(url);
    return data.toStrongRef();
}

QtPromise::QPromise<QSharedPointer<ImageData>> ImageCache::getOrCreate(QObject * context, const QUrl &url, qreal mipmap)
{
    QSharedPointer<ImageData> image = get(url);
    if (image)
        return QtPromise::resolve(image);
    if (pendings_.contains(url)) {
        return pendings_.find(url).value();
    }
    QtPromise::QPromise<QSharedPointer<ImageData>> p =
            Resource::getData(context, url).then([this, url, mipmap](QByteArray data) {
        if (data.size() < 500 * 1024) {
            QPixmap pixmap;
            if (pixmap.loadFromData(data))
                return QtPromise::resolve(put(url, pixmap, mipmap));
            else
                throw std::runtime_error("图片加载失败");
        } else {
            return load(data).then([this, url, mipmap] (QPixmap pixmap) {
                return QtPromise::resolve(put(url, pixmap, mipmap));
            });
        }
    }).finally([this, url] {
        pendings_.remove(url);
    });
    pendings_.insert(url, p);
    return p;
}

QtPromise::QPromise<QSharedPointer<ImageData> > ImageCache::getOrCreate(const QUrl &url, qreal mipmap)
{
    return getOrCreate(nullptr, url, mipmap);
}

QSharedPointer<ImageData> ImageCache::put(const QUrl &url, const QPixmap &pixmap, qreal mipmap)
{
    QSharedPointer<ImageData> data(new ImageData(pixmap, mipmap));
    cachedImages_.insert(url, data.toWeakRef());
    return data;
}

QtPromise::QPromise<QPixmap> ImageCache::load(QByteArray data)
{
    return QPromise<QPixmap>([data](
                             const QPromiseResolve<QPixmap>& resolve,
                             const QPromiseReject<QPixmap>& reject) {
        ::thread().postWork([=] {
            QPixmap pixmap;
            if (pixmap.loadFromData(data))
                resolve(pixmap);
            else
                reject(std::runtime_error("图片加载失败"));
        });
    });
}

bool ImageCache::dropOneImage()
{
    if (cachedImages_.isEmpty())
        return false;
    QSharedPointer<ImageData> image = cachedImages_.take(
                cachedImages_.firstKey()).toStrongRef();
    if (image == nullptr)
        return false;
    image->clear();
    return true;
}

static void nopdel(int *) {}

ImageData::ImageData(const QPixmap pixmap, qreal mipmap)
    : pixmap_(pixmap)
    , mipmap_(mipmap)
    , life_(reinterpret_cast<int*>(1), nopdel)
{
    constexpr QSize maxSize = sizeof (void*) == 4 ? QSize{3840, 2160} : QSize{7680, 4320};
    if (qFuzzyIsNull(mipmap_)) {
        QSize size = pixmap.size();
        while (size.width() > maxSize.width() || size.height() > maxSize.height()) {
            size /= 2;
        }
        if (size != pixmap.size()) {
            qDebug() << "ImageData downsize" << pixmap.size() << "->" << size;
            pixmap_ = pixmap.scaled(size);
        }
    } else {
        QSizeF size = pixmap.size();
        while (size.width() >= maxSize.width() && size.height() >= maxSize.height()) {
            size /= mipmap_;
            pixmap_ = pixmap_.scaledToWidth(qRound(size.width()), Qt::SmoothTransformation);
            size = pixmap_.size();
        }
        if (size != pixmap.size())
            qDebug() << "ImageData downsize" << pixmap.size() << "->" << size;
    }
}

ImageData::~ImageData()
{
    life_.reset();
    QMutexLocker l(&mutex());
}

QPromise<QPixmap> ImageData::load(const QSizeF &sizeHint)
{
    if (pixmap_.isNull())
        return QPromise<QPixmap>::reject(std::runtime_error("图片已经释放"));
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

void ImageData::clear()
{
    pixmap_ = QPixmap();
    mipmaps_.clear();
}

