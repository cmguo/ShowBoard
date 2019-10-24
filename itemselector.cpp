#include "itemselector.h"
#include "control.h"
#include "toolbarwidget.h"
#include "whitecanvas.h"
#include "selectbox.h"

#include <QPen>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsProxyWidget>
#include <QGraphicsScene>

ItemSelector::ItemSelector(QGraphicsItem * canvas, QGraphicsItem * parent)
    : QGraphicsRectItem(parent)
    , canvas_(canvas)
    , force_(false)
    , type_(0)
{
    selBox_ = new SelectBox(this);

    ToolbarWidget * toolBar = new ToolbarWidget();
    QGraphicsProxyWidget * proxy = new QGraphicsProxyWidget(this);
    proxy->setWidget(toolBar);
    toolBar_ = proxy;
    setAcceptedMouseButtons(Qt::LeftButton);

    select(nullptr);
}

void ItemSelector::select(QGraphicsItem *item)
{
    selBox_->setVisible(item);
    toolBar_->setVisible(item);
    if (item) {
        QRectF rect = item->mapToParent(item->boundingRect()).boundingRect();
        setBoxRect(rect);
        selBox_->setPos(0, 0);
        itemChange(ItemPositionHasChanged, pos());
        select_ = item;
        selectControl_ = Control::fromItem(item);
        QList<ToolButton *> buttons;
        selectControl_->getToolButtons(buttons);
        toolBar()->setToolButtons(buttons);
    } else {
        select_ = nullptr;
        selectControl_ = nullptr;
    }
}

void ItemSelector::setBoxRect(QRectF const & rect)
{
    selBox_->setRect(rect);
    toolBar_->setPos(rect.right() - toolBar_->boundingRect().width(), rect.bottom() + 10);
}

ToolbarWidget * ItemSelector::toolBar()
{
    return static_cast<ToolbarWidget*>(
                static_cast<QGraphicsProxyWidget*>(toolBar_)->widget());
}

void ItemSelector::setForce(bool force)
{
    force_ = force;
}

void ItemSelector::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    start_ = event->pos();
    type_ = select_ == nullptr ? 0 :
            selBox_->hitTest(selBox_->mapFromParent(start_), direction_);
    if (type_ == 0) {
        QList<QGraphicsItem*> items = scene()->items(event->scenePos());
        for (QGraphicsItem * item : items) {
            if (!canvas_->childItems().contains(item))
                continue;
            Control * ct = Control::fromItem(item);
            if ((ct->flags() & Control::CanSelect)
                    && (force_ || ct->selectTest(mapToItem(item, start_)))) {
                select(nullptr);
                select_ = item;
                selectControl_ = ct;
                type_ = 10;
                break;
            }
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
    QPointF pt = mapToParent(event->pos());
    QPointF d = pt - start_;
    QRectF rect = selBox_->rect();
    switch (type_) {
    case 1:
        rect.adjust(d.x(), d.y(), d.x(), d.y());
        setBoxRect(rect);
        selectControl_->move(pt - start_);
        break;
    case 2: {
        //qDebug() << rect;
        QRectF rect2 = rect.adjusted(d.x() * direction_.left(), d.y() * direction_.top(),
                    d.x() * direction_.width(), d.y() * direction_.height());
        //qDebug() << "  " << rect;
        selectControl_->scale(rect, rect2);
        pt = start_ + (rect2.center() - rect.center()) * 2;
        setBoxRect(rect2);
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
    case 10:
        select(select_);
        break;
    case 11:
        select(nullptr);
        break;
    }
    type_ = 0;
}
