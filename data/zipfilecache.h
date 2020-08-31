#ifndef ZIPFILECACHE_H
#define ZIPFILECACHE_H

#include "filecache.h"

class QuaZip;

class SHOWBOARD_EXPORT ZipFileCache : public FileCache
{
public:
    ZipFileCache(QDir const & dir, quint64 capacity, QByteArray algorithm = nullptr);

public:
    virtual QSharedPointer<QIODevice> getStream(QString const & path) override;

    virtual QByteArray getData(QString const & path) override;

public:
    void lock(QStringList const & files);

    void unlock(QStringList const & files);

protected:
    virtual bool destroy(const QString &k, const FileResource &v) override;

protected:
    void setZipWithRootName(bool b = true);

private:
    bool zipWithRootName_;
    QMap<QString, QuaZip*> lockedFiles_;
};

#endif // ZIPFILECACHE_H
