#include "imagehelper.h"

#include <QStyleOptionGraphicsItem>
#include <QSvgGenerator>


QPixmap ImageHelper::widgetToPixmap(QWidget *widget, bool destroy)
{
    QPixmap pm(widget->size());
    pm.fill(Qt::transparent);
    QPainter pt(&pm);
    pt.setRenderHint(QPainter::HighQualityAntialiasing);
    widget->render(&pt);
    pt.end();
    if (destroy)
        widget->deleteLater();
    return pm;
}

QPixmap ImageHelper::itemToPixmap(QGraphicsItem *item, QSize size, bool destroy)
{
    QPointF c = item->boundingRect().center() * 2;
    if (size.isEmpty()) {
        size = QSizeF(c.x(), c.y()).toSize();
    }
    QPointF scale(size.width() / c.x(), size.height() / c.y());
    QPixmap pm(size);
    pm.fill(Qt::transparent);
    paintItem(item, &pm);
    if (destroy)
        delete item;
    return pm;
}

QPixmap ImageHelper::opacityPixmap(QPixmap pixmap, int opacity)
{
    QPixmap pm = QPixmap(pixmap.size());
    pm.fill(Qt::transparent);
    QPainter pt(&pm);
    pt.setOpacity(opacity / 100.0);
    QRect rc(0, 0, pm.width(), pm.height());
    pt.drawPixmap(rc, pixmap, rc);
    pt.end();
    return pm;
}

void ImageHelper::paintItem(QPainter &painter, QGraphicsItem *item, QStyleOptionGraphicsItem &option)
{
    item->paint(&painter, &option, nullptr);
    for (QGraphicsItem * c : item->childItems()) {
        if (!c->isVisible())
            continue;
        painter.save();
        painter.setTransform(c->itemTransform(item), true);
        paintItem(painter, c, option);
        painter.restore();
    }
}

void ImageHelper::paintItem(QGraphicsItem *item, QPaintDevice *device)
{
    QRect rect = item->boundingRect().toAlignedRect();
    QPainter painter(device);
    painter.setBrush(Qt::white);
    painter.setTransform(QTransform::fromTranslate(-rect.left(), -rect.top()));
    QStyleOptionGraphicsItem option;
    paintItem(painter, item, option);
    painter.end();
}

QImage ImageHelper::toImage(QGraphicsItem *item)
{
    QRect rect = item->boundingRect().toAlignedRect();
    QImage image(rect.size(), QImage::Format_ARGB32);
    image.fill(Qt::transparent);
    paintItem(item, &image);
    return image;
}

void ImageHelper::saveItem(QGraphicsItem *item, const QString &file)
{
    if (file.endsWith(".svg")) {
        QSvgGenerator svg;
        svg.setFileName(file);
        svg.setTitle("Snapshot");
        svg.setDescription("Showboard Control Snapshot");
        QRect rect = item->boundingRect().toAlignedRect();
        svg.setSize(rect.size());
        svg.setViewBox(QRect(QPoint(), rect.size()));
        paintItem(item, &svg);
    } else {
        toImage(item).save(file);
    }
}
