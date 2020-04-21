#include "resource.h"
#include "lrucache.h"
#include "resourcemanager.h"
#include "dataprovider.h"

#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QMetaEnum>
#include <QDir>
#include <QStandardPaths>
#include <QTextCodec>

QNetworkAccessManager * Resource::network_ = nullptr;
FileLRUCache * Resource::cache_ = nullptr;

using namespace QtPromise;

void Resource::initCache(const QString &path, quint64 capacity)
{
    cache_ = new FileLRUCache(QDir(path), capacity);
}

FileLRUCache &Resource::getCache()
{
    if (cache_ == nullptr) {
        cache_ = new FileLRUCache(
                #ifdef QT_DEBUG
                        QDir::current().filePath("rescache"), 100 * 1024 * 1024); // 100M
                #else
                        QDir(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation)).filePath("rescache"), 1000 * 1024 * 1024); // 1G
                #endif
    }
    return *cache_;
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

QPromise<QUrl> Resource::getLocalUrl()
{
    if (url_.scheme() == "file") {
        if (QFile::exists(url_.toLocalFile()))
            return QPromise<QUrl>::resolve(url());
        else
            return QPromise<QUrl>::reject(std::invalid_argument("打开失败，请确认文件是否存在"));
    }
    QString file = cache_->getFile(url_);
    if (!file.isEmpty()) {
        return QPromise<QUrl>::resolve(QUrl::fromLocalFile(file));
    }
    return getData().then([this, l = life()] (QByteArray data) {
        if (l.isNull())
            return QUrl();
        return QUrl::fromLocalFile(cache_->put(url_, data));
    });
}

QPromise<QSharedPointer<QIODevice>> Resource::getStream(bool all)
{
    DataProvider * provider = ResourceManager::instance()->getProvider(url_.scheme().toUtf8());
    if (provider == nullptr) {
        return QPromise<QSharedPointer<QIODevice>>::reject(std::invalid_argument("打开失败，未知数据协议"));
    }
    if (provider->needCache()) {
        QString path = cache_->getFile(url_);
        if (!path.isEmpty()) {
            QSharedPointer<QIODevice> file(new QFile(path));
            file->open(QFile::ReadOnly | QFile::ExistingOnly);
            return QPromise<QSharedPointer<QIODevice>>::resolve(file);
        }
    }
    return provider->getStream(url_, all);
}

QPromise<QByteArray> Resource::getData()
{
    return getStream(true).then([url = url_](QSharedPointer<QIODevice> io) {
        QByteArray data = io->readAll();
        io->close();
        DataProvider * provider = ResourceManager::instance()->getProvider(url.scheme().toUtf8());
        if (provider->needCache())
            cache_->put(url, data);
        return data;
    });
}

static QString fromMulticode(QByteArray bytes);

QPromise<QString> Resource::getText()
{
    return getData().then([](QByteArray data) {
        return fromMulticode(data);
    });
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
