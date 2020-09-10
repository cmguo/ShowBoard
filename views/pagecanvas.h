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
    void switchPage(ResourcePage * page);

    Control * findControl(ResourceView * res);

    Control * findControl(QUrl const & url);

    /*
     * get top most control
     */
    Control * topControl();

    void relayout();

    ResourcePage * page()
    {
        return page_;
    }

    ResourcePage * subPage();

public:
    QPixmap thumbnail(QPixmap* snapshot = nullptr);

    bool hasSubCanvas(QGraphicsItem * canvas);

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
