#include "itemframe.h"

#include <QPen>
#include <QPainter>
#include <QCursor>
#include <QGraphicsScene>
#include <QGraphicsSceneResizeEvent>

ItemFrame::ItemFrame(QGraphicsItem * item, QGraphicsItem * parent)
    : QGraphicsRectItem(parent)
    , item_(item)
    , hasTopBar_(false)
    , selected_(false)
{
    setPen(Qt::NoPen);
    setBrush(Qt::transparent);
    setCursor(Qt::ClosedHandCursor);
    //setFlag(ItemIsPanel);
    item->setParentItem(this);
    updateRect();
}

void ItemFrame::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QRectF rect(boundingRect());
    if (hasTopBar_) {
        painter->save();
        painter->setBrush(brush());
        painter->setPen(pen());
        painter->drawRect(rect.adjusted(0, HEIGHT, 0, 0));
        painter->restore();
    } else {
        QGraphicsRectItem::paint(painter, option, widget);
    }
    //(void) option;
    //(void) widget;
    for (DockItem & i : dockItems_) {
        QRectF rect2(rect);
        switch (i.dock) {
        case Left:
            rect2.setWidth(i.size);
            rect.adjust(i.size, 0, 0, 0);
            break;
        case Right:
            rect.adjust(0, 0, -i.size, 0);
            rect2.adjust(rect.width(), 0, 0, 0);
            break;
        case Top:
            rect2.setHeight(i.size);
            rect.adjust(0, i.size, 0, 0);
            break;
        case Buttom:
            rect.adjust(0, 0, 0, -i.size);
            rect2.adjust(0, rect.height(), 0, 0);
            break;
        }
        if (i.item.userType() == qMetaTypeId<PaintFunc>()) {
            painter->save();
            i.item.value<PaintFunc>()(painter, rect2, this);
            painter->restore();
        } else if (i.item.userType() == QVariant::Color) {
            painter->save();
            painter->setPen(Qt::NoPen);
            painter->setBrush(i.item.value<QColor>());
            painter->drawRect(rect2);
            painter->restore();
        }
    }
}

void ItemFrame::drawTopBar(QPainter *painter, QRectF const & rect, ItemFrame * frame)
{
    painter->setPen(Qt::NoPen);
    painter->setBrush(Qt::gray);
    painter->setOpacity(frame->selected_ ? 1.0 : 0.2);
    painter->drawRect(rect);
    painter->setPen(Qt::white);
    qreal diff = rect.height() / 5;
    QPointF pt(-10, rect.center().y() - diff);
    painter->drawLine(pt, pt + QPointF(20, 0));
    pt.setY(pt.y() + diff);
    painter->drawLine(pt, pt + QPointF(20, 0));
    pt.setY(pt.y() + diff);
    painter->drawLine(pt, pt + QPointF(20, 0));
}

void ItemFrame::addTopBar()
{
    hasTopBar_ = dockItems_.isEmpty();
    addDockItem({Top, HEIGHT, QVariant::fromValue(&drawTopBar)});
}

void ItemFrame::addDockItem(Dock dock, qreal size)
{
    addDockItem({dock, size, QVariant()});
}

void ItemFrame::addDockItem(Dock dock, qreal size, QColor color)
{
    addDockItem({dock, size, color});
}

void ItemFrame::addDockItem(Dock dock, qreal size, PaintFunc paint)
{
    addDockItem({dock, size, QVariant::fromValue(paint)});
}

void ItemFrame::addDockItem(Dock dock, QGraphicsItem * item)
{
    QRectF rect(item->boundingRect());
    qreal size = dock < Top ? rect.width() : rect.height();
    item->setParentItem(this);
    addDockItem({dock, size, QVariant::fromValue(item)});
}

void ItemFrame::addDockItem(DockItem const & item)
{
    switch (item.dock) {
    case Left:
        padding_.setX(padding_.x() - item.size);
        break;
    case Right:
        padding_.setWidth(padding_.width() + item.size);
        break;
    case Top:
        padding_.setY(padding_.y() - item.size);
        break;
    case Buttom:
        padding_.setHeight(padding_.height() + item.size);
        break;
    }
    dockItems_.append(item);
}

void ItemFrame::setSelected(bool selected)
{
    if (selected == selected_)
        return;
    selected_ = selected;
    update();
}

void ItemFrame::setRect(const QRectF &rect3)
{
    QRectF rect = rect3;
    rect.moveCenter(padding_.center());
    QGraphicsRectItem::setRect(rect);
    for (DockItem & i : dockItems_) {
        QRectF rect2(rect);
        switch (i.dock) {
        case Left:
            rect2.setWidth(i.size);
            rect.adjust(i.size, 0, 0, 0);
            break;
        case Right:
            rect.adjust(0, 0, -i.size, 0);
            rect2.adjust(rect.width(), 0, 0, 0);
            break;
        case Top:
            rect2.setHeight(i.size);
            rect.adjust(0, i.size, 0, 0);
            break;
        case Buttom:
            rect.adjust(0, 0, 0, -i.size);
            rect2.adjust(0, rect.height(), 0, 0);
            break;
        }
        if (i.item.userType() == qMetaTypeId<QGraphicsItem*>()) {
            QGraphicsItem * item = i.item.value<QGraphicsItem*>();
            QGraphicsSceneResizeEvent event;
            event.setNewSize(rect2.size());
            event.setOldSize(item->boundingRect().size());
            item->scene()->sendEvent(item, &event);
            QPointF center = item->boundingRect().center();
            item->setPos(rect2.center() - center);
        }
    }
}

/*
void ItemFrame::update()
{
    QRectF rect(boundingRect());
    rect.setHeight(HEIGHT);
    QGraphicsRectItem::update(rect);
}
*/

void ItemFrame::updateRect()
{
    QRectF rect = item_->mapToParent(item_->boundingRect()).boundingRect();
    updateRectFromChild(rect);
}

void ItemFrame::updateRectFromChild(QRectF & rect)
{
    rect.adjust(padding_.x(), padding_.y(), padding_.right(), padding_.bottom());
    QRectF rect2 = rect;
    rect2.moveCenter(padding_.center());
    setRect(rect2);
}

void ItemFrame::updateRectToChild(QRectF & rect)
{
    QRectF rect2 = rect;
    rect.adjust(-padding_.x(), -padding_.y(), -padding_.right(), -padding_.bottom());
    rect2.moveCenter(padding_.center());
    setRect(rect2);
}

bool ItemFrame::hitTest(const QPointF &pt)
{
    QRectF rect = boundingRect();
    rect.adjust(-padding_.left(), -padding_.top(), -padding_.right(), -padding_.bottom());
    if (rect.contains(pt))
        return false;
    for (DockItem & i : dockItems_) {
        if (i.item.userType() == qMetaTypeId<QGraphicsItem*>()) {
            QGraphicsItem * item = i.item.value<QGraphicsItem*>();
            if (item->contains(mapToItem(item, pt)))
                return false;
        }
    }
    return true;
}
