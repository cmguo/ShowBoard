#include "dataprovider.h"
#include "zipfilecache.h"

#include <quazipfile.h>

using namespace QtPromise;

ZipFileCache::ZipFileCache(const QDir &dir, quint64 capacity, QByteArray algorithm)
    : FileCache(dir, capacity, algorithm)
{
    load([] (QString const & f) {
        return f.endsWith(".zip");
    });
}

QtPromise::QPromise<QString> ZipFileCache::putUrl(QObject * context, const QString &path, const QByteArray &hash, const QUrl &url)
{
    return putStream(context, path, hash, [url](QObject * context) {
        DataProvider * provider = DataProvider::getInstance(url.scheme().toUtf8());
        return provider->getStream(context, url, true);
    });
}

QSharedPointer<QIODevice> ZipFileCache::getStream(QString const & path)
{
    QSharedPointer<QIODevice> stream = FileCache::getStream(path);
    if (stream)
        return stream;
    QString zipFile = path;
    QString entryName;
    while (true) {
        int n = zipFile.lastIndexOf('/');
        if (n < 0)
            break;
        zipFile = zipFile.left(n);
        FileResource f = get(zipFile + ".zip");
        if (f.size > 0) {
            entryName = path.mid(zipFile.length() + 1);
            zipFile += ".zip";
            break;
        }
    }
    if (!entryName.isEmpty()) {
        QuaZipFile * file = new QuaZipFile(dir_.filePath(zipFile), entryName);
        if (file->open(QIODevice::ReadOnly)) {
            return QSharedPointer<QIODevice>(file);
        }
        delete file;
    }
    return nullptr;
}

QByteArray ZipFileCache::getData(QString const & path)
{
    QSharedPointer<QIODevice> stream = getStream(path);
    if (stream) {
        QByteArray data = stream->readAll();
        stream->close();
        return data;
    }
    return nullptr;
}
