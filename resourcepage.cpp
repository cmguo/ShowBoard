#include "resourcepage.h"
#include "resourceview.h"
#include "resourcemanager.h"

ResourcePage::ResourcePage(QObject *parent)
    : QObject(parent)
{
}

void ResourcePage::addResource(QUrl const & url)
{
    ResourceView * rv = ResourceManager::instance()->createResource(url);
    addResource(rv);
}

void ResourcePage::addResource(ResourceView * res)
{
    resources_.append(res);
    res->setParent(this);
}

void ResourcePage::removeResource(ResourceView * res)
{
    resources_.removeOne(res);
    delete res;
}
