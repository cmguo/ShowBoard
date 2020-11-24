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

    void clear();

private:
    Q_DISABLE_COPY(ImageData)

    QPixmap pixmap_;
    qreal mipmap_;
    QList<QPixmap> mipmaps_;
    QSharedPointer<int> life_;
};

class SHOWBOARD_EXPORT ImageCache : public QObject
{
    Q_OBJECT
public:
    static ImageCache & instance();

private:
    explicit ImageCache(QObject *parent = nullptr);

public:
    QSharedPointer<ImageData> get(QUrl const & url);

    QtPromise::QPromise<QSharedPointer<ImageData>> getOrCreate(QObject * context, QUrl const & url, qreal mipmap = 0.0);

    QtPromise::QPromise<QSharedPointer<ImageData>> getOrCreate(QUrl const & url, qreal mipmap = 0.0);

    QSharedPointer<ImageData> put(QUrl const & url, QPixmap const & pixmap, qreal mipmap = 0.0);

signals:
    void onLoadError(QObject * context, QUrl const & url);

private:
    static QtPromise::QPromise<QPixmap> load(QByteArray data);

    bool dropOneImage();

private:
    // use weak pointer, not keep image in memory
    QMap<QUrl, QWeakPointer<ImageData>> cachedImages_;
    QMap<QUrl, QtPromise::QPromise<QSharedPointer<ImageData>>> pendings_;
};

#endif // IMAGECACHE_H
