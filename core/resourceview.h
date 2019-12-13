#ifndef RESOURCEVIEW_H
#define RESOURCEVIEW_H

#include "ShowBoard_global.h"
#include "lifeobject.h"

#include <qexport.h>

#include <QTransform>

class Resource;
class ResourceTransform;

class SHOWBOARD_EXPORT ResourceView : public LifeObject
{
    Q_OBJECT

    Q_PROPERTY(Resource * resource READ resource())
    Q_PROPERTY(Flags const flags READ flags())
    Q_PROPERTY(QString name MEMBER name_)

public:
    static constexpr char const * EXPORT_ATTR_TYPE = "rtype";
    static constexpr char const * EXPORT_ATTR_FACTORY = "rfactory";

public:
    enum Flag {
        None = 0,
        TopMost = 1,  // always stay on top (back of list)
        BottomMost = 2,
        StickOn = 4, // moves together with it's previous resource
        StickUnder = 8, // moves together with it's next resource
        CanCopy = 16,
        CanDelete = 32,
        DefaultFlags = CanCopy | CanDelete,
        // when insert new resource under this resource,
        //  this resource will be split into two and new resource is insert between
        //  special used for stroke writen
        Splittable = 1 << 8,
        SavedSession = 1 << 9,
        DrawFinised = 1 << 10,
        DrawAttach = 1 << 11, // stick to special position
    };

    Q_DECLARE_FLAGS(Flags, Flag)

public:
    Q_INVOKABLE explicit ResourceView(Resource * res,
                                      Flags flags = None, Flags clearFlags = None);

    ResourceView(QString const & type, QUrl const & url);

public:
    virtual ResourceView * clone() const ;

protected:
    ResourceView(ResourceView const & res);

public:
    Resource * resource() const
    {
        return res_;
    }

    Flags flags() const
    {
        return flags_;
    }

    QUrl const & url() const;

    QString const & name() const
    {
        return name_;
    }

    /*
     * for move, scale, rotate
     *  these are all saved in transform
     */
    ResourceTransform& transform()
    {
        return *transform_;
    }

public:
    /*
     * move this resource to top in z-order
     */
    void moveTop();

    void setSaved();

    void removeFromPage();

    void setFlag(Flag f, bool on = true);

protected:
    Resource * res_;
    Flags flags_;
    QString name_;
    ResourceTransform* transform_;
};

/*
 * register resource view class @ctype with resource type @type
 *  @type is a list of strings seperate with ','
 */
#define REGISTER_RESOURCE_VIEW(ctype, type) \
    static QExport<ctype, ResourceView> const export_resource_##ctype(QPart::Attribute(ResourceView::EXPORT_ATTR_TYPE, type));

#endif // RESOURCEVIEW_H
