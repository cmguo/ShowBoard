#include "controltransform.h"
#include "resourcetransform.h"

ControlTransform::ControlTransform(ResourceTransform const & transform)
    : transform_(&transform)
    , type_(PureItem)
{
    QObject::connect(transform_, &ResourceTransform::changed,
                     this, &ControlTransform::update);
}

ControlTransform::ControlTransform(ControlTransform *parentTransform, ControlTransform::Type type)
    : transform_(parentTransform->transform_)
    , type_(type)
{
    setParent(parentTransform);
}

ControlTransform::ControlTransform(Type type)
    : transform_(nullptr)
    , type_(type)
{
}

ControlTransform::ControlTransform(ControlTransform *parentTransform, bool noScale, bool noRotate, bool noTranslate)
    : transform_(parentTransform->transform_)
    , type_(static_cast<Type>(NoInvert + (noScale ? 4 : 0) + (noRotate ? 2 : 0) + (noTranslate ? 1 : 0)))
{
    setParent(parentTransform);
}

ControlTransform * ControlTransform::addFrameTransform()
{
    type_ = FrameItem;
    return new ControlTransform(this, Frame);
}

void ControlTransform::update(int changes)
{
    if (type_ & changes)
        QGraphicsTransform::update();
    for (QObject * c : children())
        static_cast<ControlTransform *>(c)->update(changes);
}

void ControlTransform::attachTo(QGraphicsTransform *transform)
{
    if (parent() == transform)
        return;
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
        *matrix = matrix->toTransform() * transform_->translate();
        break;
    case Rotate:
        *matrix = matrix->toTransform() * transform_->rotate();
        break;
    case RotateTranslate: // Frame
        *matrix = matrix->toTransform() * transform_->rotateTranslate();
        break;
    case Scale: // FrameItem
        *matrix = matrix->toTransform() * transform_->scale();
        break;
    case ScaleTranslate:
        *matrix = matrix->toTransform() * (transform_->scale() * transform_->translate());
        break;
    case ScaleRotate:
        *matrix = matrix->toTransform() * transform_->scaleRotate();
        break;
    case ScaleRotateTranslate: // PureItem
        *matrix = matrix->toTransform() * transform_->transform();
        break;
    case NoInvert:
         break;
    case InvertTranslate:
        *matrix = matrix->toTransform() * transform_->translate().inverted();
        break;
    case InvertRotate:
        *matrix = matrix->toTransform() * transform_->rotate().inverted();
        break;
    case InvertRotateTranslate:
        *matrix = matrix->toTransform() * transform_->rotateTranslate().inverted();
        break;
    case InvertScale:
        *matrix = matrix->toTransform() * transform_->scale().inverted();
        break;
    case InvertScaleTranslate:
        *matrix = matrix->toTransform() * (transform_->scale() * transform_->translate()).inverted();
        break;
    case InvertScaleRotate:
        *matrix = matrix->toTransform() * transform_->scaleRotate().inverted();
        break;
    case InvertScaleRotateTranslate:
        *matrix = matrix->toTransform() * transform_->transform().inverted();
        break;
    case SelectBox:
        if (transform_)
            *matrix = matrix->toTransform() * transform_->rotateTranslate();
        break;
    case LargeCanvasTooBar:
        if (transform_)
            *matrix = matrix->toTransform() * transform_->scale().inverted();
        break;
    case SelectBoxLargeCanvas:
        if (transform_)
            *matrix = matrix->toTransform() * transform_->transform().inverted();
        break;
    }
}
