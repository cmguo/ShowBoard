#include "showboard.h"
#include "workthread.h"
#include "views/qsshelper.h"

#include <qcomponentcontainer.h>

QComponentContainer & ShowBoard::containter()
{
    static QComponentContainer c;
    static bool ok = QssHelper::applyToAllStylesheet();
    (void) ok;
    return c;
}

void ShowBoard::exit()
{
    WorkThread::quitAll();
}

