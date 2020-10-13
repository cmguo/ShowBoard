#include "qsshelper.h"
#include "selectbox.h"

#include <QPen>
#include <QCursor>

static constexpr qreal BORDER_OFFSET = 9;
static constexpr qreal ROTATE_OFFSET = 24;
static constexpr qreal BOX_RADIUS = 6.25;
static constexpr qreal BOX_RADIUS2 = 6.25;
static constexpr qreal BOX_RADIUS_TEST = 20;
static constexpr qreal LINE_WIDTH = 4;

class BorderItem : public QGraphicsPathItem
{
public:
    BorderItem(QGraphicsItem *parent = nullptr)
        : QGraphicsPathItem(parent)
    {
        setPen(QPen(Qt::black, dp(2.0)));
        setBrush(QBrush(Qt::white));
        setAcceptHoverEvents(true);
        static QPainterPath boxShape = circle(dp(BOX_RADIUS));
        setPath(boxShape);
    }
    BorderItem(int, QGraphicsItem *parent = nullptr)
        : BorderItem(parent)
    {
        setPath(circle(dp(BOX_RADIUS2)));
        setBrush(QBrush(QColor("#B4B2FF")));
        setCursor(QCursor(QPixmap(":/showboard/icon/cursor-rotate.png")));
        setAcceptHoverEvents(false);
        // line
        QGraphicsPathItem * handle = new QGraphicsPathItem(this);
        handle->setPen(QPen(QColor("#CC8D8BE9"), dp(LINE_WIDTH)));
        QPainterPath shape;
        shape.moveTo(0, dp(BOX_RADIUS));
        shape.lineTo(0, dp(ROTATE_OFFSET - LINE_WIDTH));
        handle->setPath(shape);
        handle->setFlag(QGraphicsItem::ItemStacksBehindParent);
    }
private:
    virtual QPainterPath shape() const override
    {
        static QPainterPath boxShape2 = circle(dp(BOX_RADIUS_TEST));
        return boxShape2;
    }
    virtual bool contains(const QPointF &point) const override
    {
        QSizeF size = parentItem()->boundingRect().size() / 4;
        return qAbs(point.x()) < size.width() && qAbs(point.y()) < size.height()
                && shape().contains(point);
    }
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *) override
    {
        QPointF dir = parentItem()->mapToParent(pos())
                - parentItem()->mapToParent(parentItem()->boundingRect().center());
        dir /= sqrt(QPointF::dotProduct(dir, dir));
        if (qFuzzyIsNull(rotate_ - dir.y()))
            return;
        rotate_ = dir.y();
        QTransform tr(dir.x(), dir.y(), -dir.y(), dir.x(), 0, 0);
        static QPixmap sizeh(":/showboard/icon/cursor-sizeh.png");
        QCursor cs2(sizeh.transformed(tr, Qt::SmoothTransformation));
        setCursor(cs2);
    }
private:
    static QPainterPath circle(qreal radius)
    {
        QRectF rect(-radius, -radius, radius * 2, radius * 2);
        QPainterPath path;
        path.addEllipse(rect);
        return path;
    }
private:
    qreal rotate_ = 10.0;
};

SelectBox::SelectBox(QGraphicsItem * parent)
    : QGraphicsRectItem(parent)
{
    rotate_ = new BorderItem(0, this);

    //       4
    //    1     2
    //       8
    left_ = new BorderItem(this);
    right_ = new BorderItem(this);
    top_ = new BorderItem(this);
    bottom_ = new BorderItem(this);

    leftTop_ = new BorderItem(this);
    rightBottom_ = new BorderItem(this);
    rightTop_ = new BorderItem(this);
    leftBottom_ = new BorderItem(this);

    setPen(QPen(QColor("#CC7170E5"), dp(LINE_WIDTH)));
}

void SelectBox::setRect(QRectF const & rect)
{
    qreal offset = dp(BORDER_OFFSET);
    QRectF rect2(rect.adjusted(-offset, -offset, offset, offset));
    QGraphicsRectItem::setRect(rect2);
    QPointF center = rect.center();
    rotate_->setPos(center.x(), rect2.top() - dp(ROTATE_OFFSET));
    leftTop_->setPos(rect2.left(), rect2.top());
    rightTop_->setPos(rect2.right(), rect2.top());
    rightBottom_->setPos(rect2.right(), rect2.bottom());
    leftBottom_->setPos(rect2.left(), rect2.bottom());
    left_->setPos(rect2.left(), center.y());
    right_->setPos(rect2.right(), center.y());
    top_->setPos(center.x(), rect2.top());
    bottom_->setPos(center.x(), rect2.bottom());
}

void SelectBox::setVisible(bool select, bool scale, bool scale2, bool rotate, bool mask)
{
    QGraphicsRectItem::setVisible(select || scale || scale2 || rotate);
    scale2 &= scale;
    rotate_->setVisible(rotate);
    leftTop_->setVisible(scale);
    rightTop_->setVisible(scale);
    rightBottom_->setVisible(scale);
    leftBottom_->setVisible(scale);
    left_->setVisible(scale2);
    top_->setVisible(scale2);
    right_->setVisible(scale2);
    bottom_->setVisible(scale2);
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
    QRectF rect = this->rect();
    QPointF off1 = pos - rect.center();
    QPointF off2(qAbs(off1.x()), qAbs(off1.y()));
    QPointF off3 = rect.bottomRight() - rect.center();
    if (off2.x() > off3.x() / 2) {
        if (off2.y() > off3.y() / 2) {
            direction = QRectF(off1.x() < 0 ? 1 : 0, off1.y() < 0 ? 1 : 0,
                               off1.x() < 0 ? -1 : 1, off1.y() < 0 ? -1 : 1);
        } else if (right_->isVisible()) {
            off3.setY(0); // right
            direction = QRectF(off1.x() < 0 ? 1 : 0, 0,
                               off1.x() < 0 ? -1 : 1, 0);
        }
    } else if (bottom_->isVisible()) {
        off3.setX(0); // bottom
        direction = QRectF(0, off1.y() < 0 ? 1 : 0,
                           0, off1.y() < 0 ? -1 : 1);
    }
    off1 = off2 - off3;
    if (QPointF::dotProduct(off1, off1) <= dp(BOX_RADIUS_TEST) * dp(BOX_RADIUS_TEST))
        return 2;
    if (contains(pos)) {
        return 1;
    }
    return 0;
}

