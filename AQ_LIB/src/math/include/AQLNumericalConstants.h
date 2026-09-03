#pragma once

#ifdef __GNUG__
#pragma interface
#else
#pragma warning( disable : 4290 )
#endif
#include <limits>

#define AQ_PI 3.141592653589793238462643383280
#define AQ_MACHINE_MAX std::numeric_limits<double>::max()
#define AQ_MACHINE_EPSILON std::numeric_limits<double>::epsilon()
