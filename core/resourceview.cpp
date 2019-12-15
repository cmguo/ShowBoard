#include "resourceview.h"
#include "resource.h"
#include "resourcepage.h"

#include <QMetaMethod>

ResourceView::ResourceView(Resource * res, Flags flags, Flags clearFlags)
    : res_(res)
    , flags_((DefaultFlags | flags) & ~clearFlags)
{
    QString path = res->url().path();
    name_ = path.mid(path.lastIndexOf('/') + 1);
    res_->setParent(this);
}

ResourceView::ResourceView(QString const & type, QUrl const & url)
    : ResourceView(new Resource(type, url), {}, CanCopy) // not copyable
{
}

ResourceView::ResourceView(ResourceView const & o)
    : res_(new Resource(*o.res_))
    , flags_(o.flags_)
    , transform_(o.transform_)
{
    //flags_ &= ~SavedSession;
    res_->setParent(this);
    transform_.translate({60, 60});
}

ResourceView * ResourceView::clone() const
{

    QObject * clone = metaObject()->newInstance(QGenericArgument(metaObject()->className(), this));
    if (clone)
        return qobject_cast<ResourceView*>(clone);
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

void ResourceView::setFlag(Flag f, bool on)
{
    flags_.setFlag(f, on);
}
