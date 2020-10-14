#include "filecache.h"
#include "localhttpserver.h"
#include "showboard.h"
#include "core/workthread.h"

#include <qexport.h>
#include <qcomponentcontainer.h>

#include <QtHttpServer/QtHttpServer>

static QExport<LocalHttpServer> export_(QPart::shared);

static WorkThread& thread()
{
    static WorkThread th("LocalHttpServer");
    return th;
}

LocalHttpServer *LocalHttpServer::instance()
{
    static LocalHttpServer * manager = nullptr;
    if (manager == nullptr)
        manager = ShowBoard::containter().getExportValue<LocalHttpServer>();
    return manager;
}

LocalHttpServer::LocalHttpServer(QObject * parent)
    : QObject(parent)
{
    moveToThread(&::thread());
    connect(this, &LocalHttpServer::start, this, &LocalHttpServer::start2);
    connect(this, &LocalHttpServer::addServePath, this, &LocalHttpServer::addServePath2);
    connect(this, &LocalHttpServer::addServeCache, this, &LocalHttpServer::addServeCache2);
    connect(this, &LocalHttpServer::stop, this, &LocalHttpServer::stop2);
    //addServePath("/", QDir::currentPath() + "/");
}

void LocalHttpServer::setPort(ushort port)
{
    port_ = port;
}

ushort LocalHttpServer::port() const
{
    return port_;
}

void LocalHttpServer::addServePath2(const QByteArray &prefix, const QString &path)
{
    server_->route(prefix, [path](QString const & subPath) {
        return QHttpServerResponse::fromFile(path + subPath);
    });
}

class StreamHttpRespone : public QHttpServerResponse
{
public:
    StreamHttpRespone(QByteArray mimeType, QSharedPointer<QIODevice> stream)
        : QHttpServerResponse(QHttpServerResponse::StatusCode::Ok)
        , mimeType_(mimeType)
        , stream_(stream)
    {
    }
    StreamHttpRespone(QHttpServerResponse::StatusCode code)
        : QHttpServerResponse(code)
    {
    }
public:
    virtual void write(QHttpServerResponder &&responder) const override
    {
        responder.writeStatusLine(statusCode());
        if (!mimeType_.isNull())
            responder.writeHeader("Content-Type", mimeType_);
        if (stream_) {
            responder.writeHeader("Content-Length",
                                  QByteArray::number(stream_->bytesAvailable()));
            QByteArray buf(4096, 0);
            qint64 n = 0;
            while ((n = stream_->read(buf.data(), buf.size())) > 0) {
                responder.writeBody(buf.data(), n);
            }
        } else {
            responder.writeHeader("Content-Length", "0");
            responder.writeBody(QByteArray());
        }
    }
private:
    QByteArray mimeType_;
    QSharedPointer<QIODevice> stream_;
};

void LocalHttpServer::addServeCache2(const QByteArray &prefix, FileCache *cache)
{
    server_->route(prefix, [cache](QString const & subPath) {
        QSharedPointer<QIODevice> stream = cache->getStream(subPath);
        if (stream.isNull())
            return StreamHttpRespone(QHttpServerResponse::StatusCode::NotFound);
        const QByteArray mimeType = QMimeDatabase().mimeTypeForFileNameAndData(subPath, "").name().toLocal8Bit();
        return StreamHttpRespone(mimeType, stream);
    });
}

void LocalHttpServer::addServeProgram2(const QByteArray &prefix, LocalHttpServer::LocalProgram *program)
{
    server_->route(prefix, [program](QHttpServerRequest const & request) {
        QHttpServerResponse response = program->handle(request);
        QVariant origin = request.headers().value("Origin");
        if (!origin.isNull()) {
            response.addHeader("Access-Control-Allow-Origin", origin.toByteArray());
            response.addHeader("Access-Control-Allow-Methods", "*");
            response.addHeader("Access-Control-Allow-Headers", "X-Requested-With");
        }
        return response;
    });
}

void LocalHttpServer::start2()
{
    server_ = new QHttpServer(this);
    server_->router()->addConverter(qMetaTypeId<QString>(), QLatin1String(".+"));
    server_->route("/remote_address", [](const QHttpServerRequest &request) {
        return request.remoteAddress().toString();
    });
    port_ = server_->listen(QHostAddress::Any, port_);
    if (!port_) {
        qWarning() << "LocalServer: failed to listen on a port.";
    } else {
        qInfo() << "LocalServer started at" << port_;
    }
}

void LocalHttpServer::stop2()
{
    delete server_;
}


QHttpServerResponse LocalHttpServer::LocalProgram::handle(const QHttpServerRequest & request)
{
    QByteArray body = handle(request.url(), request.body());
    try {
        const QByteArray mimeType = QMimeDatabase().mimeTypeForFileNameAndData(request.url().path(), body).name().toLocal8Bit();
        return QHttpServerResponse(mimeType, body);
    } catch (...) {
        return QHttpServerResponse(QHttpServerResponse::StatusCode::BadRequest);
    }
}
