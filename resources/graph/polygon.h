#ifndef POLYGON_H
#define POLYGON_H

#include "graph2d.h"

class Polygon : public Graph2D
{
    Q_OBJECT
public:
    Q_INVOKABLE Polygon(Resource * res);

    Polygon(QPointF const & pt);

    Polygon(Polygon const & o);

public:
    virtual bool commit(const QPointF &pt) override;
};

#endif // POLYGON_H
