#include "resourcepackage.h"
#include "resourcepage.h"
#include "qcomponentcontainer.h"
#include "resourcemanager.h"

extern QComponentContainer & ShowBoard_containter();


ResourcePackage::ResourcePackage(QObject *parent)
    : QObject(parent)
{
    newPage(0);
}

ResourcePage * ResourcePackage::newPage(int at) {
    ResourcePage * page = new ResourcePage(this);
    pages_.insert(at, page);
    return page;
}

void ResourcePackage::addPage(ResourcePage * page)
{
    page->setParent(this);
    pages_.append(page);
}
