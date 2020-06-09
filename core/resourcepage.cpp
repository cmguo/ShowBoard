#include "resourcepage.h"
#include "resource.h"
#include "resourcepackage.h"
#include "resourceview.h"
#include "resourcemanager.h"

ResourcePage::ResourcePage(QObject *parent)
    : ResourcePage(nullptr, parent)
{
}

ResourcePage::ResourcePage(const QUrl &mainUrl, QVariantMap const & settings, QObject *parent)
    : ResourcePage(createResource(mainUrl, settings), parent)
{
}

ResourcePage::ResourcePage(ResourceView* mainRes, QObject *parent)
    : QAbstractItemModel(parent)
    , canvasView_(nullptr)
    , currentSubPage_(nullptr)
    , thumbnailVersion_(0)
{
    if (parent)
        thumbnail_ = ResourcePackage::toolPage()->thumbnail();
    bool largeCanvas = mainRes && (mainRes->flags().testFlag(ResourceView::LargeCanvas));
    if (largeCanvas) {
        canvasView_ = new ResourceView(new Resource("whitecanvas", QUrl("whitecanvas:///")),
                                       ResourceView::BottomMost, ResourceView::DefaultFlags);
        canvasView_->setParent(this);
    }
    if (mainRes)
        addResource(mainRes);
}

ResourceView * ResourcePage::addResource(QUrl const & url, QVariantMap const & settings)
{
    ResourceView * rv = createResource(url, settings);
    if (rv->resource()->type().endsWith("tool")) {
        ResourcePackage::toolPage()->addResource(rv);
        return rv;
    }
    if (rv->flags().testFlag(ResourceView::Independent)) {
        qobject_cast<ResourcePackage*>(parent())->newPage(rv);
    } else {
        addResource(rv);
    }
    return rv;
}

ResourceView * ResourcePage::addResourceOrBringTop(QUrl const & url, QVariantMap const & settings)
{
    ResourcePage * page = currentSubPage_ ? currentSubPage_ : this;
    ResourceView * rv = page->findResource(url);
    if (rv) {
        rv->page()->moveResourceBack(rv);
        return rv;
    }
    ResourcePage * vpage = qobject_cast<ResourcePackage*>(parent())->findVirtualPage(url);
    if (vpage) {
        qobject_cast<ResourcePackage*>(parent())->showVirtualPage(vpage, true);
        return vpage->resources().first();
    }
    return addResource(url, settings);
}

ResourceView * ResourcePage::findResource(QUrl const & url) const
{
    for (ResourceView * res : resources()) {
        if (res->url() == url)
            return res;
    }
    if (currentSubPage_)
        return currentSubPage_->findResource(url);
    return nullptr;
}

ResourceView *ResourcePage::findResource(const QByteArray &type) const
{
    for (ResourceView * res : resources()) {
        if (res->resource()->type() == type)
            return res;
    }
    if (currentSubPage_)
        return currentSubPage_->findResource(type);
    return nullptr;
}

void ResourcePage::addResource(ResourceView * res)
{
    if (currentSubPage_) {
        currentSubPage_->addResource(res);
        return;
    }
    if (res->flags().testFlag(ResourceView::ListOfPages)) {
        currentSubPage_ = new ResourcePage(this);
        if (!resources_.empty()) {
            QList<ResourceView*> resources;
            beginRemoveRows(QModelIndex(), 0, resources_.size() - 1);
            currentSubPage_->resources_.swap(resources_);
            endRemoveRows();
        }
        subPages_.append(currentSubPage_);
        emit currentSubPageChanged(currentSubPage_);
    }
    int index = resources_.size();
    while (index > 0 && (resources_[index - 1]->flags() & ResourceView::TopMost)) {
        --index;
    }
    ResourceView * split = nullptr;
    if (index < resources_.size() && (resources_[index]->flags() & ResourceView::Splittable)) {
        split = resources_[index]->clone();
    }
    beginInsertRows(QModelIndex(), index, index);
    resources_.insert(index, res);
    res->setParent(this);
    endInsertRows();
    if (split) {
        beginInsertRows(QModelIndex(), index, index);
        resources_.insert(index, split);
        split->setParent(this);
        endInsertRows();
    }
}

