#include "positionbar.h"

#include <QPen>

PositionBar::PositionBar(QGraphicsItem * parent)
    : QGraphicsPathItem(parent)
{
    setPen(QPen(Qt::white));
    setBrush(QColor("#A0606060"));
}

void PositionBar::setInCanvas(bool in)
{
    inCanvas_ = in;
}

void PositionBar::update(const QRectF &viewRect, const QRectF &canvasRect, qreal scale, QPointF offset)
{
    constexpr qreal WIDTH = 8;
    QPointF pos = (viewRect.topLeft() - offset) - canvasRect.topLeft() * scale;
    QSizeF scl(viewRect.width() / canvasRect.width() / scale, viewRect.height() / canvasRect.height() / scale);
    QRectF vRect(viewRect.right() - WIDTH, pos.y() * scl.height() + viewRect.top(),
                 WIDTH, viewRect.height() * scl.height());
    QRectF hRect(pos.x() * scl.width() + viewRect.left(), viewRect.bottom() - WIDTH,
                 viewRect.width() * scl.width(), WIDTH);
    QPainterPath ph;
    if (vRect.height() < viewRect.height() && !qFuzzyCompare(vRect.height(), viewRect.height())) {
        if (inCanvas_) {
            vRect.translate(-offset);
            vRect = QRectF(vRect.topLeft()/ scale, vRect.size() / scale);
        }
        ph.addRoundedRect(vRect, WIDTH / 2, WIDTH / 2);
    }
    if (hRect.width() < viewRect.width() && !qFuzzyCompare(hRect.width(), viewRect.width())) {
        if (inCanvas_) {
            hRect.translate(-offset);
            hRect = QRectF(hRect.topLeft() / scale, hRect.size() / scale);
        }
        ph.addRoundedRect(hRect, WIDTH / 2, WIDTH / 2);
    }
    setPath(ph);
}
