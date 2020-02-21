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
    for (QString const & f : dir.entryList(QDir::NoFilter, QDir::Time)) {
        if (f.length() == 32 || f.indexOf('.') == 32) {
            base::put(f.left(32), dir.filePath(f));
        } else {
            QFile::remove(dir.filePath(f));
        }
    }
}

QString FileLRUCache::put(const QUrl &url, QByteArray data)
{
    QString md5 = urlMd5(url);
    QString path = base::get(md5);
    if (!path.isEmpty()) {
        int n = path.lastIndexOf('.');
        if ((n > 0 && !url.path().endsWith(path.mid(n))) || !QFile::exists(path)) {
            remove(md5);
            path.clear();
        } else {
            return path;
        }
    }
    path = url.path();
    int n = path.lastIndexOf('.');
    path = n > 0 ? md5 + path.mid(n) : md5;
    path = dir_.filePath(path);
    QFile file(path);
    file.open(QFile::WriteOnly);
    file.write(data);
    file.close();
    base::put(md5, path);
    return path;
}

QByteArray FileLRUCache::get(const QUrl &url)
{
    QString path = getFile(url);
    if (path.isEmpty())
        return QByteArray();
    QFile file(path);
    if (!file.open(QFile::ReadOnly)) {
        remove(urlMd5(url)); // rarely happen
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
    QString md5 = urlMd5(url);
    QString path = base::get(md5);
    if (path.isEmpty())
        return false;
    int n = path.lastIndexOf('.');
    if (n > 0 && !url.path().endsWith(path.mid(n)))
        return false; // can't restore order
    LRUCache::remove(md5);
    return true;
}

QString FileLRUCache::getFile(const QUrl &url)
{
    QString md5 = urlMd5(url);
    QString path = base::get(md5);
    if (path.isEmpty())
        return nullptr;
    int n = path.lastIndexOf('.');
    if (n > 0 && !url.path().endsWith(path.mid(n)))
        return nullptr;
    if (!QFile::exists(path)) {
        LRUCache::remove(md5);
        return nullptr;
    }
    QFile(path).setFileTime(QDateTime::currentDateTime(), QFile::FileModificationTime);
    return path;
}

quint64 FileLRUCache::sizeOf(const QString &v)
{
    qint64 s = QFile(v).size();
    return s < 0 ? 0 : static_cast<quint64>(s);
}

void FileLRUCache::destroy(const QString &k, const QString &v)
{
    (void) k;
    QFile::remove(v);
}

QString FileLRUCache::urlMd5(const QUrl &url)
{
    return QCryptographicHash::hash(url.toString().toUtf8(), QCryptographicHash::Md5).toHex();
}
