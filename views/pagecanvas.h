#ifndef PAGECANVAS_H
#define PAGECANVAS_H

#include "canvasitem.h"

#include <QObject>

class ResourceManager;
class ControlManager;
class ResourceView;
class ResourcePage;
class Control;

class PageCanvas : public QObject, public CanvasItem
{
    Q_OBJECT
public:
    PageCanvas(QGraphicsItem * parent = nullptr);

    enum { Type = UserType + 1 };

    virtual int type() const override { return Type; }

public:
    Control * findControl(ResourceView * res) const;

    Control * findControl(QUrl const & url) const;

    /*
     * get top most control
     */
    Control * topControl() const;

    ResourcePage * page() const { return page_; }

    ResourcePage * subPage() const;

    PageCanvas * subCanvas() const { return subCanvas_; }

public:
    void switchPage(ResourcePage * page);

    void relayout();

public:
    QPixmap thumbnail(QPixmap* snapshot = nullptr) const;

    bool hasSubCanvas(QGraphicsItem * canvas) const;

private:
    void resourceInserted(QModelIndex const &parent, int first, int last);

    void resourceRemoved(QModelIndex const &parent, int first, int last);

    void resourceMoved(QModelIndex const &parent, int start, int end,
                       QModelIndex const &destination, int row);

private:
    void subPageChanged(ResourcePage* page);

private:
    void insertResource(int layer);

    void removeResource(int layer);

private:
    ResourceManager * resource_manager_;
    ControlManager * control_manager_;

protected:
    ResourcePage * page_;
    PageCanvas* subCanvas_;
};

#endif // PAGECANVAS_H
