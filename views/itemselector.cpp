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
    , type_(None)
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
    type_ = select_ == nullptr ? None :
            static_cast<SelectType>(selBox_->hitTest(selBox_->mapFromParent(start_), direction_));
    if (type_ == None) {
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
                type_ = TempNoMove;
                if (autoTop_) {
                    selectControl_->resource()->moveTop();
                }
            }
            if (mode != Control::PassSelect)
                break;
        }
    }
    if (type_ == None) {
        if (select_) {
            select(nullptr);
            qDebug() << "mousePress: select null";
        }
        if (force_) {
            type_ = Canvas;
        } else {
            QGraphicsRectItem::mousePressEvent(event);
        }
    } else {
        qDebug() << "mousePress: select " << selectControl_->resource()->url();
    }
}

void ItemSelector::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (type_ == None) {
        QGraphicsRectItem::mouseMoveEvent(event);
        return;
    }
    QPointF pt = event->pos();
    QPointF d = pt - start_;
    QRectF rect = selBox_->rect();
    switch (type_) {
    case Translate:
        if (selectControl_->flags() & Control::CanMove) {
            rect.adjust(d.x(), d.y(), d.x(), d.y());
            selBox_->setRect(rect);
            selectControl_->move(pt - start_);
        }
        break;
    case Scale: {
        //qDebug() << rect;
        QRectF rect2 = rect.adjusted(d.x() * direction_.left(), d.y() * direction_.top(),
                    d.x() * direction_.width(), d.y() * direction_.height());
        //qDebug() << "  " << rect;
        selectControl_->scale(rect, rect2);
        pt = start_ + (rect2.center() - rect.center()) * 2;
        selBox_->setRect(rect2);
        } break;
    case Canvas: {
        QGraphicsItem * canvas = parentItem();
        QRectF crect = canvas->boundingRect().adjusted(d.x(), d.y(), d.x(), d.y());
        QRectF srect = canvas->mapFromScene(scene()->sceneRect()).boundingRect();
        if (crect.left() > srect.left())
            d.setX(d.x() + srect.left() - crect.left());
        else if (crect.right() < srect.right())
            d.setX(d.x() + srect.right() - crect.right());
        if (crect.top() > srect.top())
            d.setY(d.y() + srect.top() - crect.top());
        else if (crect.bottom() < srect.bottom())
            d.setY(d.y() + srect.bottom() - crect.bottom());
        parentItem()->moveBy(d.x(), d.y());
        pt = start_;
        } break;
    case TempNoMove:
        if ((selectControl_->flags() & Control::CanMove) == 0) {
            break;
        }
        type_ = TempMoved;
        [[clang::fallthrough]];
    case TempMoved:
        selectControl_->move(pt - start_);
        break;
    default:
        break;
    }
    start_ = pt;
}

void ItemSelector::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (type_ == None) {
        QGraphicsRectItem::mouseReleaseEvent(event);
        return;
    }
    qDebug() << "mouseRelease";
    switch (type_) {
    case TempNoMove: {
        QGraphicsItem * item = select_;
        select_ = nullptr;
        select(item);
    } break;
    case TempMoved:
        select(nullptr);
        break;
    default:
        break;
    }
    type_ = None;
}
