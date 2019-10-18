#ifndef CONTROL_H
#define CONTROL_H

#include "ShowBoard_global.h"

#include <QObject>
#include <QSizeF>
#include <QSharedPointer>

class QGraphicsItem;
class ResourceView;
class QControlTransform;

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
        CanCopy = 16,
        CanDelete = 32,
        DefaultFlags = 63, // all can
        KeepAspectRatio = 1 << 8,
    };

    Q_DECLARE_FLAGS(Flags, Flag)

    static constexpr char const * EXPORT_ATTR_TYPE = "ctrl_type";

private:
    static constexpr int ITEM_KEY_CONTROL = 1000;

public:
    static Control * fromItem(QGraphicsItem * item);

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
    void move(QPointF const & delta);

    void scale(QRectF const & origin, QRectF & result);

    void exec(QString const & cmd, QString const & args);

    struct Command
    {
        QString name;
        QString title;
        //QVariant icon;
    };

    void commands(QList<Command *> & result);

protected:
    virtual QGraphicsItem * create(ResourceView * res) = 0;

protected:
    virtual void sizeChanged(QSizeF size);

signals:

public slots:

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