ResourceView * ResourcePage::copyResource(ResourceView * res)
{
    if ((res->flags() & ResourceView::CanCopy) == 0)
        return nullptr;
    ResourceView * copy = res->clone();
    addResource(copy);
    return copy;
}

void ResourcePage::removeResource(ResourceView * res)
{
    if ((res->flags() & ResourceView::CanDelete) == 0)
        return;
    int index = resources_.indexOf(res);
    if (index < 0) {
        if (currentSubPage_)
            currentSubPage_->removeResource(res);
        return;
    }
    int pos1 = index;
    int pos2 = index;
    while (pos1 > 0 && (resources_[pos1 - 1]->flags() & ResourceView::StickUnder)
           && (resources_[pos1 - 1]->flags() & ResourceView::CanDelete)) {
        --pos1;
    }
    while (pos2 < resources_.size() - 2 && (resources_[pos2 + 1]->flags() & ResourceView::StickOn)
           && (resources_[pos2 + 1]->flags() & ResourceView::CanDelete)) {
        ++pos2;
    }
    QList<ResourceView*> list = resources_.mid(pos1, pos2 - pos1 + 1);
    beginRemoveRows(QModelIndex(), pos1, pos2);
    while (pos1 <= pos2) {
        resources_.removeAt(pos1);
        --pos2;
    }
    endRemoveRows();
    for (ResourceView* res : list)
        delete res;
}

void ResourcePage::moveResourceFront(ResourceView *res)
{
    int index = resources_.indexOf(res);
    if (index <= 0)
        return;
    moveResource(index, 0);
}

void ResourcePage::moveResourcePrevious(ResourceView *res)
{
    int index = resources_.indexOf(res);
    if (index <= 0)
        return;
    moveResource(index, index - 1);
}

void ResourcePage::moveResourceNext(ResourceView *res)
{
    int index = resources_.indexOf(res);
    if (index < 0 || index + 1 >= resources_.size())
        return;
    moveResource(index, index + 1);
}

void ResourcePage::moveResourceBack(ResourceView *res)
{
    int index = resources_.indexOf(res);
    if (index < 0 || index + 1 >= resources_.size())
        return;
    moveResource(index, resources_.size() - 1);
}

ResourceView * ResourcePage::previousNormalResource(ResourceView *res) const
{
    int index = resources_.indexOf(res);
    for (--index; index >= 0; --index) {
        if (!(resources_[index]->flags() & ResourceView::ZOrderFlags))
            return resources_[index];
    }
    return nullptr;
}

ResourceView * ResourcePage::nextNormalResource(ResourceView *res) const
{
    int index = resources_.indexOf(res);
    for (++index; index < resources_.size(); ++index) {
        if (!(resources_[index]->flags() & ResourceView::ZOrderFlags))
            return resources_[index];
    }
    return nullptr;
}

void ResourcePage::switchSubPage(int nPage)
{
    if (subPages_.size() <= nPage)
        subPages_.resize(nPage + 1);
    if (subPages_[nPage] == nullptr) {
        subPages_[nPage] = new ResourcePage(this);
        QObject* p = parent();
        while (p) {
            ResourcePackage * pkg = qobject_cast<ResourcePackage*>(p);
            if (pkg) {
                emit pkg->pageCreated(subPages_[nPage]);
                break;
            }
            p = p->parent();
        }
    }
    switchSubPage(subPages_[nPage]);
}

void ResourcePage::clearSubPages(bool exceptCurrent)
{
    if (!exceptCurrent && currentSubPage_) {
        switchSubPage(nullptr);
    }
    int n = subPages_.indexOf(currentSubPage_);
    if (n >= 0)
        subPages_.replace(n, nullptr);
    for (ResourcePage *& sp : subPages_) {
        delete sp;
        sp = nullptr;
    }
    if (n >= 0)
        subPages_.replace(n, currentSubPage_);
    subPages_.resize(n + 1);
}

bool ResourcePage::isIndependentPage() const
{
    return !resources_.isEmpty()
            && resources_.first()->flags().testFlag(ResourceView::Independent);
}

