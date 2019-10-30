#ifndef GRAPH_H
#define GRAPH_H

#include "core/resourceview.h"

#include <QtPromise>

#include <QList>
#include <QPointF>

class Graph : public ResourceView
{

public:
    virtual QPointF start() const;

    virtual bool empty() const;

    virtual void addPoint(QPointF const & pt);

    virtual void movePoint(QPointF const & pt);

    virtual bool commit(QPointF const & pt);

    virtual void clear();

    virtual void finish(QPointF const & c);

public:
    virtual QtPromise::QPromise<void> load() = 0;

protected:
    Graph(Resource * res);

    Graph(QString const & type);

    Graph(Graph const & o);

protected:
    QList<QPointF> points_;
};

#endif // GRAPH_H
