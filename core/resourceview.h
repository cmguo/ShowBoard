#ifndef RESOURCEVIEW_H
#define RESOURCEVIEW_H

#include "ShowBoard_global.h"
#include "toolbuttonprovider.h"

#include <qexport.h>

#include <QTransform>
#include <QEvent>

class Resource;
class ResourceTransform;
class ResourcePage;
class QGraphicsItem;

class SHOWBOARD_EXPORT ResourceView : public ToolButtonProvider
{
    Q_OBJECT

    Q_PROPERTY(Resource * resource READ resource())
    Q_PROPERTY(Flags const flags READ flags())
    Q_PROPERTY(QString name MEMBER name_)

    Q_PROPERTY(bool deletable READ deletable WRITE setDeletable)
    Q_PROPERTY(bool copyable READ copyable WRITE setCopyable)
    Q_PROPERTY(bool independent READ independent WRITE setIndependent)
    Q_PROPERTY(Flags pageMode READ pageMode WRITE setPageMode)
    Q_PROPERTY(QByteArray sessionGroup READ sessionGroup WRITE setSessionGroup)

public:
    static char const * const EXPORT_ATTR_FACTORY;

    static constexpr QEvent::Type EVENT_CLEAR_SESSION = QEvent::User;

public:
    enum Flag {
        None = 0,
        TopMost = 1,  // always stay on top (back of list)
        BottomMost = 2,
        StickOn = 4, // moves together with it's previous resource
        StickUnder = 8, // moves together with it's next resource
        ZOrderFlags = 15,
        CanCopy = 16,
        CanDelete = 32,
        CanFastCopy = CanCopy | 64,
#ifdef QT_DEBUG
        DefaultFlags = CanFastCopy | CanDelete,
#else
        DefaultFlags = CanCopy | CanDelete,
#endif
        // when insert new resource under this resource,
        //  this resource will be split into two and new resourcbei'songe is insert between
        //  special used for stroke writen
        Splittable = 1 << 8,
        DrawAttach = 1 << 9, // attach to when drawing
        Independent = BottomMost | (1 << 10), // standalone page
        VirtualPage = Independent | (1 << 11),
        LargeCanvas = Independent | (1 << 12),
        ListOfPages = Independent | (1 << 13),
        PersistSession = 1 << 14,
        // States
        SavedSession = 1 << 16,
        DrawFinised = 1 << 17,
    };

    Q_DECLARE_FLAGS(Flags, Flag)
    Q_FLAGS(Flags)

public:
    Q_INVOKABLE explicit ResourceView(Resource * res,
                                      Flags flags = None, Flags clearFlags = None);

    ResourceView(QByteArray const & type, QUrl const & url);

    ResourceView(ResourceView * mainRes);

    virtual ~ResourceView() override;

public:
    bool deletable() const;

    void setDeletable(bool v);

    bool copyable() const;

    void setCopyable(bool v);

    bool independent() const;

    void setIndependent(bool v);

    Flags pageMode() const;

    void setPageMode(Flags mode);

    QByteArray sessionGroup();

    void setSessionGroup(QByteArray const & session);

    QGraphicsItem* loadSession();

    void saveSession(QGraphicsItem* item);

    void clearSession();

public:
    virtual ResourceView * clone() const;

protected:
    ResourceView(ResourceView const & res);

public:
    Resource * resource() const
    {
        return res_;
    }

    Flags const flags() const
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

    bool canMoveTop();

    void setSaved();

    void removeFromPage();

    ResourcePage* page();

protected:
    virtual bool event(QEvent * event) override;

protected:
    Resource * res_;
    Flags flags_;
    QString name_;
    ResourceTransform* transform_;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ResourceView::Flags)
Q_DECLARE_METATYPE(ResourceView::Flags)

/*
 * register resource view class @ctype with resource type @type
 *  @type is a list of strings seperate with ','
 */
#define REGISTER_RESOURCE_VIEW(ctype, types) \
    static QExport<ctype, ResourceView> const export_resource_##ctype(QPart::MineTypeAttribute(types));

class SHOWBOARD_EXPORT CommonResourceTypes
{
public:
    CommonResourceTypes(char const * types, ResourceView::Flags flags, ResourceView::Flags clearFlags);
};

#define REGISTER_COMMON_RESOURCE_TYPES(group, types, flags, clearFlags) \
    static CommonResourceTypes export_common_resource_##group(types, flags, clearFlags);

#endif // RESOURCEVIEW_H
