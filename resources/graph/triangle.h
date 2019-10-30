#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "graph2d.h"

class Triangle : public Graph2D
{
    Q_OBJECT
public:
    Q_INVOKABLE Triangle(Resource * res);

    Triangle(QPointF const & pt);

    Triangle(Triangle const & o);

public:
    virtual void movePoint(QPointF const & pt) override;
};

#endif // TRIANGLE_H
