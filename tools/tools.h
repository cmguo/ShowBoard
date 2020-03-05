#ifndef TOOLS_H
#define TOOLS_H

#include "core/control.h"
#include "drawingtool.h"
#include "converttool.h"
#include <qexport.h>

REGISTER_CONTROL(DrawingTool, "drawing")
REGISTER_CONTROL(ConvertTool, "converttool")

#endif // TOOLS_H
