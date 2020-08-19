#ifndef RESOURCECACHE_H
#define RESOURCECACHE_H

#include "ShowBoard_global.h"

#include <QList>
#include <QUrl>
#include <QtPromise>

class SHOWBOARD_EXPORT ResourceCacheBase
{
public:
    ResourceCacheBase();

    virtual ~ResourceCacheBase();

public:
    virtual bool empty() = 0;

    virtual QtPromise::QPromise<void> cacheNext(QObject * context) = 0;

private:
    Q_DISABLE_COPY(ResourceCacheBase)
};

class SHOWBOARD_EXPORT ResourceCache : public ResourceCacheBase
{
public:
    ResourceCache();

    virtual ~ResourceCache() override;

public:
    void add(QUrl const & url);

    void remove(QUrl const & url);

    void reset(QList<QUrl> const & tasks);

    void clear();

    void moveFront();

    void moveBackground();

public:
    virtual bool empty() override { return tasks_.isEmpty(); }

    virtual QtPromise::QPromise<void> cacheNext(QObject * context) override;

private:
    Q_DISABLE_COPY(ResourceCache)

    bool background_ = false;
    QList<QUrl> tasks_;

public:
    static void pause();

    static void resume();

private:
    static void loadNext();
};

Q_DECLARE_METATYPE(ResourceCache*)

#endif // RESOURCECACHE_H
