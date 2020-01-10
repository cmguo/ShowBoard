#include "resourcemanager.h"
#include "resource.h"
#include "resourcefactory.h"
#include "resourceview.h"
#include "qlazy.hpp"
#include "qcomponentcontainer.h"

#include "resources/resources.h"
#include "showboard.h"

#include <QPair>

ResourceManager * ResourceManager::instance()
{
    static ResourceManager * manager = nullptr;
    if (manager == nullptr)
        manager = ShowBoard::containter().get_export_value<ResourceManager>();
    return manager;
}

static QExport<ResourceManager> export_(QPart::shared);
static QImportMany<ResourceManager, ResourceView> import_resources("resource_types", QPart::nonshared, true);

static QMap<char const *, QPair<ResourceView::Flags, ResourceView::Flags>>& commonResourceTypes()
{
    static QMap<char const *, QPair<ResourceView::Flags, ResourceView::Flags>> map;
    return map;
}

CommonResourceTypes::CommonResourceTypes(const char *types, ResourceView::Flags flags, ResourceView::Flags clearFlags)
{
    commonResourceTypes()[types] = qMakePair(flags, clearFlags);
}

ResourceManager::ResourceManager(QObject *parent)
    : QObject(parent)
{
    //Q_INIT_RESOURCE(ShowBoard);
    auto iter = commonResourceTypes().keyValueBegin();
    auto end = commonResourceTypes().keyValueEnd();
    for (; iter != end; ++iter) {
        for (auto t : QString((*iter).first).split(",", QString::SkipEmptyParts)) {
            commonResources_[t] = qMakePair(static_cast<int>((*iter).second.first),
                                      static_cast<int>((*iter).second.second));
        }
    }
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

static constexpr char DATA_SCHEME_SEP[] = { ';', ',' };

QString ResourceManager::findType(QUrl const & uri, QString& originType, QLazy *&lazy, QPair<int, int> const *&flags) const
{
    std::map<QString, QLazy*>::const_iterator iter = resources_.end();
    std::map<QString, QPair<int, int>>::const_iterator iter2 = commonResources_.end();
    QString type;
    if (uri.scheme() == "data") {
        int n = uri.path().indexOf(DATA_SCHEME_SEP);
        originType = uri.path().left(n);
        type = mapTypes_.value(originType, originType);
        iter = resources_.find(type);
        iter2 = commonResources_.find(type);
    } else {
        originType = uri.scheme();
        type = mapTypes_.value(originType, originType);
        iter = resources_.find(type);
        iter2 = commonResources_.find(type);
        if (iter == resources_.end() && iter2 == commonResources_.end()) {
            int n = uri.path().lastIndexOf('.');
            if (n > 0) {
                originType = uri.path().mid(n + 1);
                type = mapTypes_.value(originType, originType);
                iter = resources_.find(type);
                iter2 = commonResources_.find(type);
            }
        }
    }
    if (iter != resources_.end()) {
        lazy = iter->second;
    }
    if (iter2 != commonResources_.end()) {
        flags = &iter2->second;
    }
    return type;
}

void ResourceManager::mapResourceType(QString const & from, QString const & to)
{
    mapTypes_[from] = to;
}

bool ResourceManager::isExplitSupported(const QUrl &uri) const
{
    QString originType;
    QLazy * lazy = nullptr;
    QPair<int, int> const* flags = nullptr;
    QString type = findType(uri, originType, lazy, flags);
    return lazy || flags;
}

ResourceView * ResourceManager::createResource(QUrl const & uri, QString const & typeHint) const
{
    QString type;
    QString originType;
    QLazy * lazy = nullptr;
    QPair<int, int> const* flags = nullptr;
    if (typeHint.isEmpty()) {
        type = findType(uri, originType, lazy, flags);
    } else {
        type = originType = typeHint;
        auto iter = resources_.find(type);
        if (iter != resources_.end())
            lazy = iter->second;
        auto iter2 = commonResources_.find(type);
        if (iter2 != commonResources_.end())
            flags = &iter2->second;
    }
    ResourceView* rv = nullptr;
    if (lazy == nullptr) {
        if (type.isEmpty())
            type = originType = "unknown";
        if (flags == nullptr) {
            rv = new ResourceView(type, uri);
        } else {
            Resource * res = new Resource(type, uri);
            rv = new ResourceView(res, static_cast<ResourceView::Flags>(flags->first),
                                  static_cast<ResourceView::Flags>(flags->second));
        }
    } else {
        Resource * res = new Resource(type, uri);
        char const * rfactory = lazy->part()->attr(ResourceView::EXPORT_ATTR_FACTORY);
        if (rfactory && strcmp(rfactory, "true") == 0) {
            ResourceFactory * factory = lazy->get<ResourceFactory>();
            return factory->create(res);
        }
        rv = lazy->create<ResourceView>(Q_ARG(Resource*, res));
    }
    if (rv && originType != type) {
        rv->resource()->setProperty(Resource::PROP_ORIGIN_TYPE, originType);
    }
    return rv;
}

ResourceFactory * ResourceManager::getFactory(QString const & type) const
{
    std::map<QString, QLazy*>::const_iterator iter = resources_.find(type);
    if (iter == resources_.end())
        return nullptr;
    char const * rfactory = iter->second->part()->attr(ResourceView::EXPORT_ATTR_FACTORY);
    if (rfactory && strcmp(rfactory, "true") == 0) {
        return iter->second->get<ResourceFactory>();
    } else {
        return nullptr;
    }
}
