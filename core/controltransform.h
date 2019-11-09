#ifndef CONTROLTRANSFORM_H
#define CONTROLTRANSFORM_H

#include <QGraphicsTransform>

class ControlTransform : public QGraphicsTransform
{
public:
    ControlTransform(QTransform * transform);

    ControlTransform(ControlTransform * itemTransform);

public:
    void update();

private:
    virtual void applyTo(QMatrix4x4 *matrix) const override;

private:
    enum Type {
        PureItem,
        ItemWithBar,
        SelectBar
    };

    QTransform * transform_;
    Type type_;
};


#endif // CONTROLTRANSFORM_H
