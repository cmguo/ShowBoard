#include "drawingtool.h"
#include "views/whitecanvas.h"
#include "views/qsshelper.h"
#include "core/resourceview.h"
#include "core/resourcetransform.h"
#include "core/resourcepage.h"

#include <QPen>
#include <QGraphicsRectItem>
#include <QGraphicsSceneMouseEvent>
#include <QPushButton>
#include <QGraphicsProxyWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QHBoxLayout>
#include <QLabel>

static QssHelper QSS(":/showboard/qss/draw_finish.qss");

DrawingTool::DrawingTool(ResourceView *res)
    : Control(res, FullLayout, {DefaultFlags})
{
}

Control * DrawingTool::newControl()
{
    Control * control = whiteCanvas()->addResource(newUrl_, newSettings_);
    emit controlCreated(control);
    return control;
}

void DrawingTool::finishControl(Control * control)
{
    (void) control;
    whiteCanvas()->hideToolControl(this);
}

void DrawingTool::setTranslucent(bool on)
{
    item_->setFlag(CanvasItem::ItemHasNoContents, !on);
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
        setFlag(ItemIsFocusable);

        QWidget* widget = new QFrame;
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
        QGraphicsProxyWidget * item = new QGraphicsProxyWidget(this);
        item->setWidget(widget);
        item->hide();
        item->setFlag(ItemIsFocusable, false);
        finishItem_ = item;
    }

private:
    void finish()
    {
        if (control_) {
            if (!(control_->resource()->flags() & ResourceView::DrawFinised)) {
                QEvent event(QEvent::User);
                control_->event(&event);
            }
            control_ = nullptr;
            finishItem_->hide();
        }
        DrawingTool * tool = static_cast<DrawingTool *>(Control::fromItem(this));
        tool->finishControl(control_);
    }

protected:
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
            control_->event(event);
            if (control_->resource()->flags() & ResourceView::DrawFinised) {
                finish();
            } else if (event->flags() & 256) {
                finishItem_->setPos(event->pos() + QPointF(20, 20));
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
                || event->type() == QEvent::WindowDeactivate)
            finish();
        if (event->type() == QEvent::TouchBegin) {
            event->accept();
            return true;
        }
        return CanvasItem::sceneEvent(event);
    }

    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &variant) override
    {
        if (change == ItemVisibleHasChanged && variant.toBool()) {
            setFocus();
            scene()->views().first()->setFocus();
        }
        return CanvasItem::itemChange(change, variant);
    }

private:
    Control * control_;
    QGraphicsItem * finishItem_;
};

QGraphicsItem * DrawingTool::create(ResourceView *res)
{
    (void) res;
    return new DrawingItem();
}

void DrawingTool::resize(const QSizeF &size)
{
    QPointF origin(0, 0);
    QRectF rect(origin, size);
    rect.moveCenter(origin);
    static_cast<DrawingItem*>(item_)->setRect(rect);
}
