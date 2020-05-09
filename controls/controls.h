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
#include "textcontrol.h"

#include <qexport.h>

REGISTER_CONTROL(ImageControl, "bmp,gif,jpg,jpeg,png")
REGISTER_CONTROL(VideoControl, "mp4,wmv,ts,flv,m3u8,asf,avi,"
                               "mp3,wma,wav,"
                               "rtsp,rtmp,rtp")
REGISTER_CONTROL(StrokeControl, "glstroke")
REGISTER_CONTROL(WebControl, "htm,html,http,https,chrome")
REGISTER_CONTROL(QmlControl, "qml")
REGISTER_CONTROL(TextControl, "txt,js,cpp,h")

#ifdef WIN32
REGISTER_CONTROL(PptxControl, "ppt,pptx")
REGISTER_CONTROL(WordControl, "doc,docx")
#endif

#endif // CONTROLS_H
