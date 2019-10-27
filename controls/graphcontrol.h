#ifndef GRAPHCONTROL_H
#define GRAPHCONTROL_H

#include "control.h"
#include "resources/graph.h"

class GraphItem;

class GraphControl : public Control
{
public:
    GraphControl(ResourceView * res);

protected:
    friend class GraphItem;

    virtual void attach() override;

    virtual void updateGraph(Graph * gh) = 0;

    virtual QRectF bounds() = 0;

private:
    QGraphicsItem * itemFilter_;
};

#endif // GRAPHCONTROL_H
