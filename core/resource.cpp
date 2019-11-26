#include "resource.h"

#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QMetaEnum>
#include <QDir>

QNetworkAccessManager * Resource::network_ = nullptr;

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
    return getData().then([this, l = life()] (QByteArray data) {
        if (l.isNull())
            return QUrl();
        QString path = url_.path();
        path = QDir::currentPath() + path.mid(path.lastIndexOf('/'));
        QFile * temp = new QFile(path, this);
        temp->open(QFile::WriteOnly);
        temp->write(data);
        temp->close();
        return QUrl::fromLocalFile(temp->fileName());
    });
}

QPromise<QIODevice *> Resource::getStream(bool all)
{
    if (url_.scheme() == "data") {
        return QPromise<QIODevice *>::resolve(nullptr);
    } else if (url_.scheme() == "" || url_.scheme() == "file") {
        QFile * file = new QFile(url_.toLocalFile());
        file->open(QFile::ReadOnly | QFile::ExistingOnly);
        return QPromise<QIODevice *>::resolve(file);
    } else {
        //if (network_ == nullptr) {
        //    network_ = new QNetworkAccessManager();
        //    network_->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
        //}
        QNetworkAccessManager * manager = new QNetworkAccessManager;
        manager->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
        QNetworkRequest request(url());
        QNetworkReply * reply = manager->get(request);
        return QPromise<QIODevice *>([reply, all, manager](
                                         const QPromiseResolve<QIODevice *>& resolve,
                                         const QPromiseReject<QIODevice *>& reject) {
            auto readyRead = [=]() {
                resolve(reply);
            };
            auto finished = [=]() {
                if (reply->error())
                    reject(std::exception(
                               QMetaEnum::fromType<QNetworkReply::NetworkError>().key(reply->error())));
                else
                    resolve(reply);
                //delete manager;
            };
            auto error = [=](QNetworkReply::NetworkError e) {
                reject(std::exception(
                           QMetaEnum::fromType<QNetworkReply::NetworkError>().key(e)));
            };
            if (all)
                QObject::connect(manager, &QNetworkAccessManager::finished, finished);
            else
                QObject::connect(reply, &QNetworkReply::readyRead, readyRead);
            void (QNetworkReply::*p)(QNetworkReply::NetworkError) = &QNetworkReply::error;
            QObject::connect(reply, p, error);
        });
    }
}

QPromise<QByteArray> Resource::getData()
{
    if (url_.scheme() == "data") {
        return QPromise<QByteArray>::resolve(QByteArray());
    } else {
        return getStream(true).then([](QIODevice * io) {
            QByteArray data = io->readAll();
            io->close();
            //io->deleteLater();
            QNetworkReply * reply = static_cast<QNetworkReply *>(io);
            delete reply->manager();
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
