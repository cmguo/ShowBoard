#ifndef LIFEOBJECT_H
#define LIFEOBJECT_H

#include "ShowBoard_global.h"

#include <QObject>
#include <QSharedPointer>

class SHOWBOARD_EXPORT LifeObject : public QObject
{
    Q_OBJECT
public:
    explicit LifeObject(QObject *parent = nullptr);

protected:
    LifeObject(LifeObject const & o);

    QWeakPointer<int> life();

private:
    QSharedPointer<int> lifeToken_;
};

#endif // LIFEOBJECT_H
