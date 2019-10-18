#ifndef ITEMSELECTOR_H
#define ITEMSELECTOR_H

#include "ShowBoard_global.h"

#include <QGraphicsRectItem>

class Control;
class SelectBox;
class WhiteCanvas;

class SHOWBOARD_EXPORT ItemSelector : public QGraphicsRectItem
{
public:
    ItemSelector(QGraphicsItem * canvas, QGraphicsItem * parent = nullptr);

public:
    void select(QGraphicsItem * item);

public:
    void copySelection();

    void removeSelection();

    void setBoxRect(QRectF const & rect);

    void setForce(bool force);

private:
    friend class WhiteCanvas;

    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    QGraphicsItem * canvas_;
    SelectBox * selBox_;
    QGraphicsItem * toolBar_;

private:
    QGraphicsItem * select_;
    Control * selectControl_;
    bool force_;
    QPointF start_;
    QRectF direction_;
    int type_;
};

#endif // ITEMSELECTOR_H
