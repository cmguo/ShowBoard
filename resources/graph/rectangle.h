#ifndef RECTANGLE_H
#define RECTANGLE_H

#include "graph2d.h"

class Rectangle : public Graph2D
{
    Q_OBJECT
public:
    Q_INVOKABLE Rectangle(Resource * res);

    Rectangle(QPointF const & pt);

    Rectangle(Rectangle const & o);

public:
    virtual void movePoint(QPointF const & pt) override;
};

#endif // RECTANGLE_H
