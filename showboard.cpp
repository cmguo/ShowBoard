#include "showboard.h"
#include "core/workthread.h"
#include "views/qsshelper.h"

#include <qcomponentcontainer.h>

QComponentContainer & ShowBoard::containter()
{
    static QComponentContainer c;
    init();
    return c;
}

void ShowBoard::init()
{
    static bool done = false;
    if (done) return;
    done = true;
    QssHelper::applyToAllStylesheet();
}

void ShowBoard::exit()
{
    WorkThread::quitAll();
}

