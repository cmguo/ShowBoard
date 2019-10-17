#include "resourcemanager.h"
#include "resource.h"
#include "resourceview.h"
#include "qlazy.hpp"

#include "resources/resources.h"
#include "controls/controls.h"

static QExport<ResourceManager> export_(QPart::shared);
static QImportMany<ResourceManager, ResourceView> import_resources("resource_types", QPart::nonshared, true);

ResourceManager::ResourceManager(QObject *parent)
    : QObject(parent)
{
}

void ResourceManager::onComposition()
{
    for (auto & r : resource_types_) {
        QString types = r.part()->attr(ResourceView::EXPORT_ATTR_TYPE);
        for (auto t : types.split(",", QString::SkipEmptyParts)) {
            resources_[t] = &r;
        }
    }
}

void ResourceManager::mapResourceType(QString const & from, QString const & to)
{
    mapTypes_[from] = to;
}

static constexpr char DATA_SCHEME_SEP[] = { ';', ',' };

ResourceView * ResourceManager::CreateResource(QUrl const & uri)
{
    std::map<QString, QLazy*>::iterator iter = resources_.end();
    QString type = "";
    if (uri.scheme() == "data")
    {
        int n = uri.path().indexOf(DATA_SCHEME_SEP);
        type = uri.path().left(n);
        type = mapTypes_.value(type, type);
        iter = resources_.find(type);
    }
    else
    {
        type = uri.scheme();
        type = mapTypes_.value(type, type);
        iter = resources_.find(type);
        if (iter == resources_.end())
        {
            int n = uri.path().lastIndexOf('.');
            if (n > 0) {
                type = uri.path().mid(n + 1);
                type = mapTypes_.value(type, type);
                iter = resources_.find(type);
            }
        }
    }
    if (iter == resources_.end()) {
        if (type != "")
            return new ResourceView(new Resource(type, uri));
        else
            return nullptr;
    }
    Resource * res = new Resource(iter->first, uri);
    return iter->second->create<ResourceView>(Q_ARG(Resource *, res));
}
