#ifndef POSITIONBAR_H
#define POSITIONBAR_H

#include "ShowBoard_global.h"

#include <QGraphicsPathItem>

class SHOWBOARD_EXPORT PositionBar : public QGraphicsPathItem
{
public:
    PositionBar(QGraphicsItem * parent = nullptr);

public:
    void setInCanvas(bool in = true);

    void update(QRectF const & viewRect, QRectF const & canvasRect, qreal scale, QPointF offset);

private:
    bool inCanvas_ = false;
};

#endif // POSITIONBAR_H
