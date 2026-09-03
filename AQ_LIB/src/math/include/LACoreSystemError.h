#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LACoreError.h"

/*! 
    @brief Class to handle errors at the system level.
*/
class LACoreSystemError : public LACoreError
{
public:
//  LIFECYCLE
    // constructor
    LACoreSystemError(const char_t* file, unsigned int line);
    // constructor
    LACoreSystemError(const char_t* msg, const char_t* file, unsigned int line);

    // copy constructor
    LACoreSystemError(const LACoreSystemError& e);
};
