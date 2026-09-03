#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreError.h"

/*! 
    @brief Class to handle errors at the system level.
*/
class AQLCoreSystemError : public AQLCoreError
{
public:
//  LIFECYCLE
    // constructor
    AQLCoreSystemError(const char_t* file, unsigned int line);
    // constructor
    AQLCoreSystemError(const char_t* msg, const char_t* file, unsigned int line);

    // copy constructor
    AQLCoreSystemError(const AQLCoreSystemError& e);
};
