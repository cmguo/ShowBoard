#include "filecache.h"
#include "dataprovider.h"
#include "core/workthread.h"

#include <QDir>
#include <QDateTime>
#include <QBuffer>
#include <QNetworkReply>
#include <QCryptographicHash>

using namespace QtPromise;

static WorkThread& thread()
{
    static WorkThread th("FileCache");
    return th;
}

FileCache::FileCache(const QDir &dir, quint64 capacity, QByteArray algorithm)
    : base(capacity)
    , dir_(dir)
    , algorithm_(algorithm)
{
    dir.mkpath(dir.path());
}

QPromise<QString> FileCache::putStream(QString const & path, QByteArray const & hash, QSharedPointer<QIODevice> stream)
{
    QString fullPath = dir_.filePath(path);
    FileResource f = get(path, hash);
    if (f.size >= 0) { // not replace old
        return QPromise<QString>::resolve(fullPath);
    }
    auto iter = asyncPuts_.find(path);
    if (iter != asyncPuts_.end())
        return iter.value();
    QPromise<QString> asyncPut = saveStream(fullPath, stream)
            .then([this, path, fullPath, hash] (qint64 size) {
        base::put(path, FileResource {size, hash});
        return fullPath;
    }).finally([this, path] () {
        asyncPuts_.remove(path);
    });
    asyncPuts_.insert(path, asyncPut);
    return asyncPut;
}

QtPromise::QPromise<QString> FileCache::putStream(QObject *context, QString const & path, QByteArray const & hash, std::function<QtPromise::QPromise<QSharedPointer<QIODevice>> (QObject *)> openStream)
{
    QString fullPath = dir_.filePath(path);
    FileResource f = get(path, hash);
    if (f.size >= 0) {
        return QPromise<QString>::resolve(fullPath);
    }
    auto iter = asyncPuts_.find(path);
    if (iter != asyncPuts_.end())
        return iter.value();
    QPromise<QString> asyncPut = openStream(context).then([this, path, fullPath, hash] (QSharedPointer<QIODevice> stream) {
        return saveStream(fullPath, stream).then([this, path, fullPath, hash] (qint64 size) {
            base::put(path, FileResource {size, hash});
            return fullPath;
        });
    }).finally([this, path] () {
        asyncPuts_.remove(path);
    });
    asyncPuts_.insert(path, asyncPut);
    return asyncPut;
}

QString FileCache::putData(QString const & path, QByteArray const & hash, QByteArray data)
{
    QString fullPath = dir_.filePath(path);
    FileResource f = get(path, hash);
    if (f.size >= 0 && (hash.isEmpty() || f.hash == hash)) {
        return fullPath;
    }
    QFile file(fullPath + ".temp");
    bool ok = file.open(QFile::WriteOnly);
    ok = ok && file.write(data) == data.size();
    file.close();
    ok = ok && QFile::rename(file.fileName(), fullPath);
    if (!ok) {
        file.remove();
        return nullptr;
    }
    return fullPath;
}

QSharedPointer<QIODevice> FileCache::getStream(QString const & path)
{
    FileResource f = get(path);
    if (f.size < 0)
        return nullptr;
    QFile * file = new QFile(dir_.filePath(path));
    file->open(QFile::ReadOnly);
    return QSharedPointer<QIODevice>(file);
}

QByteArray FileCache::getData(QString const & path)
{
    FileResource f = get(path);
    if (f.size < 0)
        return QByteArray();
    QFile file(dir_.filePath(path));
    if (!file.open(QFile::ReadOnly)) {
        return QByteArray();
    }
    QByteArray data = file.readAll();
    file.close();
    return data;
}

QString FileCache::getFile(QString const & path)
{
    FileResource f = get(path);
    if (f.size < 0)
        return nullptr;
    return dir_.filePath(path);
}

QtPromise::QPromise<QString> FileCache::getFileAsync(QString const & path)
{
    FileResource f = get(path);
    if (f.size >= 0) { // not replace old
        return QPromise<QString>::resolve(dir_.filePath(path));
    }
    auto iter = asyncPuts_.find(path);
    if (iter != asyncPuts_.end())
        return iter.value();
    return QPromise<QString>::reject(nullptr);
}

