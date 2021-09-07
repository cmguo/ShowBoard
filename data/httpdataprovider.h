#ifndef HTTPDATAPROVIDER_H
#define HTTPDATAPROVIDER_H

#include "dataprovider.h"

#include <QNetworkAccessManager>
#include <QObject>

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

#endif // HTTPDATAPROVIDER_H
