#include "strokes.h"
#include "strokeparser.h"
#include "core/resource.h"

using namespace QtPromise;

Strokes::Strokes(Resource * res, Flags flags, Flags clearFlags)
    : ResourceView(res, flags | TopMost, clearFlags | CanDelete)
{
    if (res_->type() == "glstroke") {
        flags_.setFlag(TopMost, false);
    }
}

Strokes::Strokes(Strokes const & o)
    : ResourceView(o)
    , canvasSize_(o.canvasSize_)
    , points_(o.points_)
{
}

QPromise<void> Strokes::load()
{
    QWeakPointer<int> life(this->life());
    if (url().scheme() == res_->type())
        return QPromise<void>::resolve();
    return res_->getStream().then([this, life](QIODevice * stream) {
        if (life.isNull())
            return;
        canvasSize_ = StrokeParser::instance->load(
                    res_->property(Resource::PROP_ORIGIN_TYPE).toString(), stream, points_);
        if (canvasSize_.isEmpty())
            throw std::exception("empty canvas size");
    });
}
