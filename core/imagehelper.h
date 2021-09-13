#ifndef IMAGEHELPER_H
#define IMAGEHELPER_H

#include "ShowBoard_global.h"

#include <QGraphicsItem>
#include <QPainter>

class SHOWBOARD_EXPORT ImageHelper
{
public:

    static QPixmap widgetToPixmap(QWidget * widget, bool destroy);

    static QPixmap itemToPixmap(QGraphicsItem * item, QSize size, bool destroy);

    static QPixmap opacityPixmap(QPixmap pixmap, int opacity);

    static void paintItem(QPainter & painter, QGraphicsItem * item, QStyleOptionGraphicsItem & option);

    static void paintItem(QGraphicsItem * item, QPaintDevice * device);

    static QImage toImage(QGraphicsItem * item);

    static void saveItem(QGraphicsItem * item, QString const & file);
};

#endif // IMAGEHELPER_H
