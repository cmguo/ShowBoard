#include "selectbar.h"

#include <QPen>
#include <QPainter>
#include <QCursor>

SelectBar::SelectBar(QGraphicsItem * item, QGraphicsItem * parent)
    : QGraphicsRectItem(parent)
    , item_(item)
    , selected_(false)
{
    setPen(Qt::NoPen);
    setBrush(Qt::transparent);
    setCursor(Qt::ClosedHandCursor);
    item->setParentItem(this);
    updateRect();
}

void SelectBar::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    (void) option;
    (void) widget;
    painter->save();
    painter->setPen(Qt::NoPen);
    painter->setBrush(Qt::gray);
    painter->setOpacity(selected_ ? 1.0 : 0.2);
    QRectF rect(boundingRect());
    rect.setHeight(48);
    painter->drawRect(rect);
    painter->setPen(Qt::white);
    qreal diff = rect.height() / 5;
    QPointF pt(-10, rect.center().y() - diff);
    painter->drawLine(pt, pt + QPointF(20, 0));
    pt.setY(pt.y() + diff);
    painter->drawLine(pt, pt + QPointF(20, 0));
    pt.setY(pt.y() + diff);
    painter->drawLine(pt, pt + QPointF(20, 0));
    painter->restore();
}

void SelectBar::setSelected(bool selected)
{
    if (selected == selected_)
        return;
    selected_ = selected;
    QRectF rect(boundingRect());
    rect.setHeight(48);
    update(rect);
}

void SelectBar::updateRect()
{
    QRectF rect = item_->boundingRect();
    rect.moveCenter({0, 0});
    rect.adjust(0, -48, 0, 0);
    setRect(rect);
    QPointF center = -rect.center();
    setTransform(QTransform::fromTranslate(center.x(), center.y()));
}
