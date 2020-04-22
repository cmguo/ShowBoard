#ifndef STROKES_H
#define STROKES_H

#include "core/resourceview.h"

#include <QtPromise>

typedef std::array<ushort, 3> stroke_point_t;

class IDynamicRenderer : public QObject
{
    Q_OBJECT
public:
    virtual void setMaximun(stroke_point_t const & max) = 0;

    virtual void addPoint(stroke_point_t const & point) = 0;
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
    QtPromise::QPromise<void> load(QSizeF const & maxSize, IDynamicRenderer * dr = nullptr);

public:
    QSize canvasSize() const
    {
        return QSize(maximun_[0], maximun_[1]);
    }

    int maxPressure() const
    {
        return maximun_[2];
    }

    qreal scale() const
    {
        return scale_;
    }

    QSizeF size() const
    {
        return QSizeF(canvasSize()) * scale_;
    }

    QList<stroke_point_t> const & points() const
    {
        return points_;
    }

private:
    stroke_point_t maximun_;
    QList<stroke_point_t> points_;
    qreal scale_;
};

#endif // STROKES_H
