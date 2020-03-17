#include "stateitem.h"
#include "core/svgcache.h"

#include <QSvgRenderer>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSvgItem>
#include <QGraphicsTextItem>
#include <QPainter>
#include <QDebug>
#include <QCloseEvent>
#include <QCursor>

SvgCache * StateItem::cache_ = nullptr;
QSvgRenderer * StateItem::loading_ = nullptr;
QSvgRenderer * StateItem::failed_ = nullptr;

static void truncateText(QString & text, QFont font, int maxWidth);

static constexpr int MAX_TEXT_WIDTH = 300;

StateItem::StateItem(QString const & title, QGraphicsItem * parent)
    : QGraphicsObject(parent)
    , iconItem_(nullptr)
    , textItem_(nullptr)
    , normal_(nullptr)
    , hover_(nullptr)
    , pressed_(nullptr)
    , state_(None)
    , title_(title)
    , showBackground_(true)
    , timerId_(0)
    , touchId_(0)
{
    setAcceptedMouseButtons(Qt::LeftButton);
    setAcceptTouchEvents(true);
    iconItem_ = new QGraphicsSvgItem(this);
    QGraphicsTextItem* textItem = new QGraphicsTextItem(this);
    textItem->setFont(QFont("Microsoft YaHei", 16));
    textItem->setDefaultTextColor(Qt::white);
    textItem_ = textItem;
    truncateText(title_, textItem->font(), MAX_TEXT_WIDTH);
    if (!cache_) {
        cache_ = SvgCache::instance();
    }
    loading_ = cache_->get(QString(":/showboard/icon/loading.svg"));
    failed_ = cache_->get(QString(":/showboard/icon/error.unknown.svg"));
    setCursor(Qt::SizeAllCursor);
    updateTransform();
}

void StateItem::showBackground(bool show)
{
    if (showBackground_ == show)
        return;
    showBackground_ = show;
    static_cast<QGraphicsTextItem*>(textItem_)->setDefaultTextColor(showBackground_ ? Qt::white : Qt::black);
    update();
}

void StateItem::setLoading()
{
    if (state_ != Loading)
        setLoading("正在打开");
}

void StateItem::setLoading(const QString &msg)
{
    if (state_ != Loading)
        setSharedRenderer(loading_);
    QString text =  "<center><nobr>" + msg + "...</nobr><br/>"
                    "<font style='color:#98FFFFFF;font-size:14pt;'>" + title_ + "</font></center>";
    setText(text);
    if (state_ != Loading) {
        state_ = Loading;
        rotate_ = 45.0;
        timerId_ = startTimer(100);
    }
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
    state_ = Loaded;
    if (normal_)
        setSharedRenderer(normal_);
    setText(nullptr);
}

void StateItem::setFailed(QString const & error)
{
    QByteArray type = "unknown";
    QString errmsg = error;
    int n = error.indexOf("|");
    if (n > 0) {
        type = error.left(n).toUtf8();
        errmsg = error.mid(n + 1);
    }
    QString text = "<center><font style='color:#98FFFFFF;font-size:14pt;'>" + title_ + "</font>"
            "<br/><nobr>" + errmsg + "</nobr></center>";
    state_ = Failed;
    QSvgRenderer * svg = cache_->get(QString(":/showboard/icon/error." + type + ".svg"));
    if (svg == nullptr)
        svg = failed_;
    setSharedRenderer(svg);
    setText(text);
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
    prepareGeometryChange();
}

void StateItem::setText(const QString &text)
{
    QGraphicsTextItem* textItem = static_cast<QGraphicsTextItem*>(textItem_);
    textItem->setTextWidth(MAX_TEXT_WIDTH);
    if (text.startsWith("<") && text.endsWith(">"))
        textItem->setHtml(text);
    else
        textItem->setPlainText(text);
    textItem->adjustSize();
    QPointF center(textItem->boundingRect().center());
    center.setY(-iconItem_->boundingRect().height() / 2 - 10);
    textItem->setPos(-center);
    textItem->setVisible(!text.isEmpty());
    prepareGeometryChange();
}

void StateItem::updateTransform()
{
    QPointF center(parentItem()->boundingRect().center());
    setTransform(QTransform::fromTranslate(center.x(), center.y()));
}

QRectF StateItem::boundingRect() const
{
    QRectF rect = iconItem_->boundingRect();
    rect.moveCenter(QPointF(0, 0)); // not map to this, ignore rotate
    if (textItem_->isVisible()) {
        rect |= textItem_->mapToParent(textItem_->boundingRect()).boundingRect();
    }
    rect.adjust(-32, -12, 32, 32);
    return rect;
}

bool StateItem::hitTest(QGraphicsItem * child, const QPointF &)
{
    return child == this || receivers(SIGNAL(clicked())) == 0;
}

void StateItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    (void) option;
    (void) widget;
    if (!showBackground_)
        return;
    if (state_ == Loaded)
        return;
    painter->save();
    painter->setPen(QColor("#FF434D59"));
    painter->setBrush(QColor("#F22B3034"));
    painter->drawRoundedRect(boundingRect(), 8, 8);
    painter->restore();
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
    if (receivers(SIGNAL(clicked())) == 0) {
        event->ignore();
        return;
    }
    if (touchId_)
        return;
    if (pressed_)
        static_cast<QGraphicsSvgItem*>(iconItem_)->setSharedRenderer(pressed_);
}

void StateItem::mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
{
    (void) event;
    if (touchId_)
        return;
    if (normal_)
        static_cast<QGraphicsSvgItem*>(iconItem_)->setSharedRenderer(normal_);
    emit clicked();
}

bool StateItem::sceneEvent(QEvent *event)
{
    switch (event->type()) {
    case QEvent::TouchBegin:
        if (receivers(SIGNAL(clicked())) == 0) {
            event->ignore();
            return false;
        }
        touchId_ = static_cast<QTouchEvent*>(event)->touchPoints().first().id();
        if (pressed_)
            static_cast<QGraphicsSvgItem*>(iconItem_)->setSharedRenderer(pressed_);
        return true;
    case QEvent::TouchEnd:
        if (normal_)
            static_cast<QGraphicsSvgItem*>(iconItem_)->setSharedRenderer(normal_);
        touchId_ = 0;
        emit clicked();
        return true;
    default:
        break;
    }
    return QGraphicsObject::sceneEvent(event);
}

static void truncateText(QString & text, QFont font, int maxWidth)
{
    QFontMetrics fm(font);
    text = fm.elidedText(text, Qt::TextElideMode::ElideMiddle, maxWidth);
}
