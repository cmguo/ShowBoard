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
    /* emit before signal changed() is emited
     *   you can't discard modification, but you can call adjust functions like
     *   keepOuterOf() and keepInnerOf(), infact these funtions are only worked here
     *   elements is 3 bits of [scale, rotate, translate],
     *   for example, when only translate is changed, elements = 1
     */
    void beforeChanged(int elements);

    /* emit when changed
     * <elements> is same as <elements> in beforeChanged()
     */
    void changed(int elements);

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

    qreal angle() const;

    QPointF offset() const
    {
        return QPointF(translate_.dx(), translate_.dy());
    }

public:
    void translateTo(QPointF const & delta);

    void translate(QPointF const & delta);

    /* rotate by handle
     *   center at translate of [0, 0]
     */
    void rotate(QPointF const & from, QPointF & to);

    /* rotate by handle
     *   center at translate of center
     */
    void rotate(QPointF const & center, QPointF const & from, QPointF & to);

    /* add <delta> degree to rotation, adjust when total rotation is near 0/90/180/270 degree
     *   when adjusted, adjust delta is save in <delta>
     *   when sync is false, only update rotation_ and not triger signals
     */
    bool rotate(qreal& delta, bool sync = true);

    /*
     * set x/y sacles to <scale> absolutely
     */
    void scaleTo(qreal scale);

    /*
     * add <delta> to scale
     */
    void scale(QSizeF const & delta);

    /* Scale by border handles
     *   scale with one direction of 8 directions by delta
     *   current rotation should take account
     *   return result only apply with scale
     *   both origin and result are centered at [0, 0]
     */
    bool scale(QRectF & rect, QRectF const & direction, QPointF & delta,
               QRectF const & padding, bool KeepAspectRatio, bool layoutScale, qreal limitSize[2]);

    /* scale to <scaleTo>, and adjust translate to keep view center moves smallest
     */
    void scaleKeepToCenter(QRectF const & border, QRectF & self, qreal scaleTo);

    /* adjust tranform by two fingers guesture
     *   scale is averaged in x/y aixs
     */
    void gesture(QPointF const & from1, QPointF const & from2, QPointF & to1, QPointF & to2,
                 bool translate, bool scale, bool rotate, qreal limitScale[2], qreal * scaleOut = nullptr);

    /* take other tranform to modify at same time,
     *    when attached, modify to one transform will affect another one, and keep
     *    A*B' = A'*B, where A B is the transform states just before attaching
     */
    void attachTransform(ResourceTransform * other);

public:
    /* adjust scale and translate to keep edges out of border
     *   assume there is no rotation
     */
    void keepOuterOf(QRectF const & border, QRectF & self);

    /* adjust scale and translate to keep edges in border
     *   assume there is no rotation
     */
    void keepInnerOf(QRectF const & border, QRectF & self);

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
    void translate(QPointF const & delta, int otherChanges);

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
