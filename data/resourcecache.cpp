#include "resourcecache.h"
#include "core/resource.h"

static QList<ResourceCache*> caches;
// current cache & url
static ResourceCache* workCache = nullptr;
static QUrl workUrl;
// reset lifeToken_ to cancel current task
static QSharedPointer<QObject> lifeToken;
static bool paused = false;

ResourceCache::ResourceCache()
{
    caches.prepend(this);
}

ResourceCache::~ResourceCache()
{
    clear();
    caches.removeOne(this);
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

void ResourceCache::pause()
{
    paused = true;
    lifeToken.reset();
}

void ResourceCache::resume()
{
    if (paused) {
        paused = false;
        loadNext();
    }
}

void ResourceCache::loadNext()
{
    if (paused || workCache)
        return;
    for (ResourceCache * c : caches) {
        if (!c->tasks_.isEmpty()) {
            workCache = c;
            workUrl = workCache->tasks_.takeFirst();
            qDebug() << "ResourceCache load" << workUrl;
            if (!lifeToken)
                lifeToken.reset(new QObject);
            Resource::getLocalUrl(lifeToken.get(), workUrl)
                    .tapFail([](std::exception & e) {
                qWarning() << "ResourceCache error:" << e.what();
            }).finally([] () {
                workCache = nullptr;
                loadNext();
            });
            return;
        }
    }
}
