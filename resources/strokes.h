#ifndef STROKES_H
#define STROKES_H

#include "core/resourceview.h"
#include "istrokeparser.h"

#include <QtPromise>

class SHOWBOARD_EXPORT Strokes : public ResourceView
{
    Q_OBJECT

    Q_PROPERTY(QSizeF canvasSize READ canvasSize())
    Q_PROPERTY(QList<StrokePoint> points READ points())

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

    QList<StrokePoint> const & points() const
    {
        return points_;
    }

private:
    StrokePoint maximun_;
    QList<StrokePoint> points_;
    qreal scale_;
    QSharedPointer<QIODevice> stream_;
};

#endif // STROKES_H
