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
#include <QGraphicsProxyWidget>

#define ENBALE_TOUCH 1

ItemSelector::ItemSelector(QGraphicsItem * parent)
    : CanvasItem(parent)
    , force_(false)
    , autoTop_(false)
    , hideMenu_(false)
    , fastClone_(false)
    , select_(nullptr)
    , selectControl_(nullptr)
    , selBoxTransform_(new ControlTransform(ControlTransform::SelectBox))
    , toolBarTransform_(new ControlTransform(ControlTransform::LargeCanvasTooBar))
    , cloneControl_(nullptr)
    , type_(None)
{
    setAcceptedMouseButtons(Qt::LeftButton);
#if ENBALE_TOUCH
    setAcceptTouchEvents(true);
#endif
    setRect(QRectF(-1, -1, 1, 1));

    selBox_ = new SelectBox(this);
    selBox_->hide();
    selBox_->setTransformations({selBoxTransform_});

    ToolbarWidget * toolBar = new ToolbarWidget();
    QGraphicsProxyWidget * proxy = new QGraphicsProxyWidget(this);
    proxy->setWidget(toolBar);
    toolBar_ = proxy;
    toolBar_->setTransformations({toolBarTransform_});
    toolBar_->hide();
    QObject::connect(toolBar, &ToolbarWidget::sizeChanged, [this](QSizeF const &) {
        layoutToolbar();
    });
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
        selBoxTransform_->attachTo(selectControl_->transform());
        QList<ToolButton *> buttons;
        selectControl_->getToolButtons(buttons);
        Control * canvasControl = Control::fromItem(parentItem());
        if (canvasControl)
            toolBarTransform_->attachTo(canvasControl->transform());
        toolBar()->setToolButtons(buttons);
        layoutToolbar();
        selBox_->setVisible(true,
                            selectControl_->flags() & Control::CanScale,
                            selectControl_->flags() & Control::CanRotate);
        toolBar_->show();
        selectControl_->select(true);
        //itemChange(ItemPositionHasChanged, pos());
    } else {
        select_ = nullptr;
        if (selectControl_)
            selectControl_->select(false);
        selBoxTransform_->attachTo(nullptr);
        toolBarTransform_->attachTo(nullptr);
        selectControl_ = nullptr;
        selBox_->setVisible(false);
        toolBar_->hide();
        toolBar()->clear();
        fastClone_ = false;
        cloneControl_ = nullptr;
    }
}

void ItemSelector::selectImplied(QGraphicsItem *item)
{
    if (item != select_)
        select(item);
    selBox_->hide();
    toolBar_->hide();
}

void ItemSelector::updateSelect(QGraphicsItem *item)
{
    if (item != select_)
        return;
    rect_ = selectControl_->boundRect();
    selBox_->setRect(rect_);
    layoutToolbar();
}

bool ItemSelector::adjusting(QGraphicsItem *item)
{
    if (item != select_)
        return false;
    return type_ != None;
}

void ItemSelector::enableFastClone(bool enable)
{
    fastClone_ = enable;
}

