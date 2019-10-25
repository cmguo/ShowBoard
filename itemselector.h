#ifndef ITEMSELECTOR_H
#define ITEMSELECTOR_H

#include "ShowBoard_global.h"

#include <QGraphicsRectItem>

class Control;
class SelectBox;
class WhiteCanvas;
class ToolbarWidget;

class SHOWBOARD_EXPORT ItemSelector : public QGraphicsRectItem
{
public:
    ItemSelector(QGraphicsItem * canvas, QGraphicsItem * parent = nullptr);

public:
    void select(QGraphicsItem * item);

    void setForce(bool force);

    ToolbarWidget * toolBar();

    QGraphicsItem * selected()
    {
        return select_;
    }

private:
    friend class WhiteCanvas;

    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    QGraphicsItem * canvas_;
    SelectBox * selBox_;

private:
    QGraphicsItem * select_;
    Control * selectControl_;
    bool force_;
    QPointF start_;
    QRectF direction_;
    int type_;
};

#endif // ITEMSELECTOR_H
