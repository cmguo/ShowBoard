#ifndef RESOURCEPACKAGE_H
#define RESOURCEPACKAGE_H

#include "ShowBoard_global.h"

#include <QObject>

class ResourcePage;

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

    ResourcePage * newPage();

    ResourcePage * currentPage();

public slots:
    void gotoFront();

    void gotoNext();

    void gotoPrevious();

    void gotoBack();

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
