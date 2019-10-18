#include "showboard.h"

#include <qcomponentcontainer.h>

QComponentContainer & ShowBoard::containter()
{
    static QComponentContainer c;
    return c;
}

