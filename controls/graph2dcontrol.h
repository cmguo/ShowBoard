#ifndef GRAPH2DCONTROL_H
#define GRAPH2DCONTROL_H

#include "graphcontrol.h"

class Graph2DControl : public GraphControl
{
    Q_OBJECT
public:
    Q_INVOKABLE Graph2DControl(ResourceView *res);

protected:
    virtual QGraphicsItem * create(ResourceView * res) override;

    virtual SelectMode selectTest(QPointF const & point) override;

protected:
    virtual void updateGraph(Graph * gh) override;

    virtual QRectF bounds() override;
};

#endif // GRAPH2DCONTROL_H
