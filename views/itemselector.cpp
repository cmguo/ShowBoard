#include "itemselector.h"
#include "core/control.h"
#include "core/resourceview.h"
#include "core/controltransform.h"
#include "core/resourcetransform.h"
#include "toolbarwidget.h"
#include "whitecanvas.h"
#include "selectbox.h"
#include "qsshelper.h"
#include "controls/whitecanvascontrol.h"

#include <QPen>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QGraphicsProxyWidget>
#include <QApplication>

#include <core/resource.h>

#define ENBALE_TOUCH 1

ItemSelector::ItemSelector(QGraphicsItem * parent)
    : CanvasItem(parent)
    , force_(false)
    , autoTop_(false)
    , hideMenu_(false)
    , fastClone_(false)
    , autoUnselect_(false)
    , selBoxTransform_(new ControlTransform(ControlTransform::SelectBox))
    , selBoxCanvasTransform_(new ControlTransform(ControlTransform::SelectBoxLargeCanvas))
    , toolBarTransform_(new ControlTransform(ControlTransform::LargeCanvasTooBar))
    , selectControl_(nullptr)
    , tempControl_(nullptr)
    , cloneControl_(nullptr)
    , currentEvent_(nullptr)
    , currentEventSource_(nullptr)
    , type_(None)
    , gctx_(nullptr)
{
    setAcceptedMouseButtons(Qt::LeftButton);
#if ENBALE_TOUCH
    setAcceptTouchEvents(true);
#endif
    setRect(QRectF(-1, -1, 1, 1));

    selBox_ = new SelectBox(this);
    selBox_->hide();
    selBox_->setTransformations({selBoxTransform_, selBoxCanvasTransform_});

    ToolbarWidget * toolBar = new ToolbarWidget();
    QGraphicsProxyWidget * proxy = new QGraphicsProxyWidget(this);
    proxy->setWidget(toolBar);
    toolBar_ = proxy;
    toolBar_->setTransformations({toolBarTransform_});
    toolBar_->hide();
    QObject::connect(toolBar, &ToolbarWidget::sizeChanged, [this]() {
        layoutToolbar();
    });
}

void ItemSelector::select(Control * control)
{
    if (control && !control->flags().testFlag(Control::CanSelect))
        return;
    qInfo() << "select" << (control ? control->resource()->url() : QUrl());
    select2(control);
}

void ItemSelector::unselect(Control * control)
{
    if (control == selectControl_) {
        qInfo() << "unselect" << control->resource()->url();
        select2(nullptr);
    }
    if (control == tempControl_) {
        qInfo() << "unselect cancel";
        type_ = None;
        tempControl_ = nullptr;
    }
}

void ItemSelector::selectImplied(Control * control)
{
    if (control != selectControl_)
        select2(control);
    selBox_->hide();
    toolBar_->hide();
}

void ItemSelector::updateSelect(Control * control)
{
    if (control != selectControl_)
        return;
    rect_ = selectControl_->boundRect();
    selBox_->setRect(rect_);
    layoutToolbar();
}

bool ItemSelector::adjusting(Control * control)
{
    if (control != selectControl_)
        return false;
    return type_ != None;
}

void ItemSelector::enableFastClone(bool enable)
{
    fastClone_ = enable;
}

