#ifndef RESOURCEPACKAGE_H
#define RESOURCEPACKAGE_H

#include "ShowBoard_global.h"

#include <QObject>

class ResourcePage;
class ResourceView;

/*
 * ResourcePackage manages a collection of resource pages
 */

class SHOWBOARD_EXPORT ResourcePackage : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QList<ResourcePage *> pages READ pages())
public:
    explicit ResourcePackage(QObject *parent = nullptr);

public:
    QList<ResourcePage *> const & pages() const
    {
        return pages_;
    }

    ResourcePage * globalPage();

    /*
     * insert a newly create page,
     *  new page is inserted before current one and became current
     */
    ResourcePage * newPage();

    ResourcePage * currentPage();

    int currentNumber();

public:
    ResourcePage * newVirtualPage(ResourceView* mainRes = nullptr);

    ResourcePage * topVirtualPage();

    ResourcePage * findVirtualPage(QUrl const & mainUrl);

    void showVirtualPage(ResourcePage* page, bool show);

    void toggleVirtualPage(ResourcePage* page);

    void hideAllVirtualPages();

    void removeVirtualPage(ResourcePage* page);

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

signals:
    void currentPageChanged(ResourcePage * page);

protected:
    int addPage(ResourcePage * page);

private:
    ResourcePage * globalPage_;
    QList<ResourcePage *> pages_;
    QList<ResourcePage *> visiblePages_;
    QList<ResourcePage *> hiddenPages_;
    int current_;
};

#endif // RESOURCEPACKAGE_H
