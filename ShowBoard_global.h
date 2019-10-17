#ifndef SHOWBOARD_GLOBAL_H
#define SHOWBOARD_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(SHOWBOARD_LIBRARY)
#  define SHOWBOARD_EXPORT Q_DECL_EXPORT
#else
#  define SHOWBOARD_EXPORT Q_DECL_IMPORT
#endif

#endif // SHOWBOARD_GLOBAL_H
