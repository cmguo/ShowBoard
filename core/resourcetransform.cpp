#include "resourcetransform.h"

#include <QtMath>
#include <QDebug>

ResourceTransform::ResourceTransform(QObject * parent)
    : QObject(parent)
{
}

ResourceTransform::ResourceTransform(const ResourceTransform &o, QObject * parent)
    : QObject(parent)
    , scale_(o.scale_)
    , rotate_(o.rotate_)
    , translate_(o.translate_)
    , transform_(o.transform_)
    , scaleRotate_(o.scaleRotate_)
    , rotateTranslate_(o.rotateTranslate_)
{
}

ResourceTransform::ResourceTransform(const QTransform &o, QObject *parent)
    : QObject(parent)
    , transform_(o)
{
    if (o.type() <= QTransform::TxTranslate) {
        rotateTranslate_ = translate_ = o;
    } else if (o.type() < QTransform::TxRotate) {
        translate_.translate(o.dx(), o.dy());
        rotateTranslate_ = translate_;
        scale_ = scaleRotate_ = o * translate_.inverted();
    } else {
        scaleRotate_ = o * translate_.inverted();
        QPointF agl = scaleRotate_.map(QPointF(100, 0));
        rotate_ = QTransform().rotate(angle(agl));
        rotateTranslate_ = rotate_ * translate_;
        scale_ = scaleRotate_ * rotate_.inverted();
        QPointF sle = scale_.map(QPointF(1, 1));
        scale_ = QTransform::fromScale(sle.x(), sle.y());
        scaleRotate_ = scale_ * rotate_;
    }
}

ResourceTransform &ResourceTransform::operator=(const ResourceTransform &o)
{
    scale_ = o.scale_;
    rotate_ = o.rotate_;
    translate_ = o.translate_;
    scaleRotate_ = o.scaleRotate_;
    rotateTranslate_ = o.rotateTranslate_;
    transform_ = o.transform_;
    emit beforeChanged(7);
    emit changed(7);
    return *this;
}

qreal ResourceTransform::angle() const
{
    return angle(rotate_.map(QPointF(100, 0)));
}

void ResourceTransform::translateTo(const QPointF &offset)
{
    translate(offset - this->offset(), 0);
}

void ResourceTransform::translate(QPointF const & delta)
{
    translate(delta, 0);
}

void ResourceTransform::translate(QPointF & delta)
{
    QPointF o = offset();
    translate(delta, 0);
    delta = offset() - o;
}

void ResourceTransform::rotate(QPointF const & from, QPointF & to)
{
    QPointF center(translate_.dx(), translate_.dy());
    qreal a1 = angle(from - center);
    qreal a2 = angle(to - center);
    qreal da = a2 - a1;
    bool adjusted = rotate(da);
    if (adjusted) {
        to = center + QTransform().rotate(da).map(to - center);
    }
}

void ResourceTransform::rotate(const QPointF &center, const QPointF &from, QPointF &to)
{
    QPointF center2(translate_.dx(), translate_.dy());
    qreal a1 = angle(from - center);
    qreal a2 = angle(to - center);
    qreal da = a2 - a1;
    bool adjusted = rotate(da);
    if (adjusted) {
        to = center + QTransform().rotate(da).map(to - center);
        da = a2 - a1 + da;
    }
    QPointF t = center2 - center;
    t = QTransform().rotate(da).map(t);
    t = t + center - center2;
    translate(t);
}

bool ResourceTransform::rotate(qreal& delta, bool sync)
{
    rotate_.rotate(delta);
    bool adjust = true;
    if (rotate_.m11() > 0.999) { // ±2.56°
        delta = -asin(rotate_.m12());
        rotate_ = QTransform();
    } else if (qAbs(rotate_.m11()) < 0.0447) {
        if (rotate_.m12() > 0) {
            delta = asin(rotate_.m11());
            rotate_ = QTransform(0, 1, -1, 0, 0, 0); // right
        } else {
            delta = -asin(rotate_.m11());
            rotate_ = QTransform(0, -1, 1, 0, 0, 0);
        }
    } else if (rotate_.m11() < -0.999) {// down
        delta = asin(rotate_.m12());
        rotate_ = QTransform(-1, 0, 0, -1, 0, 0);
    } else {
        adjust = false;
    }
    if (adjust) {
        delta = delta * 180 / M_PI;
    }
    if (sync) {
        scaleRotate_ = scale_ * rotate_;
        rotateTranslate_ = rotate_ * translate_;
        transform_ = scaleRotate_ * translate_;
        emit beforeChanged(2);
        emit changed(2);
    }
    return adjust;
}