bool ResourcePage::isVirtualPage() const
{
    ResourcePackage * pkg = qobject_cast<ResourcePackage*>(parent());
    return pkg && pkg->containsVisualPage(const_cast<ResourcePage*>(this));
}

bool ResourcePage::isLargePage() const
{
    return !resources_.isEmpty()
            && resources_.first()->flags().testFlag(ResourceView::LargeCanvas);
}

bool ResourcePage::hasSubPage() const
{
    return currentSubPage_ != nullptr;
}

bool ResourcePage::isSubPage() const
{
    return parent() && parent()->metaObject()->inherits(&staticMetaObject);
}

ResourceView *ResourcePage::mainResource() const
{
    return resources_.isEmpty() ? nullptr : resources_.first();
}

void ResourcePage::setThumbnail(QPixmap thumb)
{
    thumbnail_ = thumb;
    ++thumbnailVersion_;
    ResourcePackage * pkg = qobject_cast<ResourcePackage*>(parent());
    if (pkg)
        pkg->pageChanged(this);
}

void ResourcePage::moveResource(int pos, int newPos)
{
    int pos1 = pos;
    int pos2 = pos;
    while (pos1 > 0 && (resources_[pos1 - 1]->flags() & ResourceView::StickUnder)) {
        --pos1;
        if (pos1 == newPos) --newPos;
    }
    while (pos2 < resources_.size() - 2 && (resources_[pos2 + 1]->flags() & ResourceView::StickOn)) {
        ++pos2;
        if (pos2 == newPos) ++newPos;
    }
    if (newPos < 0 || newPos >= resources_.size())
        return;
    while (newPos > pos && (resources_[newPos]->flags() & ResourceView::TopMost))
        --newPos;
    while (newPos < pos && (resources_[newPos]->flags() & ResourceView::BottomMost))
        ++newPos;
    if (newPos >= pos1 && newPos <= pos2)
        return;
    int newPos2 = newPos > pos2 ? newPos + 1 : newPos; // ItemModel diffs from QList
    beginMoveRows(QModelIndex(), pos1, pos2, QModelIndex(), newPos2);
    while (pos2 >= pos1) {
        resources_.move(pos1, newPos);
        --pos2;
    }
    endMoveRows();
}

void ResourcePage::switchSubPage(ResourcePage * subPage)
{
    if (currentSubPage_ == subPage)
        return;
    currentSubPage_ = subPage;
    emit currentSubPageChanged(currentSubPage_);
    ResourcePage * page = this;
    while (page) {
        ResourcePackage * pkg = qobject_cast<ResourcePackage*>(page->parent());
        if (pkg) {
            if (pkg->currentPage() == page) {
                if (subPage)
                    while (subPage->currentSubPage_) subPage = subPage->currentSubPage_;
                emit pkg->currentSubPageChanged(subPage);
            }
            break;
        } else {
            ResourcePage * pge = qobject_cast<ResourcePage*>(page->parent());
            if (pge->currentSubPage_ != page)
                break;
            page = pge;
        }
    }
}

QVariant ResourcePage::data(const QModelIndex &index, int role) const
{
    (void)role;
    return QVariant::fromValue(resources_[index.row()]);
}

int ResourcePage::rowCount(const QModelIndex &parent) const
{
    (void) parent;
    return resources_.size();
}

int ResourcePage::columnCount(const QModelIndex &parent) const
{
    (void) parent;
    return 1;
}

QModelIndex ResourcePage::parent(const QModelIndex &child) const
{
    (void) child;
    return QModelIndex();
}

ResourceView *ResourcePage::createResource(const QUrl &url, const QVariantMap &settings)
{
    QVariant type = settings.value("resourceType");
    ResourceView * rv = ResourceManager::instance()
            ->createResource(url, type.isValid() ? type.toByteArray().toLower() : nullptr);
    for (QString const & k : settings.keys()) {
        rv->setProperty(k.toUtf8(), settings.value(k));
    }
    return rv;
}

QModelIndex ResourcePage::index(int row, int column, const QModelIndex &parent) const
{
    (void) parent;
    return createIndex(row, column, resources_[row]);
}
