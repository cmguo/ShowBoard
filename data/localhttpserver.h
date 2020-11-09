#ifndef LOCALHTTPSERVER_H
#define LOCALHTTPSERVER_H

#include "ShowBoard_global.h"

#include <QMap>
#include <QObject>
#include <QString>

class FileCache;

class QHttpServer;
class QHttpServerResponse;
class QHttpServerRequest;
class QHttpServerResponder;
class QWebSocket;

class SHOWBOARD_EXPORT LocalHttpServer : public QObject
{
    Q_OBJECT
public:
    Q_INVOKABLE LocalHttpServer(QObject * parent = nullptr);

public:
    static LocalHttpServer * instance();

    class SHOWBOARD_EXPORT HttpStreamResponse
    {
    public:
        HttpStreamResponse(QByteArray const & mimeType, QHttpServerResponder &responder);
    protected:
        // write null to finish
        void writeBodyBlock(QByteArray const & block);
    private:
        QHttpServerResponder &responder_;
    };

    class SHOWBOARD_EXPORT LocalProgram
    {
    public:
        LocalProgram() {}
        virtual ~LocalProgram() {}
        virtual void handle(QHttpServerRequest const & request, QHttpServerResponder &responder);
        virtual QHttpServerResponse handle(QHttpServerRequest const & request);
        virtual QByteArray handle(QUrl const & url, QByteArray const & body) { (void) url; (void) body; return ""; }
    private:
        Q_DISABLE_COPY(LocalProgram)
    };

    class SHOWBOARD_EXPORT LocalWebSocketProgram
    {
    public:
        LocalWebSocketProgram() {}
        virtual ~LocalWebSocketProgram() {}
        virtual void handle(QWebSocket * socket) = 0;
    private:
        Q_DISABLE_COPY(LocalWebSocketProgram)
    };

public:
    void setPort(ushort port);

    ushort port() const;

signals:
    void start();

    void started();

    void addServePath(QByteArray const & prefix, QString const & path);

    void addServeCache(QByteArray const & prefix, FileCache * cache);

    void addServeProgram(QByteArray const & prefix, LocalProgram * program);

    void addWebSocketProgram(QByteArray const & prefix, LocalWebSocketProgram * program);

    void stop();

private:
    void start2();

    void addServePath2(QByteArray const & prefix, QString const & path);

    void addServeCache2(QByteArray const & prefix, FileCache * cache);

    void addServeProgram2(QByteArray const & prefix, LocalProgram * program);

    void addWebSocketProgram2(QByteArray const & prefix, LocalWebSocketProgram * program);

    void stop2();

private:
    QHttpServer * server_ = nullptr;
    ushort port_ = 0;
    QMap<QByteArray, LocalWebSocketProgram*> webSocketPrograms_;
};

Q_DECLARE_METATYPE(LocalHttpServer::LocalProgram*)
Q_DECLARE_METATYPE(LocalHttpServer::LocalWebSocketProgram*)

#endif // LOCALHTTPSERVER_H
