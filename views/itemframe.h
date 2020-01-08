#ifndef ITEMFRAME_H
#define ITEMFRAME_H

#include "ShowBoard_global.h"

#include <QGraphicsRectItem>
#include <QVector>

class SHOWBOARD_EXPORT ItemFrame : public QGraphicsRectItem
{
public:
    enum Dock
    {
        Left,
        Right,
        Top,
        Buttom
    };

    ItemFrame(QGraphicsItem * item, QGraphicsItem * parent = nullptr);

public:
    typedef void (*PaintFunc)(QPainter *painter, QRectF const & rect, ItemFrame * frame);

    void addTopBar();

    void addDockItem(Dock dock, qreal size);

    void addDockItem(Dock dock, qreal size, QColor color);

    void addDockItem(Dock dock, qreal size, PaintFunc paint);

    void addDockItem(Dock dock, QGraphicsItem * item);

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

    bool hitTest(QGraphicsItem * child, QPointF const & pt);

private:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

    struct DockItem
    {
        int dock;
        qreal size; // width or height
        QVariant item; // QColor or QGraphicsItem or PaintFunc
    };

    void addDockItem(DockItem const & item);

    static constexpr qreal HEIGHT = 48.0;

    static void drawTopBar(QPainter *painter, QRectF const & rect, ItemFrame * frame);

private:
    QGraphicsItem * item_;
    bool hasTopBar_;
    bool selected_;
    QRectF padding_;
    QVector<DockItem> dockItems_;
};

Q_DECLARE_METATYPE(ItemFrame::PaintFunc)

#endif // ITEMFRAME_H
