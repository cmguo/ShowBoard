#include "urlfilecache.h"
#include "dataprovider.h"

#include <QCryptographicHash>

using namespace QtPromise;

UrlFileCache::UrlFileCache(const QDir &dir, quint64 capacity)
    : FileCache(dir, capacity)
{
    load([] (QString const & f) {
        return f.length() == 32 || f.lastIndexOf('.') == 32;
    });
}

QPromise<QString> UrlFileCache::putStream(const QUrl &url, QSharedPointer<QIODevice> stream)
{
    return FileCache::putStream(md5Path(url), nullptr, stream);
}

QtPromise::QPromise<QString> UrlFileCache::putStream(QObject *context, const QUrl &url, std::function<QtPromise::QPromise<QSharedPointer<QIODevice>> (QObject *)> openStream)
{
    return FileCache::putStream(context, md5Path(url), nullptr, openStream);
}

QSharedPointer<QIODevice> UrlFileCache::getStream(const QUrl &url)
{
    return FileCache::getStream(md5Path(url));
}

QString UrlFileCache::putData(const QUrl &url, QByteArray data)
{
    return FileCache::putData(md5Path(url), nullptr, data);
}

QtPromise::QPromise<QString> UrlFileCache::putUrl(QObject *context, const QUrl &url)
{
    return putStream(context, url, [url](QObject * context) {
        DataProvider * provider = DataProvider::getInstance(url.scheme().toUtf8());
        return provider->getStream(context, url, true);
    });
}

QByteArray UrlFileCache::getData(const QUrl &url)
{
    return FileCache::getData(md5Path(url));
}

QString UrlFileCache::getFile(const QUrl &url)
{
    return FileCache::getFile(md5Path(url));
}

QtPromise::QPromise<QString> UrlFileCache::getFileAsync(const QUrl &url)
{
    return FileCache::getFileAsync(md5Path(url));
}

bool UrlFileCache::contains(const QUrl &url)
{
    return FileCache::contains(md5Path(url));
}

void UrlFileCache::remove(const QUrl &url)
{
    return FileCache::remove(md5Path(url));
}

QString UrlFileCache::md5Path(const QUrl &url)
{
    QByteArray hash = QCryptographicHash::hash(url.toString().toUtf8(),
                                    QCryptographicHash::Md5).toHex();
    QString path = url.path();
    int n = path.lastIndexOf('.');
    return n > 0 ? hash + path.mid(n) : hash;
}
