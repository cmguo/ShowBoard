#include "controltransform.h"

ControlTransform::ControlTransform(QTransform * transform)
    : transform_(transform)
    , type_(PureItem)
{
}

ControlTransform::ControlTransform(ControlTransform * itemTransform)
    : transform_(itemTransform->transform_)
    , type_(SelectBar)
{
    itemTransform->type_ = ItemWithBar;
    itemTransform->setParent(this);
}

void ControlTransform::update()
{
    QGraphicsTransform::update();
    if (type_ == SelectBar)
        static_cast<ControlTransform*>(children().last())->update();
}

void ControlTransform::applyTo(QMatrix4x4 *matrix) const
{
    switch (type_) {
    case PureItem:
        *matrix = transform_->toAffine();
        break;
    case ItemWithBar:
        *matrix = QTransform::fromScale(
                    transform_->m11(), transform_->m22()).toAffine();
        break;
    case SelectBar:
        *matrix = QTransform(*transform_).scale(
                    1 / transform_->m11(), 1 / transform_->m22()).toAffine();
        break;
    }
}