bool FileCache::contains(QString const & path)
{
    return get(path).size >= 0;
}

FileResource FileCache::get(QString const & path, QByteArray const & hash, bool touch)
{
    FileResource f = base::get(path);
    if (f.size < 0) {
        return f;
    }
    if (!hash.isEmpty() && hash != f.hash) {
        LRUCache::remove(path);
        f.size = -1;
        return f;
    }
    QString fullPath = dir_.filePath(path);
    if (!QFile::exists(fullPath)) {
        LRUCache::remove(path);
        f.size = -1;
        return f;
    }
    if (touch)
        QFile(fullPath).setFileTime(
                QDateTime::currentDateTime(), QFile::FileModificationTime);
    return f;
}

quint64 FileCache::sizeOf(const FileResource &v)
{
    return static_cast<quint64>(v.size);
}

bool FileCache::destroy(const QString &k, const FileResource &v)
{
    (void) v;
    if (!QFile::remove(dir_.filePath(k)))
        return false;
    int n = 0;
    QString p = k;
    while ((n = p.lastIndexOf('/')) > 0) {
        p = p.left(n);
        if (!dir_.rmdir(p)) {
            break;
        }
    }
    return true;
}

void FileCache::load(std::function<bool (QString const & name)> filter)
{
    QFileInfoList files;
    QList<QDir> dirs = {dir_};
    int n = dir_.path().length() + 1;
    while (!dirs.isEmpty()) {
        QDir dir = dirs.takeFirst();
        for (QFileInfo const & f : dir.entryInfoList()) {
            if (f.isDir()) {
                if (!f.fileName().endsWith(".") && !dir.rmdir(f.fileName()))
                    dirs.append(f.filePath());
                continue;
            }
            if (filter(f.fileName())) {
                files.append(f);
            } else {
                destroy(f.filePath().mid(n), {});
            }
        }
    }
    std::sort(files.begin(), files.end(), [] (QFileInfo const & l, QFileInfo const & r) {
        return l.lastModified() < r.lastModified();
    });
    if (algorithm_.isEmpty()) {
        for (QFileInfo & f : files) {
            base::put(f.filePath().mid(n), FileResource{f.size(), nullptr});
        }
        loaded();
    } else {
        QCryptographicHash::Algorithm al = QVariant(algorithm_).value<QCryptographicHash::Algorithm>();
        for (QFileInfo & f : files) {
            thread().asyncWork([al, f] () -> FileResource {
                QFile file(f.filePath());
                if (!file.open(QFile::ReadOnly)) {
                    return FileResource{-1, nullptr};
                }
                QCryptographicHash hash(al);
                hash.addData(&file);
                return FileResource{f.size(), hash.result()};
            }).then([this, p = f.filePath().mid(n)] (FileResource const & f) {
                if (f.size >= 0)
                    base::put(p, f);
            });
        }
        thread().asyncWork([] () {}).then([this] () { loaded(); });
    }
}

void FileCache::check(const QString &path, const QByteArray &hash)
{
    get(path, hash, false);
}

QtPromise::QPromise<qint64> FileCache::saveStream(const QString &path, QSharedPointer<QIODevice> stream)
{
    QDir().mkdir(path.left(path.lastIndexOf('/')));
    QSharedPointer<QIODevice> file(new QFile(path + ".temp"));
    if (!file->open(QFile::WriteOnly)) {
        return QPromise<qint64>::reject(std::runtime_error("文件打开失败"));
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
        if (QNetworkReply * reply = qobject_cast<QNetworkReply*>(stream.get())) {
            if (reply->isFinished()) {
                finished();
                return;
            }
        }
        if (HttpStream * reply = qobject_cast<HttpStream*>(stream.get())) {
            if (reply->reply()->isFinished()) {
                finished();
                return;
            }
        }
        QObject::connect(stream.get(), &QIODevice::readChannelFinished, finished);
    }).then([path, file] (qint64 size) {
        if (!QFile::rename(path + ".temp", path)) {
            QFile::remove(path + ".temp");
            throw std::runtime_error("文件写入失败");
        }
        return size;
    }, [path] (std::exception &) -> qint64 {
        QFile::remove(path + ".temp");
        throw;
    });
}
