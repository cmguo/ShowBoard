#include "resourcefactory.h"
#include "resource.h"

#include <qlazy.hpp>

ResourceFactory::ResourceFactory(QObject *parent)
    : QObject(parent)
{

}

void ResourceFactory::onComposition()
{
    for (auto & r : resource_types_) {
        QString types = r.part()->attr(ResourceView::EXPORT_ATTR_TYPE);
        for (auto t : types.split(",", QString::SkipEmptyParts)) {
            resources_[t] = &r;
        }
    }
}

ResourceView * ResourceFactory::create(Resource *res, const QString &type)
{
    QMap<QString, QLazy*>::iterator iter = resources_.find(type);
    if (iter == resources_.end()) {
        return nullptr;
    }
    res->setProperty(Resource::PROP_SUB_TYPE, type);
    return iter.value()->create<ResourceView>(Q_ARG(Resource *, res));
}

QList<QString> ResourceFactory::resourceTypes() const
{
    return resources_.keys();
}
