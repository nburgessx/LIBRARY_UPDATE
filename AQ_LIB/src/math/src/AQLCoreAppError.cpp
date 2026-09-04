/*! @file
    @brief Implementation of the class to handle errors at the application level
*/


#ifdef __GNUG__
#pragma implementation
#endif

#include "AQLCoreAppError.h"


/*!
    @brief constructor

    set error data

    @param[in] msg error message
    @param[in] file file name that error occurs
    @param[in] line line number of the file that error occurs
    
*/
AQLCoreAppError::AQLCoreAppError(const char_t*   msg, const char_t*   file, 
                                                unsigned int line)
:AQLCoreError(msg, file, line)
{
}

/*!
    @brief copy constructor
    @param[in] e original error object
*/
AQLCoreAppError::AQLCoreAppError(const AQLCoreAppError& e)
:AQLCoreError(e)
{
}

/*!
    @brief constructor

    set error data

    @param[in] msg error message
    @param[in] file file name that error occurs
    @param[in] line line number of the file that error occurs
    
*/
AQLCoreInvalidData::AQLCoreInvalidData(const char_t*   msg, const char_t*   file, 
                                                    unsigned int line)
:AQLCoreAppError(msg, file, line)
{
}

/*!
    @brief copy constructor
    @param[in] e original error object
*/
AQLCoreInvalidData::AQLCoreInvalidData(const AQLCoreInvalidData& e)
:AQLCoreAppError(e)
{
}

/*!
    @brief constructor

    set error data

    @param[in] msg error message
    @param[in] file file name that error occurs
    @param[in] line line number of the file that error occurs
    
*/
AQLCoreNumericalError::AQLCoreNumericalError(const char_t*   msg, const char_t*   file, 
                                   unsigned int line)
:AQLCoreAppError(msg, file, line)
{
}

/*!
    @brief copy constructor
    @param[in] e original error object
*/
AQLCoreNumericalError::AQLCoreNumericalError(const AQLCoreNumericalError& e)
:AQLCoreAppError(e)
{
}
