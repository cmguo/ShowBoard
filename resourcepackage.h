#ifndef RESOURCEPACKAGE_H
#define RESOURCEPACKAGE_H

#include "ShowBoard_global.h"

#include <QObject>

class ResourcePage;

class SHOWBOARD_EXPORT ResourcePackage : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QList<ResourcePage *> pages READ pages())
public:
    explicit ResourcePackage(QObject *parent = nullptr);

public:
    ResourcePage * newPage(int at);

    QList<ResourcePage *> const & pages() const
    {
        return pages_;
    }

protected:
    void addPage(ResourcePage * page);

private:
    QList<ResourcePage *> pages_;
};

#endif // RESOURCEPACKAGE_H
