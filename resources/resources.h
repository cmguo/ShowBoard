#ifndef RESOURCES_H
#define RESOURCES_H

#include "core/resourceview.h"
#include "strokes.h"

REGISTER_COMMON_RESOURCE_TYPES(image, "bmp,gif,jpg,jpeg,png", {}, {}) // copyable
REGISTER_COMMON_RESOURCE_TYPES(text, "txt,js,cpp,h", {}, ResourceView::CanCopy)
REGISTER_COMMON_RESOURCE_TYPES(doc, "ppt,pptx,html,htm", {}, ResourceView::CanCopy)
REGISTER_COMMON_RESOURCE_TYPES(video, "mp4,wmv,ts,flv,m3u8,asf,avi,"
                                      "mp3,wma,wav,"
                                      "rtsp,rtmp,rtp", {}, {ResourceView::CanCopy})
REGISTER_RESOURCE_VIEW(Strokes, "stroke,glstroke")

//export_resource_Stroke;

#endif // RESOURCES_H
