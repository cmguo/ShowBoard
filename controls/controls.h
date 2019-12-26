#ifndef CONTROLS_H
#define CONTROLS_H

#include "core/control.h"
#include "imagecontrol.h"
#include "videocontrol.h"
#include "strokecontrol.h"
#include "webcontrol.h"
#include "pptxcontrol.h"
#include "qmlcontrol.h"
#include "wordcontrol.h"

#include <qexport.h>

REGISTER_CONTROL(ImageControl, "bmp,gif,jpg,jpeg,png")
REGISTER_CONTROL(VideoControl, "mp4,flv,ts,m3u8,rtsp")
REGISTER_CONTROL(StrokeControl, "glstroke")
REGISTER_CONTROL(WebControl, "htm,html,http")
REGISTER_CONTROL(PptxControl, "ppt,pptx")
REGISTER_CONTROL(QmlControl, "qml")
REGISTER_CONTROL(WordControl, "doc,docx")

#endif // CONTROLS_H
