#include "stroke.h"
#include "strokeparser.h"
#include "resource.h"

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

QPromise<bool> Stroke::load()
{
    return res_->getStream().then([this](QIODevice * stream) {
        canvasSize_ = StrokeParser::instance->load(res_->type(), stream, points_);
        return canvasSize_.width() > 0 && canvasSize_.height() > 0;
    });
}
