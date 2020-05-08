#ifndef IMAGECACHE_H
#define IMAGECACHE_H

#include "ShowBoard_global.h"

#include <QtPromise>

#include <QMutex>
#include <QObject>
#include <QPixmap>
#include <QUrl>

class SHOWBOARD_EXPORT ImageData : public QEnableSharedFromThis<ImageData>
{
public:
    ImageData(QPixmap const pixmap, qreal mipmap);

    virtual ~ImageData();

    QPixmap pixmap() const
    {
        return pixmap_;
    }

    QtPromise::QPromise<QPixmap> load(QSizeF const & sizeHint);

private:
    Q_DISABLE_COPY(ImageData)

    QPixmap pixmap_;
    qreal mipmap_;
    QList<QPixmap> mipmaps_;
    QSharedPointer<int> life_;
};

class ImageCache : public QObject
{
    Q_OBJECT
public:
    static ImageCache & instance();

public:
    explicit ImageCache(QObject *parent = nullptr);

public:
    QSharedPointer<ImageData> get(QUrl const & url);

    QtPromise::QPromise<QSharedPointer<ImageData>> getOrCreate(QUrl const & url, qreal mipmap = 0.0);

    QSharedPointer<ImageData> put(QUrl const & url, QPixmap const & pixmap, qreal mipmap = 0.0);

private:
    // use weak pointer, not keep image in memory
    QMap<QUrl, QWeakPointer<ImageData>> cachedImages_;
};

#endif // IMAGECACHE_H
