#include "lrucache.h"

#include <QDir>
#include <QUrl>
#include <QCryptographicHash>
#include <QDateTime>

FileLRUCache::FileLRUCache(const QDir &dir, quint64 capacity)
    : base(capacity)
    , dir_(dir)
{
    dir.mkpath(dir.path());
    for (QFileInfo const & f : dir.entryInfoList(QDir::NoFilter, QDir::Time)) {
        if (f.fileName().length() == 32 || f.fileName().lastIndexOf('.') == 32) {
            base::put(f.fileName().toUtf8().left(32), {
                          f.filePath(), static_cast<quint64>(f.size())});
        } else {
            QFile::remove(f.filePath());
        }
    }
}

QString FileLRUCache::put(const QUrl &url, QByteArray data)
{
    QByteArray md5 = urlMd5(url);
    FileLRUResource f = base::get(md5);
    if (!f.path.isEmpty()) {
        int n = f.path.lastIndexOf('.');
        if ((n > 0 && !url.path().endsWith(f.path.mid(n))) || !QFile::exists(f.path)) {
            base::remove(md5);
            f.path.clear();
        } else {
            return f.path;
        }
    }
    f.path = url.path();
    f.size = static_cast<quint64>(data.size());
    int n = f.path.lastIndexOf('.');
    f.path = n > 0 ? md5 + f.path.mid(n) : md5;
    f.path = dir_.filePath(f.path);
    QFile file(f.path + ".temp");
    bool ok = file.open(QFile::WriteOnly);
    ok = ok && (file.write(data) == data.size());
    file.close();
    ok = ok && QFile::rename(f.path + ".temp", f.path);
    if (!ok) {
        file.remove();
        return nullptr;
    }
    base::put(md5, f);
    return f.path;
}

QByteArray FileLRUCache::get(const QUrl &url)
{
    QString path = getFile(url);
    if (path.isEmpty())
        return QByteArray();
    QFile file(path);
    if (!file.open(QFile::ReadOnly)) {
        base::remove(urlMd5(url)); // rarely happen
        return QByteArray();
    }
    QByteArray data = file.readAll();
    file.close();
    return data;
}

bool FileLRUCache::contains(const QUrl &url)
{
    return !getFile(url).isEmpty();
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

QString FileLRUCache::getFile(const QUrl &url)
{
    QByteArray md5 = urlMd5(url);
    FileLRUResource f = base::get(md5);
    if (f.path.isEmpty())
        return nullptr;
    int n = f.path.lastIndexOf('.');
    if (n > 0 && !url.path().endsWith(f.path.mid(n)))
        return nullptr;
    if (!QFile::exists(f.path)) {
        LRUCache::remove(md5);
        return nullptr;
    }
    QFile(f.path).setFileTime(QDateTime::currentDateTime(), QFile::FileModificationTime);
    return f.path;
}

quint64 FileLRUCache::sizeOf(const FileLRUResource &v)
{
    return v.size;
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
