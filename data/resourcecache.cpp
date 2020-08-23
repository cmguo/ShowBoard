#include "resourcecache.h"
#include "core/resource.h"

static QList<ResourceCacheBase*> caches;
// current cache & url
static ResourceCacheBase* workCache = nullptr;
static QUrl workUrl;
// reset lifeToken_ to cancel current task
static QSharedPointer<QObject> lifeToken;
static bool paused = false;

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

void ResourceCacheBase::pause()
{
    paused = true;
    lifeToken.reset();
}

void ResourceCacheBase::resume()
{
    if (paused) {
        paused = false;
        loadNext();
    }
}

void ResourceCacheBase::stop()
{
    pause();
}

void ResourceCacheBase::loadNext()
{
    if (paused || workCache)
        return;
    for (ResourceCacheBase * c : caches) {
        if (!c->empty()) {
            workCache = c;
            if (!lifeToken)
                lifeToken.reset(new QObject);
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

