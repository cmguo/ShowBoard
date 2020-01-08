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

public:
    void switchPage(ResourcePage * page);

    Control * findControl(ResourceView * res);

    Control * findControl(QUrl const & url);

    void relayout();

    ResourcePage * page()
    {
        return page_;
    }

private slots:
    void resourceInserted(QModelIndex const &parent, int first, int last);

    void resourceRemoved(QModelIndex const &parent, int first, int last);

    void resourceMoved(QModelIndex const &parent, int start, int end,
                       QModelIndex const &destination, int row);

private:
    void insertResource(int layer);

    void removeResource(int layer);

private:
    ResourceManager * resource_manager_;
    ControlManager * control_manager_;

protected:
    ResourcePage * page_;
};

#endif // PAGECANVAS_H
