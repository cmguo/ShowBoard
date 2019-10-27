#ifndef RESOURCE_H
#define RESOURCE_H

#include "ShowBoard_global.h"

#include <QtPromise>

#include <QObject>
#include <QUrl>
#include <QSizeF>
#include <QSharedPointer>

class QNetworkAccessManager;

class SHOWBOARD_EXPORT Resource : public QObject
{
    Q_OBJECT
public:
    Resource(QString const & type, QUrl const & url = QUrl("data:"));

    Resource(Resource const & o);

    Q_PROPERTY(QUrl const url READ url())
    Q_PROPERTY(QString const type READ type())
    Q_PROPERTY(QSizeF size MEMBER size_)

signals:
    void sizeChanged();

public:
    QUrl const & url() const
    {
        return url_;
    }

    QString const & type() const
    {
        return type_;
    }

public:
    QtPromise::QPromise<QUrl> getLocalUrl();

    QtPromise::QPromise<QIODevice *> getStream(bool all = false);

    QtPromise::QPromise<QByteArray> getData();

    QtPromise::QPromise<QString> getText();

private:
    static QNetworkAccessManager * network_;

private:
    QUrl const url_;
    QString const type_;
    QSizeF size_;
    QSharedPointer<int> lifeToken_;
};

#endif // RESOURCE_H
