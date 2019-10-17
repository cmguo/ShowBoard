#ifndef CONTROL_H
#define CONTROL_H

#include "ShowBoard_global.h"

#include <QObject>
#include <QTransform>
#include <QGraphicsTransform>

class QGraphicsItem;
class ResourceView;

class SHOWBOARD_EXPORT Control : public QGraphicsTransform
{
    Q_OBJECT

    Q_PROPERTY(Flags flags READ flags())

    Q_PROPERTY(ResourceView * resource READ resource())

public:
    enum Flag {
        None = 0,
        FullLayout = 1,
        TranslateFromTopLeft = 2,
        FixedLayout = FullLayout | TranslateFromTopLeft,
        NonScale = 4,
        NonRotate = 8,
        TranslateNeedHelp = 16,
        KeepAspectRatio = 32,
    };

    Q_DECLARE_FLAGS(Flags, Flag)

    static constexpr char const * EXPORT_ATTR_TYPE = "ctrl_type";

private:
    static constexpr int ITEM_KEY_CONTROL = 1000;

public:
    static Control * fromItem(QGraphicsItem * item);

public:
    explicit Control(ResourceView *res, Flags flags = None);

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

    virtual void attach(QGraphicsItem * parent);

    virtual void detach();

    virtual void save();

public:
    void move(QPointF const & delta);

    void scale(QRectF const & origin, QRectF & result);

protected:
    virtual QGraphicsItem * create(ResourceView * res) = 0;

protected slots:
    virtual void onAttach();

    virtual void onDetach();

    virtual void applyTo(QMatrix4x4 *matrix) const override;

    virtual void sizeChanged(QSizeF size);

signals:

public slots:

protected:
    Flags flags_;
    ResourceView * res_;
    QGraphicsItem * item_;
    QTransform transform_;
};

#define REGISTER_CONTROL(ctype, type) \
    static QExport<ctype, Control> const export_control_##ctype(QPart::Attribute(Control::EXPORT_ATTR_TYPE, type));

#endif // CONTROL_H
