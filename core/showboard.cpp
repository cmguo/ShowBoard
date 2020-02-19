#include "showboard.h"
#include "workthread.h"

#include <qcomponentcontainer.h>

QComponentContainer & ShowBoard::containter()
{
    static QComponentContainer c;
    return c;
}

void ShowBoard::exit()
{
    WorkThread::quitAll();
}

