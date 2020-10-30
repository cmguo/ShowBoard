#ifndef ITEMSELECTOR_H
#define ITEMSELECTOR_H

#include "ShowBoard_global.h"

#include "canvasitem.h"

class Control;
class SelectBox;
class WhiteCanvas;
class ToolbarWidget;
class ControlTransform;
class GestureContext;

class QTouchEvent;

class SHOWBOARD_EXPORT ItemSelector : public CanvasItem
{
public:
    ItemSelector(CanvasView * parent = nullptr);

public:
    void select(Control * control);

    void unselect(Control * control);

    void selectImplied(Control * control);

    void updateSelect(Control * control);

    bool adjusting(Control * control);

    void enableFastClone(bool enable = true);

    void autoMoveSelectionTop(bool enable = true);

    void hideMenuWhenEditing(bool hide = true);

    void unselectOnDeactive(bool enable = true);

    ToolbarWidget * toolBar();

    Control * selected() const { return selectControl_; }

    QEvent * currentEvent() const { return currentEvent_; }

private:
    enum SelectType
    {
        None = 0,
        Translate,
        Scale,
        Rotate,
        TempNoMove,
        TempMoved,
        AgainNoMove,
        AgainMoved,
        FastClone,
        Implied,
    };

    enum EventType
    {
        Mouse,
        Touch,
        Wheel
    };

private:
    void select2(Control * control);

    void selectAt(QPointF const & pos, QPointF const & scenePos, EventType eventType);

    void selectMove(QPointF const & pos, QPointF const & scenePos);

    void selectRelease(EventType eventType);

    void layoutToolbar();

private:
#ifdef SHOWBOARD_QUICK
#else
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
#endif

    void touchBegin(QTouchEvent* event);

    void touchUpdate(QTouchEvent* event);

    void touchEnd(QTouchEvent* event);

#ifdef SHOWBOARD_QUICK
#else
    virtual void wheelEvent(QGraphicsSceneWheelEvent *event) override;

    virtual bool sceneEvent(QEvent *event) override;
#endif

public:
    // for StylusGuestureHelper from TeachingTools
    // TODO:
#ifdef SHOWBOARD_QUICK
#else
    virtual bool sceneEventFilter(QGraphicsItem *watched, QEvent *event) override;
#endif

private:
    SelectBox * selBox_;
    CanvasView * toolBar_;

private:
    bool force_;
    bool autoTop_;
    bool hideMenu_; // WhenEditing
    bool fastClone_;
    bool autoUnselect_;

private:
    ControlTransform* selBoxTransform_;
    ControlTransform* selBoxCanvasTransform_;
    ControlTransform* toolBarTransform_;
    Control * selectControl_;
    Control * tempControl_;
    Control * cloneControl_;

private:
    QEvent * currentEvent_;
    CanvasView * currentEventSource_;
    QPointF start_;
    QMap<int, QPointF> lastPositions_;
    QRectF direction_;
    QRectF rect_;
    SelectType type_;
    GestureContext * gctx_;
};

#endif // ITEMSELECTOR_H
