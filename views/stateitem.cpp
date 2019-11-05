#include "stateitem.h"
#include "core/svgcache.h"

#include <QSvgRenderer>

StateItem::StateItem(QGraphicsItem * parent)
    : QGraphicsSvgItem(parent)
    , cache_(SvgCache::instance())
    , normal_(nullptr)
    , pressed_(nullptr)
    , timerId_(0)
{
    setAcceptedMouseButtons(Qt::LeftButton);
}

void StateItem::setSvgFile(QString const & file, qreal rotate)
{
    setSharedRenderer(cache_->get(file));
    setRotation(0);
    rotate_ = rotate;
    normal_ = pressed_ = nullptr;
    killTimer(timerId_);
    timerId_ = startTimer(100);
}

void StateItem::setSvgFiles(QString const & fileNormal, QString const & filePressed)
{
    normal_ = cache_->get(fileNormal);
    pressed_ = cache_->get(filePressed);
    killTimer(timerId_);
    timerId_ = 0;
    setSharedRenderer(normal_);
    setRotation(0);
}

void StateItem::setSharedRenderer(QSvgRenderer * renderer)
{
    QGraphicsSvgItem::setSharedRenderer(renderer);
    setPos(parentItem()->boundingRect().center() - boundingRect().center());
    setTransformOriginPoint(boundingRect().center());
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
        setSharedRenderer(pressed_);
}

void StateItem::mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
{
    (void) event;
    if (normal_)
        setSharedRenderer(normal_);
    emit clicked();
}

