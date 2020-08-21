#ifndef FILECACHE_H
#define FILECACHE_H

#include "ShowBoard_global.h"
#include "lrucache.h"

#include <QFile>
#include <QDir>
#include <QtPromise>

struct FileResource
{
    qint64 size = -1;
    QByteArray hash; // md5
};

class SHOWBOARD_EXPORT FileCache : public LRUCache<QString, FileResource>
{
    typedef LRUCache<QString, FileResource> base;

public:
    FileCache(QDir const & dir, quint64 capacity, QByteArray algorithm = nullptr);

public:
    QtPromise::QPromise<QString> putStream(QString const & path, QByteArray const & hash, QSharedPointer<QIODevice> stream);

    QtPromise::QPromise<QString> putStream(QObject *context, QString const & path, QByteArray const & hash, std::function<QtPromise::QPromise<QSharedPointer<QIODevice>> (QObject *)> openStream);

    QString putData(QString const & path, QByteArray const & hash, QByteArray data);

public:
    virtual QSharedPointer<QIODevice> getStream(QString const & path);

    virtual QByteArray getData(QString const & path);

    virtual QString getFile(QString const & path);

    virtual QtPromise::QPromise<QString> getFileAsync(QString const & path);

    virtual bool contains(QString const & path);

    virtual FileResource get(QString const & path, QByteArray const & hash = nullptr, bool touch = true);

protected:
    virtual quint64 sizeOf(const FileResource &v) override;

    virtual bool destroy(const QString &k, const FileResource &v) override;

    virtual void loaded() {}

protected:
    void load(std::function<bool (QString const & name)> filter);

    void check(QString const & path, QByteArray const & hash);

private:
    static QtPromise::QPromise<qint64> saveStream(QString const & path, QSharedPointer<QIODevice> stream);

protected:
    QDir dir_;
    QByteArray algorithm_;
    QMap<QString, QtPromise::QPromise<QString>> asyncPuts_;
};

#endif // FILECACHE_H
