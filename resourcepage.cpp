#include "resourcepage.h"
#include "resourceview.h"
#include "resourcemanager.h"

ResourcePage::ResourcePage(QObject *parent)
    : QAbstractItemModel(parent)
{
}

ResourceView * ResourcePage::addResource(QUrl const & url)
{
    ResourceView * rv = ResourceManager::instance()->createResource(url);
    addResource(rv);
    return rv;
}

ResourceView * ResourcePage::findResource(QUrl const & url)
{
    for (ResourceView * res : resources()) {
        if (res->url() == url)
            return res;
    }
    return nullptr;
}

void ResourcePage::addResource(ResourceView * res)
{
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
    QTransform * t = copy->transform();
    t->translate(40.0 / t->m11(), 40.0 / t->m22());
    addResource(copy);
    return copy;
}

void ResourcePage::removeResource(ResourceView * res)
{
    if ((res->flags() & ResourceView::CanDelete) == 0)
        return;
    int index = resources_.indexOf(res);
    if (index < 0)
        return;
    beginRemoveRows(QModelIndex(), index, index);
    resources_.removeAt(index);
    endRemoveRows();
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

void ResourcePage::moveResource(int pos, int newPos)
{
    int pos1 = pos;
    int pos2 = pos;
    while (pos1 > 0 && (resources_[pos1 - 1]->flags() & ResourceView::StickUnder)) {
        --pos1;
        if (pos1 == newPos) --newPos;
    }
    while (pos2 > resources_.size() - 2 && (resources_[pos2 + 1]->flags() & ResourceView::StickOn)) {
        ++pos2;
        if (pos2 == newPos) ++newPos;
    }
    if (newPos < 0 || newPos >= resources_.size())
        return;
    while (newPos > pos && (resources_[newPos]->flags() & ResourceView::TopMost))
        --newPos;
    while (newPos < pos && (resources_[newPos]->flags() & ResourceView::BottomMost))
        ++newPos;
    if (newPos == pos)
        return;
    beginMoveRows(QModelIndex(), pos1, pos2, QModelIndex(), newPos);
    while (pos2 >= pos1) {
        resources_.move(pos1, newPos);
        --pos2;
    }
    endMoveRows();
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

QModelIndex ResourcePage::index(int row, int column, const QModelIndex &parent) const
{
    (void) parent;
    return createIndex(row, column, resources_[row]);
}
