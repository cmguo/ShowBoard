#ifndef UNKNOWNCONTROL_H
#define UNKNOWNCONTROL_H

#include "control.h"

class UnknownControl : public Control
{
public:
    UnknownControl(ResourceView * res);

private:
    virtual QGraphicsItem * create(ResourceView *res) override;

    virtual void attached() override;
};

#endif // UNKNOWNCONTROL_H
