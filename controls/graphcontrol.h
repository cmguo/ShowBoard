#ifndef GRAPHCONTROL_H
#define GRAPHCONTROL_H

#include "core/control.h"
#include "resources/graph.h"

class GraphItem;

class GraphControl : public Control
{
public:
    GraphControl(ResourceView * res);

protected:
    friend class GraphItem;

    virtual void attached() override;

    virtual void updateGraph(Graph * gh) = 0;

    virtual QRectF bounds() = 0;

protected:
    virtual bool event(QEvent *event) override;

private:
    static QGraphicsItem * itemFilter_;
};

#endif // GRAPHCONTROL_H
