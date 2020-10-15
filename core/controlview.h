#ifndef CONTROLVIEW_H
#define CONTROLVIEW_H

#ifdef SHOWBOARD_QUICK
class QQuickItem;
typedef QQuickItem ControlView;
#else
class QGraphicsItem;
typedef QGraphicsItem ControlView;
#endif

#endif // CONTROLVIEW_H
