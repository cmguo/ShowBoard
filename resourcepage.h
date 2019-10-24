#ifndef WHITEPAGE_H
#define WHITEPAGE_H

#include "ShowBoard_global.h"

#include <QAbstractItemModel>

class ResourceView;

class SHOWBOARD_EXPORT ResourcePage : public QAbstractItemModel
{
    Q_OBJECT

    Q_PROPERTY(QList<ResourceView *> const & resources READ resources())
public:
    explicit ResourcePage(QObject *parent = nullptr);

public:
    ResourceView * addResource(QUrl const & url);

    ResourceView * findResource(QUrl const & url);

    void addResource(ResourceView * res);

    void copyResource(ResourceView * res);

    void removeResource(ResourceView * res);

    void moveResourceFront(ResourceView * res);

    void moveResourceNext(ResourceView * res);

    void moveResourcePrevious(ResourceView * res);

    void moveResourceBack(ResourceView * res);

public:
    QList<ResourceView *> const & resources()
    {
        return resources_;
    }

    using QObject::parent;

public slots:

private:
    void moveResource(int pos, int newPos);

private:
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

    QModelIndex parent(const QModelIndex &child) const override;

private:
    QList<ResourceView *> resources_;
};

#endif // WHITEPAGE_H
