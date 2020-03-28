#include "selectbox.h"

#include <QPen>
#include <QCursor>

static constexpr qreal CROSS_LENGTH = 20;
static constexpr qreal CROSS_OFFSET = 3;

class BorderItem : public QGraphicsPathItem
{
public:
    BorderItem(const QPainterPath &path, int clip, QGraphicsItem *parent = nullptr)
        : QGraphicsPathItem(path, parent)
        , clip_(clip)
    {
    }

private:
    virtual QPainterPath shape() const override
    {
        QRectF rect{-CROSS_LENGTH * 3 / 2, -CROSS_LENGTH * 3 / 2,
                    CROSS_LENGTH * 3, CROSS_LENGTH * 3};
        QPainterPath path;
        path.addRect(rect.normalized());
        return path;
    }

    bool contains(const QPointF &point) const override
    {
        QRectF prect = parentItem()->boundingRect();
        prect.adjust(-CROSS_OFFSET, -CROSS_OFFSET,
                     CROSS_OFFSET, CROSS_OFFSET);
        QSizeF psize = prect.size() / 4;
        prect.adjust(psize.width(), psize.height(), -psize.width(), -psize.height());
        prect.moveCenter(prect.center() - pos());
        QRectF rect{-CROSS_LENGTH * 3 / 2, -CROSS_LENGTH * 3 / 2,
                    CROSS_LENGTH * 3, CROSS_LENGTH * 3};
        if (clip_ & 1)
            rect.setLeft(qMax(rect.left(), prect.right()));
        if (clip_ & 2)
            rect.setTop(qMax(rect.top(), prect.bottom()));
        if (clip_ & 4)
            rect.setRight(qMin(rect.right(), prect.left()));
        if (clip_ & 8)
            rect.setBottom(qMin(rect.bottom(), prect.top()));
        return rect.contains(point);
    }

private:
    int clip_;
};

SelectBox::SelectBox(QGraphicsItem * parent)
    : QGraphicsRectItem(parent)
{
    QPainterPath lt(QPointF(0, CROSS_LENGTH));
    lt.lineTo(QPointF(0, 0));
    lt.lineTo(QPointF(CROSS_LENGTH, 0));
    leftTop_ = new BorderItem(lt, 12, this);
    leftTop_->setCursor(Qt::SizeFDiagCursor);

    QPainterPath rt(QPointF(0, CROSS_LENGTH));
    rt.lineTo(QPointF(0, 0));
    rt.lineTo(QPointF(-CROSS_LENGTH, 0));
    rightTop_ = new BorderItem(rt, 9, this);
    rightTop_->setCursor(Qt::SizeBDiagCursor);

    QPainterPath rb(QPointF(0, -CROSS_LENGTH));
    rb.lineTo(QPointF(0, 0));
    rb.lineTo(QPointF(-CROSS_LENGTH, 0));
    rightBottom_ = new BorderItem(rb, 3, this);
    rightBottom_->setCursor(Qt::SizeFDiagCursor);

    QPainterPath lb(QPointF(0, -CROSS_LENGTH));
    lb.lineTo(QPointF(0, 0));
    lb.lineTo(QPointF(CROSS_LENGTH, 0));
    leftBottom_ = new BorderItem(lb, 6, this);
    leftBottom_->setCursor(Qt::SizeBDiagCursor);

    QPainterPath lr(QPointF(0, -CROSS_LENGTH / 2));
    lr.lineTo(QPointF(0, CROSS_LENGTH / 2));
    left_ = new BorderItem(lr, 14, this);
    left_->setCursor(Qt::SizeHorCursor);
    right_ = new BorderItem(lr, 11, this);
    right_->setCursor(Qt::SizeHorCursor);

    QPainterPath tb(QPointF(-CROSS_LENGTH / 2, 0));
    tb.lineTo(QPointF(CROSS_LENGTH / 2, 0));
    top_ = new BorderItem(tb, 13, this);
    top_->setCursor(Qt::SizeVerCursor);
    bottom_ = new BorderItem(tb, 7, this);
    bottom_->setCursor(Qt::SizeVerCursor);

    QPainterPath r;
    r.addEllipse({-CROSS_LENGTH / 2, -CROSS_LENGTH / 2,
                  CROSS_LENGTH, CROSS_LENGTH});
    rotate_ = new BorderItem(r, 0, this);
    rotate_->setCursor(Qt::CrossCursor);

    QPen pen1(QColor(Qt::white), 6);
    pen1.setJoinStyle(Qt::MiterJoin);
    rotate_->setPen(pen1);
    leftTop_->setPen(pen1);
    rightTop_->setPen(pen1);
    rightBottom_->setPen(pen1);
    leftBottom_->setPen(pen1);
    left_->setPen(pen1);
    top_->setPen(pen1);
    right_->setPen(pen1);
    bottom_->setPen(pen1);

    setPen(QPen(Qt::white, 2));
    setBrush(QColor("#40808080"));
    //setOpacity(1.0);
    setCursor(Qt::SizeAllCursor);
}

