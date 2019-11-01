#include "selectbox.h"
#include "toolbarwidget.h"

#include <QPen>
#include <QGraphicsProxyWidget>

static constexpr qreal CROSS_LENGTH = 30;
static constexpr qreal CROSS_OFFSET = 4;

SelectBox::SelectBox(QGraphicsItem * parent)
    : QGraphicsRectItem(parent)
{
    QPainterPath lt(QPointF(0, CROSS_LENGTH));
    lt.lineTo(QPointF(0, 0));
    lt.lineTo(QPointF(CROSS_LENGTH, 0));
    leftTop_ = new QGraphicsPathItem(lt, this);

    QPainterPath rb(QPointF(0, -CROSS_LENGTH));
    rb.lineTo(QPointF(0, 0));
    rb.lineTo(QPointF(-CROSS_LENGTH, 0));
    rightBottom_ = new QGraphicsPathItem(rb, this);

    QPen pen1(QColor(Qt::red), 2);
    leftTop_->setPen(pen1);
    QPen pen2(QColor(Qt::blue), 2);
    rightBottom_->setPen(pen2);

    ToolbarWidget * toolBar = new ToolbarWidget();
    QGraphicsProxyWidget * proxy = new QGraphicsProxyWidget(this);
    proxy->setWidget(toolBar);
    toolBar_ = proxy;
    QObject::connect(toolBar, &ToolbarWidget::sizeChanged, [this](QSizeF const & size) {
        QRectF rect(this->rect());
        toolBar_->setPos(rect.right() - size.width(), rect.bottom() + 10);
    });

    setPen(QPen(Qt::NoPen));
    setBrush(QBrush(QColor::fromRgba(0x20202020)));
}

void SelectBox::setRect(QRectF const & rect)
{
    QGraphicsRectItem::setRect(rect);
    leftTop_->setPos(rect.left() - CROSS_OFFSET, rect.top() - CROSS_OFFSET);
    rightBottom_->setPos(rect.right() + CROSS_OFFSET, rect.bottom() + CROSS_OFFSET);
    toolBar_->setPos(rect.right() - toolBar_->boundingRect().width(), rect.bottom() + 10);
}

int SelectBox::hitTest(const QPointF &pos, QRectF &direction)
{
    if (leftTop_->contains(leftTop_->mapFromParent(pos))) {
        direction = QRectF(1, 1, 0, 0);
        return 2;
    } else if (rightBottom_->contains(rightBottom_->mapFromParent(pos))) {
        direction = QRectF(0, 0, 1, 1);
        return 2;
    } else if (contains(pos)) {
        return 1;
    }
    return 0;
}

ToolbarWidget * SelectBox::toolBar()
{
    return static_cast<ToolbarWidget*>(
                static_cast<QGraphicsProxyWidget*>(toolBar_)->widget());
}

