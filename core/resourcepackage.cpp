#include "resourcepackage.h"
#include "resourcepage.h"
#include "qcomponentcontainer.h"
#include "resourcemanager.h"
#include "resourceview.h"
#include "resourcerecord.h"

#include <QBrush>

extern QComponentContainer & ShowBoard_containter();


ResourcePackage::ResourcePackage(QObject *parent)
    : QAbstractItemModel(parent)
    , current_(-1)
    , records_(new ResourceRecordSet(this))
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
    RecordMergeScope rs(records_);
    if (mainRes && mainRes->flags().testFlag(ResourceView::VirtualPage))
        return newVirtualPage(mainRes);
    int index = current_ + 1;
    ResourcePage * page = newPage(index, mainRes);
    switchPage(index);
    return page;
}

ResourcePage *ResourcePackage::newPage(int index, ResourceView * mainRes)
{
    RecordMergeScope rs(records_);
    ResourcePage * page = new ResourcePage(mainRes, this);
    rs.add(makeDestructRecord([this, page] (bool undo) {
        if (undo) {
            emit pageRemoved(page);
            delete page;
        }
    }));
    emit pageCreated(page);
    addPage(index, page);
    return page;
}

ResourcePage * ResourcePackage::currentPage() const
{
    return visiblePages_.empty() ? (current_ < 0 ? nullptr : pages_[current_])
                                 : visiblePages_.back();
}

ResourcePage *ResourcePackage::newPage(const QUrl &mainUrl, QVariantMap const & settings)
{
    RecordMergeScope rs(records_);
    ResourcePage * page = new ResourcePage(mainUrl, settings, this);
    rs.add(makeDestructRecord([this, page] (bool undo) {
        if (undo) {
            emit pageRemoved(page);
            delete page;
        }
    }));
    emit pageCreated(page);
    if (page->isVirtualPage()) {
        hiddenPages_.push_back(page);
        showVirtualPage(page);
    } else {
        int index = current_ + 1;
        addPage(index, page);
        switchPage(index);
    }
    return page;
}

ResourcePage *ResourcePackage::newPageOrSwitchTo(const QUrl &mainUrl, const QVariantMap &settings)
{
    ResourcePage * page = findPage(mainUrl);
    if (page == nullptr) {
        page = newPage(mainUrl, settings);
    } else {
        switchPage(page);
    }
    return page;
}

ResourcePage *ResourcePackage::findPage(const QUrl &mainUrl) const
{
    ResourcePage * page = findVirtualPage(mainUrl);
    if (page)
        return page;
    for (ResourcePage * p : pages_)
        if (p->isIndependentPage() && p->mainResource()->url() == mainUrl)
            return p;
    return nullptr;
}

void ResourcePackage::removePage(ResourcePage *page)
{
    RecordMergeScope rs(records_);
    int index = pages_.indexOf(page);
    if (index < 0)
        return removeVirtualPage(page);
    bool cancel = false;
    emit pageRemoving(page, &cancel);
    if (cancel)
        return;
    if (index == current_) {
        int newIndex = index;
        if (newIndex > 0)
            --newIndex;
        else
            current_ = -1;
        switchPage(newIndex);
    }
    removePage(index);
    rs.add(makeDestructRecord([this, page] (bool undo) {
        if (!undo) {
            emit pageRemoved(page);
            delete page;
        }
    }));
    return;
}

ResourcePage *ResourcePackage::prevPage(ResourcePage *page) const
{
    if (current_ >= 0 && page == pages_[current_])
        return current_ == 0 ? nullptr : pages_[current_ - 1];
    int n = pages_.indexOf(page);
    return n > 0 ? pages_[n - 1] : nullptr;
}

