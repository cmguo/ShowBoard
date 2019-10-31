#include "graph2dcontrol.h"
#include "resources/graph/graph2d.h"

#include <QGraphicsPathItem>
#include <QBrush>

Graph2DControl::Graph2DControl(ResourceView * res)
    : GraphControl(res)
{
}

QGraphicsItem * Graph2DControl::create(ResourceView * res)
{
    Graph2D * gh = qobject_cast<Graph2D *>(res);
    QGraphicsPathItem * item = new QGraphicsPathItem();
    item->setBrush(QColor(0, 0, 255, 20));
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

Control::SelectMode Graph2DControl::selectTest(QPointF const & point)
{
    return item_->contains(point) ? Select : PassSelect;
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
