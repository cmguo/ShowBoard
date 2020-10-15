#include "drawingtool.h"
#include "views/whitecanvas.h"
#include "widget/qsshelper.h"
#include "widget/toolbarwidget.h"
#include "core/resourceview.h"
#include "core/resourcetransform.h"
#include "core/resourcepage.h"
#include "views/canvasitem.h"

#ifdef SHOWBOARD_QUICK
#include <QQuickItem>
#else
#include <QPen>
#include <QPainter>
#include <QGraphicsView>
#include <QGraphicsRectItem>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsProxyWidget>
#include <QGraphicsScene>
#endif
#include <QPushButton>
#include <QHBoxLayout>
#include <QLabel>
#include <QDebug>

static QssHelper QSS(":/showboard/qss/draw_finish.qss");

DrawingTool::DrawingTool(ResourceView *res)
    : Control(res, FullLayout, {DefaultFlags})
{
}

QColor DrawingTool::color() const
{
    return newSettings_.value("color", "write").value<QColor>();
}

void DrawingTool::setColor(QColor color)
{
    newSettings_.insert("color", color);
}

qreal DrawingTool::width() const
{
    return newSettings_.value("width", "1.0").toReal();
}

void DrawingTool::setWidth(qreal width)
{
    newSettings_.insert("width", width);
}

Control * DrawingTool::newControl()
{
    Control * control = whiteCanvas()->addResource(newUrl_, newSettings_);
    emit controlCreated(control);
    return control;
}

void DrawingTool::removeControl(Control *control)
{
    whiteCanvas()->removeResource(control);
}

void DrawingTool::finishControl(Control * control)
{
    whiteCanvas()->hideToolControl(this);
    emit drawFinished(control);
    disconnect(SIGNAL(drawFinished(bool)));
}

bool DrawingTool::translucent() const
{
#ifdef SHOWBOARD_QUICK
    return false;
#else
    return item_->flags().testFlag(CanvasItem::ItemHasNoContents);
#endif
}

void DrawingTool::setTranslucent(bool on)
{
    (void) on;
#ifdef SHOWBOARD_QUICK
#else
    item_->setFlag(CanvasItem::ItemHasNoContents, !on);
#endif
}

class DrawingItem : public CanvasItem
{
public:
    DrawingItem()
        : control_(nullptr)
    {
        setCursor(Qt::CrossCursor);
        setAcceptHoverEvents(true);
        setAcceptTouchEvents(true);
#ifdef SHOWBOARD_QUICK
#else
        setFlag(ItemIsFocusable);
#endif
        QFrameEx* widget = new QFrameEx;
        widget->setObjectName("finishwidget");
        widget->setWindowFlag(Qt::FramelessWindowHint);
        widget->setStyleSheet(QSS);
        QLayout* layout = new QHBoxLayout(widget);
        widget->setLayout(layout);
        layout->setSpacing(20);
        layout->addWidget(new QLabel("闭合顶点或点击完成"));
        QPushButton * button = new QPushButton;
        button->setText(" 完成 ");
        QObject::connect(button, &QPushButton::clicked, [this]() {
            finish();
        });
        layout->addWidget(button);
        ControlView * item = itemFromWidget(widget, this);
        item->setVisible(false);
#ifdef SHOWBOARD_QUICK
#else
        item->setFlag(ItemIsFocusable, false);
#endif
        finishItem_ = item;
    }

private:
    void finish()
    {
        DrawingTool * tool = static_cast<DrawingTool *>(Control::fromItem(this));
        Control* control = control_;
        if (control) {
            if (!(control->resource()->flags() & ResourceView::DrawFinised)) {
                QEvent event(QEvent::User);
                control->event(&event);
                if (!(control->resource()->flags() & ResourceView::DrawFinised)) {
                    qDebug() << "DrawItem: draw dropped";
                    tool->removeControl(control);
                    control = nullptr;
                }
            }
            control_ = nullptr;
            finish_ = true;
            finishItem_->setVisible(false);
            tool->finishControl(control);
        } else if (!finish_) {
            qDebug() << "DrawItem: draw canceled";
            finish_ = true;
            tool->finishControl(control);
        }
    }

protected:

#ifdef SHOWBOARD_QUICK
#else

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override
    {
        QBrush old = painter->brush();
        painter->setBrush(QColor("#01000000"));
        painter->drawRect(rect());
        painter->setBrush(old);
    }

    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override
    {
        if (!control_) {
            DrawingTool * tool = static_cast<DrawingTool *>(Control::fromItem(this));
            control_ = tool->newControl();
            // large canvas may change scale and offset, reset it
            control_->resource()->transform().translateTo({0, 0});
            control_->resource()->transform().scaleTo(1.0);
        }
        control_->event(event);
    }

    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override
    {
        if (control_)
            control_->event(event);
    }

    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override
    {
        if (control_) {
            inSetFocus_ = true; // avoid finish by lost focus
            control_->event(event);
            inSetFocus_ = false;
            if (control_->resource()->flags() & ResourceView::DrawFinised) {
                finish();
            } else if (event->flags() & 256) {
                QPointF pos = event->pos() + QPointF(20, 20);
                if (pos.x() + finishItem_->boundingRect().width() > boundingRect().right()) {
                    pos.setX(boundingRect().right() - finishItem_->boundingRect().width());
                }
                if (pos.y() + finishItem_->boundingRect().height() > boundingRect().bottom()) {
                    pos.setY(boundingRect().bottom() - finishItem_->boundingRect().height());
                }
                finishItem_->setPos(pos);
                finishItem_->show();
            }
        }
    }

    virtual void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override
    {
        if (control_)
            control_->event(event);
    }

    virtual bool sceneEvent(QEvent * event) override
    {
        if (event->type() == QEvent::FocusOut
                || event->type() == QEvent::WindowDeactivate) {
            if (!inSetFocus_)
                finish();
        } else if (event->type() == QEvent::TouchBegin) {
            event->accept();
            return true;
        }
        return CanvasItem::sceneEvent(event);
    }

    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &variant) override
    {
        if (change == ItemVisibleHasChanged && variant.toBool()) {
            qDebug() << "DrawItem: start draw";
            inSetFocus_ = true;
            scene()->views().first()->setFocus();
            inSetFocus_ = false;
            setFocus();
            finish_ = false;
        }
        return CanvasItem::itemChange(change, variant);
    }

#endif

private:
    Control * control_;
    bool finish_ = false;
    bool inSetFocus_ = false;
    ControlView * finishItem_;
};

ControlView *DrawingTool::create(ControlView *parent)
{
    (void) parent;
    return new DrawingItem();
}

void DrawingTool::resize(const QSizeF &size)
{
    QPointF origin(0, 0);
    QRectF rect(origin, size);
    rect.moveCenter(origin);
    static_cast<DrawingItem*>(item_)->setRect(rect);
}
