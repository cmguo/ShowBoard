#include "drawingtool.h"
#include "views/whitecanvas.h"
#include "core/resourceview.h"

#include <QPen>
#include <QGraphicsRectItem>
#include <QGraphicsSceneMouseEvent>
#include <QPushButton>
#include <QGraphicsProxyWidget>

DrawingTool::DrawingTool(ResourceView *res)
    : Control(res, FullLayout, {DefaultFlags})
{
}

Control * DrawingTool::newControl()
{
    return whiteCanvas()->addResource(newUrl_);
}

void DrawingTool::finishControl(Control * control)
{
    (void) control;
    whiteCanvas()->hideToolControl("drawing");
}

class DrawingItem : public QGraphicsRectItem
{
public:
    DrawingItem()
        : control_(nullptr)
    {
        setPen(QPen(Qt::NoPen));
        setCursor(Qt::CrossCursor);
        setAcceptHoverEvents(true);
        setFlag(ItemIsFocusable);
        QPushButton * button = new QPushButton("完成");
        QObject::connect(button, &QPushButton::clicked, [this](){finish();});
        QGraphicsProxyWidget * item = new QGraphicsProxyWidget(this);
        item->setWidget(button);
        item->hide();
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

    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override
    {
        if (!control_) {
            DrawingTool * tool = static_cast<DrawingTool *>(Control::fromItem(this));
            control_ = tool->newControl();
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
        return QGraphicsRectItem::sceneEvent(event);
    }

    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &variant) override
    {
        if (change == ItemVisibleHasChanged && variant.toBool()) {
            setFocus();
        }
        return QGraphicsRectItem::itemChange(change, variant);
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
