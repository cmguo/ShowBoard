#include "ellipse.h"

Ellipse::Ellipse()
{
}

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

bool Ellipse::commit(const QPointF &pt)
{
    (void) pt;
    if (points_.size() == 1)
    {
        QPointF st = points_.back();
        st += QPointF(80, 80);
        points_.append(st);
    }
    return true;
}

QPainterPath Ellipse::path()
{
    QPainterPath ph;
    if (points_.size() > 1)
    {
        QPointF off(points_.back() - points_.front());
        off.setX(qAbs(off.x()));
        off.setY(qAbs(off.y()));
        ph.addEllipse(points_.front(), off.x(), off.y());
    }
    return ph;
}

