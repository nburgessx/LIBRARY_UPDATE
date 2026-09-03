#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreError.h"

/*! 
    @brief Base class to handle errors at the application level.
*/
class AQLCoreAppError : public AQLCoreError
{
public:
//  LIFECYCLE
    // constructor
    AQLCoreAppError(const char_t*   msg, const char_t*   file, unsigned int line);
    // copy constructor
    AQLCoreAppError(const AQLCoreAppError& e);
};


/*! 
    @brief Class to handle errors related to data at the application level.
*/
class AQLCoreInvalidData : public AQLCoreAppError
{
public:
//  LIFECYCLE
    // constructor
    AQLCoreInvalidData(const char_t*   msg, const char_t*   file, unsigned int line);
    // copy constructor
    AQLCoreInvalidData(const AQLCoreInvalidData& e);
};


/*! 
    @brief Class to handle errors which occurs during the numerical calculation at the application level.
*/
class AQLCoreNumericalError : public AQLCoreAppError
{
public:
//  LIFECYCLE
    // constructor
    AQLCoreNumericalError(const char_t*   msg, const char_t*   file, 
                                            unsigned int line);
    // copy constructor
    AQLCoreNumericalError(const AQLCoreNumericalError& e);
};
