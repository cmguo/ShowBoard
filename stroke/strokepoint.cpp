#include "strokepoint.h"

StrokePoint StrokePoint::EndStorke = {
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
    0, 1, 0, 0, 0
#else
    0, 0, 1, 0, 0
#endif
};