void ItemSelector::selectAt(const QPointF &pos, QPointF const & scenePos, EventType eventType)
{
    type_ = None;
    if (selectControl_ && selBox_->isVisible()) {
        type_ = static_cast<SelectType>(
                    selBox_->hitTest(selBox_->mapFromItem(currentEventSource_, pos), direction_));
#ifndef QT_DEBUG
        if (eventType == Wheel
                && !QApplication::keyboardModifiers().testFlag(Qt::ControlModifier))
            type_ = None;
#endif
        if (fastClone_ && type_ == Translate)
            type_ = FastClone;
        if (type_ == None && eventType == Touch) // maybe hit menu bar
            return;
        if (type_ == Translate && !selectControl_->flags()
                .testFlag(Control::ShowSelectMask))
            type_ = None;
    }
    if (type_ == None) {
        QList<QGraphicsItem*> items = scene()->items(scenePos);
        QVector<QGraphicsItem*> children(items.size(), nullptr); // first child
        Control::SelectMode mode = Control::PassSelect;
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
            if (!ct) {
#ifdef QT_DEBUG
                if (force_ && item == parentItem())
                    ct = static_cast<WhiteCanvas*>(parentItem())->canvasControl();
                if (!ct)
#endif
                    continue;
            }
            bool force = force_ && (ct->flags() & Control::DefaultFlags);
            // if item can not handle touch events, we also pass through all touch events here
            // but let selectTest take effect
            if (eventType == Touch
                    && !(ct->flags() & (Control::Touchable | Control::FullSelect)))
                force = false;
            if (!force) {
                mode = ct->selectTest(children[i], item,
                                      mapToItem(ct->item(), pos),
                                      mode == Control::PassSelect2);
            }
            //qDebug() << force << ct->resource()->name() << mode;
            if (force || mode == Control::Select) {
                if (ct != selectControl_) {
                    select2(nullptr);
                    type_ = TempNoMove;
                    //ct->select(true);
                } else {
                    type_ = selBox_->isVisible() ? Translate : AgainNoMove;
                }
                if (eventType == Wheel) {
#ifndef QT_DEBUG
                    if (!QApplication::keyboardModifiers().testFlag(Qt::ControlModifier)
                            && ct->metaObject() != &WhiteCanvasControl::staticMetaObject) {
                        type_ = None;
                        continue;
                    }
#endif
                    type_ = AgainMoved;
                }
                tempControl_ = ct;
                if (autoTop_) {
                    tempControl_->resource()->moveTop();
                }
                break;
            } else if (ct == selectControl_) {
                // selectImplied, not handle
                type_ = Implied;
                return;
            }
            if (mode != Control::PassSelect && mode != Control::PassSelect2)
                break;
        }
    } else { // (type_ != None)
        tempControl_ = selectControl_;
    }
    if (type_ == None) {
        // only unselect with mouse event,
        //  if this is from touch, we will receive mouse event later
        if (selectControl_ && eventType == Mouse) {
            select2(nullptr);
            qInfo() << "selectAt null";
        }
        //if (force_) {
        //    type_ = Canvas;
        //}
    } else {
        start_ = pos;
        if (tempControl_->metaObject() == &WhiteCanvasControl::staticMetaObject
                || (tempControl_->flags() & Control::FixedOnCanvas))
            start_ = scenePos;
        qInfo() << "selectAt" << type_ << tempControl_->resource()->url();
        tempControl_->adjusting(true);
    }
}

