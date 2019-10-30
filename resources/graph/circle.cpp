#include "circle.h"

Circle::Circle(Resource * res)
    : Ellipse(res)
{
}

Circle::Circle(QPointF const & pt)
    : Ellipse(pt)
{
}

Circle::Circle(Circle const & o)
    : Ellipse(o)
{
}

QPainterPath Circle::path()
{
    QPainterPath ph;
    if (points_.size() > 1)
    {
        QPointF off(points_.back() - points_.front());
        qreal r = sqrt(QPointF::dotProduct(off, off));
        ph.addEllipse(points_.front(), r, r);
    }
    return ph;
}

