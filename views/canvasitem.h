#ifndef CANVASITEM_H
#define CANVASITEM_H

#include "ShowBoard_global.h"

#ifdef SHOWBOARD_QUICK
# include <QQuickItem>
# define CANVAS_OBJECT
typedef QQuickItem CanvasView;
#else
# include <QGraphicsItem>
# include <QBrush>
# define CANVAS_OBJECT public QObject,
typedef QGraphicsItem CanvasView;
#endif

class SHOWBOARD_EXPORT CanvasItem : public CanvasView
{
public:
    CanvasItem(CanvasView * parent = nullptr);

public:
    void setRect(QRectF const & rect);

    QRectF rect() const;

#ifdef SHOWBOARD_QUICK
#else
    void setBrush(QBrush const & brush);

    QBrush brush() const;
#endif

protected:
    virtual QRectF boundingRect() const override;

#ifdef SHOWBOARD_QUICK
#else
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
#endif

private:
    QRectF rect_;
#ifdef SHOWBOARD_QUICK
#else
    QBrush brush_;
#endif
};

#endif // CANVASITEM_H
