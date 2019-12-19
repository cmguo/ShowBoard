#include "resourcepackage.h"
#include "resourcepage.h"
#include "qcomponentcontainer.h"
#include "resourcemanager.h"
#include "resourceview.h"

extern QComponentContainer & ShowBoard_containter();


ResourcePackage::ResourcePackage(QObject *parent)
    : QObject(parent)
    , current_(-1)
{
    globalPage_ = new ResourcePage(this);
}

ResourcePage *ResourcePackage::globalPage()
{
    return globalPage_;
}

ResourcePage * ResourcePackage::newPage()
{
    ResourcePage * page = new ResourcePage(this);
    emit pageCreated(page);
    switchPage(addPage(page));
    return page;
}

ResourcePage * ResourcePackage::currentPage()
{
    return visiblePages_.empty() ? (pages_.empty() ? nullptr : pages_[current_])
                                 : visiblePages_.back();
}

int ResourcePackage::currentNumber()
{
    return current_ + 1;
}

ResourcePage * ResourcePackage::newVirtualPage(ResourceView *mainRes)
{
    ResourcePage * page = new ResourcePage(mainRes, this);
    emit pageCreated(page);
    visiblePages_.push_back(page);
    emit currentPageChanged(page);
    return page;
}

ResourcePage * ResourcePackage::topVirtualPage()
{
    return visiblePages_.empty() ? nullptr : visiblePages_.back();
}

ResourcePage * ResourcePackage::findVirtualPage(const QUrl &mainUrl)
{
    for (ResourcePage * p : visiblePages_) {
        if (p->resources().first()->url() == mainUrl)
            return p;
    }
    for (ResourcePage * p : hiddenPages_) {
        if (p->resources().first()->url() == mainUrl)
            return p;
    }
    return nullptr;
}

void ResourcePackage::showVirtualPage(ResourcePage *page, bool show)
{
    int idx1 = visiblePages_.indexOf(page);
    int idx2 = hiddenPages_.indexOf(page);
    if (idx1 < 0 && idx2 < 0)
        return;
    if (show) {
        if (idx1 == visiblePages_.size() - 1)
            return;
        if (idx1 >= 0)
            visiblePages_.removeAt(idx1);
        if (idx2 >= 0)
            hiddenPages_.removeAt(idx2);
        visiblePages_.push_back(page);
        emit currentPageChanged(page);
    } else {
        if (idx2 >= 0)
            return;
        visiblePages_.removeAt(idx1);
        hiddenPages_.push_back(page);
        emit currentPageChanged(currentPage());
    }
}

void ResourcePackage::toggleVirtualPage(ResourcePage *page)
{
    int idx1 = visiblePages_.indexOf(page);
    int idx2 = hiddenPages_.indexOf(page);
    if (idx1 < 0 && idx2 < 0)
        return;
    if (idx1 >= 0) {
        visiblePages_.removeAt(idx1);
        hiddenPages_.push_back(page);
        emit currentPageChanged(currentPage());
    } else {
        hiddenPages_.removeAt(idx2);
        visiblePages_.push_back(page);
        emit currentPageChanged(page);
    }
}

void ResourcePackage::hideAllVirtualPages()
{
    hiddenPages_.append(visiblePages_);
    visiblePages_.clear();
    emit currentPageChanged(currentPage());
}

void ResourcePackage::removeVirtualPage(ResourcePage *page)
{
    showVirtualPage(page, false);
    hiddenPages_.removeOne(page);
}

void ResourcePackage::gotoFront()
{
    switchPage(0);
}

void ResourcePackage::gotoNext()
{
    if (current_ + 1 < pages_.size())
        switchPage(current_ + 1);
}

void ResourcePackage::gotoPrevious()
{
    if (current_ > 0)
        switchPage(current_ - 1);
}

void ResourcePackage::gotoBack()
{
    switchPage(pages_.size() - 1);
}

void ResourcePackage::switchPage(int page)
{
    current_ = page;
    if (visiblePages_.empty())
        emit currentPageChanged(pages_[current_]);
}

void ResourcePackage::switchPage(ResourcePage * page)
{
    int n = pages_.indexOf(page);
    if (n < 0 || n == current_)
        return;
    switchPage(n);
}

int ResourcePackage::addPage(ResourcePage * page)
{
    page->setParent(this);
    int index = current_ + 1;
    pages_.insert(index, page);
    return index;
}
