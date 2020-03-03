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

    /*
     * get top most control
     */
    Control * topControl();

    void relayout();

    ResourcePage * page()
    {
        return page_;
    }

public:
    QPixmap thumbnail(bool snapshot);

    enum AnimateDirection
    {
        LeftToRight = 1,
        RightToLeft = 2,
        TopToBottom = 4,
        BottomToTop = 8,
        LeftTopToRight = 5,
        RightBottomToLeftTop = 10,
        LeftBottomToRightTop = 9,
        RightTopToLeftBottomTop = 6,
    };

    void startAnimate(int dir);

    void stopAnimate();

private:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

    virtual void timerEvent(QTimerEvent *event) override;

private:
    void resourceInserted(QModelIndex const &parent, int first, int last);

    void resourceRemoved(QModelIndex const &parent, int first, int last);

    void resourceMoved(QModelIndex const &parent, int start, int end,
                       QModelIndex const &destination, int row);

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
    QPixmap snapshot_;
    int animTimer_;
};

#endif // PAGECANVAS_H
