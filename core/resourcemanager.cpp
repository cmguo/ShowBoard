#include "resourcemanager.h"
#include "resource.h"
#include "resourcefactory.h"
#include "resourceview.h"
#include "qlazy.hpp"
#include "qcomponentcontainer.h"
#include "data/dataprovider.h"

#include "resources/resources.h"
#include "showboard.h"

#include <QPair>

ResourceManager * ResourceManager::instance()
{
    static ResourceManager * manager = nullptr;
    if (manager == nullptr)
        manager = ShowBoard::containter().getExportValue<ResourceManager>();
    return manager;
}

static QExport<ResourceManager> export_(QPart::shared);
static QImportMany<ResourceManager, ResourceView> import_resources("resourceTypes", QPart::nonshared);

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
    syncCommonTypes();
    // QMimeData types
    mapResourceType("x-qt-image", "image");
}

void ResourceManager::onComposition()
{
    for (auto & r : resourceTypes_) {
        QByteArray types = r.part()->attrMineType();
        for (auto t : types.split(',')) {
            resources_[t.trimmed()] = &r;
        }
    }
    for (auto & r : providerTypes_) {
        QByteArray types = r.part()->attrMineType();
        for (auto t : types.split(',')) {
            providers_[t.trimmed()] = &r;
        }
    }
}

void ResourceManager::syncCommonTypes()
{
    auto iter = commonResourceTypes().keyValueBegin();
    auto end = commonResourceTypes().keyValueEnd();
    for (; iter != end; ++iter) {
        for (auto t : QByteArray((*iter).first).split(',')) {
            commonResources_[t] = qMakePair(static_cast<int>((*iter).second.first),
                                      static_cast<int>((*iter).second.second));
        }
    }
    commonResourceTypes().clear();
}

QByteArray ResourceManager::findType(QUrl const & uri, QByteArray& originType, QLazy *&lazy, QPair<int, int> const *&flags) const
{
    std::map<QByteArray, QLazy*>::const_iterator iter = resources_.end();
    std::map<QByteArray, QPair<int, int>>::const_iterator iter2 = commonResources_.end();
    QByteArray type;
    if (uri.scheme() == "data") {
        QByteArray url = uri.toEncoded();
        int n = url.indexOf(';', 5);
        if (n < 0) n = url.indexOf(',', 5);
        if (n > 0) {
            QList<QByteArray> types = url.mid(5, n - 5).toLower().split('/');
            originType = types.back();
            type = mapTypes_.value(originType, originType);
            iter = resources_.find(type);
            iter2 = commonResources_.find(type);
            if (iter == resources_.end() && iter2 == commonResources_.end()) {
                originType = types.front();
                type = mapTypes_.value(originType, originType);
                iter = resources_.find(type);
                iter2 = commonResources_.find(type);
            }
        }
    } else {
        originType = uri.scheme().toUtf8().toLower();
        type = mapTypes_.value(originType, originType);
        iter = resources_.find(type);
        iter2 = commonResources_.find(type);
        if (iter == resources_.end() && iter2 == commonResources_.end()) {
            int n = uri.path().lastIndexOf('.');
            if (n > 0) {
                originType = uri.path().mid(n + 1).toUtf8().toLower();
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
    return type.toLower();
}

void ResourceManager::mapResourceType(QByteArray const & from, QByteArray const & to)
{
    mapTypes_[from] = to;
}

bool ResourceManager::isExplitSupported(const QUrl &uri) const
{
    QByteArray originType;
    QLazy * lazy = nullptr;
    QPair<int, int> const* flags = nullptr;
    findType(uri, originType, lazy, flags);
    return lazy || flags;
}

ResourceView * ResourceManager::createResource(QUrl const & uri, QByteArray const & typeHint) const
{
    QByteArray type;
    QByteArray originType;
    QLazy * lazy = nullptr;
    QPair<int, int> const* flags = nullptr;
    const_cast<ResourceManager*>(this)->syncCommonTypes();
    if (typeHint.isEmpty()) {
        type = findType(uri, originType, lazy, flags);
    } else {
        type = originType = mapTypes_.value(typeHint, typeHint);
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
    if (rv == nullptr)
        qWarning() << "ResourceManager::createResource failed" << uri << typeHint;
    return rv;
}

ResourceFactory * ResourceManager::getFactory(QByteArray const & type) const
{
    std::map<QByteArray, QLazy*>::const_iterator iter = resources_.find(type);
    if (iter == resources_.end())
        return nullptr;
    char const * rfactory = iter->second->part()->attr(ResourceView::EXPORT_ATTR_FACTORY);
    if (rfactory && strcmp(rfactory, "true") == 0) {
        return iter->second->get<ResourceFactory>();
    } else {
        return nullptr;
    }
}