void ItemSelector::selectMove(QPointF const & pos, QPointF const & scenePos)
{
    QPointF pt = pos;
    if (tempControl_->metaObject() == &WhiteCanvasControl::staticMetaObject
            || (tempControl_->flags() & Control::FixedOnCanvas))
        pt = scenePos;
    if (!scene()->sceneRect().contains(scenePos))
        return;
    QPointF d = pt - start_;
    switch (type_) {
    case Translate:
        if (tempControl_->flags() & Control::CanMove)
            tempControl_->move(d);
        break;
    case Scale: {
        if (!tempControl_->scale(rect_, direction_, d)) {
            pt = start_;
            break;
        }
        pt = start_ + d;
        selBox_->setRect(rect_);
        } break;
    case Rotate:
        tempControl_->rotate(start_, pt);
        break;
    case TempNoMove:
    case AgainNoMove:
        if ((tempControl_->flags() & Control::CanMove) == 0) {
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
        tempControl_->move(d);
        break;
    case FastClone:
        if (cloneControl_) {
            cloneControl_->move(d);
        } else if (qAbs(d.x()) + qAbs(d.y()) < 10) {
            pt = start_;
        } else {
            cloneControl_ = static_cast<WhiteCanvas*>(
                        parentItem())->copyResource(tempControl_);
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
    tempControl_->adjusting(false);
    switch (type_) {
    case AgainNoMove:
        select2(tempControl_); // leave Implied Select
        break;
    case TempNoMove:
        if (tempControl_->flags() & Control::CanSelect) {
            //selectControl_->select(false);
            select2(tempControl_);
            break;
        }
        Q_FALLTHROUGH();
    case AgainMoved:
        break;
    case TempMoved:
        qInfo() << "select cancel";
        select2(nullptr);
        break;
    case FastClone:
        cloneControl_ = nullptr;
        break;
    default:
        break;
    }
    type_ = None;
    tempControl_ = nullptr;
    if (hideMenu_) {
        //if (toolBar()->buttons_())
        toolBar_->setVisible(selBox_->isVisible());
        layoutToolbar();
    }
}

void ItemSelector::layoutToolbar()
{
    if (!toolBar_->isVisible())
        return;
    QRectF boxRect = selBox_->mapRectToScene(selBox_->boundRect());
    QRectF sceneRect = scene()->sceneRect();
    QSizeF size = toolBar()->size();
    qreal padding = dp(10);
    if (Control * canvasControl = Control::fromItem(parentItem())) {
        padding *= canvasControl->resource()->transform().zoom();
    }
    boxRect.adjust(0, 0, 0, size.height() + padding);
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

void ItemSelector::select2(Control *control)
{
    if (control == selectControl_) {
        if (control && !selBox_->isVisible()) {
            toolBar_->show();
            layoutToolbar();
            selBox_->setVisible(true,
                                control->flags() & Control::CanScale,
                                !(control->flags() & Control::KeepAspectRatio),
                                control->flags() & Control::CanRotate,
                                control->flags() & Control::ShowSelectMask);
            // call for geometry control
            control->select(true);
        }
        return;
    }
    if (selectControl_) {
        selectControl_->select(false);
        selectControl_->disconnect(selBoxTransform_);
        selectControl_->resource()->transform().disconnect(toolBar());
        selBoxTransform_->attachTo(nullptr);
        selBoxCanvasTransform_->attachTo(nullptr);
        toolBarTransform_->attachTo(nullptr);
        selectControl_ = nullptr;
        Control * canvasControl = Control::fromItem(parentItem());
        if (canvasControl) {
            canvasControl->resource()->transform().disconnect(toolBar());
        }
        selBox_->setVisible(false);
        toolBar_->hide();
        toolBar()->attachProvider(nullptr);
        fastClone_ = false;
        cloneControl_ = nullptr;
        type_ = None;
    }
    selectControl_ = control;
    if (selectControl_) {
        rect_ = selectControl_->boundRect();
        selBox_->setRect(rect_);
        selBoxTransform_->attachTo(selectControl_->transform());
        //QList<ToolButton *> buttons;
        //selectControl_->getToolButtons(buttons);
        Control * canvasControl = Control::fromItem(parentItem());
        if (canvasControl) {
            if (selectControl_->flags().testFlag(Control::FixedOnCanvas))
                selBoxCanvasTransform_->attachTo(canvasControl->transform());
            toolBarTransform_->attachTo(canvasControl->transform());
            QObject::connect(&canvasControl->resource()->transform(), &ResourceTransform::changed, toolBar(),
                             [this]() { layoutToolbar(); });
        }
        QObject::connect(selectControl_, &Control::destroyed,
                         selBoxTransform_, [this, control]() {
            qWarning() << "destroyed without unselect!!";
            unselect(control);
        });
        QObject::connect(&selectControl_->resource()->transform(), &ResourceTransform::changed, toolBar(),
                                 [this](int elem) {
            if (elem < 4) // if scale changed, not do here, control will updateSelect
                layoutToolbar();
        });
        toolBar()->attachProvider(selectControl_);
        toolBar_->show();
        selBox_->setVisible(true,
                            selectControl_->flags() & Control::CanScale,
                            !(selectControl_->flags() & Control::KeepAspectRatio),
                            selectControl_->flags() & Control::CanRotate,
                            selectControl_->flags() & Control::ShowSelectMask);
        layoutToolbar();
        if (autoTop_) {
            selectControl_->resource()->moveTop();
        }
        selectControl_->select(true);
        //itemChange(ItemPositionHasChanged, pos());
    }
}

void ItemSelector::autoMoveSelectionTop(bool enable)
{
    autoTop_ = enable;
}

void ItemSelector::hideMenuWhenEditing(bool hide)
{
    hideMenu_ = hide;
}

void ItemSelector::unselectOnDeactive(bool enable)
{
    autoUnselect_ = enable;
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
    //qDebug() << "mousePress";
#endif
    if (event->source() != Qt::MouseEventNotSynthesized
            && type_ == Implied) {
        CanvasItem::mousePressEvent(event);
        return;
    }
    selectAt(mapFromItem(currentEventSource_, event->pos()), event->scenePos(), Mouse);
    if (type_ == None || type_ == Implied) {
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
    if (type_ == None || type_ == Implied) {
        CanvasItem::mouseMoveEvent(event);
        return;
    }
    //qDebug() << "mouseMove";
    selectMove(mapFromItem(currentEventSource_, event->pos()), event->scenePos());
}

void ItemSelector::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
#if ENBALE_TOUCH
    if (!lastPositions_.empty()) {
        //event->ignore();
        return;
    }
#endif
    if (type_ == None || type_ == Implied) {
        CanvasItem::mouseReleaseEvent(event);
        return;
    }
    //qDebug() << "mouseRelease";
    selectRelease();
}

void ItemSelector::touchBegin(QTouchEvent *event)
{
    //qDebug() << "touchBegin";
    QTouchEvent::TouchPoint const & point(event->touchPoints().first());
    selectAt(mapFromItem(currentEventSource_, point.pos()), point.scenePos(), Touch);
    if (type_ != None && type_ != Implied) {
        bool isCanvas = tempControl_->metaObject() == &WhiteCanvasControl::staticMetaObject
                || (tempControl_->flags() & Control::FixedOnCanvas);
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
    if (tempControl_ == nullptr)
        return;
    //qDebug() << "touchUpdate";
    QMap<int, QPointF> positions;
    bool isCanvas = tempControl_->metaObject() == &WhiteCanvasControl::staticMetaObject
            || (tempControl_->flags() & Control::FixedOnCanvas);
    for (QTouchEvent::TouchPoint const & point : event->touchPoints()) {
        positions[point.id()] = isCanvas
                ? point.scenePos()
                : mapFromItem(currentEventSource_, point.pos());
    }
    bool gesture = false;
    //qDebug() << positions;
    if (event->touchPoints().size() != 2 || type_ == Scale || type_ == Rotate) {
        QTouchEvent::TouchPoint const & point(event->touchPoints().first());
        if (lastPositions_.contains(point.id())) {
            start_ = lastPositions_[point.id()];
            if (event->device()->type() == QTouchDevice::TouchPad && event->touchPoints().size() < 2) {
                // touchpad is moving mouse position, not handlecd
                if (type_ == TempNoMove || type_ == AgainNoMove) {
                    QPointF d = point.pos() = start_;
                    if (qAbs(d.x()) + qAbs(d.y()) >= 10) {
                        type_ = static_cast<SelectType>(type_ + 1);
                    }
                }
            } else {
                selectMove(point.pos(), point.scenePos());
            }
            positions[point.id()] = start_;
        } else {
            if (type_ == TempNoMove || type_ == AgainNoMove) {
                type_ = static_cast<SelectType>(type_ + 1);
            }
        }
    } else {
        QTouchEvent::TouchPoint const & point1(event->touchPoints().at(0));
        QTouchEvent::TouchPoint const & point2(event->touchPoints().at(1));
        if (scene()->sceneRect().contains(point1.scenePos())
                && scene()->sceneRect().contains(point2.scenePos())) {
            if (!lastPositions_.contains(point1.id())) {
                lastPositions_[point1.id()] = positions[point1.id()];
                // qDebug() << lastPositions_[point1.id()] << lastPositions_[point2.id()] << "<->"
                //        << positions[point1.id()] << positions[point2.id()];
            }
            if (!lastPositions_.contains(point2.id())) {
                lastPositions_[point2.id()] = positions[point2.id()];
                // qDebug() << lastPositions_[point1.id()] << lastPositions_[point2.id()] << "<->"
                //        << positions[point1.id()] << positions[point2.id()];
            }
            gesture = true;
            if (gctx_ == nullptr)
                gctx_ = new GestureContext;
            if (!gctx_->started())
                gctx_->start(lastPositions_[point1.id()], lastPositions_[point2.id()]);
            tempControl_->gesture(gctx_, positions[point1.id()], positions[point2.id()]);
            rect_ = tempControl_->boundRect();
            selBox_->setRect(rect_);
            if (type_ == TempNoMove || type_ == AgainNoMove)
                type_ = static_cast<SelectType>(type_ + 1);
            if (hideMenu_)
                toolBar_->hide();
            else
                layoutToolbar();
        }
    }
    if (!gesture && gctx_) {
        gctx_->pause();
    }
    lastPositions_.swap(positions);
}

void ItemSelector::touchEnd(QTouchEvent *)
{
    if (tempControl_ == nullptr)
        return;
    if (gctx_) {
        delete gctx_;
        gctx_ = nullptr;
    }
    //qDebug() << "touchEnd";
    lastPositions_.clear();
    selectRelease();
}

void ItemSelector::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    selectAt(mapFromItem(currentEventSource_, event->pos()), event->scenePos(), Wheel);
    if (tempControl_) {
        if (event->modifiers().testFlag(Qt::KeyboardModifier::ControlModifier)) {
            qreal delta = event->delta() > 0 ? 1.2 : 1.0 / 1.2;
            tempControl_->scale(tempControl_->item()->mapFromScene(event->scenePos()), delta);
        } else {
            QPointF d;
            if (event->modifiers().testFlag(Qt::KeyboardModifier::ShiftModifier)) {
                d.setX(event->delta());
            } else {
                d.setY(event->delta());
            }
            tempControl_->move(d);
        }
        selectRelease();
    } else {
        event->ignore();
    }
}

bool ItemSelector::sceneEvent(QEvent *event)
{
    currentEvent_ = event;
    currentEventSource_ = this;
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
    case QEvent::WindowDeactivate:
        if (autoUnselect_)
            select2(nullptr);
        break;
    default:
        bool b = CanvasItem::sceneEvent(event);
        currentEvent_ = nullptr;
        currentEventSource_ = nullptr;
        return b;
    }
    currentEvent_ = nullptr;
    currentEventSource_ = nullptr;
    return event->isAccepted();
}

bool ItemSelector::sceneEventFilter(QGraphicsItem *watched, QEvent *event)
{
    (void) watched;
    bool mouse = false;
    //qDebug() << "sceneEventFilter" << event->type();
    currentEvent_ = event;
    currentEventSource_ = watched;
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
    case QEvent::GraphicsSceneWheel:
        mouse = true;
        force_ = true;
        wheelEvent(static_cast<QGraphicsSceneWheelEvent*>(event));
        force_ = false;
        break;
    default:
        break;
    }
    currentEvent_ = nullptr;
    currentEventSource_ = nullptr;
    // for mouse events, return true to stop handling,
    //   this will cause sendEvent() return false,
    //   but return value is not used by mousePressEventHandler()
    //   if event is accepted, then grab take effect
    // for touch events, return value is used, we can't cover here
    return mouse && event->isAccepted();
}
