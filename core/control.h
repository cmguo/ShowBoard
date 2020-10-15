#ifndef CONTROL_H
#define CONTROL_H

#include "ShowBoard_global.h"
#include "core/toolbuttonprovider.h"
#include "core/controlview.h"

#include <QSizeF>
#include <QRectF>
#include <QVariant>

class ResourceView;
class GestureContext;
class StateItem;
class ToolButton;
class ItemFrame;
class WhiteCanvas;
class ControlTransform;

class QIODevice;
class QMimeData;

class SHOWBOARD_EXPORT Control : public ToolButtonProvider
{
    Q_OBJECT

    Q_CLASSINFO("version", "1.0")

    Q_PROPERTY(Control::Flags flags READ flags())
    Q_PROPERTY(ResourceView * resource READ resource)
    Q_PROPERTY(QRectF boundRect READ boundRect)
    Q_PROPERTY(QObject* itemObj READ itemObj)

    Q_PROPERTY(bool withSelectBar READ withSelectBar WRITE setWithSelectBar)
    Q_PROPERTY(bool keepAspectRatio READ keepAspectRatio WRITE setKeepAspectRatio)
    Q_PROPERTY(bool layoutScale READ layoutScale WRITE setLayoutScale)
    Q_PROPERTY(bool expandScale READ expandScale WRITE setExpandScale)
    Q_PROPERTY(bool enterAnimate READ enterAnimate WRITE setEnterAnimate)
    Q_PROPERTY(bool selectOnLoaded READ selectOnLoaded WRITE setSelectOnLoaded)
    Q_PROPERTY(Control::Flags scaleMode READ scaleMode WRITE setScaleMode)
    Q_PROPERTY(Control::Flags selectMode READ selectMode WRITE setSelectMode)

    Q_PROPERTY(QSizeF sizeHint READ sizeHint WRITE setSizeHint)
    Q_PROPERTY(QSizeF minSize READ minSize WRITE setMinSize)
    Q_PROPERTY(QSizeF maxSize READ maxSize WRITE setMaxSize)

    Q_PROPERTY(QVariant extraToolButtons READ extraToolButtons WRITE setExtraToolButtons)

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
        FixedOnCanvas = 1 << 13,
        ShowSelectMask = 1 << 14,
        SelectOnLoaded = 1 << 15,
        EnterAnimate = 1 << 16,
        AttachToPageList = 1 << 17,
        CustomFlag = 1 << 18,
        // States
        Loading = 1 << 24,
        LoadFinished = 1 << 25,
        RestoreSession = 1 << 26,
        Selected = 1 << 27,
        Adjusting = 1 << 28,
        RestorePersisted = 1 << 29,
        CustomState = 1 << 30,
    };

    Q_DECLARE_FLAGS(Flags, Flag)
    Q_FLAG(Flags)

    enum SelectMode
    {
        PassSelect,
        Select,
        NotSelect,
        PassSelect2,
    };

    Q_ENUM(SelectMode)

    enum AdjustSource
    {
        Mouse = 1,
        Touch = 2,
        Wheel = 4,
        Keyboard = 8,
        InnerLogic = 16,
    };

protected:
#ifdef SHOWBOARD_QUICK
    static constexpr char const * ITEM_KEY_CONTROL = "CONTROL";
#else
    static constexpr int ITEM_KEY_CONTROL = 1000;
#endif

public:
    /*
     * Get control from item
     */
    static Control * fromItem(ControlView const * item);

    static Control * fromChildItem(ControlView const * item);

    static void attachToItem(ControlView * item, Control * control);

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

    ControlView * item() const
    {
        return realItem_;
    }

    QObject * itemObj() const
    {
        return itemObj_;
    }

