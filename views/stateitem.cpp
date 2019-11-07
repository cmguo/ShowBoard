#include "stateitem.h"
#include "core/svgcache.h"
#include "core/transformhelper.h"

#include <QSvgRenderer>
#include <QGraphicsSceneMouseEvent>

SvgCache * StateItem::cache_ = nullptr;
QSvgRenderer * StateItem::loading_ = nullptr;
QSvgRenderer * StateItem::failed_ = nullptr;

StateItem::StateItem(QGraphicsItem * parent)
    : QGraphicsSvgItem(parent)
    , normal_(nullptr)
    , hover_(nullptr)
    , pressed_(nullptr)
    , timerId_(0)
{
    setAcceptedMouseButtons(Qt::LeftButton);
    if (!cache_) {
        cache_ = SvgCache::instance();
        loading_ = cache_->get(QString(":/showboard/icons/loading.svg"));
        failed_ = cache_->get(QString(":/showboard/icons/stop.normal.svg"));
    }
}

void StateItem::setLoading()
{
    setSharedRenderer(loading_);
    rotate_ = 45.0;
    timerId_ = startTimer(100);
}

void StateItem::setLoaded(const QString &icon)
{
    QString fileNormal(icon);
    fileNormal.replace(".svg", ".normal.svg");
    QString fileHover(icon);
    fileHover.replace(".svg", ".hover.svg");
    QString filePressed(icon);
    filePressed.replace(".svg", ".press.svg");
    normal_ = cache_->get(fileNormal);
    hover_ = cache_->get(fileHover);
    pressed_ = cache_->get(filePressed);
    if (normal_)
        setSharedRenderer(normal_);
}

void StateItem::setFailed(QString const & msg)
{
    setSharedRenderer(failed_);
}

void StateItem::setSharedRenderer(QSvgRenderer * renderer)
{
    killTimer(timerId_);
    timerId_ = 0;
    setRotation(0);
    QGraphicsSvgItem::setSharedRenderer(renderer);
    setTransformOriginPoint(boundingRect().center());
    updateTransform();
}

void StateItem::updateTransform()
{
    QTransform t;
    TransformHelper::keepAtParent(t, this, {0, 0});
    QPointF center(boundingRect().center());
    setTransform(QTransform::fromTranslate(-center.x(), -center.y()) * t);
}

void StateItem::timerEvent(QTimerEvent * event)
{
    (void) event;
    setRotation(rotation() + rotate_);
}

void StateItem::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
    (void) event;
    if (pressed_)
        QGraphicsSvgItem::setSharedRenderer(pressed_);
    else
        event->ignore();
}

void StateItem::mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
{
    (void) event;
    if (normal_)
        QGraphicsSvgItem::setSharedRenderer(normal_);
    else
        event->ignore();
    emit clicked();
}

