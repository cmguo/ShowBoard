#include "resource.h"
#include "lrucache.h"

#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QMetaEnum>
#include <QDir>

QNetworkAccessManager * Resource::network_ = nullptr;
FileLRUCache Resource::cache_(QDir::current().filePath("rescache"), 1000 * 1024 * 1024); // 1G

using namespace QtPromise;

Resource::Resource(QString const & type, QUrl const & url)
    : url_(url)
    , type_(type)
{
}

Resource::Resource(Resource const & o)
    : url_(o.url_)
    , type_(o.type_)
{
}

QPromise<QUrl> Resource::getLocalUrl()
{
    if (url_.scheme() == "file") {
        return QPromise<QUrl>::resolve(url());
    }
    QString file = cache_.getFile(url_);
    if (!file.isEmpty()) {
        return QPromise<QUrl>::resolve(QUrl::fromLocalFile(file));
    }
    return getData().then([this, l = life()] (QByteArray data) {
        if (l.isNull())
            return QUrl();
        return QUrl::fromLocalFile(cache_.put(url_, data));
    });
}

QPromise<QSharedPointer<QIODevice>> Resource::getStream(bool all)
{
    if (url_.scheme() == "data") {
        return QPromise<QSharedPointer<QIODevice>>::resolve(nullptr);
    } else if (url_.scheme() == "" || url_.scheme() == "file") {
        QSharedPointer<QIODevice> file(new QFile(url_.toLocalFile()));
        file->open(QFile::ReadOnly | QFile::ExistingOnly);
        return QPromise<QSharedPointer<QIODevice>>::resolve(file);
    } else {
        QString path = cache_.getFile(url_);
        if (!path.isEmpty()) {
            QSharedPointer<QIODevice> file(new QFile(path));
            file->open(QFile::ReadOnly | QFile::ExistingOnly);
            return QPromise<QSharedPointer<QIODevice>>::resolve(file);
        }
        if (network_ == nullptr) {
            network_ = new QNetworkAccessManager();
            network_->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
        }
        QNetworkRequest request(url());
        QSharedPointer<QNetworkReply> reply(network_->get(request));
        return QPromise<QSharedPointer<QIODevice>>([reply, all](
                                         const QPromiseResolve<QSharedPointer<QIODevice>>& resolve,
                                         const QPromiseReject<QSharedPointer<QIODevice>>& reject) {
            auto readyRead = [=]() {
                resolve(reply);
            };
            auto finished = [=]() {
                if (reply->error()) {
                    reply->deleteLater();
                    reject(std::exception(
                               QMetaEnum::fromType<QNetworkReply::NetworkError>().key(reply->error())));
                } else {
                    resolve(reply);
                }
            };
            auto error = [=](QNetworkReply::NetworkError e) {
                reply->deleteLater();
                reject(std::exception(
                           QMetaEnum::fromType<QNetworkReply::NetworkError>().key(e)));
            };
            if (all)
                QObject::connect(reply.get(), &QNetworkReply::finished, finished);
            else
                QObject::connect(reply.get(), &QNetworkReply::readyRead, readyRead);
            void (QNetworkReply::*p)(QNetworkReply::NetworkError) = &QNetworkReply::error;
            QObject::connect(reply.get(), p, error);
        });
    }
}

QPromise<QByteArray> Resource::getData()
{
    if (url_.scheme() == "data") {
        return QPromise<QByteArray>::resolve(QByteArray());
    } else {
        return getStream(true).then([url = url_](QSharedPointer<QIODevice> io) {
            QByteArray data = io->readAll();
            io->close();
            if (io->metaObject() != &QFile::staticMetaObject)
                cache_.put(url, data);
            return data;
        });
    }
}

QPromise<QString> Resource::getText()
{
    return getData().then([](QByteArray data) {
        return QString(data);
    });
}