void ResourceTransform::scaleTo(qreal scale)
{
    this->scale(QSizeF(scale / scale_.m11(), scale / scale_.m22()));
}

void ResourceTransform::scale(QSizeF const & delta)
{
    scale_.scale(delta.width(), delta.height());
    scaleRotate_ = scale_ * rotate_;
    transform_ = scaleRotate_ * translate_;
    emit beforeChanged(4);
    emit changed(4);
}

static constexpr qreal SCALE_MIN_ADJUST = 1.000001;
static constexpr qreal SCALE_MAX_ADJUST = 0.999999;

bool ResourceTransform::scale(QRectF & rect, QPointF const & center, qreal & delta,
                              QRectF const & padding, bool layoutScale, QSizeF limitSize[2])
{
    QRectF origin = rect.adjusted(-padding.x(), -padding.y(), -padding.right(), -padding.bottom());
    QRectF result{origin.topLeft() * delta, origin.bottomRight() * delta};
    QPointF c1 = scaleRotate_.map(center);
    if (limitSize) {
        if (limitSize[0].width() > 0 && origin.width() >= limitSize[0].width() && result.width() < limitSize[0].width()) {
            qreal w = limitSize[0].width() * SCALE_MIN_ADJUST;
            result.setHeight(result.height() * w / result.width());
            result.setWidth(w);
            delta = w / origin.width();
        } else if (limitSize[1].width() > 0 && origin.width() <= limitSize[1].width() && result.width() > limitSize[1].width()) {
            qreal w = limitSize[1].width() * SCALE_MAX_ADJUST;
            result.setHeight(result.height() * w / result.width());
            result.setWidth(w);
            delta = w / origin.width();
        }
        if (limitSize[0].height() > 0 && origin.height() >= limitSize[0].height() && result.height() < limitSize[0].height()) {
            qreal h = limitSize[0].height() * SCALE_MIN_ADJUST;
            result.setWidth(result.width() * h / result.height());
            result.setHeight(h);
            delta = h / origin.height();
        } else if (limitSize[1].height() > 0 && origin.height() <= limitSize[1].height() && result.height() > limitSize[1].height()) {
            qreal h = limitSize[1].height() * SCALE_MAX_ADJUST;
            result.setWidth(result.width() * h / result.height());
            result.setHeight(h);
            delta = h / origin.height();
        }
    }
    if (layoutScale) {
        scaleRotate_ = rotate_;
        translate(c1 - rotate_.map(center * delta), 4);
    } else {
        scale_.scale(delta, delta);
        scaleRotate_ = scale_ * rotate_;
        translate(c1 - scaleRotate_.map(center), 4);
    }
    result.moveCenter({0, 0});
    result.adjust(padding.x(), padding.y(), padding.right(), padding.bottom());
    rect = result;
    return true;
}

