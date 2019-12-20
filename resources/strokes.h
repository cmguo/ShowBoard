#ifndef STROKES_H
#define STROKES_H

#include "core/resourceview.h"

#include <QtPromise>

#include <QSizeF>

typedef std::array<float, 3> stroke_point_t;

class IDynamicStrokes
{
public:
    virtual ~IDynamicStrokes() {}
    virtual void addPoint(float point[3]) = 0;
};

class SHOWBOARD_EXPORT Strokes : public ResourceView
{
    Q_OBJECT

    Q_PROPERTY(QSizeF canvasSize READ canvasSize())
    Q_PROPERTY(QList<stroke_point_t> points READ points())

public:
    Q_INVOKABLE Strokes(Resource * res, Flags flags = nullptr, Flags clearFlags = nullptr);

    Q_INVOKABLE Strokes(Strokes const & o);

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

#endif // STROKES_H
