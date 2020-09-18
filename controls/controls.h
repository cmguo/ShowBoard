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

REGISTER_CONTROL(ImageControl, "image,bmp,gif,jpg,jpeg,png")
#ifdef QT_DEBUG
REGISTER_CONTROL(VideoControl, "video,mp4,wmv,ts,flv,m3u8,asf,avi,"
                               "audio,mp3,wma,wav,"
                               "rtsp,rtmp,rtp")
REGISTER_CONTROL(StrokeControl, "glstroke")
#endif
REGISTER_CONTROL(WebControl, "htm,html,http,https,chrome,swf")
#ifdef QT_DEBUG
REGISTER_CONTROL(QmlControl, "qml")
#endif
REGISTER_CONTROL(TextControl, "text,txt,js,cpp,h,qss,css")

#ifdef WIN32
REGISTER_CONTROL(PptxControl, "ppt,pptx")
# ifdef QT_DEBUG
REGISTER_CONTROL(WordControl, "doc,docx")
# endif
#endif

#endif // CONTROLS_H
