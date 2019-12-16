#ifndef ITEMSELECTOR_H
#define ITEMSELECTOR_H

#include "ShowBoard_global.h"

#include <QGraphicsRectItem>

class Control;
class SelectBox;
class WhiteCanvas;
class ToolbarWidget;
class ControlTransform;
class QTouchEvent;

class SHOWBOARD_EXPORT ItemSelector : public QGraphicsRectItem
{
public:
    ItemSelector(QGraphicsItem * parent = nullptr);

public:
    void select(QGraphicsItem * item);

    void selectImplied(QGraphicsItem * item);

    void enableFastClone(bool enable);

    void autoTop(bool force);

    ToolbarWidget * toolBar();

    QGraphicsItem * selected()
    {
        return select_;
    }

    void updateSelect();

private:
    enum SelectType
    {
        None = 0,
        Translate,
        Scale,
        Rotate,
        Canvas,
        TempNoMove,
        TempMoved,
        AgainNoMove,
        AgainMoved,
        FastClone,
    };

private:
    void selectAt(QPointF const & pos);

    void selectMove(QPointF const & pos);

    void selectRelease();

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

private:
    bool force_;
    bool autoTop_;
    bool fastClone_;

private:
    QGraphicsItem * select_;
    Control * selectControl_;
    ControlTransform* transform_;
    Control * cloneControl_;

private:
    QPointF start_;
    QMap<int, QPointF> lastPositions_;
    QRectF direction_;
    QRectF rect_;
    SelectType type_;
};

#endif // ITEMSELECTOR_H
