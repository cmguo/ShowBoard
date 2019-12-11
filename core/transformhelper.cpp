#include "transformhelper.h"

#include <QGraphicsItem>
#include <QtMath>

qreal TransformHelper::angle(QPointF const & vec)
{
    if (qFuzzyIsNull(vec.x()))
        return vec.y() < 0 ? 270.0 : 90.0;
    //if (qFuzzyIsNull(vec.y()))
    //    return vec.x() > 0 ? 0.0 : 180.0;
    qreal r = atan(vec.y() / vec.x());
    if (vec.x() < 0)
        r += M_PI;
    if (r < 0)
        r += M_PI * 2.0;
    //qDebug() << vec << r * 180.0 / M_PI;
    return r * 180.0 / M_PI;
}


void TransformHelper::apply(QTransform & tf, QGraphicsItem * item,
                            QRectF const & rect, qreal rotate)
{
    QRectF irect = item->boundingRect();
    irect.moveCenter({0, 0});
    tf.reset();
    tf.rotate(rotate);
    QSizeF scale(rect.width() / irect.width(), rect.height() / irect.height());
    tf.scale(scale.width(), scale.height());
    QPointF translate(rect.center());
    tf.translate(translate.x() / tf.m11(), translate.y() / tf.m22());
}

void TransformHelper::keepAtParent(QTransform &tf, QGraphicsItem *item,
                                  QPointF const & center)
{
    QGraphicsItem * parent = item->parentItem();
    QRectF rect(item->boundingRect());
    QTransform t = parent->sceneTransform();
    rect.setWidth(rect.width() / t.m11());
    rect.setHeight(rect.height() / t.m22());
    rect.moveCenter(parent->boundingRect().center() + center);
    apply(tf, item, rect, 0.0);
}

void TransformHelper::keepAtScene(QTransform & tf, QGraphicsItem * item,
                                  QPointF const & center)
{
    QRectF rect(item->boundingRect());
    QTransform t = item->parentItem()->sceneTransform().inverted();
    rect.translate(t.dx(), t.dy());
    rect.setWidth(rect.width() * t.m11());
    rect.setHeight(rect.height() * t.m22());
    rect.moveCenter(t.map(center));
    apply(tf, item, rect, 0.0);
}