bool ResourceTransform::scale(QRectF & rect, QRectF const & direction, QPointF & delta,
                              QRectF const & padding, bool KeepAspectRatio, bool layoutScale, QSizeF limitSize[2])
{
    bool byWidth = qFuzzyIsNull(direction.height());
    bool byHeight = qFuzzyIsNull(direction.width());
    QRectF origin = rect.adjusted(-padding.x(), -padding.y(), -padding.right(), -padding.bottom());
    // origin must be centered at [0, 0]
    // Q_ASSERT(qFuzzyIsNull(origin.center().x()) && qFuzzyIsNull(origin.center().y()));
    if (!rotate_.isIdentity())
        delta = rotate_.inverted().map(delta);
    QRectF result = origin.adjusted(
                delta.x() * direction.left(), delta.y() * direction.top(),
                delta.x() * direction.right(), delta.y() * direction.bottom());
    if ((byWidth && qFuzzyIsNull(result.width())) || (byHeight && qFuzzyIsNull(result.height()))) {
        delta = QPointF();
        return false;
    }
    if (KeepAspectRatio) {
        QSizeF s(result.width() / origin.width(), result.height() / origin.height());
        if (byHeight || (s.width() < s.height() && !byWidth)) {
            result.setWidth(origin.width() * s.height());
        } else {
            result.setHeight(origin.height() * s.width());
        }
        if (limitSize) {
            if (limitSize[0].width() > 0 && origin.width() >= limitSize[0].width() && result.width() < limitSize[0].width()) {
                qreal w = limitSize[0].width() * SCALE_MIN_ADJUST;
                result.setHeight(result.height() * w / result.width());
                result.setWidth(w);
            } else if (limitSize[1].width() > 0 && origin.width() <= limitSize[1].width() && result.width() > limitSize[1].width()) {
                qreal w = limitSize[1].width() * SCALE_MAX_ADJUST;
                result.setHeight(result.height() * w / result.width());
                result.setWidth(w);
            }
            if (limitSize[0].height() > 0 && origin.height() >= limitSize[0].height() && result.height() < limitSize[0].height()) {
                qreal h = limitSize[0].height() * SCALE_MIN_ADJUST;
                result.setWidth(result.width() * h / result.height());
                result.setHeight(h);
            } else if (limitSize[1].height() > 0 && origin.height() <= limitSize[1].height() && result.height() > limitSize[1].height()) {
                qreal h = limitSize[1].height() * SCALE_MAX_ADJUST;
                result.setWidth(result.width() * h / result.height());
                result.setHeight(h);
            }
        }
    } else {
        if (limitSize) {
            if (limitSize[0].width() > 0 && origin.width() >= limitSize[0].width() && result.width() < limitSize[0].width()) {
                result.setWidth(limitSize[0].width() * SCALE_MIN_ADJUST);
                KeepAspectRatio = true;
            } else if (limitSize[1].width() > 0 && origin.width() <= limitSize[1].width() && result.width() > limitSize[1].width()) {
                result.setWidth(limitSize[1].width() * SCALE_MAX_ADJUST);
                KeepAspectRatio = true;
            }
            if (limitSize[0].height() > 0 && origin.height() >= limitSize[0].height() && result.height() < limitSize[0].height()) {
                result.setHeight(limitSize[0].height() * SCALE_MIN_ADJUST);
                KeepAspectRatio = true;
            } else if (limitSize[1].height() > 0 && origin.height() <= limitSize[1].height() && result.height() > limitSize[1].height()) {
                result.setHeight(limitSize[1].height() * SCALE_MAX_ADJUST);
                KeepAspectRatio = true;
            }
        }
        byWidth = byHeight = false;
    }
    if (KeepAspectRatio) {
        delta = QPointF((result.width() - origin.width()) * direction.width(),
                    (result.height() - origin.height()) * direction.height());
        if (byHeight) {
            result = QRectF(QPointF(result.left(), origin.top() + delta.y() * direction.top()),
                            QSizeF(result.width(), origin.height() + delta.y() * direction.height()));
        } else if (byWidth) {
            result = QRectF(QPointF(origin.left() + delta.x() * direction.left(), result.top()),
                            QSizeF(origin.width() + delta.x() * direction.width(), result.height()));
        } else {
            result = origin.adjusted(
                            delta.x() * direction.left(), delta.y() * direction.top(),
                            delta.x() * direction.right(), delta.y() * direction.bottom());

        }
    }
    if (layoutScale) {
        scaleRotate_ = rotate_;
    } else {
        scale_.scale(result.width() / origin.width(), result.height() / origin.height());
        scaleRotate_ = scale_ * rotate_;
    }
    QPointF d = result.center() - origin.center();
    if (!rotate_.isIdentity()) {
        delta = rotate_.map(delta);
        d = rotate_.map(d);
    }
    translate(d, 4);
    result.moveCenter({0, 0});
    result.adjust(padding.x(), padding.y(), padding.right(), padding.bottom());
    rect = result;
    return true;
}

void ResourceTransform::scaleKeepToCenter(const QRectF &border, QRectF &self, qreal scaleTo)
{
    QPointF center = transform_.inverted().map(border.center());
    scale_.scale(scaleTo / scale_.m11(), scaleTo / scale_.m22());
    scaleRotate_ = scale_ * rotate_;
    transform_ = scale_ * rotateTranslate_;
    QRectF rect = transform_.map(self).boundingRect();
    center = rect.center() + border.center() - transform_.map(center);
    rect.moveCenter(center);
    if (rect.width() < border.width())
        center.setX(border.center().x());
    else if (rect.left() > border.left())
        center.setX(center.x() + border.left() - rect.left());
    else if (rect.right() < border.right())
        center.setX(center.x() + border.right() - rect.right());
    if (rect.height() < border.height())
        center.setY(border.center().y());
    else if (rect.top() > border.top())
        center.setY(center.y() + border.top() - rect.top());
    else if (rect.bottom() < border.bottom())
        center.setY(center.y() + border.bottom() - rect.bottom());
    rect.moveCenter(center);
    self = rect;
    translate(rect.topLeft() - offset(), 4);
}

