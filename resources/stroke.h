#ifndef STROKE_H
#define STROKE_H

#include "core/resourceview.h"

#include <QtPromise>

#include <QSizeF>

typedef std::array<float, 3> stroke_point_t;

class IDynamicStroke
{
public:
    virtual ~IDynamicStroke() {}
    virtual void addPoint(float point[3]) = 0;
};

class Stroke : public ResourceView
{
    Q_OBJECT

    Q_PROPERTY(QSizeF canvasSize READ canvasSize())
    Q_PROPERTY(QList<stroke_point_t> points READ points())

public:
    Q_INVOKABLE Stroke(Resource * res);

    Q_INVOKABLE Stroke(Stroke const & res);

public:
    QtPromise::QPromise<void> load();

public:
    QSizeF canvasSize() const
    {
        return  canvasSize_;
    }

    QList<stroke_point_t> const & points() const
    {
        return points_;
    }

private:
    QSizeF canvasSize_;
    QList<stroke_point_t> points_;
};

#endif // STROKE_H
