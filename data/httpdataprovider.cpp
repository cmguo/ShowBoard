#include "httpdataprovider.h"
#include "httpstream.h"

using namespace QtPromise;

HttpDataProvider::HttpDataProvider(QObject *parent)
    : DataProvider(parent)
{
    network_ = new QNetworkAccessManager();
    network_->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
}

QtPromise::QPromise<QSharedPointer<QIODevice>> HttpDataProvider::getStream(QObject * context, const QUrl &url, bool all)
{
    QNetworkRequest request(url);
    QSharedPointer<HttpStream> reply(new HttpStream(context, network_->get(request)));
    reply->setObjectName(url.toString());
    return QPromise<QSharedPointer<QIODevice>>([reply, all](
                                     const QPromiseResolve<QSharedPointer<QIODevice>>& resolve,
                                     const QPromiseReject<QSharedPointer<QIODevice>>& reject) {

        auto error = [reply, reject](QNetworkReply::NetworkError) {
            reject(std::invalid_argument("network|打开失败，请检查网络再试"));
        };
        QObject::connect(reply.get(), &HttpStream::error, error);
        if (all) {
            auto finished = [reply, resolve, error]() {
                resolve(reply);
            };
            QObject::connect(reply.get(), &HttpStream::finished, finished);
        } else {
            auto readyRead = [reply, resolve]() {
                 resolve(reply);
            };
            QObject::connect(reply.get(), &HttpStream::readyRead, readyRead);
        }
    }).finally([reply] () {
        reply->disconnect();
    });
}