void ItemSelector::selectAt(const QPointF &pos, QPointF const & scenePos, bool fromTouch)
{
    start_ = pos;
    type_ = None;
    if (select_ && selBox_->isVisible()) {
        type_ = static_cast<SelectType>(
                    selBox_->hitTest(selBox_->mapFromParent(pos), direction_));
        if (fastClone_ && type_ == Translate)
            type_ = FastClone;
        if (type_ == None && fromTouch) { // maybe hit menu bar
            return;
        }
    }
    if (type_ == None) {
        QList<QGraphicsItem*> items = scene()->items(scenePos);
        QVector<QGraphicsItem*> children(items.size(), nullptr); // first child
        for (int i = 0; i < items.size(); ++i) {
            QGraphicsItem * item = items[i];
            if (children[i] == nullptr)
                children[i] = item;
            QGraphicsItem * parent = item->parentItem();
            while (parent) {
                int j = items.indexOf(parent, i + 1);
                if (j > i && children[j] == nullptr)
                    children[j] = item;
                parent = parent->parentItem();
            }
            Control * ct = Control::fromItem(item);
            if (!ct)
                continue;
            Control::SelectMode mode = Control::NotSelect;
            bool force = force_ && (ct->flags() & Control::DefaultFlags);
            // if item can not handle touch events, we also pass through all touch events here
            // but let selectTest take effect
            if (fromTouch
                    && !(ct->flags() & (Control::Touchable | Control::FullSelect)))
                force = false;
            if (!force) {
                mode = ct->selectTest(children[i], item, mapToItem(ct->item(), pos));
            }
            if (force || mode == Control::Select) {
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
        if (select_ && !fromTouch) {
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
        selectControl_->adjusting(true);
    }
}

void ItemSelector::selectMove(QPointF const & pos, QPointF const & scenePos)
{
    QPointF pt = pos;
    if (selectControl_->metaObject() == &WhiteCanvasControl::staticMetaObject)
        pt = scenePos;
    if (!scene()->sceneRect().contains(scenePos))
        return;
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
    if (hideMenu_)
        toolBar_->hide();
    else
        layoutToolbar();
    start_ = pt;
}

void ItemSelector::selectRelease()
{
    if (type_ == None)
        return;
    selectControl_->adjusting(false);
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
    if (hideMenu_) {
        toolBar_->setVisible(selBox_->isVisible());
        layoutToolbar();
    }
}

void ItemSelector::layoutToolbar()
{
    if (!toolBar_->isVisible())
        return;
    QRectF boxRect = selBox_->mapToScene(selBox_->boundingRect()).boundingRect();
    QRectF sceneRect = scene()->sceneRect();
    QSizeF size = toolBar()->size();
    boxRect.adjust(0, 0, 0, size.height() + 10);
    boxRect &= sceneRect;
    QPointF pos(boxRect.center().x() - size.width() / 2, boxRect.bottom() - size.height());
    if (pos.x() < sceneRect.left())
        pos.setX(sceneRect.left());
    if (pos.x() + size.width()  > sceneRect.right())
        pos.setX(sceneRect.right() - size.width());
    if (pos.y() < sceneRect.top())
        pos.setY(sceneRect.top());
    toolBar_->setPos(mapFromScene(pos));
}

ToolbarWidget * ItemSelector::toolBar()
{
    return static_cast<ToolbarWidget*>(
                static_cast<QGraphicsProxyWidget*>(toolBar_)->widget());
}

void ItemSelector::autoMoveSelectionTop(bool enable)
{
    autoTop_ = enable;
}

void ItemSelector::hideMenuWhenEditing(bool hide)
{
    hideMenu_ = hide;
}

QVariant ItemSelector::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemSceneHasChanged) {
#if ENBALE_TOUCH
        parentItem()->setAcceptTouchEvents(true);
#endif
        parentItem()->setAcceptedMouseButtons(Qt::LeftButton);
        parentItem()->installSceneEventFilter(this);
    }
    return value;
}

void ItemSelector::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
#if ENBALE_TOUCH
    if (!lastPositions_.empty()) {
        //event->ignore();
        return;
    }
#endif
    selectAt(event->pos(), event->scenePos(), false);
    if (type_ == None) {
        CanvasItem::mousePressEvent(event);
    }
}

void ItemSelector::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
#if ENBALE_TOUCH
    if (!lastPositions_.empty()) {
        //event->ignore();
        return;
    }
#endif
    if (type_ == None) {
        CanvasItem::mouseMoveEvent(event);
        return;
    }
    selectMove(event->pos(), event->scenePos());
}

void ItemSelector::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
#if ENBALE_TOUCH
    if (!lastPositions_.empty()) {
        //event->ignore();
        return;
    }
#endif
    if (type_ == None) {
        CanvasItem::mouseReleaseEvent(event);
        return;
    }
    qDebug() << "mouseRelease";
    selectRelease();
}

void ItemSelector::touchBegin(QTouchEvent *event)
{
    QTouchEvent::TouchPoint const & point(event->touchPoints().first());
    selectAt(point.pos(), point.scenePos(), true);
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
    if (selectControl_ == nullptr)
        return;
    QMap<int, QPointF> positions;
    bool isCanvas = selectControl_->metaObject() == &WhiteCanvasControl::staticMetaObject;
    for (QTouchEvent::TouchPoint const & point : event->touchPoints()) {
        positions[point.id()] = isCanvas ? point.scenePos() : point.pos();
    }
    if (event->touchPoints().size() != 2 || type_ == Scale || type_ == Rotate) {
        QTouchEvent::TouchPoint const & point(event->touchPoints().first());
        if (lastPositions_.contains(point.id())) {
            start_ = lastPositions_[point.id()];
            selectMove(point.pos(), point.scenePos());
            positions[point.id()] = start_;
        }
    } else {
        QTouchEvent::TouchPoint const & point1(event->touchPoints().at(0));
        QTouchEvent::TouchPoint const & point2(event->touchPoints().at(1));
        if (scene()->sceneRect().contains(point1.scenePos())
                && scene()->sceneRect().contains(point2.scenePos())) {
            //if (lastPositions_.size() < 2) {
                if (!lastPositions_.contains(point2.id()))
                    lastPositions_[point2.id()] = positions[point2.id()];
            //}
            selectControl_->gesture(lastPositions_[point1.id()], lastPositions_[point2.id()],
                    positions[point1.id()], positions[point2.id()]);
            rect_ = selectControl_->boundRect();
            selBox_->setRect(rect_);
            if (type_ == TempNoMove || type_ == AgainNoMove)
                type_ = static_cast<SelectType>(type_ + 1);
            if (hideMenu_)
                toolBar_->hide();
            else
                layoutToolbar();
        }
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
        return CanvasItem::sceneEvent(event);
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