public:
    bool withSelectBar() const;

    void setWithSelectBar(bool b = true);

    Flags scaleMode() const;

    void setScaleMode(Flags mode);

    Flags selectMode() const;

    void setSelectMode(Flags mode);

    bool keepAspectRatio() const;

    void setKeepAspectRatio(bool b = true);

    bool layoutScale() const;

    void setLayoutScale(bool b = true);

    bool expandScale() const;

    void setExpandScale(bool b = true);

    bool selectOnLoaded() const;

    void setSelectOnLoaded(bool b = true);

    bool enterAnimate() const;

    void setEnterAnimate(bool b = true);

public:
    /*
     * load from resource
     * be called when create, will call create(res)
     */
    void attachTo(ControlView * parent, ControlView * before);

    /*
     * save to resource
     */
    void detachFrom(ControlView * parent, ControlView * before);

    /*
     * called when attached to canvas or canvas is resized
     */
    void relayout();

public:
    /*
     * move (shift) this item, is saved at transform
     */
    virtual void move(QPointF & delta);

    /*
     * scale this item, is saved at transform
     */
    virtual bool scale(QRectF & rect, QRectF const & direction, QPointF & delta);

    /*
     * pinch scale/move/rotate this item, is saved at transform
     */
    virtual void gesture(GestureContext * ctx,
                         QPointF const & to1, QPointF const & to2);

public:
    /*
     * scale this item, is saved at transform
     */
    bool scale(QRectF const & direction, QPointF & delta);

    /*
     * scale this item, is saved at transform
     */
    bool scale(QPointF const & center, qreal & delta);

    /*
     * rotate this item against it's center, is saved at transform
     */
    void rotate(QPointF const & from, QPointF & to);

    /*
     * rotate this item against <center>, is saved at transform
     */
    void rotate(QPointF const & center, QPointF const & from, QPointF & to);

public:
    void setGeometry(QRectF const & geometry);

    QRectF boundRect() const;

    ControlTransform* transform() const
    {
        return transform_;
    }

public:
    /*
     * when flag HelpSelect is set, this function is called
     *   to help test if then click at @point selects this item
     */
    virtual SelectMode selectTest(QPointF const & point);

    virtual SelectMode selectTest(ControlView * child, ControlView * parent, QPointF const & point, bool onlyAssist);

    /*
     * set when select state change
     */
    virtual void select(bool selected);

    /*
     * set when adjusting state change
     */
    virtual void adjustStart(int source);

    virtual void adjustEnd(int source);

    virtual void beforeClone();

    virtual void afterClone(Control * control);

    virtual void copy(QMimeData & data);

    static void paste(QMimeData const & data, WhiteCanvas * canvas);

protected:
    /*
     * override this to do really creation of item
     */
    virtual ControlView * create(ControlView * parent) = 0;

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

    void loadFinished();

    // can only be called from catch block
    void loadFailed();

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

    QVariant extraToolButtons();

    void setExtraToolButtons(QVariant const & toolButtons);

    WhiteCanvas * whiteCanvas();

    StateItem * stateItem();

    ItemFrame * itemFrame();

protected:
    void loadStream();

    void loadData();

    void loadText();

    void reload();

    void updateTransform(int elem);

    virtual void onStream(QIODevice* stream);

    virtual void onData(QByteArray data);

    virtual void onText(QString text);

    virtual void doAnimate();

public:
    using ToolButtonProvider::getToolButtons;

    using ToolButtonProvider::handleToolButton;

protected:
    virtual void getToolButtons(QList<ToolButton *> &buttons, ToolButton * parent) override;

    virtual bool handleToolButton(ToolButton *button, const QStringList &args) override;

private:
    void loadFinished2(bool ok, QString const & iconOrMsg);

    Q_DISABLE_COPY(Control)

protected:
    Flags flags_;
    ResourceView * res_;
    ControlTransform * transform_;
    ControlView * item_;
    QObject* itemObj_;
    ControlView * realItem_;
    StateItem * stateItem_;
    QSizeF minMaxSize_[2];
    int adjustSources_;
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
