#include "lrucache.h"

#include <QDir>
#include <QUrl>
#include <QCryptographicHash>
#include <QDateTime>
#include <QBuffer>
#include <QNetworkReply>

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
    QPromise<QString> asyncPut = saveStream(f.path, stream)
            .then([f] () { return f.path; })
            .finally([this, url] () {
        asyncPuts_.remove(url);
    });
    asyncPuts_.insert(url, asyncPut);
    return asyncPut;
}

QtPromise::QPromise<QString> FileLRUCache::putStream(QObject *context, const QUrl &url, std::function<QtPromise::QPromise<QSharedPointer<QIODevice>> (QObject *)> openStream)
{
    FileLRUResource f = get(url, true);
    if (f.size >= 0) {
        return QPromise<QString>::resolve(f.path);
    }
    auto iter = asyncPuts_.find(url);
    if (iter != asyncPuts_.end())
        return iter.value();
    QPromise<QString> asyncPut = openStream(context).then([f, this, url] (QSharedPointer<QIODevice> stream) {
        return saveStream(f.path, stream)
                .then([f] () { return f.path; })
                .finally([this, url] () {
            asyncPuts_.remove(url);
        });
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

QtPromise::QPromise<QString> FileLRUCache::getFileAsync(const QUrl &url)
{
    FileLRUResource f = get(url, true);
    if (f.size >= 0) { // not replace old
        return QPromise<QString>::resolve(f.path);
    }
    auto iter = asyncPuts_.find(url);
    if (iter != asyncPuts_.end())
        return iter.value();
    return QPromise<QString>::reject(nullptr);
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

QtPromise::QPromise<void> FileLRUCache::saveStream(const QString &path, QSharedPointer<QIODevice> stream)
{
    QSharedPointer<QIODevice> file(new QFile(path + ".temp"));
    if (!file->open(QFile::WriteOnly)) {
        return QPromise<void>::reject(std::runtime_error("文件打开失败"));
    }
    return QPromise<qint64>([file, stream](
                             const QPromiseResolve<qint64>& resolve,
                             const QPromiseReject<qint64>& reject) {
        QSharedPointer<qint64> size(new qint64(0));
        auto read = [=] () {
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
        };
        char c;
        if (stream->peek(&c, 1) > 0)
            read();
        QObject::connect(stream.get(), &QIODevice::readyRead, read);
        auto finished = [=] () {
            if (*size >= 0) {
                file->close();
                resolve(*size);
            }
        };
        QNetworkReply * reply = qobject_cast<QNetworkReply*>(stream.get());
        if (reply && reply->isFinished()) {
            finished();
            return;
        }
        QObject::connect(stream.get(), &QIODevice::readChannelFinished, finished);
    }).then([this, path, file] (qint64 size) {
        if (!QFile::rename(path + ".temp", path)) {
            QFile::remove(path + ".temp");
            throw std::runtime_error("文件写入失败");
        }
        base::put(path.mid(path.lastIndexOf('/') + 1).left(32).toUtf8(), FileLRUResource{ path, size });
        return;
    }, [path] (std::exception &) {
        QFile::remove(path + ".temp");
        throw;
    });
}

QByteArray FileLRUCache::urlMd5(const QUrl &url)
{
    return QCryptographicHash::hash(url.toString().toUtf8(), QCryptographicHash::Md5).toHex();
}
