#ifndef BASETOOL_H
#define BASETOOL_H

#include "control.h"

class BaseTool : public Control
{
public:
    BaseTool(Flags flags = None, Flags clearFlags = None);

private:
    virtual void attaching() override;

    virtual void detached() override;
};

#endif // BASETOOL_H
