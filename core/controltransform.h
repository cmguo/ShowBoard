#ifndef CONTROLTRANSFORM_H
#define CONTROLTRANSFORM_H

#include "ShowBoard_global.h"

#include <QGraphicsTransform>

class ResourceTransform;

class SHOWBOARD_EXPORT ControlTransform : public QGraphicsTransform
{
public:
    ControlTransform(ResourceTransform const & transform);

    ControlTransform(ControlTransform * itemTransform);

    // for SelectBox like SelectBar
    ControlTransform(int unused);

    ControlTransform(ControlTransform * parentTransform, bool noScale, bool noRotate, bool noTranslate);

public:
    void attachTo(QGraphicsTransform * transform);

protected:
    void update();

private:
    virtual void applyTo(QMatrix4x4 *matrix) const override;

private:
    enum Type {
        Identity = 0,
        Translate,
        Rotate,
        RotateTranslate,
        Scale,
        ScaleTranslate,
        ScaleRotate,
        ScaleRotateTranslate,
        NoInvert, // Identity
        InvertTranslate,
        InvertRotate,
        InvertRotateTranslate,
        InvertScale,
        InvertScaleTranslate,
        InvertScaleRotate,
        InvertScaleRotateTranslate,
        // alias
        PureItem = ScaleRotateTranslate,
        FrameItem = Scale,
        Frame = RotateTranslate,
        SelectBox = 16, // nullable RotateTranslate
    };

    ResourceTransform const * transform_;
    Type type_;
};


#endif // CONTROLTRANSFORM_H
