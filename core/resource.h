#ifndef RESOURCE_H
#define RESOURCE_H

#include "ShowBoard_global.h"
#include "lifeobject.h"

#include <QtPromise>

#include <QUrl>

class QNetworkAccessManager;

/*
 * Resource is pure data, while ResourceView is struct data
 */

class SHOWBOARD_EXPORT Resource : public LifeObject
{
    Q_OBJECT

    Q_PROPERTY(QUrl const url READ url)
    Q_PROPERTY(QString const type READ type)

public:
    static constexpr char const * PROP_ORIGIN_TYPE = "RES_ORIGIN_TYPE";

    static constexpr char const * PROP_SUB_TYPE = "RES_SUB_TYPE";

    static constexpr char const * PROP_SUB_TYPE2 = "RES_SUB_TYPE2";

public:
    /*
     * new resource with type @type and url @url
     *  you should not direct new resource,
     *  always create resource by ResourceManager or by add url to ResourcePage
     */
    Resource(QString const & type, QUrl const & url = QUrl("data:"));

    // copy constructor
    Resource(Resource const & o);

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
    /*
     * get local url
     *  sometime, a local file is needed by control,
     *  if original url is remote, it's downloaded and translate to local url
     *  original url is not changed
     */
    QtPromise::QPromise<QUrl> getLocalUrl();

    /*
     * get read stream
     */
    QtPromise::QPromise<QIODevice *> getStream(bool all = false);

    /*
     * get resource raw data
     */
    QtPromise::QPromise<QByteArray> getData();

    /*
     * get resource as text, decode by utf8
     */
    QtPromise::QPromise<QString> getText();

private:
    static QNetworkAccessManager * network_;

private:
    QUrl const url_;
    QString const type_;
};

#endif // RESOURCE_H
