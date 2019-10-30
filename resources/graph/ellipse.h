#ifndef ELLIPSE_H
#define ELLIPSE_H

#include "graph2d.h"

class Ellipse : public Graph2D
{
    Q_OBJECT
public:
    Q_INVOKABLE Ellipse(Resource * res);

    Ellipse(QPointF const & pt);

    Ellipse(Ellipse const & o);

public:
    virtual QPainterPath path() override;
};

#endif // ELLIPSE_H
