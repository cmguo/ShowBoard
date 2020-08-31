#include "resource.h"
#include "data/urlfilecache.h"
#include "data/dataprovider.h"
#include "oomhandler.h"

#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QMetaEnum>
#include <QDir>
#include <QStandardPaths>
#include <QTextCodec>

QNetworkAccessManager * Resource::network_ = nullptr;
UrlFileCache * Resource::cache_ = nullptr;

using namespace QtPromise;

void Resource::initCache(const QString &path, quint64 capacity)
{
    cache_ = new UrlFileCache(QDir(path), capacity);
}

UrlFileCache &Resource::getCache()
{
    if (cache_ == nullptr) {
        cache_ = new UrlFileCache(
                #ifdef QT_DEBUG
                        QDir::current().filePath("rescache"), 100 * 1024 * 1024); // 100M
                #else
                        QDir(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation)).filePath("rescache"), 1000 * 1024 * 1024); // 1G
                #endif
    }
    return *cache_;
}

void Resource::registerOutOfMemoryHandler(int level, std::function<bool(void)> handler)
{
    oomHandler.addHandler(level, handler);
}

Resource::Resource(QByteArray const & type, QUrl const & url)
    : url_(url)
    , type_(type)
{
    getCache();
}

Resource::Resource(Resource const & o)
    : LifeObject(o)
    , url_(o.url_)
    , type_(o.type_)
{
}

QtPromise::QPromise<QUrl> Resource::getLocalUrl()
{
    return getLocalUrl(this, url_);
}

QtPromise::QPromise<QSharedPointer<QIODevice> > Resource::getStream(bool all)
{
    return getStream(this, url_, all);
}

QtPromise::QPromise<QByteArray> Resource::getData()
{
    return getData(this, url_);
}

QtPromise::QPromise<QString> Resource::getText()
{
    return getText(this, url_);
}

QPromise<QUrl> Resource::getLocalUrl(QObject *context, QUrl const & url)
{
    if (url.scheme() == "file") {
        if (QFile::exists(url.toLocalFile()))
            return QPromise<QUrl>::resolve(url);
        else
            return QPromise<QUrl>::reject(std::invalid_argument("打开失败，请重试"));
    }
    return cache_->putUrl(context, url)
            .then([] (QString const & file) {
        return QUrl::fromLocalFile(file);
    });
}

QtPromise::QPromise<QUrl> Resource::getLocalUrl(const QUrl &url)
{
    return getLocalUrl(nullptr, url);
}

QPromise<QSharedPointer<QIODevice>> Resource::getStream(QObject *context, QUrl const & url, bool all)
{
    DataProvider * provider = DataProvider::getProvider(url.scheme().toUtf8());
    if (provider == nullptr) {
        return QPromise<QSharedPointer<QIODevice>>::reject(std::invalid_argument("打开失败，未知数据协议"));
    }
    // We not cache stream, but may use already cached file
    if (provider->needCache()) {
        QSharedPointer<QIODevice> file = cache_->getStream(url);
        if (file) {
            return QPromise<QSharedPointer<QIODevice>>::resolve(file);
        }
    }
    return provider->getStream(context, url, all);
}

QtPromise::QPromise<QSharedPointer<QIODevice> > Resource::getStream(const QUrl &url, bool all)
{
    return getStream(nullptr, url, all);
}

QtPromise::QPromise<QByteArray> Resource::getData(QObject *context, const QUrl &url)
{
    return getStream(context, url, true).then([url](QSharedPointer<QIODevice> io) {
        QByteArray data = io->readAll();
        io->close();
        DataProvider * provider = DataProvider::getProvider(url.scheme().toUtf8());
        if (provider->needCache())
            cache_->putData(url, data);
        return data;
    });
}

QPromise<QByteArray> Resource::getData(QUrl const & url)
{
    return getData(nullptr, url);
}

static QString fromMulticode(QByteArray bytes);

QtPromise::QPromise<QString> Resource::getText(QObject *context, const QUrl &url)
{
    return getData(context, url).then([](QByteArray data) {
        return fromMulticode(data);
    });
}

QPromise<QString> Resource::getText(QUrl const & url)
{
    return getText(nullptr, url);
}

static QString fromMulticode(QByteArray bytes)
{
    QTextCodec::ConverterState state;
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    QString text = codec->toUnicode(bytes.constData(), bytes.size(), &state);
    if (state.invalidChars > 0) {
        text = QTextCodec::codecForName("GBK")->toUnicode(bytes);
    }
    return text;
}
