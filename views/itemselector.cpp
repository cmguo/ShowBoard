#include "itemselector.h"
#include "core/control.h"
#include "core/resourceview.h"
#include "toolbarwidget.h"
#include "whitecanvas.h"
#include "selectbox.h"

#include <QPen>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>

ItemSelector::ItemSelector(QGraphicsItem * parent)
    : QGraphicsRectItem(parent)
    , force_(false)
    , autoTop_(false)
    , select_(nullptr)
    , selectControl_(nullptr)
    , type_(0)
{
    setPen(QPen(Qt::NoPen));
    setAcceptedMouseButtons(Qt::LeftButton);

    selBox_ = new SelectBox(this);
    selBox_->setVisible(false);
}

void ItemSelector::select(QGraphicsItem *item)
{
    if (item == select_)
        return;
    if (item) {
        QRectF rect = item->mapToParent(item->boundingRect()).boundingRect();
        selBox_->setRect(rect);
        itemChange(ItemPositionHasChanged, pos());
        select_ = item;
        selectControl_ = Control::fromItem(item);
        QList<ToolButton *> buttons;
        selectControl_->getToolButtons(buttons);
        toolBar()->setToolButtons(buttons);
        selBox_->setVisible(true);
    } else {
        select_ = nullptr;
        selectControl_ = nullptr;
        selBox_->setVisible(false);
    }
}

ToolbarWidget * ItemSelector::toolBar()
{
    return selBox_->toolBar();
}

void ItemSelector::setForce(bool force)
{
    force_ = force;
}

void ItemSelector::autoTop(bool force)
{
    autoTop_ = force;
}

void ItemSelector::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    start_ = event->pos();
    type_ = select_ == nullptr ? 0 :
            selBox_->hitTest(selBox_->mapFromParent(start_), direction_);
    if (type_ == 0) {
        QList<QGraphicsItem*> items = scene()->items(event->scenePos());
        for (QGraphicsItem * item : items) {
            Control * ct = Control::fromItem(item);
            if (!ct)
                continue;
            Control::SelectMode mode = Control::NotSelect;
            if ((force_ && (ct->flags() & Control::CanSelect))
                    || (mode = ct->selectTest(mapToItem(item, start_))) == Control::Select) {
                select(nullptr);
                select_ = item;
                selectControl_ = ct;
                type_ = 10;
                if (autoTop_) {
                    selectControl_->resource()->moveTop();
                }
            }
            if (mode != Control::PassSelect)
                break;
        }
    }
    if (type_ == 0) {
        if (select_)
            select(nullptr);
        else
            QGraphicsRectItem::mousePressEvent(event);
    }
}

void ItemSelector::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (type_ == 0) {
        QGraphicsRectItem::mouseMoveEvent(event);
        return;
    }
    QPointF pt = event->pos();
    QPointF d = pt - start_;
    QRectF rect = selBox_->rect();
    switch (type_) {
    case 1:
        rect.adjust(d.x(), d.y(), d.x(), d.y());
        selBox_->setRect(rect);
        selectControl_->move(pt - start_);
        break;
    case 2: {
        //qDebug() << rect;
        QRectF rect2 = rect.adjusted(d.x() * direction_.left(), d.y() * direction_.top(),
                    d.x() * direction_.width(), d.y() * direction_.height());
        //qDebug() << "  " << rect;
        selectControl_->scale(rect, rect2);
        pt = start_ + (rect2.center() - rect.center()) * 2;
        selBox_->setRect(rect2);
        } break;
    case 10:
    case 11:
        type_ = 11;
        selectControl_->move(pt - start_);
        break;
    }
    start_ = pt;
}

void ItemSelector::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (type_ == 0) {
        QGraphicsRectItem::mouseReleaseEvent(event);
        return;
    }
    switch (type_) {
    case 10: {
        QGraphicsItem * item = select_;
        select_ = nullptr;
        select(item);
    } break;
    case 11:
        select(nullptr);
        break;
    }
    type_ = 0;
}
