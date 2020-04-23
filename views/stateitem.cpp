#include "qsshelper.h"
#include "stateitem.h"
#include "core/svgcache.h"
#include "core/control.h"
#include "core/resourceview.h"
#include "core/resource.h"

#include <QSvgRenderer>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSvgItem>
#include <QGraphicsTextItem>
#include <QPainter>
#include <QDebug>
#include <QCloseEvent>
#include <QCursor>
#include <QMovie>

SvgCache * StateItem::cache_ = nullptr;
QMovie * StateItem::loading2_ = nullptr;
QSvgRenderer * StateItem::loading_ = nullptr;
QSvgRenderer * StateItem::failed_ = nullptr;

static void truncateText(QString & text, QFont font, int maxWidth);

static constexpr int MAX_TEXT_WIDTH = 250;
static constexpr int ID_PADDING = 1000;
static constexpr int ID_MOVIE = 1001;


StateItem::StateItem(QGraphicsItem * parent)
    : QGraphicsObject(parent)
    , iconItem_(nullptr)
    , textItem_(nullptr)
    , btnItem_(nullptr)
    , normal_(nullptr)
    , hover_(nullptr)
    , pressed_(nullptr)
    , state_(None)
    , showBackground_(true)
    , timerId_(0)
    , animate_(0)
    , touchId_(0)
{
    if (!cache_) {
        cache_ = SvgCache::instance();
        loading2_ = cache_->getMovie(QString(":/showboard/icon/loading2.svg"));
        loading_ = cache_->get(QString(":/showboard/icon/loading.svg"));
        failed_ = cache_->get(QString(":/showboard/icon/error.unknown.svg"));
    }

    setAcceptedMouseButtons(Qt::LeftButton);
    setAcceptTouchEvents(true);
    setCursor(Qt::SizeAllCursor);

    Control * control = Control::fromItem(parent);
    bool independent = control->resource()->flags().testFlag(ResourceView::Independent);

    decideStyles(independent, control->resource()->resource()->type().endsWith("tool"));

    title_ = control->resource()->name();
    iconItem_ = createIconItem(this, independent);
    textItem_ = createTextItem(this, independent);
    truncateText(title_, static_cast<QGraphicsTextItem*>(textItem_)->font(), MAX_TEXT_WIDTH);
    btnItem_ = createButtonItem(this, independent);

    updateTransform();
}

void StateItem::setLoading()
{
    if (state_ != Loading)
        setLoading("正在打开");
}

void StateItem::setLoading(const QString &msg)
{
    if (state_ != Loading) {
        //setMovie(loading2_);
        state_ = Loading;
        setSvg(loading_);
    }
    setText(msg);
    btnItem_->hide();
    updateLayout();
}

void StateItem::setLoaded(const QString &icon)
{
    state_ = Loaded;
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
        setSvg(normal_);
    setText(nullptr);
    updateLayout();
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
    state_ = Failed;
    QSvgRenderer * svg = cache_->get(QString(":/showboard/icon/error." + type + ".svg"));
    if (svg == nullptr)
        svg = failed_;
    setSvg(svg);
    setText(errmsg);
    btnItem_->show();
    updateLayout();
}

void StateItem::setSvg(QSvgRenderer * renderer)
{
    static QSvgRenderer emptySvg;
    QGraphicsSvgItem * svgIcon =
        static_cast<QGraphicsSvgItem*>(iconItem_->childItems()[0]);
    QSvgRenderer* old = svgIcon->renderer();
    if (old == renderer)
        return;
    if (old != &emptySvg && old != nullptr) {
        svgIcon->setSharedRenderer(&emptySvg);
        if (animate_ == 1000) {
            old->disconnect(svgIcon);
            SvgCache::instance()->unlock(old);
        }
    }
    killTimer(timerId_);
    timerId_ = 0;
    animate_ = 0;
    if (renderer == nullptr) {
        svgIcon->hide();
        return;
    }
    setMovie(nullptr);
    svgIcon->setSharedRenderer(renderer);
    svgIcon->setRotation(0);
    if (state_ == Loading) {
        if (renderer->animated()) {
            renderer = SvgCache::instance()->lock(renderer);
            QObject::connect(renderer, &QSvgRenderer::repaintNeeded, svgIcon, [svgIcon]() {
                svgIcon->update();
            });
            animate_ = 1000;
        } else {
            timerId_ = startTimer(200);
            svgIcon->setTransformOriginPoint(svgIcon->boundingRect().center());
        }
    }
    svgIcon->show();
    QRectF rect = svgIcon->mapToParent(svgIcon->boundingRect()).boundingRect();
    static_cast<QGraphicsRectItem*>(iconItem_)->setRect(rect);
}

