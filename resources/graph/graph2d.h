#ifndef GRAPH2D_H
#define GRAPH2D_H

#include "resources/graph.h"
#include "core/resourcefactory.h"

#include <QPainterPath>

class Graph2D : public Graph
{
    Q_OBJECT
public:
    Graph2D(Resource * res);

    Graph2D(QPointF const & pt);

    Graph2D(Graph2D const & o);

public:
    virtual bool empty() const override;

    virtual QtPromise::QPromise<void> load() override;

public:
    virtual bool commit(const QPointF &pt) override;

    virtual QPainterPath path();
};

class Graph2DFactory : ResourceFactory
{
    Q_OBJECT
public:
    Q_INVOKABLE Graph2DFactory();

public:
    virtual ResourceView * create(Resource * res) override;

    virtual QUrl newUrl(const QString &type) const override;
};

#define REGISTER_GRAPH_2D(ctype, type) \
    REGISTER_RESOURCE_VIEW_WITH_FACTORY(Graph2D, ctype, type)

#endif // GRAPH2D_H
