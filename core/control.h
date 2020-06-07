#ifndef CONTROL_H
#define CONTROL_H

#include "ShowBoard_global.h"
#include "toolbuttonprovider.h"

#include <QSizeF>

class QGraphicsItem;
class ResourceView;
class QGraphicsTransform;
class StateItem;
class ToolButton;
class ItemFrame;
class QIODevice;
class WhiteCanvas;

class SHOWBOARD_EXPORT Control : public ToolButtonProvider
{
    Q_OBJECT

    Q_PROPERTY(Flags flags READ flags())
    Q_PROPERTY(ResourceView * resource READ resource())

    Q_PROPERTY(bool withSelectBar READ withSelectBar WRITE setWithSelectBar)
    Q_PROPERTY(bool keepAspectRatio READ keepAspectRatio WRITE setKeepAspectRatio)
    Q_PROPERTY(bool layoutScale READ layoutScale WRITE setLayoutScale)
    Q_PROPERTY(bool expandScale READ expandScale WRITE setExpandScale)

    Q_PROPERTY(QSizeF sizeHint READ sizeHint WRITE setSizeHint)
    Q_PROPERTY(QSizeF minSize READ minSize WRITE setMinSize)
    Q_PROPERTY(QSizeF maxSize READ maxSize WRITE setMaxSize)

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
        HalfSelect = 1 << 6,
        FullSelect = 1 << 7,
        WithSelectBar = 1 << 8,
        AutoPosition = 1 << 9,
        LayoutScale = 1 << 10,
        ExpandScale = 1 << 11,
        Touchable = 1 << 12,
        ImpliedEditable = 1 << 13, // for geometry
        FixedOnCanvas = 1 << 14,
        ShowSelectMask = 1 << 15,
        // States
        Loading = 1 << 24,
        LoadFinished = 1 << 25,
        RestoreSession = 1 << 26,
        Selected = 1 << 27,
        Adjusting = 1 << 28,
        RestorePersisted = 1 << 29,
    };

    Q_DECLARE_FLAGS(Flags, Flag)
    Q_FLAGS(Flags)

    enum SelectMode
    {
        PassSelect,
        Select,
        NotSelect,
        PassSelect2,
    };

    Q_ENUM(SelectMode)

protected:
    static constexpr int ITEM_KEY_CONTROL = 1000;

public:
    /*
     * Get control from item
     */
    static Control * fromItem(QGraphicsItem const * item);

    static ToolButton btnTop;
    static ToolButton btnCopy;
    static ToolButton btnFastCopy;
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
        return realItem_;
    }

public:
    bool withSelectBar() const;

    void setWithSelectBar(bool b);

    bool keepAspectRatio() const;

    void setKeepAspectRatio(bool b);

    bool layoutScale() const;

    void setLayoutScale(bool b);

    bool expandScale() const;

    void setExpandScale(bool b);

public:
    /*
     * load from resource
     * be called when create, will call create(res)
     */
    void attachTo(QGraphicsItem * parent, QGraphicsItem * before);

    /*
     * save to resource
     */
    void detachFrom(QGraphicsItem * parent, QGraphicsItem * before);

    /*
     * called when attached to canvas or canvas is resized
     */
    void relayout();

public:
    /*
     * move (shift) this item, is saved at transform
     */
    void move(QPointF & delta);

    /*
     * scale this item, is saved at transform
     */
    bool scale(QRectF & rect, QRectF const & direction, QPointF & delta);

    /*
     * scale this item, is saved at transform
     */
    bool scale(QRectF const & direction, QPointF & delta);

    void gesture(QPointF const & from1, QPointF const & from2, QPointF & to1, QPointF & to2);

    /*
     * rotate this item against it's center, is saved at transform
     */
    void rotate(QPointF const & from, QPointF & to);

    /*
     * rotate this item against <center>, is saved at transform
     */
    void rotate(QPointF const & center, QPointF const & from, QPointF & to);

    void setGeometry(QRectF const & geometry);

    QRectF boundRect() const;

    QGraphicsTransform* transform() const
    {
        return transform_;
    }

public:
    /*
     * when flag HelpSelect is set, this function is called
     *   to help test if then click at @point selects this item
     */
    virtual SelectMode selectTest(QPointF const & point);

    virtual SelectMode selectTest(QGraphicsItem * child, QGraphicsItem * parent, QPointF const & point, bool onlyAssist);

    /*
     * set when select state change
     */
    virtual void select(bool selected);

    /*
     * set when adjusting state change
     */
    virtual void adjusting(bool be);

    virtual void beforeClone();

    virtual void afterClone(Control * control);

protected:
    /*
     * override this to do really creation of item
     */
    virtual QGraphicsItem * create(ResourceView * res) = 0;

    /*
     * called when attached to canvas or canvas is resized
     */
    virtual void resize(QSizeF const & size);

    virtual void sizeChanged();

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

    void initPosition();

    /*
     * called by child control to notify it's geometry is ready
     *  this function will calc suitable init scale for item
     */
    void loadFinished(bool ok, QString const & iconOrMsg = QString());

    void initScale();

    /*
     * called by child, delay changed when adjusting
     */
    void setSize(QSizeF const & size);

protected:
    QSizeF sizeHint();

    void setSizeHint(QSizeF const & size);

    QSizeF minSize();

    void setMinSize(QSizeF const & size);

    QSizeF maxSize();

    void setMaxSize(QSizeF const & size);

    WhiteCanvas * whiteCanvas();

    StateItem * stateItem();

    ItemFrame * itemFrame();

protected:
    void loadStream();

    void loadData();

    void loadText();

    void reload();

    virtual void onStream(QIODevice* stream);

    virtual void onData(QByteArray data);

    virtual void onText(QString text);

public:
    using ToolButtonProvider::getToolButtons;

protected:
    virtual void getToolButtons(QList<ToolButton *> &buttons, ToolButton * parent) override;

private:
    Q_DISABLE_COPY(Control)

protected:
    Flags flags_;
    ResourceView * res_;
    QGraphicsTransform * transform_;
    QGraphicsItem * item_;
    QObject* itemObj_;
    QGraphicsItem * realItem_;
    StateItem * stateItem_;
    QSizeF minMaxSize_[2];
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Control::Flags)
Q_DECLARE_METATYPE(Control::Flags)

/*
 * register control of @ctype with resource type @type
 *  @type is a list of strings seperate with ','
 */
#define REGISTER_CONTROL(ctype, type) \
    static QExport<ctype, Control> const export_control_##ctype(QPart::MineTypeAttribute(type));

#endif // CONTROL_H
