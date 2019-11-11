#ifndef RESOURCES_H
#define RESOURCES_H

#include "core/resourceview.h"
#include "graph/graphs.h"
#include "stroke.h"

REGISTER_RESOURCE_VIEW(ResourceView, "bmp,gif,jpg,jpeg,png") // copyable
REGISTER_RESOURCE_VIEW(Stroke, "stroke")

//export_resource_Stroke;

#endif // RESOURCES_H
