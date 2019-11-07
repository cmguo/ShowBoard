#include "selectbox.h"
#include "toolbarwidget.h"

#include <QPen>
#include <QGraphicsProxyWidget>

static constexpr qreal CROSS_LENGTH = 48;
static constexpr qreal CROSS_OFFSET = 8;

SelectBox::SelectBox(QGraphicsItem * parent)
    : QGraphicsRectItem(parent)
{
    QPainterPath lt(QPointF(0, CROSS_LENGTH));
    lt.lineTo(QPointF(0, 0));
    lt.lineTo(QPointF(CROSS_LENGTH, 0));
    leftTop_ = new QGraphicsPathItem(lt, this);

    QPainterPath rt(QPointF(0, CROSS_LENGTH));
    rt.lineTo(QPointF(0, 0));
    rt.lineTo(QPointF(-CROSS_LENGTH, 0));
    rightTop_ = new QGraphicsPathItem(rt, this);

    QPainterPath rb(QPointF(0, -CROSS_LENGTH));
    rb.lineTo(QPointF(0, 0));
    rb.lineTo(QPointF(-CROSS_LENGTH, 0));
    rightBottom_ = new QGraphicsPathItem(rb, this);

    QPainterPath lb(QPointF(0, -CROSS_LENGTH));
    lb.lineTo(QPointF(0, 0));
    lb.lineTo(QPointF(CROSS_LENGTH, 0));
    leftBottom_ = new QGraphicsPathItem(lb, this);

    QPen pen1(QColor(Qt::red), 2);
    leftTop_->setPen(pen1);
    rightTop_->setPen(pen1);
    rightBottom_->setPen(pen1);
    leftBottom_->setPen(pen1);

    ToolbarWidget * toolBar = new ToolbarWidget();
    toolBar->setStyleSheet("ToolbarWidget{background-color:#00000000;}");
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
    toolBar_->setPos(rect.right() - toolBar_->boundingRect().width(),
                     rect.bottom() + 10);
    QRectF rect2(rect.adjusted(-CROSS_OFFSET, -CROSS_OFFSET,
                               CROSS_OFFSET, CROSS_OFFSET));
    leftTop_->setPos(rect2.left(), rect2.top());
    rightTop_->setPos(rect2.right(), rect2.top());
    rightBottom_->setPos(rect2.right(), rect2.bottom());
    leftBottom_->setPos(rect2.left(), rect2.bottom());
}

int SelectBox::hitTest(const QPointF &pos, QRectF &direction)
{
    if (leftTop_->contains(leftTop_->mapFromParent(pos))) {
        direction = QRectF(1, 1, 0, 0);
        return 2;
    } else if (rightTop_->contains(rightTop_->mapFromParent(pos))) {
        direction = QRectF(0, 1, 1, 0);
        return 2;
    } else if (rightBottom_->contains(rightBottom_->mapFromParent(pos))) {
        direction = QRectF(0, 0, 1, 1);
        return 2;
    } else if (leftBottom_->contains(leftBottom_->mapFromParent(pos))) {
        direction = QRectF(1, 0, 0, 1);
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

