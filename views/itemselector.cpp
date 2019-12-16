#include "itemselector.h"
#include "core/control.h"
#include "core/resourceview.h"
#include "core/controltransform.h"
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
    , fastClone_(false)
    , select_(nullptr)
    , selectControl_(nullptr)
    , transform_(new ControlTransform)
    , cloneControl_(nullptr)
    , type_(None)
{
    setPen(QPen(Qt::NoPen));
    setAcceptedMouseButtons(Qt::LeftButton);
    setAcceptTouchEvents(true);

    selBox_ = new SelectBox(this);
    selBox_->setVisible(false);
    selBox_->setTransformations({transform_});
}

void ItemSelector::select(QGraphicsItem *item)
{
    if (item == select_) {
        if (selBox_->isVisible())
            return;
    }
    if (item) {
        select_ = item;
        selectControl_ = Control::fromItem(item);
        rect_ = selectControl_->boundRect();
        selBox_->setRect(rect_);
        transform_->setResourceTransform(&selectControl_->resource()->transform());
        transform_->setParent(selectControl_->transform());
        QList<ToolButton *> buttons;
        selectControl_->getToolButtons(buttons);
        toolBar()->setToolButtons(buttons);
        selBox_->setVisible(true, selectControl_->flags() & Control::CanScale,
                            (selectControl_->flags() & Control::CanRotate));
        selectControl_->select(true);
        //itemChange(ItemPositionHasChanged, pos());
    } else {
        select_ = nullptr;
        if (selectControl_)
            selectControl_->select(false);
        transform_->setResourceTransform(nullptr);
        transform_->setParent(nullptr);
        selectControl_ = nullptr;
        selBox_->setVisible(false);
        fastClone_ = false;
        cloneControl_ = nullptr;
    }
}

void ItemSelector::selectImplied(QGraphicsItem *item)
{
    if (item != select_)
        select(item);
    selBox_->hide();
}

void ItemSelector::enableFastClone(bool enable)
{
    fastClone_ = enable;
}

void ItemSelector::updateSelect()
{
    if (!select_)
        return;
    selBox_->setRect(selectControl_->boundRect());
}

void ItemSelector::selectAt(const QPointF &pos)
{
    type_ = None;
    if (select_ && selBox_->isVisible()) {
        type_ = static_cast<SelectType>(
                    selBox_->hitTest(selBox_->mapFromParent(start_), direction_));
        if (fastClone_ && type_ == Translate)
            type_ = FastClone;
    }
    if (type_ == None) {
        QList<QGraphicsItem*> items = scene()->items(pos);
        for (QGraphicsItem * item : items) {
            Control * ct = Control::fromItem(item);
            if (!ct)
                continue;
            Control::SelectMode mode = Control::NotSelect;
            if ((force_ && (ct->flags() & Control::DefaultFlags))
                    || (mode = ct->selectTest(mapToItem(item, start_))) == Control::Select) {
                type_ = TempNoMove;
                if (ct != selectControl_) {
                    select(nullptr);
                    select_ = ct->item();
                    selectControl_ = ct;
                    selectControl_->select(true);
                } else {
                    type_ = AgainNoMove;
                }
                if (autoTop_) {
                    selectControl_->resource()->moveTop();
                }
            } else if (ct == selectControl_) {
                // selectImplied, not handle
                return;
            }
            if (mode != Control::PassSelect)
                break;
        }
    }
    if (type_ == None) {
        if (select_) {
            select(nullptr);
            qDebug() << "select null";
        }
        //if (force_) {
        //    type_ = Canvas;
        //}
    } else {
        qDebug() << "select" << type_ << selectControl_->resource()->url();
    }
}

void ItemSelector::selectMove(QPointF const & pos)
{
    QPointF pt = pos;
    QPointF d = pt - start_;
    switch (type_) {
    case Translate:
        if (selectControl_->flags() & Control::CanMove) {
            selectControl_->move(d);
        }
        break;
    case Scale: {
        //qDebug() << rect;
        if (!selectControl_->scale(rect_, direction_, d)) {
            pt = start_;
            break;
        }
        pt = start_ + d;
        selBox_->setRect(rect_);
        } break;
    case Rotate:
        selectControl_->rotate(start_, pt);
        break;
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
    case AgainNoMove:
        if ((selectControl_->flags() & Control::CanMove) == 0) {
            break;
        }
        if (qAbs(d.x()) + qAbs(d.y()) < 10) {
            pt = start_;
            break;
        }
        qDebug() << type_ << d;
        type_ = static_cast<SelectType>(type_ + 1);
        Q_FALLTHROUGH();
    case TempMoved:
    case AgainMoved:
        selectControl_->move(d);
        break;
    case FastClone:
        if (cloneControl_) {
            cloneControl_->move(d);
        } else if (qAbs(d.x()) + qAbs(d.y()) < 10) {
            pt = start_;
        } else {
            cloneControl_ = static_cast<WhiteCanvas*>(
                        parentItem())->copyResource(selectControl_);
        }
        break;
    default:
        break;
    }
    start_ = pt;
}

