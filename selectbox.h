#ifndef SELECTBOX_H
#define SELECTBOX_H

#include <QGraphicsRectItem>

class QGraphicsPathItem;

class SelectBox : public QGraphicsRectItem
{
public:
    SelectBox(QGraphicsItem * parent = nullptr);

public:
    void setRect(QRectF const & rect);

    int hitTest(QPointF const & pos, QRectF & direction);

private:
    QGraphicsPathItem * leftTop_;
    QGraphicsPathItem * rightBottom_;
};

#endif // SELECTBOX_H
