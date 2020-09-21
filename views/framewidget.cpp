#include "framewidget.h"
#include "qsshelper.h"
#include "core/resourcemanager.h"

#include <QBitmap>
#include <QEvent>
#include <QMetaMethod>
#include <QPainter>
#include <QQuickWidget>
#include <QStyleOption>
#include <qcomponentcontainer.h>

FrameWidget::FrameWidget(QWidget * content, QWidget *parent)
    : QWidget(parent ? parent : content->parentWidget())
    , content_(content)
    , backgroundColor_(Qt::white)
    , borderColor_("#434D59")
    , borderSize_(1) // fixed
    , borderRadius_(dp(8))
    , paddingSize_(0)
    , arrowSize_(dp(30), dp(12))
    , arrowPos_(0, 0)
    , arrowDir_(2)
    , arrowOff_(-1)
{
    setObjectName(content->objectName());
    content_->setParent(this);
    content_->setAttribute(Qt::WA_TranslucentBackground);
    content_->installEventFilter(this);
    updateShape();
}

QWidget *FrameWidget::content()
{
    return content_;
}

void FrameWidget::setBackground(const QColor &color)
{
    backgroundColor_ = color;
    flags_ |= BackgroundSet;
}

void FrameWidget::setBorder(QColor const & color, int size, int radius, int padding)
{
    borderColor_ = color;
    borderSize_ = dp(size);
    borderRadius_ = dp(radius);
    paddingSize_ = dp(padding);
    updateShape();
}

void FrameWidget::setArrowSize(const QSize &size)
{
    arrowSize_ = dp(size);
    updateShape();
}

void FrameWidget::setArrowPosition(QPoint pos, int dir, int off)
{
    arrowPos_ = pos;
    arrowDir_ = dir;
    arrowOff_ = off;
    updateShape();
}

void FrameWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QStyleOption option;
    option.initFrom(content_);
    painter.setRenderHint(QPainter::Antialiasing);
    if (borderSize_ == 0)
        painter.setPen(Qt::NoPen);
    else
        painter.setPen(QPen(borderColor_, borderSize_));
    if (flags_ & BackgroundSet)
        painter.setBrush(QBrush(backgroundColor_));
    else
        painter.setBrush(option.palette.background());
    painter.drawPath(path_);
    QWidget::paintEvent(event);
}

bool FrameWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == content_) {
        if (event->type() == QEvent::Hide) {
            this->hide();
            content_->show();
        } else if (event->type() == QEvent::Resize) {
            mask_ = QRegion();
            updateShape();
        }
    }
    return false;
}

