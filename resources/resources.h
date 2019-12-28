#ifndef RESOURCES_H
#define RESOURCES_H

#include "core/resourceview.h"
#include "strokes.h"

REGISTER_COMMON_RESOURCE_TYPES(image, "bmp,gif,jpg,jpeg,png", {}, {}) // copyable
REGISTER_COMMON_RESOURCE_TYPES(video, "mp4,mp3,ts,flv,m3u8,asf,avi,rtsp,rtmp,rtp", {}, {ResourceView::CanCopy})
REGISTER_RESOURCE_VIEW(Strokes, "stroke,glstroke")

//export_resource_Stroke;

#endif // RESOURCES_H
