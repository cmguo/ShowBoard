#ifndef RESOURCETRANSFORM_H
#define RESOURCETRANSFORM_H

#include "ShowBoard_global.h"

#include <QTransform>

// transform orders:
//   scale
//   rotate
//   translate

class GestureContext;

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

    qreal zoom() const;

    QSizeF zoom2d() const;

    qreal angle() const;

    QPointF offset() const
    {
        return QPointF(translate_.dx(), translate_.dy());
    }

public:
    void translateTo(QPointF const & delta);

    void translate(QPointF const & delta);

    void translate(QPointF & delta);

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

    /*
     * add <delta> to scale, not moving center
     */
    bool scale(QRectF & rect, QPointF const & center, qreal & delta,
               QRectF const & padding, bool layoutScale, QSizeF limitSize[2]);

    /* Scale by border handles
     *   scale with one direction of 8 directions by delta
     *   current rotation should take account
     *   return result only apply with scale
     *   both origin and result are centered at [0, 0]
     */
    bool scale(QRectF & rect, QRectF const & direction, QPointF & delta,
               QRectF const & padding, bool KeepAspectRatio, bool layoutScale, QSizeF limitSize[2]);

    /* scale to <scaleTo>, and adjust translate to keep view center moves smallest
     */
    void scaleKeepToCenter(QRectF const & border, QRectF & self, qreal scaleTo);

    /* adjust tranform by two fingers guesture
     *   scale is averaged in x/y aixs
     */
    void gesture(GestureContext * context, QPointF const & to1, QPointF const & to2);

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

    void keepAtOrigin();

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
    void translate(QPointF & delta, int otherChanges);

    friend class GestureContext;

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

class GestureContext
{
public:
    GestureContext();

public:
    void start(QPointF const & start1, QPointF const & start2);

    void pause();

    bool started() { return started_; }

public:
    bool configged() const { return configged_; }

    void config(bool scale, bool rotate, bool translate, bool layoutScale);

    void limitScales(qreal sw, qreal sh);

    bool layoutScale() { return canScale_ && layoutScale_; }

    void adjustOffsetAfterCommit(QPointF const & offset);

    void commit(QPointF const & to1, QPointF const & to2, QPointF const & off);

public:
    qreal scale() const { return scale_; }

    qreal rotate() const { return rotate_; }

    QPointF translate() { return translate_; }

    QPointF from1() const { return from1_; }

    QPointF from2() const { return from2_; }

    QPointF to1() const { return to1_; }

    QPointF to2() const { return to2_; }

private:
    friend class ResourceTransform;

    void push(QPointF const & to1, QPointF const & to2, QPointF const & off);

    bool adjustScale(QSizeF const & zoom2);

    void adjustRotate(bool adjust, qreal r1);

    void adjustTranslate();

    void adjustZoom(qreal zoom);

    void adjustOffset(QPointF const & offset);

    void commit(QPointF const & to1, QPointF const & to2);

private:
    bool configged_ = false;
    bool canTranslate_;
    bool canScale_;
    bool canRotate_;
    bool layoutScale_;
    qreal limitScales_[2];

private:
    bool started_ = false;
    QPointF from1_;
    QPointF from2_;
    qreal len_;
    qreal agl_;

    QPointF to1_;
    QPointF to2_;

    qreal scale_;
    qreal rotate_;
    QPointF translate_;

    QPointF st2_;
    QPointF st_;

private:
    QPointF tc_;
    QPointF t0_;
    //QPointF t1_;
    QPointF t2_;
};

#endif // RESOURCETRANSFORM_H
