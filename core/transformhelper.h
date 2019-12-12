#ifndef TRANSFORMHELPER_H
#define TRANSFORMHELPER_H

#include <QRectF>

class QGraphicsItem;

class TransformHelper
{
public:
    static void translate(QTransform & tf, QPointF & delta);

    static void rotate(QTransform & tf, QPointF const & center,
                       QPointF const & from, QPointF const & to, qreal& result);

    static void split(QTransform const & tf, QRectF & bound, QPointF & center, qreal & rotate);

    static void apply(QTransform & tf, QGraphicsItem * item, QRectF const & rect, qreal rotate);

    static void keepAtParent(QTransform & tf, QGraphicsItem * item, QPointF const & center);

    static void keepAtScene(QTransform & tf, QGraphicsItem * item, QPointF const & center);

private:
    static qreal length(QPointF const & vec);

    static qreal angle(QPointF const & vec);
};

#endif // TRANSFORMHELPER_H
