#include "stateitem.h"
#include "core/svgcache.h"
#include "core/control.h"
#include "core/resourceview.h"

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
QSvgRenderer * StateItem::loading_ = nullptr;
QSvgRenderer * StateItem::failed_ = nullptr;
QMovie * StateItem::loadingi_ = nullptr;

static void truncateText(QString & text, QFont font, int maxWidth);

static constexpr int MAX_TEXT_WIDTH = 300;

StateItem::StateItem(QGraphicsItem * parent)
    : QGraphicsObject(parent)
    , iconItem_(nullptr)
    , textItem_(nullptr)
    , btnItem_(nullptr)
    , normal_(nullptr)
    , hover_(nullptr)
    , pressed_(nullptr)
    , state_(None)
    , independent_(false)
    , showBackground_(true)
    , timerId_(0)
    , animate_(0)
    , touchId_(0)
{
    setAcceptedMouseButtons(Qt::LeftButton);
    setAcceptTouchEvents(true);
    Control * control = Control::fromItem(parent);
    independent_ = control->resource()->flags().testFlag(ResourceView::Independent);
    title_ = control->resource()->name();
    iconItem_ = createIconItem(this);
    textItem_ = createTextItem(this);
    truncateText(title_, static_cast<QGraphicsTextItem*>(textItem_)->font(), MAX_TEXT_WIDTH);
    btnItem_ = createButtonItem(this, independent_);
    if (!cache_) {
        cache_ = SvgCache::instance();
        loading_ = cache_->get(QString(":/showboard/icon/loading.svg"));
        loadingi_ = cache_->getMovie(QString(":/showboard/icon/loadingi.svg"));
        failed_ = cache_->get(QString(":/showboard/icon/error.independent.svg"));
    }
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
    if (state_ != Loading) {
        if (independent_) {
            setMovie(loadingi_);
        } else {
            setSvg(loading_);
            if (loading_->animated()) {
                int interval = 1000 / loading_->framesPerSecond();
                timerId_ = startTimer(interval);
            } else {
                timerId_ = startTimer(100);
            }
        }
        state_ = Loading;
    }
    if (independent_) {
        setText(nullptr);
    } else {
        QString title = title_.isEmpty() ? ""
                                         : "<br/><font style='color:#98FFFFFF;font-size:14pt;'>"
                    + title_ + "</font>";
        QString text =  "<center><nobr>" + msg + "...</nobr>" + title + "</center>";
        setText(text);
    }
    btnItem_->hide();
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
        setSvg(normal_);
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
    if (independent_) {
        text = "<center><font style='color:#98FFFFFF;font-size:18pt;'>" + error + "</font></center>";
        type = "independent";
    }
    state_ = Failed;
    QSvgRenderer * svg = cache_->get(QString(":/showboard/icon/error." + type + ".svg"));
    if (svg == nullptr)
        svg = failed_;
    setSvg(svg);
    setText(text);
    if (btnItem_) {
        QPointF center(btnItem_->boundingRect().center());
        center.setY(-btnItem_->boundingRect().height() / 2 - textItem_->pos().y()
                    - textItem_->boundingRect().height() - 10);
        btnItem_->setPos(-center);
        btnItem_->show();
    }
}

void StateItem::setSvg(QSvgRenderer * renderer)
{
    static QSvgRenderer emptySvg;
    QGraphicsSvgItem * svgIcon =
        static_cast<QGraphicsSvgItem*>(iconItem_->childItems()[0]);
    if (svgIcon->renderer() == renderer)
        return;
    killTimer(timerId_);
    timerId_ = 0;
    if (renderer == nullptr) {
        svgIcon->setSharedRenderer(&emptySvg);
        svgIcon->hide();
        return;
    }
    setMovie(nullptr);
    svgIcon->setSharedRenderer(renderer);
    QRectF rect = svgIcon->boundingRect();
    QPointF center(rect.center());
    svgIcon->setRotation(0);
    svgIcon->setTransformOriginPoint(center);
    svgIcon->setPos(-center);
    svgIcon->show();
    rect.moveCenter({0, 0});
    static_cast<QGraphicsRectItem*>(iconItem_)->setRect(rect);
    prepareGeometryChange();
}

void StateItem::setMovie(QMovie *movie /* = loadingi_ */)
{
    QGraphicsPixmapItem * movieIcon =
        static_cast<QGraphicsPixmapItem*>(iconItem_->childItems()[1]);
    QMovie * old = movieIcon->data(1000).value<QMovie*>();
    if (old == movie)
        return;
    if (old) {
        old->disconnect(this);
        old->stop();
    }
    movieIcon->setData(1000, QVariant::fromValue(movie));
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
    QRectF rect = movieIcon->boundingRect();
    QPointF center(rect.center());
    movieIcon->setPos(-center);
    movieIcon->show();
    static_cast<QGraphicsRectItem*>(iconItem_)->setRect(rect);
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
    if (btnItem_->isVisible()) {
        rect |= btnItem_->mapToParent(btnItem_->boundingRect()).boundingRect();
    }
    rect.adjust(-32, -12, 32, 32);
    return rect;
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
    painter->setPen(QColor("#FF434D59"));
    painter->setBrush(QColor("#F22B3034"));
    painter->drawRoundedRect(boundingRect(), 8, 8);
    painter->restore();
}

void StateItem::timerEvent(QTimerEvent * event)
{
    (void) event;
    //qDebug() << "timerEvent";
    ++animate_;
    QGraphicsSvgItem * svgIcon =
        static_cast<QGraphicsSvgItem*>(iconItem_->childItems()[0]);
    if (svgIcon->renderer()->animated()) {
        svgIcon->renderer()->setCurrentFrame(animate_);
    } else {
        svgIcon->setRotation(iconItem_->rotation() + animate_ * 45.0);
    }
}

void StateItem::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
    (void) event;
    if (receivers(SIGNAL(clicked())) == 0) {
        event->ignore();
        return;
    }
    QGraphicsItem* hitItem = state_ == Failed ? btnItem_ : iconItem_;
    if (!hitItem->contains(hitItem->mapFromParent(event->pos())))
        return;
    if (touchId_)
        return;
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

QGraphicsItem *StateItem::createIconItem(QGraphicsItem *parent)
{
    QGraphicsRectItem * iconItem = new QGraphicsRectItem(parent);
    iconItem->setPen(Qt::NoPen);
    new QGraphicsSvgItem(iconItem);
    new QGraphicsPixmapItem(iconItem);
    return iconItem;
}

QGraphicsItem *StateItem::createTextItem(QGraphicsItem *parent)
{
    QGraphicsTextItem* textItem = new QGraphicsTextItem(parent);
    textItem->setFont(QFont("Microsoft YaHei", 16));
    textItem->setDefaultTextColor(Qt::white);
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
        path.addRoundedRect({-64, -16, 128, 32}, 16, 16);
    btnItem->setPath(path);
    QGraphicsTextItem * btnText = new QGraphicsTextItem(btnItem);
    btnText->setFont(QFont("Microsoft YaHei", independent ? 18 : 16));
    btnText->setDefaultTextColor(Qt::white);
    btnText->setPlainText("重新尝试");
    btnText->adjustSize();
    btnText->setPos(-btnText->boundingRect().center());
    btnItem->hide();
    return btnItem;
}

static void truncateText(QString & text, QFont font, int maxWidth)
{
    QFontMetrics fm(font);
    text = fm.elidedText(text, Qt::TextElideMode::ElideMiddle, maxWidth);
}