void ResourceTransform::gesture(const QPointF &from1, const QPointF &from2, QPointF &to1, QPointF &to2,
                                bool translate, bool scale, bool rotate, qreal limitScale[2], qreal * scaleOut)
{
    // line1: from1 -- from2
    // line2: to1 -- to2
    // split into scale, rotate and translate
    //qDebug() << "from1" << from1 << "from2" << from2 << "to1" << to1 << "to2" << to2;
    qreal s = scale ? length(to2 - to1) / length(from2 - from1) : 1;
    qreal r = rotate ? angle(to2 - to1) - angle(from2 - from1) : 0;
    QPointF t0 = QPointF(translate_.dx(), translate_.dy());
    QPointF t = from2 - t0;
    if (scale) {
        qreal s0 = s;
        if (limitScale[0] > 0 && s * scale_.m11() < limitScale[0])
            s = limitScale[0] / scale_.m11();
        else if (limitScale[1] > 0 && s * scale_.m11() > limitScale[1])
            s = limitScale[1] / scale_.m11();
        else
            limitScale = nullptr;
        if (limitScale) {
            to2 = (to2 - to1) * s / s0 + to1;
        }
        if (scaleOut) {
            *scaleOut = s;
            scale = false;
        } else {
            scale_.scale(s, s);
        }
        if (translate)
            t *= s;
    }
    if (rotate) {
        qreal r1 = r;
        bool adjusted = this->rotate(r1, false);
        if (adjusted) {
            r += r1;
            QTransform tr; tr.rotate(r1);
            to2 = to1 + tr.map(to2 - to1);
        }
        if (translate)
            t = QTransform().rotate(r).map(t);
    }
    if (translate) {
        t = to2 - t0 - t;
        translate_.translate(t.x(), t.y());
    }
    //qDebug() << "scale" << s << "rotate" << r << "translate" << t;
    scaleRotate_ = scale_ * rotate_;
    rotateTranslate_ = rotate_ * translate_;
    transform_ = scaleRotate_ * translate_;
    int elements = (scale ? 4 : 0) + (rotate ? 2 : 0) + (translate ? 1 : 0);
    s = scale_.m11();
    t = offset();
    emit beforeChanged(elements);
    if (!qFuzzyIsNull(s / scale_.m11() - 1.0))
        to2 = to1 + (to2 - to1) * scale_.m11() / s;
    if (t != offset()) {
        to1 += offset() - t;
        to2 += offset() - t;
    }
    emit changed(elements);
}

void ResourceTransform::keepOuterOf(QRectF const &border, QRectF &self)
{
    QRectF crect = transform_.map(self).boundingRect();
    //qDebug() << "before" << border << crect << transform_;
    if (border.width() > crect.width() || border.height() > crect.height()) {
        qreal s = qMax(border.width() / crect.width(), border.height() / crect.height());
        scale_.scale(s, s);
        scaleRotate_ = scale_ * rotate_;
        transform_ = scaleRotate_ * translate_;
        crect = transform_.map(self).boundingRect();
    }
    QPointF d;
    if (crect.left() > border.left())
        d.setX(border.left() - crect.left());
    else if (crect.right() < border.right())
        d.setX(border.right() - crect.right());
    if (crect.top() > border.top())
        d.setY(border.top() - crect.top());
    else if (crect.bottom() < border.bottom())
        d.setY(border.bottom() - crect.bottom());
    if (!d.isNull()) {
        crect.translate(d.x(), d.y());
        translate_.translate(d.x(), d.y());
        transform_ = scaleRotate_ * translate_;
        if (sender()) {
            qDebug() << "warning!!!!!!!!!!!" << d;
        }
    }
    //qDebug() << "after" << border << crect << transform_;
    self = crect;
}

void ResourceTransform::keepAtOrigin()
{
    translate_.reset();
}

void ResourceTransform::translate(const QPointF &delta, int otherChanges)
{
    translate_.translate(delta.x(), delta.y());
    rotateTranslate_ = rotate_ * translate_;
    transform_ = scaleRotate_ * translate_;
    emit beforeChanged(otherChanges | 1);
    emit changed(otherChanges | 1);
}

void ResourceTransform::attachTransform(ResourceTransform *other)
{
    qDebug() << "attachTransform" << other->transform() << transform_;
    QTransform t = other->transform();
    QTransform it = t.inverted();
    QTransform s = transform_;
    QTransform is = s.inverted();
    QObject::connect(other, &ResourceTransform::changed, this, [=]() {
        if (other->sender())
            return;
        *this = ResourceTransform(it * other->transform() * s);
    });
    QObject::connect(this, &ResourceTransform::changed, other, [=]() {
        if (this->sender())
            return;
        *other = ResourceTransform(t * this->transform_ * is);
    });
}

qreal ResourceTransform::angle(QPointF const & vec)
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

qreal ResourceTransform::length(QPointF const & vec)
{
    return sqrt(QPointF::dotProduct(vec, vec));
}
