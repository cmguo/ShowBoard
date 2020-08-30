#ifndef LOCALHTTPSERVER_H
#define LOCALHTTPSERVER_H

#include "ShowBoard_global.h"

#include <QMap>
#include <QObject>
#include <QString>

class QHttpServer;
class FileCache;

class SHOWBOARD_EXPORT LocalHttpServer : public QObject
{
    Q_OBJECT
public:
    Q_INVOKABLE LocalHttpServer(QObject * parent = nullptr);

public:
    static LocalHttpServer * instance();

public:
    void setPort(ushort port);

    ushort port() const;

public:
    void addServePath(QByteArray const & prefix, QString const & path);

    void addServePath(QByteArray const & prefix, FileCache & cache);

signals:
    void start();

    void stop();

private:
    void start2();

    void stop2();

private:
    QHttpServer * server_ = nullptr;
    ushort port_ = 0;
};

#endif // LOCALHTTPSERVER_H
