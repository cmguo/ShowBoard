#include "drawingtool.h"
#include "views/whitecanvas.h"
#include "core/resourceview.h"

#include <QPen>
#include <QGraphicsRectItem>
#include <QGraphicsSceneMouseEvent>

DrawingTool::DrawingTool(ResourceView *res)
    : Control(res, FullLayout, CanMove)
{
}

Control * DrawingTool::newControl()
{
    WhiteCanvas * canvas = static_cast<WhiteCanvas *>(item_->parentItem()->parentItem());
    return canvas->addResource(newUrl_);
}

void DrawingTool::finishControl(Control * control)
{
    (void) control;
    WhiteCanvas * canvas = static_cast<WhiteCanvas *>(item_->parentItem()->parentItem());
    canvas->hideToolControl("drawing");
}

class DrawingItem : public QGraphicsRectItem
{
public:
    DrawingItem()
        : control_(nullptr)
    {
        setPen(QPen(Qt::NoPen));
    }

private:
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
            if (event->flags() & 256) {
                DrawingTool * tool = static_cast<DrawingTool *>(Control::fromItem(this));
                tool->finishControl(control_);
                control_ = nullptr;
            }
        }
    }

private:
    Control * control_;
};

QGraphicsItem * DrawingTool::create(ResourceView *res)
{
    (void) res;
    return new DrawingItem();
}
