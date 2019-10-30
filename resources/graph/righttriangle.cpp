#include "righttriangle.h"

RightTriangle::RightTriangle(Resource * res)
    : Triangle(res)
{
}

RightTriangle::RightTriangle(QPointF const & pt)
    : Triangle(pt)
{
}

RightTriangle::RightTriangle(RightTriangle const & o)
    : Triangle(o)
{
}

void RightTriangle::movePoint(const QPointF &pt)
{
    if (points_.size() == 3) {
        points_.pop_back();
        points_.pop_back();
    }
    points_.append(pt);
    points_.append(QPointF(points_.first().x(), pt.y()));
}
