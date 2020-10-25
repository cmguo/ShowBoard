#ifndef WHITEPAGE_H
#define WHITEPAGE_H

#include "ShowBoard_global.h"

#include <QAbstractItemModel>
#include <QPixmap>
#include <QSizeF>

class ResourceView;
class ResourcePackage;

/*
 * ResourcePage mananges a collection of resources (views)
 *   manage adds/removes and z-orders
 *   worked as ItemModel to data drive ui (canvas)
 */

class SHOWBOARD_EXPORT ResourcePage : public QAbstractItemModel
{
    Q_OBJECT

    Q_PROPERTY(QPixmap thumbnail READ thumbnail WRITE setThumbnail)
    Q_PROPERTY(QList<ResourceView *> const & resources READ resources)

public:
    explicit ResourcePage(QObject *parent = nullptr);

    explicit ResourcePage(QUrl const & mainUrl, QVariantMap const & settings, QObject *parent = nullptr);

    explicit ResourcePage(ResourceView* mainRes, QObject *parent = nullptr);

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
    ResourceView * findResource(QUrl const & url) const;

    /*
     * find resource by type, return first if more than one
     */
    ResourceView * findResource(QByteArray const & type) const;

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

public:
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

    ResourceView * previousNormalResource(ResourceView * res) const;

    ResourceView * nextNormalResource(ResourceView * res) const;

public:
    void switchSubPage(int nPage);

    ResourcePage* currentSubPage() const;

    int currentSubNumber() const { return currentSubPage_; }

    void clearSubPages(bool exceptCurrent = false);

    ResourcePackage * package() const;

    void removeFromPackage();

public:
    bool isIndependentPage() const;

    bool isVirtualPage() const;

    bool isLargePage() const;

    bool hasSubPage() const;

    bool isSubPage() const;

public:
    ResourceView* canvasView() const
    {
        return canvasView_;
    }

    ResourceView* mainResource() const;

    QList<ResourceView *> const & resources() const
    {
        return resources_;
    }

    using QObject::parent;

    QPixmap thumbnail() const
    {
        return thumbnail_;
    }

    int thumbnailVersion() const
    {
        return thumbnailVersion_;
    }

    void setThumbnail(QPixmap thumb);

signals:
    void currentSubPageChanged(ResourcePage* page);

private:
    void insertResource(int index, QList<ResourceView *> ress);

    void moveResource(int pos, int newPos);

    void removeResource(int index, QList<ResourceView *> ress);

    void onSubPageChanged(ResourcePage* subPage);

private:
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

    QModelIndex parent(const QModelIndex &child) const override;

private:
    static ResourceView* createResource(QUrl const & url, QVariantMap const & settings);

private:
    ResourceView* canvasView_;
    QList<ResourceView *> resources_;
    int currentSubPage_;
    QVector<ResourcePage*> subPages_;
    QPixmap thumbnail_;
    int thumbnailVersion_;
};

#endif // WHITEPAGE_H
