#ifndef CANVASITEM_H
#define CANVASITEM_H

#include "ShowBoard_global.h"

#include <QBrush>
#include <QGraphicsItem>

class SHOWBOARD_EXPORT CanvasItem : public QGraphicsItem
{
public:
    CanvasItem(QGraphicsItem * parent = nullptr);

public:
    void setRect(QRectF const & rect);

    void setBrush(QBrush const & brush);

    QRectF rect() const;

    QBrush brush() const;

protected:
    virtual QRectF boundingRect() const override;

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

private:
    QRectF rect_;
    QBrush brush_;
};

#endif // CANVASITEM_H
