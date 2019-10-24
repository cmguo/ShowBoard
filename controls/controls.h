#ifndef CONTROLS_H
#define CONTROLS_H

#include "control.h"
#include "imagecontrol.h"
#include "videocontrol.h"
#include "strokecontrol.h"
#include "webcontrol.h"
#include "pptxcontrol.h"

#include <qexport.h>

REGISTER_CONTROL(ImageControl, "bmp,gif,jpg,jpeg,png")
REGISTER_CONTROL(VideoControl, "mp4,flv,ts,m3u8,rtsp")
REGISTER_CONTROL(StrokeControl, "stroke")
REGISTER_CONTROL(WebControl, "htm,html")
REGISTER_CONTROL(PptxControl, "ppt,pptx")

#endif // CONTROLS_H
