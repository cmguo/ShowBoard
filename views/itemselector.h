#ifndef ITEMSELECTOR_H
#define ITEMSELECTOR_H

#include "ShowBoard_global.h"

#include <QGraphicsRectItem>

class Control;
class SelectBox;
class WhiteCanvas;
class ToolbarWidget;
class ControlTransform;

class SHOWBOARD_EXPORT ItemSelector : public QGraphicsRectItem
{
public:
    ItemSelector(QGraphicsItem * parent = nullptr);

public:
    void select(QGraphicsItem * item);

    void selectImplied(QGraphicsItem * item);

    void startFastClone();

    void autoTop(bool force);

    ToolbarWidget * toolBar();

    QGraphicsItem * selected()
    {
        return select_;
    }

    void updateSelect();

private:
    friend class WhiteCanvas;

    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

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

private:
    QPointF start_;
    QRectF direction_;
    QRectF rect_;
    qreal rotate_;

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

    SelectType type_;
};

#endif // ITEMSELECTOR_H
