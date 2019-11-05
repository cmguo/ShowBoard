#include "stroke.h"
#include "strokeparser.h"
#include "core/resource.h"

using namespace QtPromise;

REGISTER_RESOURCE_VIEW(Stroke, "stroke")

Stroke::Stroke(Resource * res)
    : ResourceView(res, {TopMost, Splittable})
{
}

Stroke::Stroke(Stroke const & o)
    : ResourceView(o)
    , canvasSize_(o.canvasSize_)
    , points_(o.points_)
{
}

Stroke * Stroke::clone() const
{
    return new Stroke(*this);
}

QPromise<void> Stroke::load()
{
    QWeakPointer<int> life(this->life());
    return res_->getStream().then([this, life](QIODevice * stream) {
        if (life.isNull())
            return;
        canvasSize_ = StrokeParser::instance->load(res_->type(), stream, points_);
        if (canvasSize_.isEmpty())
            throw std::exception("empty canvas size");
    });
}
