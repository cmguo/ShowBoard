#ifndef ITEMSELECTOR_H
#define ITEMSELECTOR_H

#include "ShowBoard_global.h"

#include "canvasitem.h"

class Control;
class SelectBox;
class WhiteCanvas;
class ToolbarWidget;
class ControlTransform;
class QTouchEvent;

class SHOWBOARD_EXPORT ItemSelector : public CanvasItem
{
public:
    ItemSelector(QGraphicsItem * parent = nullptr);

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

private:
    void select2(Control * control);

    void selectAt(QPointF const & pos, QPointF const & scenePos, bool fromTouch);

    void selectMove(QPointF const & pos, QPointF const & scenePos);

    void selectRelease();

    void layoutToolbar();

private:
    virtual QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value) override;

    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

    virtual void touchBegin(QTouchEvent* event);

    virtual void touchUpdate(QTouchEvent* event);

    virtual void touchEnd(QTouchEvent* event);

    virtual bool sceneEvent(QEvent *event) override;

    virtual bool sceneEventFilter(QGraphicsItem *watched, QEvent *event) override;

private:
    SelectBox * selBox_;
    QGraphicsItem * toolBar_;

private:
    bool force_;
    bool autoTop_;
    bool hideMenu_; // WhenEditing
    bool fastClone_;
    bool autoUnselect_;

private:
    Control * selectControl_;
    Control * tempControl_;
    ControlTransform* selBoxTransform_;
    ControlTransform* selBoxCanvasTransform_;
    ControlTransform* toolBarTransform_;
    Control * cloneControl_;

private:
    QPointF start_;
    QMap<int, QPointF> lastPositions_;
    QRectF direction_;
    QRectF rect_;
    SelectType type_;
};

#endif // ITEMSELECTOR_H