void StateItem::setMovie(QMovie *movie /* = loadingi_ */)
{
    QGraphicsPixmapItem * movieIcon =
        static_cast<QGraphicsPixmapItem*>(iconItem_->childItems()[1]);
    QMovie * old = movieIcon->data(ID_MOVIE).value<QMovie*>();
    if (old == movie)
        return;
    if (old) {
        old->disconnect(this);
        old->stop();
    }
    movieIcon->setData(ID_MOVIE, QVariant::fromValue(movie));
    if (movie == nullptr) {
        movieIcon->setPixmap(QPixmap());
        movieIcon->hide();
        return;
    }
    setSvg(nullptr);
    connect(movie, &QMovie::updated, this, [movieIcon, movie] () {
        movieIcon->setPixmap(movie->currentPixmap());
    });
    movie->start();
    movieIcon->show();
    QRectF rect = movieIcon->mapToParent(movieIcon->boundingRect()).boundingRect();
    static_cast<QGraphicsRectItem*>(iconItem_)->setRect(rect);
}

void StateItem::setText(const QString &msg)
{
    QString messg = "<font style='color:" + QVariant(textColor1_).toByteArray()
            + ";font-size:16pt;'>" + msg + "</font>";
    QString title = "<font style='color:" + QVariant(textColor2_).toByteArray()
            + ";font-size:14pt;'>" + title_ + "</font>";
    QString text =  "<center><nobr>" + messg + "</nobr><br/>" + title + "</center>";
    QGraphicsTextItem* textItem = static_cast<QGraphicsTextItem*>(textItem_);
    textItem->setTextWidth(MAX_TEXT_WIDTH);
    if (text.startsWith("<") && text.endsWith(">"))
        textItem->setHtml(text);
    else
        textItem->setPlainText(text);
    textItem->adjustSize();
    textItem->setVisible(!text.isEmpty());
}

void StateItem::decideStyles(bool independent, bool isTool)
{
    borderRadius_ = dp(8.0);
    if (!isTool) {
        textColor1_ = "#CC2B2B2B"; // 80%
        textColor2_ = "#FFB6B6B6";
        brush_ = QBrush(Qt::white);
        pen_ = QPen(QColor("#FFE2E3E4"), 1);
    } else {
        textColor1_ = "#FFFFFFFF";
        textColor2_ = "#FF7E7E7E";
        brush_ = QBrush(QColor("#FF2B3034"));
        pen_ = QPen(QColor("#FF434D59"), 1);
    }
    if (independent) {
        textSize1_ = textSize2_ = QssHelper::fontSizeScale(18);
        showBackground_ = false;
    } else {
        textSize2_ = QssHelper::fontSizeScale(16);
        textSize2_ = QssHelper::fontSizeScale(14);
        if (parentItem()->boundingRect().isEmpty()) {
            showBackground_ = true;
            fixedSize_ = true;
            rect_ = QRectF({0, 0}, dp(QSizeF(400, 300)));
            rect_.moveCenter({0, 0});
        }
    }
}

void StateItem::updateLayout()
{
    qreal width = 0;
    qreal height = 0;
    for (QGraphicsItem * item : childItems()) {
        if (item->isVisible()) {
            QRectF r(item->boundingRect());
            if (r.width() > width)
                width = r.width();
            if (height > 0)
                height += item->data(ID_PADDING).toInt();
            height += r.height();
        }
    }
    if (!fixedSize_) {
        prepareGeometryChange();
        rect_ = QRectF(0, 0, width + 64, height + 64);
        rect_.moveCenter({0, 0});
    }
    height *= -0.5;
    QPointF pos(0, height);
    for (QGraphicsItem * item : childItems()) {
        if (item->isVisible()) {
            QRectF r(item->boundingRect());
            if (pos.y() > height)
                pos.setY(pos.y() + item->data(ID_PADDING).toInt());
            pos.setY(pos.y() + r.height() / 2);
            item->setPos(pos - r.center());
            pos.setY(pos.y() + r.height() / 2);
        }
    }
}

void StateItem::updateTransform()
{
    QPointF center(parentItem()->boundingRect().center());
    setTransform(QTransform::fromTranslate(center.x(), center.y()));
}

QRectF StateItem::boundingRect() const
{
    return rect_;
}

