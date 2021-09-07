#ifndef FILELRUCACHE_H
#define FILELRUCACHE_H

#include "filecache.h"

#include <QUrl>

struct FileLRUResource
{
    QString path;
    qint64 size;
};

class SHOWBOARD_EXPORT UrlFileCache : public FileCache
{
    typedef LRUCache<QByteArray, FileLRUResource> base;

public:
    UrlFileCache(QDir const & dir, quint64 capacity);

public:
    QtPromise::QPromise<QString> putStream(QUrl const & url, QSharedPointer<QIODevice> stream);

    QtPromise::QPromise<QString> putStream(QObject *context, QUrl const & url, std::function<QtPromise::QPromise<QSharedPointer<QIODevice>> (QObject *)> openStream);

    QString putData(QUrl const & url, QByteArray data);

    QtPromise::QPromise<QString> putUrl(QObject * context, QUrl const & url);

    QSharedPointer<QIODevice> getStream(QUrl const & url);

    QByteArray getData(QUrl const & url);

    QString getFile(QUrl const & url);

    QtPromise::QPromise<QString> getFileAsync(QUrl const & url);

    bool contains(QUrl const & url);

    bool remove(QUrl const & url);

    FileLRUResource get(QUrl const & url, bool put = false);

private:
    static QString md5Path(QUrl const & url);
};

#endif // FILELRUCACHE_H
