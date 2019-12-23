#ifndef RESOURCES_H
#define RESOURCES_H

#include "core/resourceview.h"
#include "strokes.h"

REGISTER_COMMON_RESOURCE_TYPES("bmp,gif,jpg,jpeg,png", {}, {}) // copyable
REGISTER_RESOURCE_VIEW(Strokes, "stroke,glstroke")

//export_resource_Stroke;

#endif // RESOURCES_H
