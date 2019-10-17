#include "stroke.h"
#include "strokeparser.h"
#include "resource.h"

using namespace QtPromise;

REGISTER_RESOURCE_VIEW(Stroke, "stroke")

Stroke::Stroke(Resource * res)
    : ResourceView(res)
{
}

QPromise<bool> Stroke::load()
{
    return getStream().then([this](QIODevice * stream) {
        canvasSize_ = StrokeParser::instance->load(res_->type(), stream, points_);
        return canvasSize_.width() > 0 && canvasSize_.height() > 0;
    });
}
