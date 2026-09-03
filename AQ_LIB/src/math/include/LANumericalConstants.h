#pragma once

#ifdef __GNUG__
#pragma interface
#else
#pragma warning( disable : 4290 )
#endif
#include <limits>

#define MLIB_PI 3.141592653589793238462643383280
#define MLIB_MACHINE_MAX std::numeric_limits<double>::max()
#define MLIB_MACHINE_EPSILON std::numeric_limits<double>::epsilon()
