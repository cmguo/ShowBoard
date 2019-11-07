#ifndef CONTROL_H
#define CONTROL_H

#include "ShowBoard_global.h"
#include "lifeobject.h"

#include <QSizeF>

class QGraphicsItem;
class ResourceView;
class QGraphicsTransform;
class StateItem;
struct ToolButton;

class SHOWBOARD_EXPORT Control : public LifeObject
{
    Q_OBJECT

    Q_PROPERTY(Flags flags READ flags())

    Q_PROPERTY(ResourceView * resource READ resource())

public:
    enum Flag {
        None = 0,
        CanSelect = 1,
        CanMove = 2,
        CanScale = 4,
        CanRotate = 8,
        DefaultFlags = 15, // all can
        KeepAspectRatio = 1 << 4,
        FullLayout = 1 << 5,
        HelpSelect = 1 << 6,
        FullSelect = 1 << 7,
        CanvasBackground = 1 << 8,
        RestoreSession = 1 << 9,
    };

    Q_DECLARE_FLAGS(Flags, Flag)

    enum SelectMode
    {
        PassSelect,
        Select,
        NotSelect
    };

    static constexpr char const * EXPORT_ATTR_TYPE = "ctrl_type";

private:
    static constexpr int ITEM_KEY_CONTROL = 1000;

public:
    /*
     * Get control from item
     */
    static Control * fromItem(QGraphicsItem * item);

    static ToolButton btnCopy;
    static ToolButton btnDelete;

public:
    /*
     * realFlags = (DefaultFlags | flags) & ~clearFlags
     */
    explicit Control(ResourceView *res, Flags flags = None, Flags clearFlags = None);

    virtual ~Control() override;

public:
    Flags flags() const
    {
        return flags_;
    }

    ResourceView * resource() const
    {
        return res_;
    }

    QGraphicsItem * item() const
    {
        return item_;
    }

public:
    /*
     * load from resource
     * be called when create, will call create(res)
     */
    void attachTo(QGraphicsItem * parent);

    /*
     * save to resource
     */
    void detachFrom(QGraphicsItem * parent);

    /*
     * called when attached to canvas or canvas is resized
     */
    void relayout();

    /*
     * move (shift) this item, is saved at transform
     */
    void move(QPointF const & delta);

    /*
     * scale this item, is saved at transform
     */
    void scale(QRectF const & origin, bool positive, QRectF & result);

    /*
     * invoke slot by name, use for lose relation call
     */
    void exec(QString const & cmd, QGenericArgument arg0 = QGenericArgument(),
              QGenericArgument arg1 = QGenericArgument(), QGenericArgument arg2 = QGenericArgument());

public:
    /*
     * when flag HelpSelect is set, this function is called
     *   to help test if then click at @point selects this item
     */
    virtual SelectMode selectTest(QPointF const & point);

    /*
     * collect context menu of this control
     *  copy, delete is add according to flags
     *  other menus can be defined with toolsString()
     */
    virtual void getToolButtons(QList<ToolButton *> & buttons);

    /*
     * handle button click,
     *  copy, delete are handled by canvas and should not go here
     */
    virtual void handleToolButton(ToolButton * button);

protected:
    /*
     * override this to do really creation of item
     */
    virtual QGraphicsItem * create(ResourceView * res) = 0;

    /*
     * stringlized definition of context menus
     *   menu strings is seperated with ';' and menu define parts with '|'
     *   for example:
     *     "open()|打开|:/showboard/icons/icon_open.png;"
     */
    virtual QString toolsString() const;

    /*
     * called before item is attached to canvas
     * override this to do more preparing work
     */
    virtual void attaching();

    /*
     * load settings from resource view,
     * all dynamic properties from resource view are applied to control
     */
    virtual void loadSettings();

    /*
     * called after item is attached to canvas
     * override this to do more post attach work
     */
    virtual void attached();

    /*
     * called before item is detached from canvas
     * override this to do more release work
     */
    virtual void detaching();

    /*
     * save settings to resource view,
     * all properties (include dynamic properties) are saved to resource view
     */
    virtual void saveSettings();

    /*
     * called after item is detached from canvas
     * override this to do more release work
     */
    virtual void detached();

protected:
    virtual void initPosition(QGraphicsItem *parent);

    /*
     * called by child control to notify it's geometry is ready
     *  this function will calc suitable init scale for item
     */
    virtual void initScale(QSizeF unused = QSizeF());

    /*
     * called when attached to canvas or canvas is resized
     */
    virtual void resize(QSizeF const & size);

    StateItem * stateItem();

    void clearStateItem();

    void updateTransform();

private:
    QList<ToolButton *> & tools();

protected:
    Flags flags_;
    ResourceView * res_;
    QGraphicsTransform * transform_;
    QGraphicsItem * item_;
    StateItem * stateItem_;
};

/*
 * register control of @ctype with resource type @type
 *  @type is a list of strings seperate with ','
 */
#define REGISTER_CONTROL(ctype, type) \
    static QExport<ctype, Control> const export_control_##ctype(QPart::Attribute(Control::EXPORT_ATTR_TYPE, type));

#endif // CONTROL_H