void SelectBox::setRect(QRectF const & rect)
{
    QRectF rect2(rect.adjusted(-2, -2, 2, 2));
    QGraphicsRectItem::setRect(rect2);
    rect2.adjust(-CROSS_OFFSET, -CROSS_OFFSET,
                 CROSS_OFFSET, CROSS_OFFSET);
    QPointF center = rect.center();
    rotate_->setPos(center.x(), rect2.top() - CROSS_LENGTH);
    leftTop_->setPos(rect2.left(), rect2.top());
    rightTop_->setPos(rect2.right(), rect2.top());
    rightBottom_->setPos(rect2.right(), rect2.bottom());
    leftBottom_->setPos(rect2.left(), rect2.bottom());
    left_->setPos(rect2.left(), center.y());
    right_->setPos(rect2.right(), center.y());
    top_->setPos(center.x(), rect2.top());
    bottom_->setPos(center.x(), rect2.bottom());
}

void SelectBox::setVisible(bool select, bool scale, bool rotate)
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
}

int SelectBox::hitTest(const QPointF &pos, QRectF &direction)
{
    if (rotate_->isVisible() && rotate_->contains(rotate_->mapFromParent(pos))) {
        return 3;
    }
    if (!leftTop_->isVisible())
        return contains(pos) ? 1 : 0;
    QRectF rect = this->rect().adjusted(-CROSS_OFFSET, -CROSS_OFFSET,
                                      CROSS_OFFSET, CROSS_OFFSET);
    QPointF off1 = pos - rect.center();
    QPointF off2(qAbs(off1.x()), qAbs(off1.y()));
    QPointF pos1 = off2 + rect.center();
    QPointF off3 = rect.bottomRight() - rect.center();
    if (off2.x() > off3.x() / 2) {
        if (off2.y() > off3.y() / 2) {
            if (rightBottom_->contains(rightBottom_->mapFromParent(pos1))) {
                direction = QRectF(pos.x() < 0 ? 1 : 0, pos.y() < 0 ? 1 : 0,
                                   pos.x() < 0 ? -1 : 1, pos.y() < 0 ? -1 : 1);
                return 2;
            }
        } else {
            if (right_->contains(right_->mapFromParent(pos1))) {
                direction = QRectF(pos.x() < 0 ? 1 : 0, 0,
                                   pos.x() < 0 ? -1 : 1, 0);
                return 2;
            }
        }
    } else if (bottom_->contains(bottom_->mapFromParent(off1))) {
        direction = QRectF(0, pos.y() < 0 ? 1 : 0,
                           0, pos.y() < 0 ? -1 : 1);
        return 2;
    }
    if (contains(pos)) {
        return 1;
    }
    return 0;
}

