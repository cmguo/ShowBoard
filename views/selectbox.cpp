#include "qsshelper.h"
#include "selectbox.h"

#include <QPen>
#include <QCursor>

static constexpr qreal BORDER_OFFSET = 3;
static constexpr qreal ROTATE_OFFSET = 30;
static constexpr qreal BOX_RADIUS = 8;
static constexpr qreal BOX_RADIUS2 = 20;

class BorderItem : public QGraphicsPathItem
{
public:
    BorderItem(QGraphicsItem *parent = nullptr)
        : QGraphicsPathItem(parent)
    {
        setPen(QPen(Qt::black, 2.0));
        setBrush(QBrush(Qt::white));
        static QPainterPath boxShape = circle(QssHelper::sizeScale(BOX_RADIUS));
        setPath(boxShape);
    }
    BorderItem(int, QGraphicsItem *parent = nullptr)
        : BorderItem(parent)
    {
        QGraphicsPathItem * handle = new QGraphicsPathItem(this);
        handle->setPen(QPen(QColor("#990091FF"), 3));
        QPainterPath shape;
        shape.moveTo(0, QssHelper::sizeScale(BOX_RADIUS));
        shape.lineTo(0, QssHelper::sizeScale(ROTATE_OFFSET));
        handle->setPath(shape);
        handle->setFlag(QGraphicsItem::ItemStacksBehindParent);
    }
private:
    virtual QPainterPath shape() const override
    {
        static QPainterPath boxShape2 = circle(QssHelper::sizeScale(BOX_RADIUS2));
        return boxShape2;
    }

    bool contains(const QPointF &point) const override
    {
        QSizeF size = parentItem()->boundingRect().size() / 4;
        return qAbs(point.x()) < size.width() && qAbs(point.y()) < size.height()
                && shape().contains(point);
    }

private:
    static QPainterPath circle(qreal radius)
    {
        QRectF rect(-radius, -radius, radius * 2, radius * 2);
        QPainterPath path;
        path.addEllipse(rect);
        return path;
    }
};

SelectBox::SelectBox(QGraphicsItem * parent)
    : QGraphicsRectItem(parent)
{
    rotate_ = new BorderItem(0, this);
    rotate_->setCursor(Qt::CrossCursor);

    //       4
    //    1     2
    //       8
    left_ = new BorderItem(this);
    left_->setCursor(Qt::SizeHorCursor);
    right_ = new BorderItem(this);
    right_->setCursor(Qt::SizeHorCursor);
    top_ = new BorderItem(this);
    top_->setCursor(Qt::SizeVerCursor);
    bottom_ = new BorderItem(this);
    bottom_->setCursor(Qt::SizeVerCursor);

    leftTop_ = new BorderItem(this);
    leftTop_->setCursor(Qt::SizeFDiagCursor);
    rightBottom_ = new BorderItem(this);
    rightBottom_->setCursor(Qt::SizeFDiagCursor);
    rightTop_ = new BorderItem(this);
    rightTop_->setCursor(Qt::SizeBDiagCursor);
    leftBottom_ = new BorderItem(this);
    leftBottom_->setCursor(Qt::SizeBDiagCursor);

    setPen(QPen(QColor("#990091FF"), 3));
}

void SelectBox::setRect(QRectF const & rect)
{
    qreal offset = QssHelper::sizeScale(BORDER_OFFSET);
    QRectF rect2(rect.adjusted(-offset, -offset, offset, offset));
    QGraphicsRectItem::setRect(rect2);
    QPointF center = rect.center();
    rotate_->setPos(center.x(), rect2.top() - QssHelper::sizeScale(BOX_RADIUS + ROTATE_OFFSET));
    leftTop_->setPos(rect2.left(), rect2.top());
    rightTop_->setPos(rect2.right(), rect2.top());
    rightBottom_->setPos(rect2.right(), rect2.bottom());
    leftBottom_->setPos(rect2.left(), rect2.bottom());
    left_->setPos(rect2.left(), center.y());
    right_->setPos(rect2.right(), center.y());
    top_->setPos(center.x(), rect2.top());
    bottom_->setPos(center.x(), rect2.bottom());
}

void SelectBox::setVisible(bool select, bool scale, bool rotate, bool mask)
{
    QGraphicsRectItem::setVisible(select || scale || rotate);
    rotate_->setVisible(rotate);
    leftTop_->setVisible(scale);
    rightTop_->setVisible(scale);
    rightBottom_->setVisible(scale);
    leftBottom_->setVisible(scale);
    left_->setVisible(scale);
    top_->setVisible(scale);
    right_->setVisible(scale);
    bottom_->setVisible(scale);
    if (mask) {
        setBrush(QColor("#40808080"));
        setCursor(Qt::SizeAllCursor);
    } else {
        setBrush(Qt::transparent);
        unsetCursor();
    }
}

int SelectBox::hitTest(const QPointF &pos, QRectF &direction)
{
    if (rotate_->isVisible() && pos.y() < rect().top()
            && rotate_->contains(rotate_->mapFromParent(pos))) {
        return 3;
    }
    if (!leftTop_->isVisible())
        return contains(pos) ? 1 : 0;
    QRectF rect = this->rect().adjusted(-BORDER_OFFSET, -BORDER_OFFSET,
                                      BORDER_OFFSET, BORDER_OFFSET);
    QPointF off1 = pos - rect.center();
    QPointF off2(qAbs(off1.x()), qAbs(off1.y()));
    QPointF off3 = rect.bottomRight() - rect.center();
    if (off2.x() > off3.x() / 2) {
        if (off2.y() > off3.y() / 2) {
            direction = QRectF(pos.x() < 0 ? 1 : 0, pos.y() < 0 ? 1 : 0,
                               pos.x() < 0 ? -1 : 1, pos.y() < 0 ? -1 : 1);
        } else {
            off3.setY(0); // right
            direction = QRectF(pos.x() < 0 ? 1 : 0, 0,
                               pos.x() < 0 ? -1 : 1, 0);
        }
    } else {
        off3.setX(0); // left
        direction = QRectF(0, pos.y() < 0 ? 1 : 0,
                           0, pos.y() < 0 ? -1 : 1);
    }
    off1 = off2 - off3;
    if (QPointF::dotProduct(off1, off1) <= BOX_RADIUS2 * BOX_RADIUS2)
        return 2;
    if (contains(pos)) {
        return 1;
    }
    return 0;
}

