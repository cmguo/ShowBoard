#include "rectangle.h"

Rectangle::Rectangle(Resource * res)
    : Graph2D(res)
{
}

Rectangle::Rectangle(QPointF const & pt)
    : Graph2D(pt)
{
}

Rectangle::Rectangle(Rectangle const & o)
    : Graph2D(o)
{
}

void Rectangle::movePoint(const QPointF &pt)
{
    if (points_.size() == 4) {
        points_.pop_back();
        points_.pop_back();
        points_.pop_back();
    }
    QPointF const & st = points_.first();
    QPointF p(st.x(), pt.y());
    points_.append(p);
    points_.append(pt);
    p.setX(pt.x());
    p.setY(st.y());
    points_.append(p);
}
