#ifndef QQUICKSHAPEPATH_H
#define QQUICKSHAPEPATH_H

#include "ShowBoard_global.h"

#include <QObject>

class QPainterPath;

class SHOWBOARD_EXPORT QQuickShapePath : public QObject
{
public:
    static QQuickShapePath * create(QObject * context);

public:
    void setStrokeColor(QColor const & color);

    void setStrokeWidth(qreal width);

    void setFillColor(QColor const & color);

    void addPath(const QPainterPath &ph);

    void clearPath();
};

#endif // QQUICKSHAPEPATH_H
