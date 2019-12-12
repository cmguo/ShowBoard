#ifndef RESOURCETRANSFORM_H
#define RESOURCETRANSFORM_H

#include "ShowBoard_global.h"

#include <QTransform>

class SHOWBOARD_EXPORT ResourceTransform
{
public:
    ResourceTransform();

public:
    QTransform const & transform() const
    {
        return transform_;
    }

    QTransform const & scale() const
    {
        return scale_;
    }

    QTransform const & rotate() const
    {
        return rotate_;
    }

    QTransform const & translate() const
    {
        return translate_;
    }

    // for geometry, when inner adjust pos
    QTransform const & scaleRotate() const
    {
        return scaleRotate_;
    }

    // for wrap frame and select box, scale by setRect,
    //   then apply rotate and translate
    QTransform const & rotateTranslate() const
    {
        return rotateTranslate_;
    }

public:
    void translate(QPointF const & delta);

    /* rotate by handle
     *   center at translate of [0, 0]
     */
    void rotate(QPointF const & from, QPointF & to);

    void scaleTo(qreal scale);

    void scale(QSizeF const & delta);

    /* Scale by border handles
     *   scale with one direction of 8 directions by delta
     *   current rotation should take account
     *   return result only apply with scale
     *   both origin and result are centered at [0, 0]
     */
    bool scale(QRectF & rect, QRectF const & direction, QPointF & delta,
               QRectF const & padding, bool KeepAspectRatio, bool layoutScale, qreal minSize);

private:
    static qreal length(QPointF const & vec);

    static qreal angle(QPointF const & vec);

private:
    QTransform scale_;
    QTransform rotate_;
    QTransform translate_;
    QTransform transform_;
    QTransform scaleRotate_;
    QTransform rotateTranslate_;
};

#endif // RESOURCETRANSFORM_H
