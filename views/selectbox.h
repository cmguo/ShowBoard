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

    void setVisible(bool select, bool scale = false, bool scale2 = false, bool rotate = false, bool mask = false);

    int hitTest(QPointF const & pos, QRectF & direction);

private:
    QGraphicsPathItem * rotate_;
    QGraphicsPathItem * leftTop_;
    QGraphicsPathItem * rightTop_;
    QGraphicsPathItem * rightBottom_;
    QGraphicsPathItem * leftBottom_;
    QGraphicsPathItem * left_;
    QGraphicsPathItem * top_;
    QGraphicsPathItem * right_;
    QGraphicsPathItem * bottom_;
};

#endif // SELECTBOX_H
