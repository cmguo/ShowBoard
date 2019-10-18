#ifndef WHITEPAGE_H
#define WHITEPAGE_H

#include "ShowBoard_global.h"

#include <QObject>

class ResourceView;

class SHOWBOARD_EXPORT ResourcePage : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QList<ResourceView *> const & resources READ resources())
public:
    explicit ResourcePage(QObject *parent = nullptr);

public:
    void addResource(QUrl const & url);

    void addResource(ResourceView * res);

    void removeResource(ResourceView * res);

public:
    QList<ResourceView *> const & resources()
    {
        return resources_;
    }

public slots:

private:
    QList<ResourceView *> resources_;
};

#endif // WHITEPAGE_H
