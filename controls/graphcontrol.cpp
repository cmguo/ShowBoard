#include "graphcontrol.h"

#include <QGraphicsItem>
#include <QGraphicsSceneEvent>

QGraphicsItem * GraphControl::itemFilter_ = nullptr;

GraphControl::GraphControl(ResourceView * res)
    : Control(res)
{
}

void GraphControl::attached()
{
    Graph * gh = static_cast<Graph *>(res_);
    if (gh->empty()) {
    } else {
        QWeakPointer<int> life(this->life());
        gh->load().then([this, gh, life]() {
            if (life.isNull())
                return;
            updateGraph(gh);
        });
    }
}

bool GraphControl::event(QEvent *event)
{
    Graph * graph = static_cast<Graph *>(resource());
    switch (event->type()) {
    case QEvent::GraphicsSceneMousePress: {
        QGraphicsSceneMouseEvent * me = static_cast<QGraphicsSceneMouseEvent *>(event);
        graph->addPoint(me->pos());
        updateGraph(graph);
        return true;
    }
    case QEvent::GraphicsSceneMouseMove: {
        QGraphicsSceneMouseEvent * me = static_cast<QGraphicsSceneMouseEvent *>(event);
        graph->movePoint(me->pos());
        updateGraph(graph);
        return true;
    }
    case QEvent::GraphicsSceneMouseRelease: {
        QGraphicsSceneMouseEvent * me = static_cast<QGraphicsSceneMouseEvent *>(event);
        if (graph->commit(me->pos())) {
            graph->finish(bounds().center());
            me->setFlags(static_cast<Qt::MouseEventFlags>(256));
            updateTransform();
        }
        updateGraph(graph);
        return true;
    }
    default:
        return false;
    }
}
