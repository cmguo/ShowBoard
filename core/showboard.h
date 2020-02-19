#ifndef SHOWBOARD_H
#define SHOWBOARD_H

#include "ShowBoard_global.h"

class QComponentContainer;

class SHOWBOARD_EXPORT ShowBoard
{
public:
    static QComponentContainer & containter();

    static void exit();
};

#endif // SHOWBOARD_H
