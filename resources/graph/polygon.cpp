#include "polygon.h"

Polygon::Polygon(Resource * res)
    : Graph2D(res)
{
}

Polygon::Polygon(QPointF const & pt)
    : Graph2D(pt)
{
}

Polygon::Polygon(Polygon const & o)
    : Graph2D(o)
{
}

bool Polygon::commit(const QPointF &pt)
{
    if (points_.size() < 4)
        return false;
    QPointF d = pt - points_.first();
    if (QPointF::dotProduct(d, d) >= 25)
        return false;
    points_.pop_back();
    return true;
}
