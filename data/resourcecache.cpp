#include "resourcecache.h"
#include "core/resource.h"

static QList<ResourceCacheBase*> caches;
// current cache & url
static ResourceCacheBase* workCache = nullptr;
static QUrl workUrl;
// reset lifeToken_ to cancel current task
static QSharedPointer<ResourceCacheLife> lifeToken;
static QList<void*> pauseContexts;

ResourceCacheBase::ResourceCacheBase()
{
    caches.prepend(this);
}

ResourceCacheBase::~ResourceCacheBase()
{
    caches.removeOne(this);
}

ResourceCache::ResourceCache()
{
}

ResourceCache::~ResourceCache()
{
    clear();
}

void ResourceCache::add(const QUrl &url)
{
    tasks_.append(url);
    loadNext();
}

void ResourceCache::remove(const QUrl &url)
{
    tasks_.removeOne(url);
    if (workCache == this && url == workUrl)
        lifeToken.reset();
}

void ResourceCache::reset(const QList<QUrl> &tasks)
{
    tasks_ = tasks;
    if (workCache == this) {
        if (!tasks_.removeOne(workUrl))
            lifeToken.reset();
    }
    loadNext();
}

void ResourceCache::clear()
{
    tasks_.clear();
    if (workCache == this)
        lifeToken.reset();
}

void ResourceCache::moveFront()
{
    int i = caches.indexOf(this);
    if (i > 0)
        caches.prepend(caches.takeAt(i));
}

void ResourceCache::moveBackground()
{
    int i = caches.indexOf(this);
    if (i > 0)
        caches.append(caches.takeAt(i));
}

QtPromise::QPromise<void> ResourceCache::cacheNext(QObject * context)
{
    workUrl = tasks_.takeFirst();
    return Resource::getLocalUrl(context, workUrl).then([](){});
}

void ResourceCacheBase::pause(void * context)
{
    if (!pauseContexts.contains(context))
        pauseContexts.append(context);
    if (lifeToken)
        lifeToken->pause();
}

void ResourceCacheBase::resume(void * context)
{
    if (pauseContexts.removeOne(context) && pauseContexts.isEmpty()) {
        if (lifeToken)
            lifeToken->resume();
        loadNext();
    }
}

void ResourceCacheBase::stop()
{
    pauseContexts.append(&pauseContexts);
    lifeToken.reset();
}

void ResourceCacheBase::loadNext()
{
    if (!pauseContexts.isEmpty() || workCache)
        return;
    for (ResourceCacheBase * c : caches) {
        if (!c->empty()) {
            workCache = c;
            if (!lifeToken)
                lifeToken.reset(new ResourceCacheLife);
            c->cacheNext(lifeToken.get())
                    .tapFail([](std::exception & e) {
                qWarning() << "ResourceCache error:" << e.what();
            }).finally([] () {
                workCache = nullptr;
                loadNext();
            });
            qDebug() << "ResourceCache load" << workUrl;
            return;
        }
    }
}

void ResourceCacheBase::setWorkUrl(const QUrl &url)
{
    workUrl = url;
}

