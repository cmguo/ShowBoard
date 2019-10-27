#ifndef GRAPH2DCONTROL_H
#define GRAPH2DCONTROL_H

#include "graphcontrol.h"

class Graph2DControl : public GraphControl
{
    Q_OBJECT
public:
    Q_INVOKABLE Graph2DControl(ResourceView *res);

protected:
    virtual QGraphicsItem * create(ResourceView * res);

    virtual void updateGraph(Graph * gh) override;

    virtual QRectF bounds() override;
};

#endif // GRAPH2DCONTROL_H
