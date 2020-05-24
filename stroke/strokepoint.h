#ifndef STROKEPOINT_H
#define STROKEPOINT_H

#include "ShowBoard_global.h"

#include <QPointF>

// for head (range)
// x: x range
// y: y range
// s: 1
// p: presure range ( < 17783)
// t: time units (in 1ms)
//   0: no time
//   1: 1ms unit, 65 seconds, 1 minute
//   10: 10ms unit, 655 seconds, 10 minutes
//   100: 100ms unit, 6553 seconds, 109 minutes

class SHOWBOARD_EXPORT StrokePoint
{
public:
    ushort t; // time
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
    ushort s: 1; // state (1 bit)
    ushort p : 15; // pressure or flags if state is 1
#else
    ushort p : 15;
    ushort s: 1;
#endif
    ushort x;
    ushort y;

public:
    static StrokePoint EndStorke;
    operator QPoint() const { return {x, y}; }
    operator QPointF() const { return QPoint{x, y}; }
    char * data() { return reinterpret_cast<char *>(this); }
    char const * data() const { return reinterpret_cast<char const *>(this); }
};

Q_STATIC_ASSERT(sizeof (StrokePoint) == 8);

#endif // STROKEPOINT_H
