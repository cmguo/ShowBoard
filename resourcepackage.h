#ifndef RESOURCEPACKAGE_H
#define RESOURCEPACKAGE_H

#include "ShowBoard_global.h"

#include <QObject>

class ResourcePage;

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

    /*
     * insert a newly create page,
     *  new page is inserted before current one and became current
     */
    ResourcePage * newPage();

    ResourcePage * currentPage();

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
    void addPage(ResourcePage * page);

private:
    QList<ResourcePage *> pages_;
    int current_;
};

#endif // RESOURCEPACKAGE_H
