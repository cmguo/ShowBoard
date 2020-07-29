#include "lrucache.h"

#include <QDir>
#include <QUrl>
#include <QCryptographicHash>
#include <QDateTime>
#include <QBuffer>

using namespace QtPromise;

FileLRUCache::FileLRUCache(const QDir &dir, quint64 capacity)
    : base(capacity)
    , dir_(dir)
{
    dir.mkpath(dir.path());
    for (QFileInfo const & f : dir.entryInfoList(QDir::NoFilter, QDir::Time)) {
        if (f.fileName().length() == 32 || f.fileName().lastIndexOf('.') == 32) {
            base::put(f.fileName().toUtf8().left(32), {
                          f.filePath(), f.size()});
        } else {
            QFile::remove(f.filePath());
        }
    }
}

QPromise<QString> FileLRUCache::putStream(const QUrl &url, QSharedPointer<QIODevice> stream)
{
    FileLRUResource f = get(url, true);
    if (f.size >= 0) { // not replace old
        return QPromise<QString>::resolve(f.path);
    }
    auto iter = asyncPuts_.find(url);
    if (iter != asyncPuts_.end())
        return iter.value();
    QSharedPointer<QIODevice> file(new QFile(f.path + ".temp"));
    if (!file->open(QFile::WriteOnly)) {
        return QPromise<QString>::reject(std::runtime_error("文件打开失败"));
    }
    QPromise<QString> asyncPut = QPromise<qint64>([file, stream](
                             const QPromiseResolve<qint64>& resolve,
                             const QPromiseReject<qint64>& reject) {
        QSharedPointer<qint64> size(new qint64(0));
        QObject::connect(stream.get(), &QIODevice::readyRead, [=] () {
            QByteArray data = stream->readAll();
            if (data.isEmpty()) {
                *size = -1;
                file->close();
                reject(std::runtime_error("文件下载失败"));
                return;
            }
            if (file->write(data) != data.size()) {
                file->close();
                *size = -1;
                reject(std::runtime_error("文件写入失败"));
                return;
            }
            *size += static_cast<quint64>(data.size());
        });
        QObject::connect(stream.get(), &QIODevice::readChannelFinished, [=] () {
            if (*size >= 0) {
                file->close();
                resolve(*size);
            }
        });
    }).then([this, f, file] (qint64 size) {
        if (!QFile::rename(f.path + ".temp", f.path)) {
            QFile::remove(f.path + ".temp");
            throw std::runtime_error("文件写入失败");
        }
        FileLRUResource f2 = f;
        f2.size = size;
        base::put(f.path.mid(f.path.lastIndexOf('/') + 1).left(32).toUtf8(), f2);
        return f.path;
    }, [f] (std::exception &) -> QString {
        QFile::remove(f.path + ".temp");
        throw;
    }).finally([this, url] () {
        asyncPuts_.remove(url);
    });
    asyncPuts_.insert(url, asyncPut);
    return asyncPut;
}

QSharedPointer<QIODevice> FileLRUCache::getStream(const QUrl &url)
{
    FileLRUResource f = get(url);
    if (f.path.isEmpty())
        return nullptr;
    QFile * file = new QFile(f.path);
    file->open(QFile::ReadOnly);
    return QSharedPointer<QIODevice>(file);
}

QString FileLRUCache::putData(const QUrl &url, QByteArray data)
{
    FileLRUResource f = get(url, true);
    if (f.size >= 0) {
        return f.path;
    }
    QFile file(f.path + ".temp");
    bool ok = file.open(QFile::WriteOnly);
    ok = ok && file.write(data) == data.size();
    file.close();
    ok = ok && QFile::rename(file.fileName(), f.path);
    if (!ok) {
        file.remove();
        return nullptr;
    }
    return f.path;
}

QByteArray FileLRUCache::getData(const QUrl &url)
{
    FileLRUResource f = get(url);
    if (f.path.isEmpty())
        return QByteArray();
    QFile file(f.path);
    if (!file.open(QFile::ReadOnly)) {
        base::remove(urlMd5(url)); // rarely happen
        return QByteArray();
    }
    QByteArray data = file.readAll();
    file.close();
    return data;
}

QString FileLRUCache::getFile(const QUrl &url)
{
    return get(url).path;
}

bool FileLRUCache::contains(const QUrl &url)
{
    return !get(url).path.isEmpty();
}

bool FileLRUCache::remove(const QUrl &url)
{
    QByteArray md5 = urlMd5(url);
    FileLRUResource f = base::get(md5);
    if (f.path.isEmpty())
        return false;
    int n = f.path.lastIndexOf('.');
    if (n > 0 && !url.path().endsWith(f.path.mid(n)))
        return false; // can't restore order
    LRUCache::remove(md5);
    return true;
}

FileLRUResource FileLRUCache::get(const QUrl &url, bool put)
{
    QByteArray md5 = urlMd5(url);
    FileLRUResource f = base::get(md5);
    if (f.path.isEmpty()) {
        if (put) {
            f.path = url.path();
            f.size = 0;
            int n = f.path.lastIndexOf('.');
            f.path = n > 0 ? md5 + f.path.mid(n) : md5;
            f.path = dir_.filePath(f.path);
        }
        f.size = -1;
        return f;
    }
    int n = f.path.lastIndexOf('.');
    if (n > 0 && !url.path().endsWith(f.path.mid(n))) {
        if (!put)
            f.path.clear();
        f.size = -1;
        return f;
    }
    if (!QFile::exists(f.path)) {
        LRUCache::remove(md5);
        if (!put)
            f.path.clear();
        f.size = -1;
        return f;
    }
    QFile(f.path).setFileTime(QDateTime::currentDateTime(), QFile::FileModificationTime);
    return f;
}

quint64 FileLRUCache::sizeOf(const FileLRUResource &v)
{
    assert(v.size >= 0);
    return static_cast<quint64>(v.size);
}

void FileLRUCache::destroy(const QByteArray &k, const FileLRUResource &v)
{
    (void) k;
    QFile::remove(v.path);
}

QByteArray FileLRUCache::urlMd5(const QUrl &url)
{
    return QCryptographicHash::hash(url.toString().toUtf8(), QCryptographicHash::Md5).toHex();
}
