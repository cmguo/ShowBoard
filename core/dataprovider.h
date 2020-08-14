#ifndef DATAPROVIDER_H
#define DATAPROVIDER_H

#include "ShowBoard_global.h"

#include <QtPromise>

#include <QObject>
#include <QNetworkReply>

class SHOWBOARD_EXPORT DataProvider : public QObject
{
    Q_OBJECT
public:
    explicit DataProvider(QObject *parent = nullptr);

public:
    virtual bool needCache() { return false; }

    virtual QtPromise::QPromise<QSharedPointer<QIODevice>> getStream(QObject * context, QUrl const & url, bool all) = 0;
};

class DataDataProvider : public DataProvider
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit DataDataProvider(QObject *parent = nullptr);

public:
    virtual QtPromise::QPromise<QSharedPointer<QIODevice>> getStream(QObject * context, QUrl const & url, bool all) override;
};

class FileDataProvider : public DataProvider
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit FileDataProvider(QObject *parent = nullptr);

public:
    virtual QtPromise::QPromise<QSharedPointer<QIODevice>> getStream(QObject * context, QUrl const & url, bool all) override;
};

class HttpStream : public QIODevice
{
    Q_OBJECT
public:
    HttpStream(QObject * context, QNetworkReply * reply);

    virtual ~HttpStream() override;

    QNetworkReply * reply() { return reply_; }

signals:
    void finished();

    void error(QNetworkReply::NetworkError);

private:
    void onError(QNetworkReply::NetworkError);

    void onFinished();

    void reopen();

protected:
    virtual qint64 readData(char *data, qint64 maxlen) override;
    virtual qint64 writeData(const char *data, qint64 len) override;

private:
    QNetworkReply * reply_;
    QByteArray data_;
    bool aborted_;
    //int pos_ = 0;
};

class SHOWBOARD_EXPORT HttpDataProvider : public DataProvider
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit HttpDataProvider(QObject *parent = nullptr);

public:
    virtual bool needCache() override { return true; }

    virtual QtPromise::QPromise<QSharedPointer<QIODevice>> getStream(QObject * context, QUrl const & url, bool all) override;

private:
    QNetworkAccessManager * network_ = nullptr;
};

/*
 * register data provider class @ctype with scheme type @type
 *  @type is a list of strings seperate with ','
 */
#define REGISTER_DATA_RPOVIDER(ctype, types) \
    static QExport<ctype, DataProvider> const export_provider_##ctype(QPart::MineTypeAttribute(types));

#endif // DATAPROVIDER_H
