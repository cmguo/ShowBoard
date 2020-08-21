#ifndef ZIPFILECACHE_H
#define ZIPFILECACHE_H

#include "filecache.h"

class SHOWBOARD_EXPORT ZipFileCache : public FileCache
{
public:
    ZipFileCache(QDir const & dir, quint64 capacity, QByteArray algorithm = nullptr);

public:
    QtPromise::QPromise<QString> putUrl(QObject * context, QString const & path, QByteArray const & hash, QUrl const & url);

    virtual QSharedPointer<QIODevice> getStream(QString const & path) override;

    virtual QByteArray getData(QString const & path) override;

protected:
    void setZipWithRootName(bool b = true);

private:
    bool zipWithRootName_;
};

#endif // ZIPFILECACHE_H
