#include "triangle.h"

Triangle::Triangle(Resource * res)
    : Graph2D(res)
{
}

Triangle::Triangle(QPointF const & pt)
    : Graph2D(pt)
{
}

Triangle::Triangle(Triangle const & o)
    : Graph2D(o)
{
}

void Triangle::movePoint(const QPointF &pt)
{
    if (points_.size() == 3) {
        points_.pop_back();
        points_.pop_back();
    }
    points_.append(pt);
    points_.append(QPointF(points_.first().x() * 2 - pt.x(), pt.y()));
}
