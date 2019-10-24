#include "resourceview.h"
#include "resource.h"

#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTemporaryFile>
#include <QDir>

using namespace QtPromise;

QNetworkAccessManager * ResourceView::network_ = nullptr;

static void nopdel(int *) {}

ResourceView::ResourceView(Resource * res, Flags flags, Flags clearFlags)
    : res_(res)
    , flags_((DefaultFlags | flags) & ~clearFlags)
    , lifeToken_(reinterpret_cast<int*>(1), nopdel)
{
    res_->setParent(this);
}

ResourceView::ResourceView(ResourceView const & o)
    : res_(new Resource(*o.res_))
    , flags_(o.flags_)
    , transform_(o.transform_)
{
    transform_.translate(20 / transform_.m11(), 20 / transform_.m22());
}

ResourceView * ResourceView::clone() const
{
    return new ResourceView(*this);
}

QUrl const & ResourceView::url() const
{
    return res_->url();
}

QPromise<QUrl> ResourceView::getLocalUrl()
{
    if (res_->url().scheme() == "file") {
        return QPromise<QUrl>::resolve(res_->url());
    }
    QWeakPointer<int> life(lifeToken_);
    return getData().then([this, life] (QByteArray data) {
        if (life.isNull())
            return QUrl();
        QString path = res_->url().path();
        path = QDir::currentPath() + path.mid(path.lastIndexOf('/'));
        QFile * temp = new QFile(path, this);
        temp->open(QFile::WriteOnly);
        temp->write(data);
        temp->close();
        return QUrl::fromLocalFile(temp->fileName());
    });
}

QPromise<QIODevice *> ResourceView::getStream(bool all)
{
    if (res_->url().scheme() == "data") {
        return QPromise<QIODevice *>::resolve(nullptr);
    } else if (res_->url().scheme() == "" || res_->url().scheme() == "file") {
        QFile * file = new QFile(url().toLocalFile());
        file->open(QFile::ReadOnly | QFile::ExistingOnly);
        return QPromise<QIODevice *>::resolve(file);
    } else {
        if (network_ == nullptr) {
            network_ = new QNetworkAccessManager();
            network_->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
        }
        QNetworkReply * reply = network_->get(QNetworkRequest(res_->url()));
        return QPromise<QIODevice *>([reply, all](
                                         const QPromiseResolve<QIODevice *>& resolve,
                                         const QPromiseReject<QIODevice *>& reject) {
            auto readyRead = [=]() {
                resolve(reply);
            };
            auto finished = [=]() {
                if (reply->error())
                    reject(reply->error());
                else
                    resolve(reply);
            };
            auto error = [=](QNetworkReply::NetworkError e) {
                reject(e);
            };
            void (QNetworkReply::*p)(QNetworkReply::NetworkError) = &QNetworkReply::error;
            if (all)
                QObject::connect(reply, &QNetworkReply::finished, finished);
            else
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
        return getStream(true).then([](QIODevice * io) {
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
