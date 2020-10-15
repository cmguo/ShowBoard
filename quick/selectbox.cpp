#include "selectbox.h"

SelectBox::SelectBox(QQuickItem * parent)
    : QQuickItem(parent)
{

}

void SelectBox::setRect(QRectF const & rect)
{
    (void) rect;
}

void SelectBox::setVisible(bool select, bool scale, bool scale2, bool rotate, bool mask)
{
    (void) select;
    (void) scale;
    (void) scale2;
    (void) rotate;
    (void) mask;
}

int SelectBox::hitTest(QPointF const & pos, QRectF & direction)
{
    (void) pos;
    (void) direction;
    return 0;
}

QRectF SelectBox::boundRect() const
{
    return QRectF();
}
