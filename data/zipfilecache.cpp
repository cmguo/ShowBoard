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
        DataProvider * provider = DataProvider::getProvider(url.scheme().toUtf8());
        if (provider == nullptr) {
            return QPromise<QSharedPointer<QIODevice>>::reject(std::invalid_argument("打开失败，未知数据协议"));
        }
        return provider->getStream(context, url, false);
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
        // XXX/1.html
        // XXX/YYY/1.html
        int n = zipFile.lastIndexOf('/');
        if (n < 0)
            break;
        zipFile = zipFile.left(n);
        // XXX
        // XXX/YYY
        FileResource f = get(zipFile + ".zip");
        if (f.size > 0) {
            if (zipWithRootName_)
                n = zipFile.lastIndexOf('/'); // maybe -1
            // XXX.zip -> 1.html
            // XXX.zip -> XXX/1.html
            // XXX/YYY.zip -> 1.html
            // XXX/YYY.zip -> YYY/1.html
            entryName = path.mid(n + 1);
            zipFile += ".zip";
            break;
        }
        // XXX/XXX.zip -> 1.html
        // XXX/YYY/YYY.zip -> 1.html
        int n1 = zipFile.lastIndexOf('/');
        f = get(path.left(n + 1) + zipFile.mid(n1 + 1) + ".zip");
        if (f.size > 0) {
            entryName = path.mid(n + 1);
            zipFile = path.left(n + 1) + zipFile.mid(n1 + 1) + ".zip";
            break;
        }
    }
    if (!entryName.isEmpty()) {
        qDebug() << "ZipFileCache " << zipFile << entryName;
        QuaZipFile * file = new QuaZipFile(dir_.filePath(zipFile), entryName);
        if (file->open(QIODevice::ReadOnly)) {
            return QSharedPointer<QIODevice>(file);
        }
        delete file;
    }
    qWarning() << "ZipFileCache not found" << path;
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

void ZipFileCache::setZipWithRootName(bool b)
{
    zipWithRootName_ = b;
}