void FrameWidget::updateShape()
{
    // Content geometry, will translate (same as setPos) to make room for arrow
    QRect rc(0, 0, content_->width(), content_->height());
    // Frame geometry, base on origin content geometry,
    //   add size from arrow and padding
    //   offset by arrowPos_
    QRect rf = rc.adjusted(0, 0, paddingSize_ * 2, paddingSize_ * 2);
    // Frame geometry when auto position arrow, same size as rf
    //   TODO:
    QRect go = geometry();
    rc.translate(paddingSize_, paddingSize_);
    QPolygonF polygon(QRectF{rf});
    if (!arrowSize_.isEmpty()) {
        if (arrowDir_ == 0) { // up
            int off = arrowOff_ < 0 ? rf.width() / 2 : arrowOff_;
            polygon.insert(1, QPoint{off - arrowSize_.width() / 2, 0});
            polygon.insert(2, QPoint{off, -arrowSize_.height()});
            polygon.insert(3, QPoint{off + arrowSize_.width() / 2, 0});
            polygon.translate(0, arrowSize_.height());
            rf.adjust(0, 0, 0, arrowSize_.height());
            go = rf.translated(go.left() + go.width() / 2 - rf.width() / 2, go.top());
            rc.translate(0, arrowSize_.height());
        } else if (arrowDir_ == 1) { // right
            int off = arrowOff_ < 0 ? rf.height() / 2 : arrowOff_;
            polygon.insert(2, QPoint{rf.right(), off - arrowSize_.height() / 2});
            polygon.insert(3, QPoint{rf.right() + arrowSize_.width(), off});
            polygon.insert(4, QPoint{rf.right(), off + arrowSize_.height() / 2});
            rf.adjust(0, 0, arrowSize_.width(), 0);
            go = rf.translated(go.right() - rf.width(), go.top() + go.width() / 2 - rf.width() / 2);
         } else if (arrowDir_ == 2) { // bottom
            int off = arrowOff_ < 0 ? rf.width() / 2 : arrowOff_;
            polygon.insert(3, QPoint{off + arrowSize_.width() / 2, rf.bottom()});
            polygon.insert(4, QPoint{off, rf.bottom() + arrowSize_.height()});
            polygon.insert(5, QPoint{off - arrowSize_.width() / 2, rf.bottom()});
            rf.adjust(0, 0, 0, arrowSize_.height());
            go = rf.translated(go.left() + go.width() / 2 - rf.width() / 2, go.bottom() - rf.height());
        } else if (arrowDir_ == 3) { // left
            int off = arrowOff_ < 0 ? rf.height() / 2 : arrowOff_;
            polygon.insert(4, QPoint{0, off - arrowSize_.height() / 2});
            polygon.insert(5, QPoint{-arrowSize_.width(), off});
            polygon.insert(6, QPoint{0, off + arrowSize_.height() / 2});
            polygon.translate(0, arrowSize_.height());
            polygon.translate(arrowSize_.width(), 0);
            rf.adjust(0, 0, arrowSize_.width(), 0);
            go = rf.translated(go.left(), go.top() + go.width() / 2 - rf.width() / 2);
            rc.translate(arrowSize_.width(), 0);
        }
        QPoint arrowPos = polygon[arrowDir_ + 2].toPoint();
        if (arrowPos_.isNull()) {
            setGeometry(go);
        } else {
            rf.translate(arrowPos_ - arrowPos);
            setGeometry(rf);
        }
        content_->setGeometry(rc);
        if (qobject_cast<QQuickWidget*>(content_)
                && QQuickWindow::sceneGraphBackend() != "software") {
            if (mask_.isNull()) {
                mask_ = roundMask(rc.adjusted(paddingSize_, paddingSize_,
                                              -paddingSize_, -paddingSize_), borderRadius_);
                content_->setMask(mask_);
            }
            qobject_cast<QQuickWidget*>(content_)->setClearColor(backgroundColor_);
        }
        QVector<qreal> radiuses(7, borderRadius_);
        radiuses[arrowDir_ + 1] = radiuses[arrowDir_ + 3] = 0;
        radiuses[arrowDir_ + 2] = borderRadius_ / 2;
        path_ = toRoundPolygon(polygon, radiuses);
    }
}

QRegion FrameWidget::roundMask(QRect const & rect, int radius)
{
    QSize sz(radius, radius);
    QRect corners[] = {
        QRect(rect.topLeft(), QSize(radius, radius)),
        QRect(rect.topRight(), QSize(-radius, radius)),
        QRect(rect.bottomRight(), QSize(-radius, -radius)),
        QRect(rect.bottomLeft(), QSize(radius, -radius))
    };
    QRegion r(rect);
    for (QRect & c : corners) {
        r -= c.normalized();
        c.setSize(c.size() * 2);
        r += QRegion(c.normalized(), QRegion::Ellipse);
    }
    return r;
}

QPainterPath FrameWidget::toRoundPolygon(const QPolygonF &polygon, QVector<qreal> const & radiuses)
{
   QPainterPath path;
   static QObject* geometryHelper = QComponentContainer::globalInstance().getExportValue("GeometryHelper");
   if (geometryHelper) {
        bool ok = QMetaObject::invokeMethod(geometryHelper, "toRoundPolygon", Q_RETURN_ARG(QPainterPath,path),
                      Q_ARG(QPolygonF,polygon), Q_ARG(QVector<qreal>,radiuses));
        if (ok)
            return path;
   }
   path.addPolygon(polygon);
   return path;
}
