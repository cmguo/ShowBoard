#ifndef RESOURCEVIEW_H
#define RESOURCEVIEW_H

#include "ShowBoard_global.h"

#include <qexport.h>

#include <QObject>
#include <QTransform>
#include <QSharedPointer>

class Resource;

class SHOWBOARD_EXPORT ResourceView : public QObject
{
    Q_OBJECT
public:
    static constexpr char const * EXPORT_ATTR_TYPE = "rtype";
    static constexpr char const * EXPORT_ATTR_FACTORY = "rfactory";

public:
    enum Flag {
        None = 0,
        TopMost = 1,
        BottomMost = 2,
        StickOn = 4,
        StickUnder = 8,
        CanCopy = 16,
        CanDelete = 32,
        DefaultFlags = CanCopy | CanDelete,
        Splittable = 1 << 8,
    };

    Q_DECLARE_FLAGS(Flags, Flag)

public:
    explicit ResourceView(Resource * res, Flags flags = None, Flags clearFlags = None);

    Q_PROPERTY(Resource * resource READ resource())
    Q_PROPERTY(Flags const flags READ flags())
    Q_PROPERTY(QTransform * transform READ transform())

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

    Flags flags() const
    {
        return flags_;
    }

    QUrl const & url() const;

    QTransform * transform()
    {
        return &transform_;
    }

protected:
    Resource * res_;
    Flags flags_;
    QTransform transform_;
    QSharedPointer<int> lifeToken_;
};

#define REGISTER_RESOURCE_VIEW(ctype, type) \
    static QExport<ctype, ResourceView> const export_resource_##ctype(QPart::Attribute(ResourceView::EXPORT_ATTR_TYPE, type));

#endif // RESOURCEVIEW_H
