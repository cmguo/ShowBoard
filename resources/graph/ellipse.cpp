#include "ellipse.h"

Ellipse::Ellipse(Resource * res)
    : Graph2D(res)
{
}

Ellipse::Ellipse(QPointF const & pt)
    : Graph2D(pt)
{
}

Ellipse::Ellipse(Ellipse const & o)
    : Graph2D(o)
{
}

QPainterPath Ellipse::path()
{
    QPainterPath ph;
    if (points_.size() > 1)
    {
        QPointF off(points_.back() - points_.front());
        off.setX(qAbs(off.x()) * sqrt(2.0));
        off.setY(qAbs(off.y()) * sqrt(2.0));
        ph.addEllipse(points_.front(), off.x(), off.y());
    }
    return ph;
}

