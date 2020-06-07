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
        QByteArray types = r.part()->attrMineType();
        for (auto t : types.split(',')) {
            resources_[t] = &r;
        }
    }
}

ResourceView * ResourceFactory::create(Resource *res, const QByteArray &type)
{
    QByteArray type1(type);
    QByteArray type2;
    int n = type.indexOf('.');
    if (n > 0) {
        type1 = type.left(n);
        type2 = type.mid(n + 1);
    }
    QMap<QByteArray, QLazy*>::iterator iter = resources_.find(type1);
    if (iter == resources_.end()) {
        return nullptr;
    }
    res->setProperty(Resource::PROP_SUB_TYPE, type1);
    if (!type2.isEmpty())
        res->setProperty(Resource::PROP_SUB_TYPE2, type2);
    return iter.value()->create<ResourceView>(Q_ARG(Resource *, res));
}

QList<QByteArray> ResourceFactory::resourceTypes() const
{
    return resources_.keys();
}
