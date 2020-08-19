#include "filecache.h"
#include "localhttpserver.h"
#include "showboard.h"

#include <QtHttpServer/QtHttpServer>

#include <qexport.h>
#include <qcomponentcontainer.h>

static QExport<LocalHttpServer> export_(QPart::shared);

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
    server_ = new QHttpServer(this);
    server_->router()->addConverter(qMetaTypeId<QString>(), QLatin1String(".+"));
    server_->route("/remote_address", [](const QHttpServerRequest &request) {
        return request.remoteAddress().toString();
    });
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

void LocalHttpServer::addServePath(const QByteArray &prefix, const QString &path)
{
    server_->route(prefix, [path](QString const & subPath) {
        return QHttpServerResponse::fromFile(path + subPath);
    });
}

void LocalHttpServer::addServePath(const QByteArray &prefix, FileCache &cache)
{
    server_->route(prefix, [&cache](QString const & subPath) {
        const QByteArray data = cache.getData(subPath);
        const QByteArray mimeType = QMimeDatabase().mimeTypeForFileNameAndData(subPath, data).name().toLocal8Bit();
        return QHttpServerResponse(mimeType, data);
    });
}

void LocalHttpServer::start()
{
    port_ = server_->listen(QHostAddress::Any, port_);
    if (!port_) {
        qWarning() << "LocalServer: failed to listen on a port.";
    } else {
        qInfo() << "LocalServer started at" << port_;
    }
}

void LocalHttpServer::stop()
{
    delete server_;
}


