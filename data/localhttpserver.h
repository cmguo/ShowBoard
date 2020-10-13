#ifndef LOCALHTTPSERVER_H
#define LOCALHTTPSERVER_H

#include "ShowBoard_global.h"

#include <QMap>
#include <QObject>
#include <QString>

class QHttpServer;
class QHttpServerResponse;
class QHttpServerRequest;
class FileCache;

class SHOWBOARD_EXPORT LocalHttpServer : public QObject
{
    Q_OBJECT
public:
    Q_INVOKABLE LocalHttpServer(QObject * parent = nullptr);

public:
    static LocalHttpServer * instance();

    class LocalProgram
    {
    public:
        virtual ~LocalProgram() {}
        virtual QHttpServerResponse handle(QHttpServerRequest const &);
    };

public:
    void setPort(ushort port);

    ushort port() const;

signals:
    void start();

    void addServePath(QByteArray const & prefix, QString const & path);

    void addServeCache(QByteArray const & prefix, FileCache * cache);

    void addServeProgram(QByteArray const & prefix, LocalProgram * program);

    void stop();

private:
    void start2();

    void addServePath2(QByteArray const & prefix, QString const & path);

    void addServeCache2(QByteArray const & prefix, FileCache * cache);

    void addServeProgram2(QByteArray const & prefix, LocalProgram * program);

    void stop2();

private:
    QHttpServer * server_ = nullptr;
    ushort port_ = 0;
};

Q_DECLARE_METATYPE(LocalHttpServer::LocalProgram)

#endif // LOCALHTTPSERVER_H
