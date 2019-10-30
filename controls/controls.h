#ifndef CONTROLS_H
#define CONTROLS_H

#include "core/control.h"
#include "imagecontrol.h"
#include "videocontrol.h"
#include "strokecontrol.h"
#include "webcontrol.h"
#include "pptxcontrol.h"
#include "graph2dcontrol.h"

#include <qexport.h>

REGISTER_CONTROL(ImageControl, "bmp,gif,jpg,jpeg,png")
REGISTER_CONTROL(VideoControl, "mp4,flv,ts,m3u8,rtsp")
REGISTER_CONTROL(StrokeControl, "stroke")
REGISTER_CONTROL(WebControl, "htm,html")
REGISTER_CONTROL(PptxControl, "ppt,pptx")
REGISTER_CONTROL(Graph2DControl, "graph2d")

#endif // CONTROLS_H
