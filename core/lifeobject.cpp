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

static void nopdel(LifeObject *) {}

QWeakPointer<LifeObject> LifeObject::life()
{
    if (lifeToken_.isNull())
        lifeToken_.reset(this, nopdel);
    return lifeToken_;
}

QWeakPointer<LifeObject> LifeObject::uniqeLife()
{
    lifeToken_.reset(this, nopdel);
    return lifeToken_;
}
