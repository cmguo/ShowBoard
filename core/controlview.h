#ifndef CONTROLVIEW_H
#define CONTROLVIEW_H

#ifdef SHOWBOARD_QUICK
class QQuickItem;
typedef QQuickItem ControlView;
#else
class QGraphicsItem;
typedef QGraphicsItem ControlView;
#endif

class QRectF;
class QPointF;
class QWidget;

QRectF itemSceneRect(ControlView const * item);

QPointF itemPosition(ControlView const * item);

void setItemPosition(ControlView * item, QPointF const & pos);

ControlView * itemFromWidget(QWidget * widget, ControlView * parent = nullptr);

QWidget * widgetFromItem(ControlView const * item);

#endif // CONTROLVIEW_H
