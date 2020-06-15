#include "lifeobject.h"

#include <QVariant>

LifeObject::LifeObject(QObject *parent)
    : QObject(parent)
    , lifeToken_(nullptr)
{
}

LifeObject::LifeObject(const LifeObject &o)
    : lifeToken_(nullptr)
{
    for (QByteArray & k : o.dynamicPropertyNames())
        setProperty(k, o.property(k));
}

static void nopdel(int *) {}

QWeakPointer<int> LifeObject::life()
{
    if (lifeToken_.isNull())
        lifeToken_.reset(reinterpret_cast<int*>(1), nopdel);
    return lifeToken_;
}

QWeakPointer<int> LifeObject::uniqeLife()
{
    lifeToken_.reset(reinterpret_cast<int*>(1), nopdel);
    return lifeToken_;
}
