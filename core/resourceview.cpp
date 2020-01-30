#include "resourceview.h"
#include "resource.h"
#include "resourcetransform.h"
#include "resourcepage.h"

#include <QMetaMethod>

char const * ResourceView::EXPORT_ATTR_TYPE = "rtype";
char const * ResourceView::EXPORT_ATTR_FACTORY = "rfactory";


ResourceView::ResourceView(Resource * res, Flags flags, Flags clearFlags)
    : res_(res)
    , flags_((DefaultFlags | flags) & ~clearFlags)
    , transform_(new ResourceTransform(this))
{
    QString path = res->url().path();
    if (path.size() > 1)
        name_ = path.mid(path.lastIndexOf('/') + 1);
    else
        name_ = res->url().toString();
    res_->setParent(this);
}

ResourceView::ResourceView(QString const & type, QUrl const & url)
    : ResourceView(new Resource(type, url), {}, CanCopy) // not copyable
{
}

ResourceView::~ResourceView()
{
}

ResourceView::ResourceView(ResourceView const & o)
    : res_(new Resource(*o.res_))
    , flags_(o.flags_)
    , transform_(new ResourceTransform(*o.transform_, this))
{
    //flags_ &= ~SavedSession;
    res_->setParent(this);
    //transform_->translate({60, 60});
    for (QByteArray & k : o.dynamicPropertyNames())
        setProperty(k, o.property(k));
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
    if (flags_ & (ResourceView::ZOrderFlags))
        return;
    qobject_cast<ResourcePage*>(parent())->moveResourceBack(this);
}

bool ResourceView::canMoveTop()
{
    if (flags_ & (ResourceView::ZOrderFlags))
        return false;
    return qobject_cast<ResourcePage*>(parent())->nextNormalResource(this);
}

void ResourceView::removeFromPage()
{
    qobject_cast<ResourcePage*>(parent())->removeResource(this);
}

void ResourceView::setSaved()
{
    flags_ |= SavedSession;
}

