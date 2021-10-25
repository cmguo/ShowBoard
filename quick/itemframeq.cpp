#include "itemframe.h"

#include <QCursor>

ItemFrame::ItemFrame(QQuickItem * item, QQuickItem * parent)
    : QQuickItem(parent)
    , item_(item)
    , hasTopBar_(false)
    , selected_(false)
{
    setCursor(Qt::ClosedHandCursor);
    //setFlag(ItemIsPanel);
    item->setParentItem(this);
    updateRect();
}

void ItemFrame::addTopBar()
{
    hasTopBar_ = dockItems_.isEmpty();
    QRectF pad(-TOP_BAR_WIDTH, -TOP_BAR_WIDTH - TOP_BAR_HEIGHT,
               TOP_BAR_WIDTH * 2, TOP_BAR_WIDTH * 2 + TOP_BAR_HEIGHT);
    padding_ = pad;
    dockItems_.append({Surround, pad, QVariant::fromValue(nullptr)});
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

void ItemFrame::addDockItem(Dock dock, QQuickItem * item)
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
    (void) rect3;
}

/*
void ItemFrame::update()
{
    QRectF rect(boundingRect());
    rect.setHeight(HEIGHT);
    QQuickItem::update(rect);
}
*/

void ItemFrame::updateRect()
{
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

bool ItemFrame::hitTest(QQuickItem * child, const QPointF &pt)
{
    QRectF rect = boundingRect();
    rect.adjust(-padding_.left(), -padding_.top(), -padding_.right(), -padding_.bottom());
    if (rect.contains(pt))
        return false;
    return child == this || child->parentItem() == this;
}
