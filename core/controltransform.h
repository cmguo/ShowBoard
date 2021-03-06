#ifndef CONTROLTRANSFORM_H
#define CONTROLTRANSFORM_H

#include "ShowBoard_global.h"

#ifdef SHOWBOARD_QUICK
# include <QQuickTransform>
#else
# include <QGraphicsTransform>
#endif

class ResourceTransform;

#ifdef SHOWBOARD_QUICK
class SHOWBOARD_EXPORT ControlTransform : public QQuickTransform
#else
class SHOWBOARD_EXPORT ControlTransform : public QGraphicsTransform
#endif
{
public:
    enum Type {
        Identity = 0,
        Translate,
        Rotate,
        RotateTranslate,
        Scale,
        ScaleTranslate,
        ScaleRotate,
        ScaleRotateTranslate,
        NoInvert, // Identity = 8
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
        SelectBox = 16 | RotateTranslate, // nullable
        LargeCanvasTooBar = 16 | InvertScale, // nullable
        SelectBoxLargeCanvas = 16 | InvertScaleRotateTranslate, // nullable
    };

public:
    ControlTransform(ResourceTransform const & transform, ControlTransform::Type type = PureItem);

    ControlTransform(ControlTransform * parentTransform, Type type);

    // for SelectBox, ToolBar, delay attach
    ControlTransform(Type type);

    ControlTransform(ControlTransform * parentTransform, bool noScale, bool noRotate, bool noTranslate);

public:
    ControlTransform * addFrameTransform();

    void attachTo(ControlTransform * transform);

public:
#ifdef SHOWBOARD_QUICK
    static void removeAllTransforms(QQuickItem * item);

    static void shiftLastTranform(QQuickItem * from, QQuickItem * to);
#else
    void appendToItem(QGraphicsItem * item);

    void prependToItem(QGraphicsItem * item);

    static void removeAllTransforms(QGraphicsItem * item);

    static void shiftLastTranform(QGraphicsItem * from, QGraphicsItem * to);
#endif

protected:
    void update(int changes = 7);

private:
    virtual void applyTo(QMatrix4x4 *matrix) const override;

private:
    ResourceTransform const * transform_;
    Type type_;
};


#endif // CONTROLTRANSFORM_H
