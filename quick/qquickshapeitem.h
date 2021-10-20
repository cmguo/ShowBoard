#ifndef QQUICKSHAPEITEM_H
#define QQUICKSHAPEITEM_H

#include <QQuickItem>

class QQuickShapePath;

class QQuickShapeItem : public QQuickItem
{
public:
    static QQuickShapeItem * create(QObject * context);

public:
    void setPath(QPainterPath const & path);

private:
    QQuickShapePath * shapePath();
};

#endif // QQUICKSHAPEITEM_H