bool StateItem::hitTest(QGraphicsItem * child, const QPointF &)
{
    QGraphicsItem* hitItem = state_ == Failed ? btnItem_ : iconItem_;
    return (child != hitItem && child->parentItem() != hitItem)
            || receivers(SIGNAL(clicked())) == 0;
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
    painter->setPen(pen_);
    painter->setBrush(brush_);
    painter->drawRoundedRect(boundingRect(), borderRadius_, borderRadius_);
    painter->restore();
}

void StateItem::timerEvent(QTimerEvent * event)
{
    (void) event;
    //qDebug() << "timerEvent";
    ++animate_;
    QGraphicsSvgItem * svgIcon =
        static_cast<QGraphicsSvgItem*>(iconItem_->childItems()[0]);
    svgIcon->setRotation(iconItem_->rotation() + animate_ * 45.0);
}

void StateItem::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
    (void) event;
    if (receivers(SIGNAL(clicked())) == 0) {
        event->ignore();
        return;
    }
    QGraphicsItem* hitItem = state_ == Failed ? btnItem_ : iconItem_;
    if (!hitItem->contains(hitItem->mapFromParent(event->pos()))) {
        event->ignore();
        return;
    }
    if (touchId_) {
        event->ignore();
        return;
    }
    touchId_ = 1;
    if (pressed_)
        setSvg(pressed_);
}

void StateItem::mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
{
    (void) event;
    if (touchId_ != 1)
        return;
    if (normal_)
        setSvg(normal_);
    emit clicked();
    touchId_ = 0;
}

bool StateItem::sceneEvent(QEvent *event)
{
    switch (event->type()) {
    case QEvent::TouchBegin:
        if (receivers(SIGNAL(clicked())) == 0) {
            event->ignore();
            return false;
        } else {
            QTouchEvent::TouchPoint const & pt = static_cast<QTouchEvent*>(event)->touchPoints().first();
            QGraphicsItem* hitItem = state_ == Failed ? btnItem_ : iconItem_;
            if (!hitItem->contains(hitItem->mapFromParent(pt.pos())))
                return false;
            touchId_ = pt.id();
            if (pressed_)
                static_cast<QGraphicsSvgItem*>(iconItem_)->setSharedRenderer(pressed_);
            return true;
        }
    case QEvent::TouchEnd:
        if (normal_)
            static_cast<QGraphicsSvgItem*>(iconItem_)->setSharedRenderer(normal_);
        if (touchId_)
            emit clicked();
        touchId_ = 0;
        return true;
    default:
        break;
    }
    return QGraphicsObject::sceneEvent(event);
}

QGraphicsItem *StateItem::createIconItem(QGraphicsItem *parent, bool independent)
{
    QGraphicsRectItem * iconItem = new QGraphicsRectItem(parent);
    iconItem->setPen(Qt::NoPen);
    QGraphicsSvgItem * svg = new QGraphicsSvgItem(iconItem);
    svg->setScale(dp(independent ? 4.0 : 1.0));
    new QGraphicsPixmapItem(iconItem);
    return iconItem;
}

QGraphicsItem *StateItem::createTextItem(QGraphicsItem *parent, bool independent)
{
    QGraphicsTextItem* textItem = new QGraphicsTextItem(parent);
    textItem->setFont(QFont("Microsoft YaHei"));
    textItem->setDefaultTextColor(Qt::white);
    textItem->setData(ID_PADDING, dp(independent ? 16 : 8));
    return textItem;
}

QGraphicsItem *StateItem::createButtonItem(QGraphicsItem *parent, bool independent)
{
    QGraphicsPathItem * btnItem = new QGraphicsPathItem(parent);
    btnItem->setPen(Qt::NoPen);
    btnItem->setBrush(QColor("#FF008FFF"));
    QPainterPath path;
    if (independent)
        path.addRoundedRect({-128, -32, 256, 64}, 32, 32);
    else
        path.addRoundedRect({-42, -20, 84, 40}, 16, 16);
    btnItem->setPath(path);
    QGraphicsTextItem * btnText = new QGraphicsTextItem(btnItem);
    btnText->setFont(QFont("Microsoft YaHei", QssHelper::fontSizeScale(independent ? 18 : 16)));
    btnText->setDefaultTextColor(Qt::white);
    btnText->setPlainText("重试");
    btnText->adjustSize();
    btnText->setPos(-btnText->boundingRect().center());
    btnItem->hide();
    btnItem->setData(ID_PADDING, dp(independent ? 12 : 24));
    return btnItem;
}

static void truncateText(QString & text, QFont font, int maxWidth)
{
    QFontMetrics fm(font);
    text = fm.elidedText(text, Qt::TextElideMode::ElideMiddle, maxWidth);
}
