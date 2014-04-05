#ifndef DLLGLOBAL_H
#define DLLGLOBAL_H

#include <QtCore/qglobal.h>

#ifndef AS_DYNAMIC_LIB
# define AS_DYNAMIC_LIB Q_DECL_EXPORT
#else
# define AS_DYNAMIC_LIB Q_DECL_IMPORT
#endif

#endif
