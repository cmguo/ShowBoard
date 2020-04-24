#ifndef STROKES_H
#define STROKES_H

#include "core/resourceview.h"
#include "stroke/strokereader.h"

#include <QtPromise>

class SHOWBOARD_EXPORT Strokes : public ResourceView
{
    Q_OBJECT

public:
    Q_INVOKABLE Strokes(Resource * res, Flags flags = nullptr, Flags clearFlags = nullptr);

    Q_INVOKABLE Strokes(Strokes const & o);

public:
    QtPromise::QPromise<StrokeReader*> load();

private:
    QSharedPointer<QIODevice> stream_;
};

#endif // STROKES_H
