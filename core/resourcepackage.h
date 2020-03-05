#ifndef RESOURCEPACKAGE_H
#define RESOURCEPACKAGE_H

#include "ShowBoard_global.h"

#include <QAbstractItemModel>

class ResourcePage;
class ResourceView;

/*
 * ResourcePackage manages a collection of resource pages
 */

class SHOWBOARD_EXPORT ResourcePackage : public QAbstractItemModel
{
    Q_OBJECT

    Q_PROPERTY(QList<ResourcePage *> pages READ pages())
public:
    explicit ResourcePackage(QObject *parent = nullptr);

public:
    static ResourcePage* toolPage();

    QList<ResourcePage *> const & pages() const
    {
        return pages_;
    }

    /*
     *  total page count
     */
    int pageCount() const
    {
        return pages_.size();
    }

    ResourcePage * globalPage() const;

    /*
     * insert a newly create page,
     *  new page is inserted after current one and became current
     */
    ResourcePage * newPage();

    /*
     * insert a newly create page,
     *  new page is inserted at index, current page is not changed
     */
    ResourcePage * newPage(int index);

    ResourcePage * currentPage() const;

    /*
     *  current page index, start from 0
     */
    int currentIndex() const
    {
        return current_;
    }

    /*
     *  current page number, start from 1
     */
    int currentNumber() const;

    QModelIndex currentModelIndex() const;

signals:
    void pageCreated(ResourcePage* page);

    void currentPageChanged(ResourcePage* page);

    void pageCountChanged(int count);

public:
    ResourcePage * newVirtualPage(ResourceView* mainRes = nullptr);

    ResourcePage * topVirtualPage() const;

    ResourcePage * findVirtualPage(QUrl const & mainUrl) const;

    void showVirtualPage(ResourcePage* page, bool show);

    void toggleVirtualPage(ResourcePage* page);

    void removeVirtualPage(ResourcePage* page);

    void showVirtualPage(QUrl const & mainUrl, bool show);

    void toggleVirtualPage(QUrl const & mainUrl);

    void removeVirtualPage(QUrl const & mainUrl);

    void hideAllVirtualPages();

public slots:
    /*
     * set current page to first page
     */
    void gotoFront();

    /*
     * set current page to next page
     */
    void gotoNext();

    /*
     * set current page to previous page
     */
    void gotoPrevious();

    /*
     * set current page to last page
     */
    void gotoBack();

    /*
     * set page index at @page to be current
     */
    void switchPage(int page);

    /*
     * set page @page to be current
     */
    void switchPage(ResourcePage * page);

protected:
    void addPage(int index, ResourcePage * page);

    friend class ResourcePage;
    void pageChanged(ResourcePage* page);

private:
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

    QModelIndex parent(const QModelIndex &child) const override;

private:
    ResourcePage * globalPage_;
    QList<ResourcePage *> pages_;
    QList<ResourcePage *> visiblePages_;
    QList<ResourcePage *> hiddenPages_;
    int current_;
};

#endif // RESOURCEPACKAGE_H
