#ifndef RIGHTTRIANGLE_H
#define RIGHTTRIANGLE_H

#include "triangle.h"

class RightTriangle : public Triangle
{
    Q_OBJECT
public:
    Q_INVOKABLE RightTriangle(Resource * res);

    RightTriangle(QPointF const & pt);

    RightTriangle(RightTriangle const & o);

public:
    virtual void movePoint(QPointF const & pt) override;
};

#endif // RIGHTTRIANGLE_H
