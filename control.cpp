#include "control.h"

#include <QGraphicsItem>
#include <QGraphicsScene>

Control * Control::fromItem(QGraphicsItem * item)
{
    return item->data(ITEM_KEY_CONTROL).value<Control *>();
}

Control::Control(ResourceView *res, Flags flags)
    : res_(res)
    , flags_(flags)
{
}

Control::~Control()
{
    delete item_;
    item_ = nullptr;
    res_ = nullptr;
}

void Control::load()
{
    item_ = create(res_);
}

void Control::attach(QGraphicsItem *parent)
{
    item_->setData(ITEM_KEY_CONTROL, QVariant::fromValue(this));
    item_->setTransformations({this});
    item_->setParentItem(parent);
}

void Control::detach()
{
    item_->scene()->removeItem(item_);
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
    transform_.scale(scale, scale);
    update();
}

void Control::move(QPointF const & delta)
{
    transform_.translate(delta.x() / transform_.m11(), delta.y() / transform_.m22());
    update();
}

void Control::scale(QRectF const & origin, QRectF & result)
{
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
            result.setHeight(s1.width() * s.width());
        }
        result.moveCenter(origin.center() + d);
    } else {
        s2.scale(1.0 / s1.width(), 1.0 / s1.height(), Qt::IgnoreAspectRatio);
    }
    transform_.scale(s.width(), s.height());
    move(d);
}

void Control::onAttach()
{

}

void Control::onDetach()
{

}

void Control::applyTo(QMatrix4x4 *matrix) const
{
    *matrix = transform_.toAffine();
}
