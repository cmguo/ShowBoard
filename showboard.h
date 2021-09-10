#ifndef SHOWBOARD_H
#define SHOWBOARD_H

#include "ShowBoard_global.h"

class QComponentContainer;
class QScreen;

class SHOWBOARD_EXPORT ShowBoard
{
public:
    static QComponentContainer & containter();

    static void init(QScreen * screen);

    static void exit();
};

#endif // SHOWBOARD_H
