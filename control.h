#ifndef CONTROL_H
#define CONTROL_H

#include "ShowBoard_global.h"

#include <QObject>
#include <QSizeF>
#include <QSharedPointer>

class QGraphicsItem;
class ResourceView;
class QControlTransform;
struct ToolButton;

class SHOWBOARD_EXPORT Control : public QObject
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
    };

    Q_DECLARE_FLAGS(Flags, Flag)

    static constexpr char const * EXPORT_ATTR_TYPE = "ctrl_type";

private:
    static constexpr int ITEM_KEY_CONTROL = 1000;

public:
    static Control * fromItem(QGraphicsItem * item);

    static ToolButton btnCopy;
    static ToolButton btnDelete;

public:
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
    virtual void load();

    virtual void attach();

    virtual void detach();

    virtual void save();

public:
    virtual void relayout();

    virtual bool selectTest(QPointF const & point);

public:
    void move(QPointF const & delta);

    void scale(QRectF const & origin, QRectF & result);

    void exec(QString const & cmd, QVariantList const & args = QVariantList());

    virtual void getToolButtons(QList<ToolButton *> & buttons);

    virtual void handleToolButton(ToolButton * button);

protected:
    virtual QGraphicsItem * create(ResourceView * res) = 0;

    virtual QString toolsString() const;

protected:
    virtual void sizeChanged(QSizeF size);

private:
    QList<ToolButton *> & tools();

protected:
    Flags flags_;
    ResourceView * res_;
    QControlTransform * transform_;
    QGraphicsItem * item_;
    QSharedPointer<int> lifeToken_;
};

#define REGISTER_CONTROL(ctype, type) \
    static QExport<ctype, Control> const export_control_##ctype(QPart::Attribute(Control::EXPORT_ATTR_TYPE, type));

#endif // CONTROL_H
