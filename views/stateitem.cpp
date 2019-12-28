#include "stateitem.h"
#include "core/svgcache.h"

#include <QSvgRenderer>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSvgItem>
#include <QGraphicsTextItem>
#include <QPainter>
#include <QDebug>

SvgCache * StateItem::cache_ = nullptr;
QSvgRenderer * StateItem::loading_ = nullptr;
QSvgRenderer * StateItem::failed_ = nullptr;

StateItem::StateItem(QGraphicsItem * parent)
    : QGraphicsObject(parent)
    , iconItem_(nullptr)
    , textItem_(nullptr)
    , normal_(nullptr)
    , hover_(nullptr)
    , pressed_(nullptr)
    , timerId_(0)
{
    setAcceptedMouseButtons(Qt::LeftButton);
    iconItem_ = new QGraphicsSvgItem(this);
    QGraphicsTextItem* textItem = new QGraphicsTextItem(this);
    textItem->setFont(QFont("Microsoft YaHei", 18));
    textItem_ = textItem;
    if (!cache_) {
        cache_ = SvgCache::instance();
    }
    loading_ = cache_->get(QString(":/showboard/icons/loading.svg"));
    failed_ = cache_->get(QString(":/showboard/icons/stop.normal.svg"));
    updateTransform();
}

void StateItem::setLoading(QString const & title)
{
    setSharedRenderer(loading_);
    setText(title);
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
    setText(nullptr);
}

void StateItem::setFailed(QString const & msg)
{
    setSharedRenderer(failed_);
    setText(msg);
}

void StateItem::setSharedRenderer(QSvgRenderer * renderer)
{
    killTimer(timerId_);
    timerId_ = 0;
    static_cast<QGraphicsSvgItem*>(iconItem_)->setSharedRenderer(renderer);
    QPointF center(iconItem_->boundingRect().center());
    iconItem_->setRotation(0);
    iconItem_->setTransformOriginPoint(center);
    iconItem_->setPos(-center);
}

void StateItem::setText(const QString &text)
{
    if (text.startsWith("<") && text.endsWith(">"))
        static_cast<QGraphicsTextItem*>(textItem_)->setHtml(text);
    else
        static_cast<QGraphicsTextItem*>(textItem_)->setPlainText(text);
    QPointF center(textItem_->boundingRect().center());
    center.setY(-iconItem_->boundingRect().height() / 2 - 20 + center.y());
    textItem_->setPos(-center);
}

void StateItem::updateTransform()
{
    QPointF center(parentItem()->boundingRect().center());
    setTransform(QTransform::fromTranslate(center.x(), center.y()));
}

QRectF StateItem::boundingRect() const
{
    return iconItem_->mapToParent(iconItem_->boundingRect()).boundingRect()
            | textItem_->mapToParent(textItem_->boundingRect()).boundingRect();
}

QPainterPath StateItem::shape() const
{
    return iconItem_->mapToParent(iconItem_->shape()) | textItem_->mapToParent(textItem_->shape());
}

void StateItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    (void) painter;
    (void) option;
    (void) widget;
}

void StateItem::timerEvent(QTimerEvent * event)
{
    (void) event;
    //qDebug() << "timerEvent";
    iconItem_->setRotation(iconItem_->rotation() + rotate_);
}

void StateItem::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
    (void) event;
    if (pressed_)
        static_cast<QGraphicsSvgItem*>(iconItem_)->setSharedRenderer(pressed_);
}

void StateItem::mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
{
    (void) event;
    if (normal_)
        static_cast<QGraphicsSvgItem*>(iconItem_)->setSharedRenderer(normal_);
    emit clicked();
}

