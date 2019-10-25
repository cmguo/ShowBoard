#include "resourceview.h"
#include "resource.h"

ResourceView::ResourceView(Resource * res, Flags flags, Flags clearFlags)
    : res_(res)
    , flags_((DefaultFlags | flags) & ~clearFlags)
{
    res_->setParent(this);
}

ResourceView::ResourceView(ResourceView const & o)
    : res_(new Resource(*o.res_))
    , flags_(o.flags_)
    , transform_(o.transform_)
{
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

