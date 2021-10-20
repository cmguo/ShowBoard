#include "qquickshapeitem.h"
#include "qquickshapepath.h"
#include "quickhelper.h"

#include <QPainterPath>

QQuickShapeItem *QQuickShapeItem::create(QObject *context)
{
    QQmlComponent qc(qmlEngine(context));
    qc.loadUrl(QUrl("qrc:/showboard/qml/ShapeItem.qml"));
    return reinterpret_cast<QQuickShapeItem *>(qc.create());
}

void QQuickShapeItem::setPath(const QPainterPath &path)
{
    shapePath()->clearPath();
    shapePath()->addPath(path);
}

QQuickShapePath *QQuickShapeItem::shapePath()
{
    return reinterpret_cast<QQuickShapePath *>(QuickHelper::findChild(this, "QShapePath"));
}
