#include "dataprovider.h"
#include "zipfilecache.h"

#include <quazipfile.h>

using namespace QtPromise;

ZipFileCache::ZipFileCache(const QDir &dir, quint64 capacity, QByteArray algorithm)
    : FileCache(dir, capacity, algorithm)
{
    load([] (QString const & f) {
        return f.endsWith(".zip") || f.endsWith(".zip.temp");
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
        std::lock_guard<std::mutex> l(FileCache::lock());
        if (lockedFiles_.contains(zipFile)) {
            QuaZip *& zip = lockedFiles_[zipFile];
            if (zip == nullptr) {
                zip = new QuaZip(dir_.filePath(zipFile));
                zip->open(QuaZip::mdUnzip);
            }
            if (!zip->isOpen())
                return nullptr;
            zip->setCurrentFile(entryName);
            QuaZipFile * file = new QuaZipFile(zip);
            if (file->open(QIODevice::ReadOnly)) {
                return QSharedPointer<QIODevice>(file);
            }
            delete file;
        } else {
            QuaZipFile * file = new QuaZipFile(dir_.filePath(zipFile), entryName);
            if (file->open(QIODevice::ReadOnly)) {
                return QSharedPointer<QIODevice>(file);
            }
            delete file;
        }
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

void ZipFileCache::lock(const QStringList &files)
{
    std::lock_guard<std::mutex> l(FileCache::lock());
    for (auto & f : files) {
        if (!lockedFiles_.contains(f))
            lockedFiles_.insert(f, nullptr);
    }
}

void ZipFileCache::unlock(const QStringList &files)
{
    std::lock_guard<std::mutex> l(FileCache::lock());
    for (auto & f : files) {
        auto zip = lockedFiles_.take(f);
        delete zip;
    }
}

bool ZipFileCache::destroy(const QString &k, const FileResource &v)
{
    // in lock
    if (lockedFiles_.contains(k))
        return false;
    return FileCache::destroy(k, v);
}

void ZipFileCache::setZipWithRootName(bool b)
{
    zipWithRootName_ = b;
}
