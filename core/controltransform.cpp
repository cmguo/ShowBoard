#include "controltransform.h"
#include "resourcetransform.h"

ControlTransform::ControlTransform(ResourceTransform const & transform)
    : transform_(&transform)
    , type_(PureItem)
{
    QObject::connect(transform_, &ResourceTransform::changed,
                     this, &ControlTransform::update);
}

ControlTransform::ControlTransform(ControlTransform * itemTransform)
    : transform_(itemTransform->transform_)
    , type_(Frame)
{
    itemTransform->type_ = FrameItem;
    itemTransform->setParent(this);
}

ControlTransform::ControlTransform(int)
    : transform_(nullptr)
    , type_(SelectBox)
{
}

ControlTransform::ControlTransform(ControlTransform *parentTransform, bool noScale, bool noRotate, bool noTranslate)
    : transform_(parentTransform->transform_)
    , type_(static_cast<Type>(NoInvert + (noScale ? 4 : 0) + (noRotate ? 2 : 0) + (noTranslate ? 1 : 0)))
{
    setParent(parentTransform);
}

void ControlTransform::update()
{
    QGraphicsTransform::update();
    for (QObject * c : children())
        static_cast<ControlTransform *>(c)->update();
}

void ControlTransform::attachTo(QGraphicsTransform *transform)
{
    transform_ = transform ? static_cast<ControlTransform *>(transform)->transform_
                           : nullptr;
    setParent(transform);
    update();
}

void ControlTransform::applyTo(QMatrix4x4 *matrix) const
{
    switch (type_) {
    case Identity:
        break;
    case Translate:
        *matrix *= transform_->translate().toAffine();
        break;
    case Rotate:
        *matrix *= transform_->rotate().toAffine();
        break;
    case RotateTranslate: // Frame
        *matrix *= transform_->rotateTranslate().toAffine();
        break;
    case Scale: // FrameItem
        *matrix *= transform_->scale().toAffine();
        break;
    case ScaleTranslate:
        *matrix *= (transform_->scale() * transform_->translate()).toAffine();
        break;
    case ScaleRotate:
        *matrix *= transform_->scaleRotate().toAffine();
        break;
    case ScaleRotateTranslate: // PureItem
        *matrix *= transform_->transform().toAffine();
        break;
    case NoInvert:
         break;
    case InvertTranslate:
        *matrix *= transform_->translate().inverted().toAffine();
        break;
    case InvertRotate:
        *matrix *= transform_->rotate().inverted().toAffine();
        break;
    case InvertRotateTranslate:
        *matrix *= transform_->rotateTranslate().inverted().toAffine();
        break;
    case InvertScale:
        *matrix *= transform_->scale().inverted().toAffine();
        break;
    case InvertScaleTranslate:
        *matrix *= (transform_->scale() * transform_->translate()).inverted().toAffine();
        break;
    case InvertScaleRotate:
        *matrix *= transform_->scaleRotate().inverted().toAffine();
        break;
    case InvertScaleRotateTranslate:
        *matrix *= transform_->transform().inverted().toAffine();
        break;
    case SelectBox:
        if (transform_)
            *matrix *= transform_->rotateTranslate().toAffine();
        break;
    }
}

