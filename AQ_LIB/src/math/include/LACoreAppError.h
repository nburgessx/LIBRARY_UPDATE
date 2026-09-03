#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LACoreError.h"

/*! 
    @brief Base class to handle errors at the application level.
*/
class LACoreAppError : public LACoreError
{
public:
//  LIFECYCLE
    // constructor
    LACoreAppError(const char_t*   msg, const char_t*   file, unsigned int line);
    // copy constructor
    LACoreAppError(const LACoreAppError& e);
};


/*! 
    @brief Class to handle errors related to data at the application level.
*/
class LACoreInvalidData : public LACoreAppError
{
public:
//  LIFECYCLE
    // constructor
    LACoreInvalidData(const char_t*   msg, const char_t*   file, unsigned int line);
    // copy constructor
    LACoreInvalidData(const LACoreInvalidData& e);
};


/*! 
    @brief Class to handle errors which occurs during the numerical calculation at the application level.
*/
class LACoreNumericalError : public LACoreAppError
{
public:
//  LIFECYCLE
    // constructor
    LACoreNumericalError(const char_t*   msg, const char_t*   file, 
                                            unsigned int line);
    // copy constructor
    LACoreNumericalError(const LACoreNumericalError& e);
};
