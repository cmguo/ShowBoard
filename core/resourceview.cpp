#include "resourceview.h"
#include "resource.h"
#include "resourcepage.h"

ResourceView::ResourceView(Resource * res, Flags flags, Flags clearFlags)
    : res_(res)
    , flags_((DefaultFlags | flags) & ~clearFlags)
{
    QString path = res->url().path();
    name_ = path.mid(path.lastIndexOf('/') + 1);
    res_->setParent(this);
}

ResourceView::ResourceView(ResourceView const & o)
    : res_(new Resource(*o.res_))
    , flags_(o.flags_)
    , transform_(o.transform_)
{
    flags_ &= ~SavedSession;
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

void ResourceView::removeFromPage()
{
    qobject_cast<ResourcePage*>(parent())->removeResource(this);
}

void ResourceView::setSaved()
{
    flags_ |= SavedSession;
}
