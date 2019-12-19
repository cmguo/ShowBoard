#include "itemselector.h"
#include "core/control.h"
#include "core/resourceview.h"
#include "core/controltransform.h"
#include "toolbarwidget.h"
#include "whitecanvas.h"
#include "selectbox.h"
#include "controls/whitecanvascontrol.h"

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
    , transform_(new ControlTransform(1))
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
        transform_->attachTo(selectControl_->transform());
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
        transform_->attachTo(nullptr);
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

void ItemSelector::selectAt(const QPointF &pos, QPointF const & scenePos)
{
    start_ = pos;
    type_ = None;
    if (select_ && selBox_->isVisible()) {
        type_ = static_cast<SelectType>(
                    selBox_->hitTest(selBox_->mapFromParent(pos), direction_));
        if (fastClone_ && type_ == Translate)
            type_ = FastClone;
    }
    if (type_ == None) {
        QList<QGraphicsItem*> items = scene()->items(scenePos);
        for (QGraphicsItem * item : items) {
            Control * ct = Control::fromItem(item);
            if (!ct)
                continue;
            Control::SelectMode mode = Control::NotSelect;
            if ((force_ && (ct->flags() & Control::DefaultFlags))
                    || (mode = ct->selectTest(mapToItem(item, pos))) == Control::Select) {
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
        if (selectControl_->metaObject() == &WhiteCanvasControl::staticMetaObject)
            start_ = scenePos;
        qDebug() << "select" << type_ << selectControl_->resource()->url();
    }
}

void ItemSelector::selectMove(QPointF const & pos, QPointF const & scenePos)
{
    QPointF pt = pos;
    if (selectControl_->metaObject() == &WhiteCanvasControl::staticMetaObject)
        pt = scenePos;
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
        parentItem()->setAcceptTouchEvents(true);
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
    selectAt(event->pos(), event->scenePos());
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
    selectMove(event->pos(), event->scenePos());
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
    selectAt(point.pos(), point.scenePos());
    if (type_ != None) {
        bool isCanvas = selectControl_->metaObject() == &WhiteCanvasControl::staticMetaObject;
        for (QTouchEvent::TouchPoint const & point : event->touchPoints()) {
            lastPositions_[point.id()] = isCanvas ? point.scenePos() : point.pos();
        }
        return;
    }
    // for type_ Rotate and Scale, fall back to mouse event
    event->ignore();
}

void ItemSelector::touchUpdate(QTouchEvent *event)
{
    QMap<int, QPointF> positions;
    bool isCanvas = selectControl_->metaObject() == &WhiteCanvasControl::staticMetaObject;
    for (QTouchEvent::TouchPoint const & point : event->touchPoints()) {
        positions[point.id()] = isCanvas ? point.scenePos() : point.pos();
    }
    if (event->touchPoints().size() != 2 || type_ == Scale || type_ == Rotate) {
        QTouchEvent::TouchPoint const & point(event->touchPoints().first());
        selectMove(point.pos(), point.scenePos());
        positions[point.id()] = start_;
    } else {
        QTouchEvent::TouchPoint const & point1(event->touchPoints().at(0));
        QTouchEvent::TouchPoint const & point2(event->touchPoints().at(1));
        if (lastPositions_.size() < 2) {
            if (!lastPositions_.contains(point2.id()))
                lastPositions_[point2.id()] = isCanvas ? point2.lastScenePos() : point2.lastPos();
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
    return event->isAccepted();
}

bool ItemSelector::sceneEventFilter(QGraphicsItem *watched, QEvent *event)
{
    (void) watched;
    bool mouse = false;
    switch (event->type()) {
    case QEvent::GraphicsSceneMousePress:
        mouse = true;
        force_ = true;
        mousePressEvent(static_cast<QGraphicsSceneMouseEvent*>(event));
        force_ = false;
        break;
    case QEvent::GraphicsSceneMouseMove:
        mouse = true;
        mouseMoveEvent(static_cast<QGraphicsSceneMouseEvent*>(event));
        break;
    case QEvent::GraphicsSceneMouseRelease:
        mouse = true;
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
        break;
    }
    // for mouse events, return true to stop handling,
    //   this will cause sendEvent() return false,
    //   but return value is not used by mousePressEventHandler()
    //   if event is accepted, then grab take effect
    // for touch events, return value is used, we can't cover here
    return mouse && event->isAccepted();
}