ResourcePage *ResourcePackage::nextPage(ResourcePage *page) const
{
    if (current_ >= 0 && page == pages_[current_])
        return current_ + 1 == pages_.size() ? nullptr : pages_[current_ + 1];
    int n = pages_.indexOf(page) + 1;
    return (n >= 0 && n < pages_.size()) ? pages_[n] : nullptr;
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
    RecordMergeScope rs(records_);
    int idx1 = visiblePages_.indexOf(page);
    int idx2 = hiddenPages_.indexOf(page);
    if (idx1 < 0 && idx2 < 0)
        return;
    if (rs)
        rs.add(makeFunctionRecord(
                          [this, page, show] () { showVirtualPage(page, !show); },
                          [this, page, show] () { showVirtualPage(page, !show); }
        ));
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
    RecordMergeScope rs(records_);
    if (rs)
        rs.add(makeFunctionRecord(
                          [this, page] () { toggleVirtualPage(page); },
                          [this, page] () { toggleVirtualPage(page); }
        ));
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
    if (visiblePages_.empty())
        return;
    RecordMergeScope rs(records_);
    if (rs)
        rs.add(makeFunctionRecord(
                          [this, n = hiddenPages_.size()] () {
                                visiblePages_ = hiddenPages_.mid(n);
                                hiddenPages_.erase(hiddenPages_.begin() + n, hiddenPages_.end());
                                emit currentPageChanged(currentPage());
                            },
                          [this] () { hideAllVirtualPages(); }
        ));
    hiddenPages_.append(visiblePages_);
    visiblePages_.clear();
    emit currentPageChanged(currentPage());
}

void ResourcePackage::removeVirtualPage(ResourcePage *page)
{
    RecordMergeScope rs(records_);
    showVirtualPage(page, false);
    if (!hiddenPages_.contains(page)) {
        rs.drop();
        return;
    }
    bool cancel = false;
    emit pageRemoving(page, &cancel);
    if (cancel)
        return;
    if (rs)
        rs.add(makeFunctionRecord(
                          [this, n = hiddenPages_.size()] () {
                                visiblePages_ = hiddenPages_.mid(n);
                                hiddenPages_.erase(hiddenPages_.begin() + n, hiddenPages_.end());
                                emit currentPageChanged(currentPage());
                            },
                          [this] () { hideAllVirtualPages(); }
        ));
    hiddenPages_.removeOne(page);
    rs.add(makeDestructRecord([this, page] (bool undo) {
        if (!undo) {
            emit pageRemoved(page);
            delete page;
        }
    }));
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
    RecordMergeScope rs(records_);
    if (rs)
        rs.add(makeFunctionRecord(
                          [this, page = current_] () { switchPage(page); },
                          [this, page] () { switchPage(page); }
        ));
    std::swap(current_, page);
    if (visiblePages_.empty())
        emit currentPageChanged(pages_[current_]);
}

void ResourcePackage::switchPage(ResourcePage * page)
{
    RecordMergeScope rs(records_);
    int n = pages_.indexOf(page);
    if (n < 0) {
        showVirtualPage(page, true);
    } else if (n != current_) {
        switchPage(n);
    }
}

void ResourcePackage::addPage(int index, ResourcePage * page)
{
    RecordMergeScope rs(records_);
    if (rs)
        rs.add(makeFunctionRecord(
                          [this, index] () { removePage(index); },
                          [this, index, page] () { addPage(index, page); }
        ));
    beginInsertRows(QModelIndex(), index, index);
    pages_.insert(index, page);
    endInsertRows();
    emit pageCountChanged(pages_.size());
}

void ResourcePackage::removePage(int index)
{
    RecordMergeScope rs(records_);
    ResourcePage * page = pages_[index];
    if (rs)
        rs.add(makeFunctionRecord(
                          [this, index] () { removePage(index); },
                          [this, index, page] () { addPage(index, page); }
        ));
    beginRemoveRows(QModelIndex(), index, index);
    pages_.removeAt(index);
    endRemoveRows();
    emit pageRemoved(page);
    emit pageCountChanged(pages_.size());
}

void ResourcePackage::pageChanged(ResourcePage *page)
{
    int i = pages_.indexOf(page);
    if (i >= 0)
        dataChanged(index(i, 0), index(i, 0));
}

enum PageRole {
    ThumbRole = Qt::UserRole + 1,
};

QHash<int, QByteArray> ResourcePackage::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[ThumbRole] = "thumb";
    return roles;
}

QVariant ResourcePackage::data(const QModelIndex &index, int role) const
{
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
