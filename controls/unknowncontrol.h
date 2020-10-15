#ifndef UNKNOWNCONTROL_H
#define UNKNOWNCONTROL_H

#include "core/control.h"

class UnknownControl : public Control
{
public:
    UnknownControl(ResourceView * res);

private:
    virtual ControlView * create(ControlView * parent) override;

    virtual void attached() override;
};

#endif // UNKNOWNCONTROL_H
