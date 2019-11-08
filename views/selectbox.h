#ifndef SELECTBOX_H
#define SELECTBOX_H

#include "ShowBoard_global.h"

#include <QGraphicsRectItem>

class QGraphicsPathItem;
class ToolbarWidget;

/*
 * A selection box with context menu
 */

class SHOWBOARD_EXPORT SelectBox : public QGraphicsRectItem
{
public:
    SelectBox(QGraphicsItem * parent = nullptr);

public:
    void setRect(QRectF const & rect);

    int hitTest(QPointF const & pos, QRectF & direction);

    ToolbarWidget * toolBar();

private:
    QGraphicsPathItem * leftTop_;
    QGraphicsPathItem * rightTop_;
    QGraphicsPathItem * rightBottom_;
    QGraphicsPathItem * leftBottom_;
    QGraphicsPathItem * left_;
    QGraphicsPathItem * top_;
    QGraphicsPathItem * right_;
    QGraphicsPathItem * bottom_;
    QGraphicsItem * toolBar_;
};

#endif // SELECTBOX_H
