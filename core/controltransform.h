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
    ControlTransform();

    ControlTransform(ControlTransform * parentTransform, bool childNoRotate);

public:
    void update();

    void setResourceTransform(ResourceTransform const * transform);

private:
    virtual void applyTo(QMatrix4x4 *matrix) const override;

private:
    enum Type {
        PureItem,
        ItemWithBar,
        SelectBar,
        SelectBox,
        ChildItem,
        ChildItemNoRotate,
    };

    ResourceTransform const * transform_;
    Type type_;
};


#endif // CONTROLTRANSFORM_H
