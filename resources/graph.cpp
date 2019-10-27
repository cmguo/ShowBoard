#include "graph.h"
#include "resource.h"

Graph::Graph(Resource * res)
    : ResourceView(res)
{
}

Graph::Graph(QString const & type)
    : ResourceView(new Resource(type))
{
}

Graph::Graph(Graph const & o) : ResourceView(o)
{
    points_ = o.points_;
}

QPointF Graph::start() const
{
    return points_.front();
}

bool Graph::empty() const
{
    return points_.isEmpty();
}

void Graph::addPoint(const QPointF &pt)
{
    points_.append(pt);
}

void Graph::movePoint(const QPointF &pt)
{
    if (points_.size() > 1)
        points_.pop_back();
    points_.append(pt);
}

bool Graph::commit(const QPointF & pt)
{
    (void) pt;
    return false;
}

void Graph::clear()
{
    points_.clear();
}

void Graph::finsh(const QPointF &c)
{
    for (int i = 0; i < points_.size(); ++i)
    {
        QPointF & pt = points_[i];
        pt -= c;
    }
}
