#include "resourcepackage.h"
#include "resourcepage.h"
#include "qcomponentcontainer.h"
#include "resourcemanager.h"
#include "resourceview.h"

#include <QBrush>

extern QComponentContainer & ShowBoard_containter();


ResourcePackage::ResourcePackage(QObject *parent)
    : QAbstractItemModel(parent)
    , current_(-1)
{
    globalPage_ = new ResourcePage(this);
}

ResourcePage *ResourcePackage::toolPage()
{
    static ResourcePage p;
    return &p;
}

ResourcePage *ResourcePackage::globalPage() const
{
    return globalPage_;
}

ResourcePage * ResourcePackage::newPage(ResourceView * mainRes)
{
    if (mainRes && mainRes->flags().testFlag(ResourceView::VirtualPage))
        return newVirtualPage(mainRes);
    int index = current_ + 1;
    ResourcePage * page = newPage(index, mainRes);
    switchPage(index);
    return page;
}

ResourcePage *ResourcePackage::newPage(int index, ResourceView * mainRes)
{
    ResourcePage * page = new ResourcePage(mainRes, this);
    emit pageCreated(page);
    addPage(index, page);
    return page;
}

ResourcePage * ResourcePackage::currentPage() const
{
    return visiblePages_.empty() ? (current_ < 0 ? nullptr : pages_[current_])
                                 : visiblePages_.back();
}

void ResourcePackage::removePage(ResourcePage *page)
{
    int index = pages_.indexOf(page);
    if (index >= 0) {
        bool cancel = false;
        emit pageRemoving(page, &cancel);
        if (cancel)
            return;
        beginRemoveRows(QModelIndex(), index, index);
        pages_.removeAt(index);
        endRemoveRows();

        if (index == current_) {
            if (index > 0)
                --index;
            current_ = -1;
            switchPage(index);
        }
        emit pageRemoved(page);
        if (index + 1 < pages_.size())
           dataChanged(this->index(index + 1, 0), this->index(pages_.size() - 1, 0));
        emit pageCountChanged(pages_.size());
        delete page;
        return;
    }
    removeVirtualPage(page);
}

int ResourcePackage::currentNumber() const
{
    return current_ + 1;
}

QModelIndex ResourcePackage::currentModelIndex() const
{
    return index(current_, 0);
}

ResourcePage * ResourcePackage::newVirtualPage(ResourceView *mainRes)
{
    ResourcePage * page = new ResourcePage(mainRes, this);
    emit pageCreated(page);
    visiblePages_.push_back(page);
    emit currentPageChanged(page);
    return page;
}

ResourcePage *ResourcePackage::newVirtualPage(const QUrl &mainUrl, QVariantMap const & settings)
{
    ResourcePage * page = new ResourcePage(mainUrl, settings, this);
    emit pageCreated(page);
    visiblePages_.push_back(page);
    emit currentPageChanged(page);
    return page;
}

ResourcePage *ResourcePackage::newVirtualPageOrBringTop(const QUrl &mainUrl, const QVariantMap &settings)
{
    ResourcePage * page = findVirtualPage(mainUrl);
    if (page)
        showVirtualPage(page, true);
    else
        page = newVirtualPage(mainUrl, settings);
    return page;
}

ResourcePage * ResourcePackage::topVirtualPage() const
{
    return visiblePages_.empty() ? nullptr : visiblePages_.back();
}

bool ResourcePackage::containsVisualPage(ResourcePage *page) const
{
    return visiblePages_.contains(page) || hiddenPages_.contains(page);
}

ResourcePage * ResourcePackage::findVirtualPage(const QUrl &mainUrl) const
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
        if (idx1 >= 0) {
            if (idx1 == visiblePages_.size() - 1)
                return;
            visiblePages_.removeAt(idx1);
        } else {
            hiddenPages_.removeAt(idx2);
        }
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
    if (!hiddenPages_.contains(page))
        return;
    bool cancel = false;
    emit pageRemoving(page, &cancel);
    if (cancel)
        return;
    hiddenPages_.removeOne(page);
    emit pageRemoved(page);
    delete page;
}

void ResourcePackage::showVirtualPage(const QUrl &mainUrl, bool show)
{
    showVirtualPage(findVirtualPage(mainUrl), show);
}

void ResourcePackage::toggleVirtualPage(const QUrl &mainUrl)
{
    toggleVirtualPage(findVirtualPage(mainUrl));
}

void ResourcePackage::removeVirtualPage(const QUrl &mainUrl)
{
    removeVirtualPage(findVirtualPage(mainUrl));
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
    if (page == current_)
        return;
    std::swap(current_, page);
    if (page >= 0)
        dataChanged(index(page, 0), index(page, 0));
    dataChanged(index(current_, 0), index(current_, 0));
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

void ResourcePackage::addPage(int index, ResourcePage * page)
{
    beginInsertRows(QModelIndex(), index, index);
    pages_.insert(index, page);
    endInsertRows();
    if (index + 1 < pages_.size())
        dataChanged(this->index(index + 1, 0), this->index(pages_.size() - 1, 0));
    emit pageCountChanged(pages_.size());
}

void ResourcePackage::pageChanged(ResourcePage *page)
{
    int i = pages_.indexOf(page);
    if (i >= 0)
        dataChanged(index(i, 0), index(i, 0));
}

enum PageRole {
    IndexRole = Qt::UserRole + 1,
    ThumbRole
};

QHash<int, QByteArray> ResourcePackage::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IndexRole] = "index";
    roles[ThumbRole] = "thumb";
    return roles;
}

QVariant ResourcePackage::data(const QModelIndex &index, int role) const
{
    if (role == IndexRole)
        return index.row();
    if (role == ThumbRole)
        return QString("%1.%2").arg(index.row()).arg(pages_[index.row()]->thumbnailVersion());
    return QVariant();
}

int ResourcePackage::rowCount(const QModelIndex &parent) const
{
    (void) parent;
    return pages_.size();
}

int ResourcePackage::columnCount(const QModelIndex &parent) const
{
    (void) parent;
    return 1;
}

QModelIndex ResourcePackage::parent(const QModelIndex &child) const
{
    (void) child;
    return QModelIndex();
}

QModelIndex ResourcePackage::index(int row, int column, const QModelIndex &parent) const
{
    (void) parent;
    return createIndex(row, column, pages_[row]);
}
