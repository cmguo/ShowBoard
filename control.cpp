#include "control.h"
#include "resourceview.h"

#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QTransform>
#include <QGraphicsTransform>
#include <QMetaMethod>

class QControlTransform : public QGraphicsTransform
{
public:
    QControlTransform (QTransform * t)
        : t_(t)
    {
    }

    virtual void applyTo(QMatrix4x4 *matrix) const override
    {
        *matrix = t_->toAffine();
    }

    using QGraphicsTransform::update;

private:
    QTransform * t_;
};

Control * Control::fromItem(QGraphicsItem * item)
{
    return item->data(ITEM_KEY_CONTROL).value<Control *>();
}

static void nopdel(int *) {}

Control::Control(ResourceView *res, Flags flags, Flags clearFlags)
    : flags_(DefaultFlags | flags & ~clearFlags)
    , res_(res)
    , lifeToken_(reinterpret_cast<int*>(1), nopdel)
{
    transform_ = new QControlTransform(res->transform());
}

Control::~Control()
{
    delete transform_;
    delete item_;
    item_ = nullptr;
    transform_ = nullptr;
    res_ = nullptr;
}

void Control::load()
{
    item_ = create(res_);
}

void Control::attach()
{
    item_->setData(ITEM_KEY_CONTROL, QVariant::fromValue(this));
    item_->setTransformations({transform_});
}

void Control::detach()
{
    item_->setTransformations({});
    item_->setData(ITEM_KEY_CONTROL, QVariant());
    deleteLater();
}

void Control::save()
{

}

void Control::sizeChanged(QSizeF size)
{
    if (item_->parentItem() == nullptr)
        return;
    QSizeF ps = item_->parentItem()->boundingRect().size();
    qreal scale = 1.0;
    while (size.width() > ps.width() || size.height() > ps.height()) {
        size /= 2.0;
        scale /= 2.0;
    }
    res_->transform()->scale(scale, scale);
    transform_->update();
}

void Control::move(QPointF const & delta)
{
    QTransform * t = res_->transform();
    t->translate(delta.x() / t->m11(), delta.y() / t->m22());
    transform_->update();
}

void Control::scale(QRectF const & origin, QRectF & result)
{
    //result = origin;
    //result.adjust(0, 0, origin.width() / -2, origin.height() / -2);
    QSizeF s1 = origin.size();
    QSizeF s2 = result.size();
    QSizeF s(s2.width() / s1.width(), s2.height() / s1.height());
    QPointF d = result.center() - origin.center();
    if (flags_ & KeepAspectRatio) {
        if (s.width() > s.height()) {
            s.setWidth(s.height());
            d.setX(d.y() * s1.width() / s1.height());
            result.setWidth(s1.width() * s.width());
        } else {
            s.setHeight(s.width());
            d.setY(d.x() * s1.height() / s1.width());
            result.setHeight(s1.height() * s.height());
        }
        result.moveCenter(origin.center() + d);
    } else {
        s2.scale(1.0 / s1.width(), 1.0 / s1.height(), Qt::IgnoreAspectRatio);
    }
    QTransform * t = res_->transform();
    QPointF p1 = t->inverted().map(origin.topLeft());
    t->scale(s.width(), s.height());
    p1 = t->map(p1);
    QPointF p2 = result.topLeft();
    d = p2 - p1;
    t->translate(d.x() / t->m11(), d.y() / t->m22());
    transform_->update();
}

void Control::exec(QString const & cmd, QString const & args)
{
    int index = metaObject()->indexOfSlot(cmd.toUtf8());
    if (index < 0)
        return;
    QMetaMethod method = metaObject()->method(index);
    if (method.parameterCount() == 0)
        method.invoke(this);
    else if (method.parameterCount() == 1)
        method.invoke(this, Q_ARG(QString const *, &args));
}

void Control::commands(QList<Command *> & result)
{
    int n = metaObject()->methodCount();
    for (int i = staticMetaObject.methodCount(); i < n; ++i) {
        QMetaMethod method = metaObject()->method(i);
        if (method.methodType() != QMetaMethod::Slot)
            continue;
        int idesc = metaObject()->indexOfClassInfo(method.name());
        if (idesc < 0)
            continue;
        //QString desc = metaObject()->classInfo(idesc);
    }
}
