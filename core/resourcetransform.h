#ifndef RESOURCETRANSFORM_H
#define RESOURCETRANSFORM_H

#include "ShowBoard_global.h"

#include <QTransform>

// transform orders:
//   scale
//   rotate
//   translate

class SHOWBOARD_EXPORT ResourceTransform : public QObject
{
    Q_OBJECT
public:
    ResourceTransform(QObject * parent = nullptr);

    ResourceTransform(ResourceTransform const & o, QObject * parent = nullptr);

    explicit ResourceTransform(QTransform const & o, QObject * parent = nullptr);

    ResourceTransform& operator=(ResourceTransform const & o);

signals:
    void beforeChanged();

    void changed();

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
    void translateTo(QPointF const & delta);

    void translate(QPointF const & delta);

    /* rotate by handle
     *   center at translate of [0, 0]
     */
    void rotate(QPointF const & from, QPointF & to);

    bool rotate(qreal& delta, bool sync = true);

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

    void scaleKeepToCenter(QRectF const & border, QRectF & self, qreal scaleTo);

    void gesture(QPointF const & from1, QPointF const & from2, QPointF & to1, QPointF & to2,
                 bool translate, bool scale, bool rotate);

    void multiple(ResourceTransform const & o);

    void keepOuterOf(QRectF const & border, QRectF & self);

    void keepInnerOf(QRectF const & border, QRectF & self);

    void attachTransform(ResourceTransform * other);

public:
    friend ResourceTransform operator*(ResourceTransform const & l, ResourceTransform const & r)
    {
        return ResourceTransform(l.transform() * r.transform());
    }

    friend ResourceTransform operator*(ResourceTransform const & l, QTransform const & r)
    {
        return ResourceTransform(l.transform() * r);
    }

    friend ResourceTransform operator*(QTransform const & l, ResourceTransform const & r)
    {
        return ResourceTransform(l * r.transform());
    }

    ResourceTransform& operator*=(ResourceTransform const & l)
    {
        (*this) = (*this) * l;
        return *this;
    }

    ResourceTransform& operator*=(QTransform const & l)
    {
        (*this) = (*this) * l;
        return *this;
    }

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
