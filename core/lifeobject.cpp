#include "lifeobject.h"

LifeObject::LifeObject(QObject *parent)
    : QObject(parent)
    , lifeToken_(nullptr)
{
}

static void nopdel(int *) {}

QWeakPointer<int> LifeObject::life()
{
    lifeToken_.reset(reinterpret_cast<int*>(1), nopdel);
    return lifeToken_;
}
