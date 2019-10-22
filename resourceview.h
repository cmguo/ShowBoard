#ifndef RESOURCEVIEW_H
#define RESOURCEVIEW_H

#include <qexport.h>

#include <QtPromise>

#include <QObject>
#include <QTransform>
#include <QSharedPointer>

class QNetworkAccessManager;
class Resource;

class ResourceView : public QObject
{
    Q_OBJECT
public:
    static constexpr char const * EXPORT_ATTR_TYPE = "rtype";

public:
    explicit ResourceView(Resource * res);

    Q_PROPERTY(Resource * resource READ resource())

    Q_PROPERTY(QTransform * transform READ transform())

public:
    QtPromise::QPromise<QUrl> getLocalUrl();

    QtPromise::QPromise<QIODevice *> getStream(bool all = false);

    QtPromise::QPromise<QByteArray> getData();

    QtPromise::QPromise<QString> getText();

public:
    virtual ResourceView * clone() const ;

protected:
    ResourceView(ResourceView const & res);

signals:

public slots:
    Resource * resource() const
    {
        return res_;
    }

    QUrl const & url() const;

    QTransform * transform()
    {
        return &transform_;
    }

private:
    static QNetworkAccessManager * network_;

protected:
    Resource * res_;
    QTransform transform_;
    QSharedPointer<int> lifeToken_;
};

#define REGISTER_RESOURCE_VIEW(ctype, type) \
    static QExport<ctype, ResourceView> const export_resource_##ctype(QPart::Attribute(ResourceView::EXPORT_ATTR_TYPE, type));

#endif // RESOURCEVIEW_H
