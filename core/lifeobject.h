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

    ~LifeObject() override;

signals:
    void lifeExpired();

protected:
    LifeObject(LifeObject const & o);

    QWeakPointer<LifeObject> life();

    QWeakPointer<LifeObject> uniqeLife();

    void resetLife();

    friend class ResourcePage;
    friend class HttpStream;

private:
    QSharedPointer<LifeObject> lifeToken_;
};

#endif // LIFEOBJECT_H