void ItemSelector::selectRelease()
{
    switch (type_) {
    case AgainNoMove:
        select(select_);
        break;
    case TempNoMove:
        if (selectControl_->flags() & Control::CanSelect) {
            QGraphicsItem * item = select_;
            selectControl_->select(false);
            select_ = nullptr;
            select(item);
            break;
        }
        Q_FALLTHROUGH();
    case TempMoved:
        select(nullptr);
        break;
    case FastClone:
        cloneControl_ = nullptr;
        break;
    default:
        break;
    }
    type_ = None;
}

ToolbarWidget * ItemSelector::toolBar()
{
    return selBox_->toolBar();
}

void ItemSelector::autoTop(bool force)
{
    autoTop_ = force;
}

QVariant ItemSelector::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemSceneHasChanged) {
        parentItem()->installSceneEventFilter(this);
    }
    return value;
}

void ItemSelector::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->source() != Qt::MouseEventNotSynthesized) {
        event->ignore();
        return;
    }
    start_ = event->pos();
    selectAt(event->scenePos());
    if (type_ == None) {
        QGraphicsRectItem::mousePressEvent(event);
    }
}

void ItemSelector::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->source() != Qt::MouseEventNotSynthesized) {
        event->ignore();
        return;
    }
    if (type_ == None) {
        QGraphicsRectItem::mouseMoveEvent(event);
        return;
    }
    selectMove(event->pos());
}

void ItemSelector::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->source() != Qt::MouseEventNotSynthesized) {
        event->ignore();
        return;
    }
    if (type_ == None) {
        QGraphicsRectItem::mouseReleaseEvent(event);
        return;
    }
    qDebug() << "mouseRelease";
    selectRelease();
}

void ItemSelector::touchBegin(QTouchEvent *event)
{
    QTouchEvent::TouchPoint const & point(event->touchPoints().first());
    start_ = point.pos();
    selectAt(point.scenePos());
    if (type_ != None) {
        for (QTouchEvent::TouchPoint const & point : event->touchPoints()) {
            lastPositions_[point.id()] = point.pos();
        }
        return;
    }
    // for type_ Rotate and Scale, fall back to mouse event
    event->ignore();
}

void ItemSelector::touchUpdate(QTouchEvent *event)
{
    QMap<int, QPointF> positions;
    for (QTouchEvent::TouchPoint const & point : event->touchPoints()) {
        positions[point.id()] = point.pos();
    }
    if (event->touchPoints().size() != 2 || type_ == Scale || type_ == Rotate) {
        QTouchEvent::TouchPoint const & point(event->touchPoints().first());
        selectMove(point.pos());
        positions[point.id()] = start_;
    } else {
        QTouchEvent::TouchPoint const & point1(event->touchPoints().at(0));
        QTouchEvent::TouchPoint const & point2(event->touchPoints().at(1));
        if (lastPositions_.size() < 2) {
            if (!lastPositions_.contains(point2.id()))
                lastPositions_[point2.id()] = point2.lastPos();
        }
        selectControl_->gesture(lastPositions_[point1.id()], lastPositions_[point2.id()],
                positions[point1.id()], positions[point2.id()]);
        rect_ = selectControl_->boundRect();
        selBox_->setRect(rect_);
        if (type_ == TempNoMove || type_ == AgainNoMove)
            type_ = static_cast<SelectType>(type_ + 1);
    }
    lastPositions_.swap(positions);
}

void ItemSelector::touchEnd(QTouchEvent *event)
{
    (void) event;
    lastPositions_.clear();
    selectRelease();
}

bool ItemSelector::sceneEvent(QEvent *event)
{
    switch (event->type()) {
    case QEvent::TouchBegin:
        touchBegin(static_cast<QTouchEvent*>(event));
        break;
    case QEvent::TouchUpdate:
        touchUpdate(static_cast<QTouchEvent*>(event));
        break;
    case QEvent::TouchEnd:
        touchEnd(static_cast<QTouchEvent*>(event));
        break;
    default:
        return QGraphicsRectItem::sceneEvent(event);
    }
    return true;
}

bool ItemSelector::sceneEventFilter(QGraphicsItem *watched, QEvent *event)
{
    (void) watched;
    switch (event->type()) {
    case QEvent::GraphicsSceneMousePress:
        force_ = true;
        mousePressEvent(static_cast<QGraphicsSceneMouseEvent*>(event));
        force_ = false;
        break;
    case QEvent::GraphicsSceneMouseMove:
        mouseMoveEvent(static_cast<QGraphicsSceneMouseEvent*>(event));
        break;
    case QEvent::GraphicsSceneMouseRelease:
        mouseReleaseEvent(static_cast<QGraphicsSceneMouseEvent*>(event));
        break;
    case QEvent::TouchBegin:
        force_ = true;
        touchBegin(static_cast<QTouchEvent*>(event));
        force_ = false;
        break;
    case QEvent::TouchUpdate:
        touchUpdate(static_cast<QTouchEvent*>(event));
        break;
    case QEvent::TouchEnd:
        touchEnd(static_cast<QTouchEvent*>(event));
        break;
    default:
        event->ignore();
        break;
    }
    return event->isAccepted();
}
