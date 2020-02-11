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
//        painter->save();
//        painter->setBrush(brush());
//        painter->setPen(pen());
//        QRectF rect2 = rect.adjusted(TOP_BAR_WIDTH, TOP_BAR_WIDTH + TOP_BAR_HEIGHT,
//                   -TOP_BAR_WIDTH, -TOP_BAR_WIDTH);
//        painter->drawRect(rect2);
//        painter->restore();
    } else {
        QGraphicsRectItem::paint(painter, option, widget);
    }
    //(void) option;
    //(void) widget;
    for (DockItem & i : dockItems_) {
        QRectF rect2 = reversePadding(i, rect);
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

void ItemFrame::drawTopBar(QPainter *painter, QRectF const & rect, ItemFrame *)
{
    static QPixmap icon(":/showboard/icons/drag.png");
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor("#FFF4F4F4"));
    //painter->setOpacity(frame->selected_ ? 1.0 : 0.2);
    painter->drawRect(rect);
    QRectF rect2 = rect;
    rect2.setHeight(TOP_BAR_HEIGHT);
    QLinearGradient gradient(rect2.left(), rect2.top(), rect2.left(), rect2.bottom());
    gradient.setColorAt(0, QColor("#FFF9F9F9"));
    gradient.setColorAt(1, QColor("#FFECECEC"));
    painter->setBrush(gradient);
    painter->drawRect(rect2);
    QRectF iconRect(0, 0, icon.width(), icon.height());
    iconRect.moveCenter(rect2.center());
    painter->drawPixmap(iconRect.toRect(), icon);
}

void ItemFrame::addTopBar()
{
    hasTopBar_ = dockItems_.isEmpty();
    QRectF pad(-TOP_BAR_WIDTH, -TOP_BAR_WIDTH - TOP_BAR_HEIGHT,
               TOP_BAR_WIDTH * 2, TOP_BAR_WIDTH * 2 + TOP_BAR_HEIGHT);
    padding_ = pad;
    dockItems_.append({Surround, pad, QVariant::fromValue(&drawTopBar)});
}

void ItemFrame::addDockItem(Dock dock, qreal size)
{
    addDockItem(dock, size, QVariant());
}

void ItemFrame::addDockItem(Dock dock, qreal size, QColor color)
{
    addDockItem(dock, size, QVariant(color));
}

void ItemFrame::addDockItem(Dock dock, qreal size, PaintFunc paint)
{
    addDockItem(dock, size, QVariant::fromValue(paint));
}

void ItemFrame::addDockItem(Dock dock, QGraphicsItem * item)
{
    QRectF rect(item->boundingRect());
    qreal size = dock < Top ? rect.width() : rect.height();
    item->setParentItem(this);
    addDockItem(dock, size, QVariant::fromValue(item));
}

void ItemFrame::addDockItem(Dock dock, qreal size, QVariant item)
{
    QRectF pad;
    switch (dock) {
    case Left:
        pad = QRectF(-size, 0, size, 0);
        break;
    case Right:
        pad = QRectF(0, 0, size, 0);
        break;
    case Top:
        pad = QRectF(0, -size, 0, size);
        break;
    case Buttom:
        pad = QRectF(0, 0, 0, size);
        break;
    default:
        break;
    }
    padding_.adjust(pad.left(), pad.top(), pad.right(), pad.bottom());
    dockItems_.append({dock, pad, item});
}

QRectF ItemFrame::reversePadding(DockItem & i, QRectF &rect)
{
    QRectF rect2(rect);
    // (0, 0, 4, 4)
    // (-1, 0, 1, 0) -> (0, 0, 1, 4)
    // (0, -1, 0, 1) -> (0, 0, 4, 1)
    // (0, 0, 1, 0) -> (3, 0, 1, 4)
    // (0, 0, 0, 1) -> (0, 3, 4, 1)
    rect.adjust(-i.pad.left(), -i.pad.top(), -i.pad.right(), -i.pad.bottom());
    if (i.dock < Surround)
        rect2 = QRectF(qFuzzyIsNull(i.pad.right()) ? rect2.left() : rect.right(),
                       qFuzzyIsNull(i.pad.bottom()) ? rect2.top() : rect.bottom(),
                       qFuzzyIsNull(i.pad.width()) ? rect2.width() : i.pad.width(),
                       qFuzzyIsNull(i.pad.height()) ? rect2.height() : i.pad.height());
    return rect2;
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
        QRectF rect2 = reversePadding(i, rect);
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

bool ItemFrame::hitTest(QGraphicsItem * child, const QPointF &pt)
{
    QRectF rect = boundingRect();
    rect.adjust(-padding_.left(), -padding_.top(), -padding_.right(), -padding_.bottom());
    if (rect.contains(pt))
        return false;
    return child == this || child->parentItem() == this;
}
