#include "qquickshapepath.h"
#include "quickhelper.h"

#include <QColor>
#include <QPainterPath>
#include <QVariant>

QQuickShapePath *QQuickShapePath::create(QObject *context)
{
    QObject * shapePath = QuickHelper::createObject(context, "ShapePath", "QtQuick.Shapes");
    return reinterpret_cast<QQuickShapePath *>(shapePath);
}

void QQuickShapePath::setStrokeColor(const QColor &color)
{
    setProperty("strokeColor", color);
}

void QQuickShapePath::setStrokeWidth(qreal width)
{
    setProperty("strokeWidth", width);
}

void QQuickShapePath::setFillColor(const QColor &color)
{
    setProperty("fillColor", color);
}

void QQuickShapePath::addPath(const QPainterPath &ph)
{
    QObject * seg = nullptr;
    constexpr char const * classNames[] = {"PathMove", "PathLine", "PathCubic"};
    for (int i = 0; i < ph.elementCount(); ++i) {
        QPainterPath::Element e = ph.elementAt(i);
        switch (e.type) {
        case QPainterPath::MoveToElement:
        case QPainterPath::LineToElement:
            seg = QuickHelper::createObject(this, classNames[e.type], "QtQuick", "2.12");
            seg->setProperty("x", e.x);
            seg->setProperty("y", e.y);
            break;
        case QPainterPath::CurveToElement:
            if (i + 2 < ph.elementCount()) {
                seg = QuickHelper::createObject(this, classNames[e.type], "QtQuick", "2.12");
                seg->setProperty("control1X", e.x);
                seg->setProperty("control1X", e.y);
                QPainterPath::Element e1 = ph.elementAt(++i);
                QPainterPath::Element e2 = ph.elementAt(++i);
                seg->setProperty("control2X", e1.x);
                seg->setProperty("control2Y", e1.y);
                seg->setProperty("x", e2.x);
                seg->setProperty("y", e2.y);
            }
            break;
        default:
            break;
        }
        QuickHelper::appendChild(this, seg);
        seg = nullptr;
    }
}

void QQuickShapePath::clearPath()
{
    QuickHelper::clearChildren(this);
}
