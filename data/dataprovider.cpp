#include "dataprovider.h"
#include <showboard.h>

#include <qexport.h>
#include <qlazy.h>
#include <qcomponentcontainer.h>

#include <QFile>
#include <QNetworkReply>
#include <QNetworkRequest>

using namespace QtPromise;

REGISTER_DATA_RPOVIDER(DataDataProvider,"data")
REGISTER_DATA_RPOVIDER(FileDataProvider,"file,qrc,")
REGISTER_DATA_RPOVIDER(HttpDataProvider,"http,https")

DataProvider *DataProvider::getInstance(const QByteArray &scheme)
{
    static QVector<QLazy> types;
    static QMap<QByteArray, QLazy*> readerTypes;
    if (readerTypes.empty()) {
         types = ShowBoard::containter().getExports<DataProvider>(QPart::nonshared);
         for (auto & r : types) {
             QByteArray types = r.part()->attrMineType();
             for (auto t : types.split(',')) {
                 readerTypes[t.trimmed()] = &r;
             }
         }
    }
    auto iter = readerTypes.find(scheme);
    if (iter == readerTypes.end())
        return nullptr;
    return iter.value()->get<DataProvider>();
}

DataProvider::DataProvider(QObject *parent)
    : QObject(parent)
{
}

/* DataDataProvider */

DataDataProvider::DataDataProvider(QObject *parent)
    : DataProvider(parent)
{
}

QtPromise::QPromise<QSharedPointer<QIODevice> > DataDataProvider::getStream(QObject *, const QUrl &url, bool all)
{
    (void) url;
    (void) all;
    return QPromise<QSharedPointer<QIODevice>>::resolve(nullptr);
}

/* FileDataProvider */

FileDataProvider::FileDataProvider(QObject *parent)
    : DataProvider(parent)
{
}

QtPromise::QPromise<QSharedPointer<QIODevice> > FileDataProvider::getStream(QObject *, const QUrl &url, bool all)
{
    (void) all;
    QString path = url.scheme() == "qrc" ? ":" + url.path() : url.toLocalFile();
    QSharedPointer<QIODevice> file(new QFile(path));
    if (file->open(QFile::ReadOnly | QFile::ExistingOnly)) {
        return QPromise<QSharedPointer<QIODevice>>::resolve(file);
    } else {
        qDebug() << "FileDataProvider:" << file->errorString();
        return QPromise<QSharedPointer<QIODevice>>::reject(std::invalid_argument("打开失败，请重试"));
    }
}

/* HttpDataProvider */

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
    return QPromise<QSharedPointer<QIODevice>>([reply, all](
                                     const QPromiseResolve<QSharedPointer<QIODevice>>& resolve,
                                     const QPromiseReject<QSharedPointer<QIODevice>>& reject) {

        auto error = [reply, reject](QNetworkReply::NetworkError) {
            reply->disconnect();
            reject(std::invalid_argument("network|打开失败，请检查网络再试"));
        };
        QObject::connect(reply.get(), &HttpStream::error, error);
        if (all) {
            auto finished = [reply, resolve, error]() {
                reply->disconnect();
                resolve(reply);
            };
            QObject::connect(reply.get(), &HttpStream::finished, finished);
        } else {
            auto readyRead = [reply, resolve]() {
                reply->disconnect();
                resolve(reply);
            };
            QObject::connect(reply.get(), &HttpStream::readyRead, readyRead);
        }
    });
}

HttpStream::HttpStream(QObject * context, QNetworkReply *reply)
    : reply_(reply)
    , aborted_(false)
{
    open(ReadOnly);
    if (context) {
        QObject::connect(context, &QObject::destroyed, this, [this]() {
            aborted_ = true;
            reply_->abort();
        });
    }
    reopen();
}

HttpStream::~HttpStream()
{
    reply_->deleteLater();
}

bool HttpStream::connect(QIODevice * stream, std::function<void ()> finished,
                         std::function<void (std::exception &&)> error)
{
    QNetworkReply * reply = qobject_cast<QNetworkReply*>(stream);
    if (reply == nullptr) {
        if (HttpStream * http = qobject_cast<HttpStream*>(stream))
            reply = http->reply_;
    }
    if (reply) {
        auto error2 = [error](QNetworkReply::NetworkError e) {
            qDebug() << "HttpStream:" << e;
            error(std::invalid_argument("network|打开失败，请检查网络再试"));
        };
        if (reply->isFinished()) {
            if (reply->error())
                error2(reply->error());
            else
                finished();
            return true;
        }
        if (HttpStream * http = qobject_cast<HttpStream*>(stream)) {
            QObject::connect(http, &HttpStream::error, error2);
        } else {
            void (QNetworkReply::*p)(QNetworkReply::NetworkError) = &QNetworkReply::error;
            QObject::connect(reply, p, error2);
        }
    }
    return false;
}

void HttpStream::onError(QNetworkReply::NetworkError e)
{
    if (!aborted_ && (e <= QNetworkReply::UnknownNetworkError
            || e >= QNetworkReply::ProtocolUnknownError
            /*|| e == QNetworkReply::OperationCanceledError*/)) {
        data_.append(reply_->readAll());
        qint64 size = pos() + data_.size();
        QNetworkRequest request = reply_->request();
        qDebug() << "HttpStream retry" << e << size;
        if (size > 0)
            request.setRawHeader("Range", "bytes=" + QByteArray::number(size) + "-");
        QNetworkReply * reply = reply_->manager()->get(request);
        std::swap(reply, reply_);
        //delete reply;
        reopen();
        return;
    }
    setErrorString(reply_->errorString());
    emit error(e);
}

void HttpStream::onFinished()
{
    qDebug() << "HttpStream onFinished";
    if (sender() == reply_) {
        emit readChannelFinished();
        emit finished();
    } else {
        // we are retrying
        sender()->deleteLater();
    }
}

void HttpStream::reopen()
{
    QObject::connect(reply_, &QNetworkReply::finished, this, &HttpStream::onFinished);
    QObject::connect(reply_, &QNetworkReply::readyRead, this, &HttpStream::readyRead);
    void (QNetworkReply::*p)(QNetworkReply::NetworkError) = &QNetworkReply::error;
    QObject::connect(reply_, p, this, &HttpStream::onError);
    //pos_ = 0;
}

qint64 HttpStream::readData(char *data, qint64 maxlen)
{
    if (!data_.isEmpty()) {
        if (maxlen > data_.size())
            maxlen = data_.size();
        memcpy(data, data_, static_cast<size_t>(maxlen));
        data_.remove(0, static_cast<int>(maxlen));
        return maxlen;
    }
    if (!reply_->isOpen())
        return 0;
    qint64 result = reply_->read(data, maxlen);
    /*
    if (result) {
        if (pos_ == 0) {
            QByteArray msg;
            for (auto h : reply_->rawHeaderPairs())
                msg.append(h.first + ':' + h.second + '\n');
            qDebug() << msg;
        }
        pos_ += result;
        if (pos_ > 10000000) {
            QTimer::singleShot(0, reply_, &QNetworkReply::abort);
        }
    }
    */
    return result;
}

qint64 HttpStream::writeData(const char *, qint64)
{
    assert(false);
    return 0;
}
