#ifndef WHITEPAGE_H
#define WHITEPAGE_H

#include "ShowBoard_global.h"

#include <QAbstractItemModel>
#include <QSizeF>

class ResourceView;

/*
 * ResourcePage mananges a collection of resources (views)
 *   manage adds/removes and z-orders
 *   worked as ItemModel to data drive ui (canvas)
 */

class SHOWBOARD_EXPORT ResourcePage : public QAbstractItemModel
{
    Q_OBJECT

    Q_PROPERTY(QList<ResourceView *> const & resources READ resources())
public:
    explicit ResourcePage(QObject *parent = nullptr);

public:
    /*
     * add resource from url
     *  @see addResource(res)
     *  @return newly added resource
     */
    ResourceView * addResource(QUrl const & url, QVariantMap const & settings = QVariantMap());

    /*
     * add resource from url
     *  if a resource with same url is already exists in this page,
     *   it's bring to top
     *  @see addResource(res)
     *  @return newly added or already existing resource
     */
    ResourceView * addResourceOrBringTop(QUrl const & url, QVariantMap const & settings = QVariantMap());

    /*
     * find resource by url
     */
    ResourceView * findResource(QUrl const & url);

    /*
     * add resource at back (top), but under resources with flag TopMost
     */
    void addResource(ResourceView * res);

    /*
     * copy resource @res and add to page
     *  @see ResourceView::clone
     */
    ResourceView * copyResource(ResourceView * res);

    /*
     * remove resource from page
     */
    void removeResource(ResourceView * res);

    /*
     * move resource to front of collection
     *   means backmost in z-order, but not cross resources with flag ButtomMost
     */
    void moveResourceFront(ResourceView * res);

    /*
     * move resource to back of collection,
     *   means topmost in z-order, but not cross resources with flag TopMmost
     */
    void moveResourceNext(ResourceView * res);

    /*
     * move resource to take postion of previous one,
     *   means goes down in z-order, but not cross resources with flag ButtomMost
     */
    void moveResourcePrevious(ResourceView * res);

    /*
     * move resource to take postion of next one,
     *   means goes up in z-order, but not cross resources with flag TopMmost
     */
    void moveResourceBack(ResourceView * res);

    ResourceView * previousNormalResource(ResourceView * res);

    ResourceView * nextNormalResource(ResourceView * res);

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
