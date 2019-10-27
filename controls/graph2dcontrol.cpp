#include "graph2dcontrol.h"
#include "resources/graph/graph2d.h"

#include <QGraphicsPathItem>

Graph2DControl::Graph2DControl(ResourceView * res)
    : GraphControl(res)
{
}

QGraphicsItem * Graph2DControl::create(ResourceView * res)
{
    Graph2D * gh = qobject_cast<Graph2D *>(res);
    QGraphicsPathItem * item = new QGraphicsPathItem();
    QWeakPointer<int> life(lifeToken_);
    if (!gh->empty()) {
        gh->load().then([item, gh, life]() {
            if (life.isNull())
                return;
            item->setPath(gh->path());
        });
    }
    return item;
}

void Graph2DControl::updateGraph(Graph * gh)
{
    QGraphicsPathItem * item = static_cast<QGraphicsPathItem *>(item_);
    item->setPath(qobject_cast<Graph2D *>(gh)->path());
}

QRectF Graph2DControl::bounds()
{
    return item_->boundingRect();
}
