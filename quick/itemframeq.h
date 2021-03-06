#ifndef ITEMFRAME_H
#define ITEMFRAME_H

#include "ShowBoard_global.h"

#include <QQuickItem>
#include <QVector>

class SHOWBOARD_EXPORT ItemFrame : public QQuickItem
{
public:
    enum Dock
    {
        Left,
        Right,
        Top,
        Buttom,
        Surround,
    };

    ItemFrame(QQuickItem * item, QQuickItem * parent = nullptr);

public:
    typedef void (*PaintFunc)(QPainter *painter, QRectF const & rect, ItemFrame * frame);

    void addTopBar();

    void addDockItem(Dock dock, qreal size);

    void addDockItem(Dock dock, qreal size, QColor color);

    void addDockItem(Dock dock, qreal size, PaintFunc paint);

    void addDockItem(Dock dock, QQuickItem * item);

    QRectF padding()
    {
        return padding_;
    }

    /*
     * when select top bar is opacity
     */
    void setSelected(bool selected);

    /*
     * set whole rect after calc
     */
    void setRect(QRectF const & rect);

    /*
     * update rect from child
     *   @see updateRectFromChild
     */
    void updateRect();

    /*
     * add borders to rect
     */
    void updateRectFromChild(QRectF & rect);

    /*
     * cut off borders from rect
     */
    void updateRectToChild(QRectF & rect);

    //void updateTransform();

    //void update();

    bool hitTest(QQuickItem * child, QPointF const & pt);

private:
    struct DockItem
    {
        Dock dock;
        QRectF pad; // width or height
        QVariant item; // QColor or QGraphicsItem or PaintFunc
    };

    void addDockItem(Dock dock, qreal size, QVariant item);

    QRectF reversePadding(DockItem & i, QRectF & rect);

    static constexpr qreal TOP_BAR_WIDTH = 24.0;
    static constexpr qreal TOP_BAR_HEIGHT = 16.0;

private:
    QQuickItem * item_;
    bool hasTopBar_;
    bool selected_;
    QRectF padding_;
    QVector<DockItem> dockItems_;
};

Q_DECLARE_METATYPE(ItemFrame::PaintFunc)

#endif // ITEMFRAME_H
