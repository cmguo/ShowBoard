#ifndef DATAPROVIDER_H
#define DATAPROVIDER_H

#include "ShowBoard_global.h"

#include <QtPromise>

#include <QObject>
#include <QIODevice>
#include <QSharedPointer>

class SHOWBOARD_EXPORT DataProvider : public QObject
{
    Q_OBJECT
public:
    static DataProvider * getProvider(QByteArray const & scheme);

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
    virtual QtPromise::QPromise<QSharedPointer<QIODevice>> getStream(
            QObject * context, QUrl const & url, bool all) override;
};

class FileDataProvider : public DataProvider
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit FileDataProvider(QObject *parent = nullptr);

public:
    virtual QtPromise::QPromise<QSharedPointer<QIODevice>> getStream(
            QObject * context, QUrl const & url, bool all) override;
};


/*
 * register data provider class @ctype with scheme type @type
 *  @type is a list of strings seperate with ','
 */
#define REGISTER_DATA_RPOVIDER(ctype, types) \
    static QExport<ctype, DataProvider> const export_provider_##ctype(QPart::MineTypeAttribute(types));

#endif // DATAPROVIDER_H
