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

void ResourcePage::addResource(ResourceView * res)
{
    int index = resources_.size();
    beginInsertRows(QModelIndex(), index, index);
    resources_.insert(index, res);
    res->setParent(this);
    endInsertRows();
}

void ResourcePage::removeResource(ResourceView * res)
{
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
    beginMoveRows(QModelIndex(), pos, pos, QModelIndex(), newPos);
    resources_.move(pos, newPos);
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
