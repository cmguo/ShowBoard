#include "resource.h"
#include "resourcecache.h"

ResourceCache* ResourceCache::cache_ = nullptr;
QList<ResourceCache*> ResourceCache::caches_;

ResourceCache::ResourceCache()
{
    caches_.prepend(this);
}

ResourceCache::~ResourceCache()
{
    caches_.removeOne(this);
}

void ResourceCache::add(const QUrl &url)
{
    tasks_.append(url);
    loadNext();
}

void ResourceCache::remove(const QUrl &url)
{
    tasks_.removeOne(url);
}

void ResourceCache::clear()
{
    tasks_.clear();
}

void ResourceCache::moveFront()
{
    int i = caches_.indexOf(this);
    if (i > 0)
        caches_.prepend(caches_.takeAt(i));
}

void ResourceCache::moveBackground()
{
    int i = caches_.indexOf(this);
    if (i > 0)
        caches_.append(caches_.takeAt(i));
}

void ResourceCache::loadNext()
{
    cache_ = nullptr;
    for (ResourceCache * c : caches_) {
        if (!c->tasks_.isEmpty()) {
            cache_ = c;
            QUrl url = cache_->tasks_.takeFirst();
            qDebug() << "ResourceCache load" << url;
            Resource::getLocalUrl(url).then([] (QUrl const &) {
                loadNext();
            });
            return;
        }
    }
}
