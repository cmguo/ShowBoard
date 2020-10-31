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
    int change = 7;
    if (zoom2d() != o.zoom2d()) {
        scale_ = o.scale_;
        change |= 4;
    }
    if (rotate_ != o.rotate_) {
        rotate_ = o.rotate_;
        change |= 2;
    }
    if (offset() != o.offset()) {
        translate_ = o.translate_;
        change |= 1;
    }
    if (change & 6)
        scaleRotate_ = o.scaleRotate_;
    if (change & 3)
        rotateTranslate_ = o.rotateTranslate_;
    if (change) {
        transform_ = o.transform_;
        emit beforeChanged(change);
        emit changed(change);
    }
    return *this;
}

qreal ResourceTransform::zoom() const
{
    return scale_.m11();
}

QSizeF ResourceTransform::zoom2d() const
{
    return {scale_.m11(), scale_.m22()};
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
    QRectF rect = transform_.mapRect(self);
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

void ResourceTransform::gesture(GestureContext *context, QPointF const &to1, QPointF const &to2)
{
    // line1: from1 -- from2
    // line2: to1 -- to2
    // split into scale, rotate and translate
    //qDebug() << "ResourceTransform::gesture: fr1" << from1 << "fr2" << from2;
    //qDebug() << "ResourceTransform::gesture: to1" << to1 << "to2" << to2;
    context->push(to1, to2, offset());
    bool scale = context->adjustScale(zoom2d());
    if (scale)
        scale_.scale(context->scale(), context->scale());
    if (context->canRotate_) {
        qreal r1 = context->rotate();
        bool adjusted = this->rotate(r1, false);
        context->adjustRotate(adjusted, r1);
    } else {
        context->adjustRotate(true, -context->rotate());
    }
    if (context->canTranslate_) {
        context->adjustTranslate();
        QPointF t = context->translate();
        //qDebug() << "ResourceTransform::gesture: translate" << t;
        translate_.translate(t.x(), t.y());
    }
    //qDebug() << "ResourceTransform::gesture: scale" << s << "rotate" << r << "translate" << t2;
    //qDebug() << "ResourceTransform::gesture: fr1" << from1 << "fr2" << from2;
    //qDebug() << "ResourceTransform::gesture: to1" << to1 << "to2" << to2;
    scaleRotate_ = scale_ * rotate_;
    rotateTranslate_ = rotate_ * translate_;
    transform_ = scaleRotate_ * translate_;
    int elements = (scale ? 4 : 0) + (context->canRotate_ ? 2 : 0) + (context->canTranslate_ ? 1 : 0);
    qreal z = zoom();
    QPointF o = offset();
    emit beforeChanged(elements);
    if (!qFuzzyIsNull(z / zoom() - 1.0)) {
        context->adjustZoom(zoom() / z);
        o = o * zoom() / z;
    }
    //qDebug() << "ResourceTransform::gesture: zoom" << z << zoom();
    //qDebug() << "ResourceTransform::gesture: offset" << o << offset();
    if (o != offset())
        context->adjustOffset(offset() - o);
    context->commit(to1, to2);
    //qDebug() << "ResourceTransform::gesture: fr1" << from1 << "fr2" << from2;
    //qDebug() << "ResourceTransform::gesture: to1" << to1 << "to2" << to2;
    emit changed(elements);
}

void ResourceTransform::keepOuterOf(QRectF const &border, QRectF &self)
{
    QRectF crect = transform_.mapRect(self);
    //qDebug() << "before" << border << crect << transform_;
    if (border.width() > crect.width() || border.height() > crect.height()) {
        qreal s = qMax(border.width() / crect.width(), border.height() / crect.height());
        scale_.scale(s, s);
        scaleRotate_ = scale_ * rotate_;
        transform_ = scaleRotate_ * translate_;
        crect = transform_.mapRect(self);
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

GestureContext::GestureContext()
{
}

void GestureContext::start(const QPointF &start1, const QPointF &start2)
{
    started_ = true;
    from1_ = start1;
    from2_ = start2;
    len_ = ResourceTransform::length(start2 - start1);
    agl_ = ResourceTransform::angle(start2 - start1);
    st_ = st2_ = QPointF();
}

void GestureContext::pause()
{
    started_ = false;
}

void GestureContext::config(bool scale, bool rotate, bool translate, bool layoutScale)
{
    configged_ = true;
    canScale_ = scale;
    canRotate_ = rotate;
    canTranslate_ = translate;
    layoutScale_ = layoutScale;
}

void GestureContext::limitScales(qreal sw, qreal sh)
{
    limitScales_[0] = sw;
    limitScales_[1] = sh;
}

void GestureContext::adjustOffsetAfterCommit(const QPointF &offset)
{
    from1_ += offset;
    from2_ += offset;
}

void GestureContext::commit(const QPointF &to1, const QPointF &to2, const QPointF &off)
{
    push(to1, to2, off);
    if (canRotate_)
        t2_ = QTransform().rotate(rotate_).map(t2_);
    commit(to1, to2);
}

void GestureContext::push(const QPointF &to1, const QPointF &to2, QPointF const & off)
{
    //qDebug() << "GestureContext::push: to1" << to1 << "to2" << to2 << "off" << off;
    to1_ = to1;
    to2_ = to2;
    qreal l = ResourceTransform::length(to2 - to1);
    qreal a = ResourceTransform::angle(to2 - to1);
    scale_ = l / len_;
    rotate_ = a - agl_;
    tc_ = (to2 + to1) / 2;
    if (canTranslate_) {
        t0_ = off;
        //t1 = from1_ - t0;
        t2_ = (from2_ - t0_) * scale_;
    }
}

bool GestureContext::adjustScale(QSizeF const & zoom2)
{
    qreal s0 = scale_;
    if (!canScale_) {
        adjustZoom(1 / s0);
        return false;
    }
    bool a = false;
    if (limitScales_[0] > 0 && scale_ * zoom2.width() < limitScales_[0]) {
        scale_ = limitScales_[0] / zoom2.width();
        a = true;
    }
    if (limitScales_[1] > 0 && scale_ * zoom2.height() > limitScales_[1]) {
        scale_ = limitScales_[1] / zoom2.height();
        a = true;
    }
    if (a)
        adjustZoom(scale_ / s0);
    return !layoutScale_;
}

void GestureContext::adjustRotate(bool adjusted, qreal r1)
{
    if (adjusted) {
        rotate_ += r1;
        QTransform tr; tr.rotate(r1);
        //qDebug() << "GestureContext::adjustRotate: to1" << to1_ << "to2" << to2_;
        to1_ = tc_ + tr.map(to1_ - tc_);
        to2_ = tc_ + tr.map(to2_ - tc_);
        //qDebug() << "GestureContext::adjustRotate: to1" << to1_ << "to2" << to2_;
    }
    if (canRotate_ && canTranslate_) {
        //qDebug() << "GestureContext::adjustRotate: rotate" << rotate_ << "t2" << t2_;
        //t1_ = QTransform().rotate(rotate_).map(t1_);
        t2_ = QTransform().rotate(rotate_).map(t2_);
    }
}

void GestureContext::adjustTranslate()
{
    translate_ = to2_ - t0_ - t2_;
}

void GestureContext::adjustZoom(qreal zoom)
{
    //qDebug() << "GestureContext::adjustZoom: zoom" << zoom << "to1" << to1_ << "to2" << to2_;
    to1_ = tc_ + (to1_ - tc_) * zoom;
    to2_ = tc_ + (to2_ - tc_) * zoom;
    //qDebug() << "GestureContext::adjustZoom: to1" << to1_ << "to2" << to2_;
    if (canTranslate_) {
        t2_ *= zoom;
        translate_ *= zoom;
    }
}

void GestureContext::adjustOffset(const QPointF &offset)
{
    to1_ += offset;
    to2_ += offset;
    translate_ += offset;
    st_ += offset;
}

void GestureContext::commit(QPointF const & to1, QPointF const & to2)
{
    if (canScale_ || canRotate_) {
        st2_ += from2_ - t0_ - t2_;
        st_ += to2_ - t0_ - t2_;
        //qDebug() << "GestureContext::commit" << st2_ << st_;
        qreal l = ResourceTransform::length(st_);
        qreal l2 = ResourceTransform::length(st2_);
        if (l > 9 && l > l2 * 4) {
            //qDebug() << "GestureContext::commit disable scale & rotate";
            canScale_ = canRotate_ = false;
        } else if (l2 > 9) {
            st_ = st2_ = QPointF();
        }
    }
    //qDebug() << "GestureContext::commit " << from1_ << to1_;
    //qDebug() << "GestureContext::commit " << from2_ << to2_;
    from1_ = to1_;
    from2_ = to2_;
    len_ = ResourceTransform::length(to2_ - to1_);
    agl_ = ResourceTransform::angle(to2_ - to1_);
    to1_ = to1;
    to2_ = to2;
}

