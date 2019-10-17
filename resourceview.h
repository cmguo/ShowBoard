#ifndef RESOURCEVIEW_H
#define RESOURCEVIEW_H

#include <qexport.h>

#include <QtPromise>

#include <QObject>

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

public:
    QtPromise::QPromise<QIODevice *> getStream();

    QtPromise::QPromise<QByteArray> getData();

    QtPromise::QPromise<QString> getText();

public:
    virtual ResourceView * clone();

signals:

public slots:
    Resource * resource() const
    {
        return res_;
    }

private:
    static QNetworkAccessManager * network_;

protected:
    Resource * const res_;
};

#define REGISTER_RESOURCE_VIEW(ctype, type) \
    static QExport<ctype, ResourceView> const export_resource_##ctype(QPart::Attribute(ResourceView::EXPORT_ATTR_TYPE, type));

#endif // RESOURCEVIEW_H
