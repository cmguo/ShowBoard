#ifndef RESOURCECACHE_H
#define RESOURCECACHE_H

#include "ShowBoard_global.h"

#include <QList>

class SHOWBOARD_EXPORT ResourceCache
{
public:
    ResourceCache();

    ~ResourceCache();

public:
    void add(QUrl const & url);

    void remove(QUrl const & url);

    void clear();

    void moveFront();

    void moveBackground();

private:
    Q_DISABLE_COPY(ResourceCache)

    bool background_ = false;
    QList<QUrl> tasks_;

private:
    static void loadNext();

private:
    static ResourceCache* cache_;
    static QList<ResourceCache*> caches_;
};

Q_DECLARE_METATYPE(ResourceCache*)

#endif // RESOURCECACHE_H
