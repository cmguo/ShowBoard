#include "controltransform.h"
#include "resourcetransform.h"

ControlTransform::ControlTransform(ResourceTransform const & transform)
    : transform_(&transform)
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

ControlTransform::ControlTransform()
    : transform_(nullptr)
    , type_(SelectBox)
{
}

ControlTransform::ControlTransform(ControlTransform *parentTransform, bool childNoRotate)
    : transform_(parentTransform->transform_)
    , type_(childNoRotate ? ChildItemNoRotate : ChildItem)
{
    setParent(parentTransform);
}

void ControlTransform::update()
{
    QGraphicsTransform::update();
    for (QObject * c : children())
        static_cast<ControlTransform *>(c)->update();
}

void ControlTransform::setResourceTransform(const ResourceTransform *transform)
{
    transform_ = transform;
    update();
}

void ControlTransform::applyTo(QMatrix4x4 *matrix) const
{
    switch (type_) {
    case PureItem:
        *matrix = transform_->transform().toAffine();
        break;
    case ItemWithBar:
        *matrix = transform_->scale().toAffine();
        break;
    case SelectBar:
        *matrix = transform_->rotateTranslate().toAffine();
        break;
    case SelectBox:
        if (transform_)
            *matrix = transform_->rotateTranslate().toAffine();
        break;
    case ChildItem:
        *matrix = transform_->scale().inverted().toAffine();
        break;
    case ChildItemNoRotate:
        *matrix = transform_->scaleRotate().inverted().toAffine();
        break;
    }
}

