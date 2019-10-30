#include "square.h"

Square::Square(Resource * res)
    : Rectangle(res)
{
}

Square::Square(QPointF const & pt)
    : Rectangle(pt)
{
}

Square::Square(Square const & o)
    : Rectangle(o)
{
}

void Square::movePoint(const QPointF &pt)
{
    QPointF st = points_.first();
    QSizeF sz = QSizeF(qAbs(pt.x() - st.x()), qAbs(pt.y() - st.y()));
    QPointF p(pt);
    if (sz.width() > sz.height()) {
        p.setY(pt.y() >= st.y() ? st.y() + sz.width() : st.y() - sz.width());
    } else {
        p.setX(pt.x() >= st.x() ? st.x() + sz.height() : st.x() - sz.height());
    }
    Rectangle::movePoint(p);
}
