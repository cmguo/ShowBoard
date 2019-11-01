#include "resourceview.h"
#include "resource.h"
#include "resourcepage.h"

static void nopdel(int *) {}

ResourceView::ResourceView(Resource * res, Flags flags, Flags clearFlags)
    : res_(res)
    , flags_((DefaultFlags | flags) & ~clearFlags)
    , lifeToken_(reinterpret_cast<int*>(1), nopdel)
{
    res_->setParent(this);
}

ResourceView::ResourceView(ResourceView const & o)
    : res_(new Resource(*o.res_))
    , flags_(o.flags_)
    , transform_(o.transform_)
    , lifeToken_(reinterpret_cast<int*>(1), nopdel)
{
    res_->setParent(this);
    transform_.translate(20 / transform_.m11(), 20 / transform_.m22());
}

ResourceView * ResourceView::clone() const
{
    return new ResourceView(*this);
}

QUrl const & ResourceView::url() const
{
    return res_->url();
}

void ResourceView::moveTop()
{
    qobject_cast<ResourcePage*>(parent())->moveResourceBack(this);
}
