#include "graphcontrol.h"

#include <QGraphicsItem>
#include <QGraphicsSceneEvent>

class GraphItem : public QGraphicsItem
{
public:
    GraphItem()
        : active_(true)
    {
    }

public:
    QRectF boundingRect() const override
    {
        return QRectF();
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override
    {
        (void) painter;
        (void) option;
        (void) widget;
    }

    bool contains(const QPointF &point) const override
    {
        (void) point;
        return active_;
    }

    bool sceneEventFilter(QGraphicsItem *watched, QEvent *event) override
    {
        GraphControl * control = static_cast<GraphControl *>(Control::fromItem(watched));
        Graph * graph = static_cast<Graph *>(control->resource());
        switch (event->type()) {
        case QGraphicsSceneEvent::MouseButtonPress: {
            QGraphicsSceneMouseEvent * me = static_cast<QGraphicsSceneMouseEvent *>(event);
            graph->addPoint(me->pos());
            control->updateGraph(graph);
            return true;
        }
        case QGraphicsSceneEvent::MouseMove: {
            if (!active_)
                return false;
            QGraphicsSceneMouseEvent * me = static_cast<QGraphicsSceneMouseEvent *>(event);
            graph->movePoint(me->pos());
            control->updateGraph(graph);
            return true;
        }
        case QGraphicsSceneEvent::MouseButtonRelease: {
            if (!active_)
                return false;
            QGraphicsSceneMouseEvent * me = static_cast<QGraphicsSceneMouseEvent *>(event);
            if (graph->commit(me->pos())) {
                active_ = false;
                watched->removeSceneEventFilter(this);
                watched->setActive(false);
                graph->finsh(control->bounds().center());
            } else {
                control->updateGraph(graph);
            }
            return true;
        }
        default:
            return false;
        }
    }

private:
    bool active_;
};

GraphControl::GraphControl(ResourceView * res)
    : Control(res)
{
    itemFilter_ = new GraphItem;
}

void GraphControl::attach()
{
    Graph * gh = static_cast<Graph *>(res_);
    QWeakPointer<int> life(lifeToken_);
    if (!gh->empty()) {
        gh->load().then([this, gh, life]() {
            if (life.isNull())
                return;
            updateGraph(gh);
        });
    }
    item_->installSceneEventFilter(itemFilter_);
}
