#include "resourceview.h"
#include "resource.h"

#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>

using namespace QtPromise;

QNetworkAccessManager * ResourceView::network_ = nullptr;

ResourceView::ResourceView(Resource * res)
    : res_(res)
{
    res_->setParent(this);
}

ResourceView * ResourceView::clone()
{
    return new ResourceView(new Resource(*res_));
}

QPromise<QIODevice *> ResourceView::getStream()
{
    if (res_->url().scheme() == "data") {
        return QPromise<QIODevice *>::resolve(nullptr);
    } else if (res_->url().scheme() == "" || res_->url().scheme() == "file") {
        QFile * file = new QFile(res_->url().toLocalFile());
        file->open(QFile::ReadOnly | QFile::ExistingOnly);
        return QPromise<QIODevice *>::resolve(file);
    } else {
        if (network_ == nullptr) {
            network_ = new QNetworkAccessManager();
            network_->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
        }
        QNetworkReply * reply = network_->get(QNetworkRequest(res_->url()));
        return QPromise<QIODevice *>([reply](
                                         const QPromiseResolve<QIODevice *>& resolve,
                                         const QPromiseReject<QIODevice *>& reject) {
            auto readyRead = [=]() {
                resolve(reply);
            };
            auto error = [=](QNetworkReply::NetworkError e) {
                reject(e);
            };
            void (QNetworkReply::*p)(QNetworkReply::NetworkError) = &QNetworkReply::error;
            QObject::connect(reply, &QNetworkReply::readyRead, readyRead);
            QObject::connect(reply, p, error);
        });
    }
}

QPromise<QByteArray> ResourceView::getData()
{
    if (res_->url().scheme() == "data") {
        return QPromise<QByteArray>::resolve(QByteArray());
    } else {
        return getStream().then([](QIODevice * io) {
            QByteArray data = io->readAll();
            io->close();
            io->deleteLater();
            return data;
        });
    }
}

QPromise<QString> ResourceView::getText()
{
    return getData().then([](QByteArray data) {
        return QString(data);
    });
}
