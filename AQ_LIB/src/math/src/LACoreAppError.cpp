/*! @file
    @brief Implementation of the class to handle errors at the application level

*/


#ifdef __GNUG__
#pragma implementation
#endif

#include "LACoreAppError.h"


/*!
    @brief constructor

    set error data

    @param[in] msg error message
    @param[in] file file name that error occurs
    @param[in] line line number of the file that error occurs
    
*/
LACoreAppError::LACoreAppError(const char_t*   msg, const char_t*   file, 
                                                unsigned int line)
:LACoreError(msg, file, line)
{
}

/*!
    @brief copy constructor
    @param[in] e original error object
*/
LACoreAppError::LACoreAppError(const LACoreAppError& e)
:LACoreError(e)
{
}

/*!
    @brief constructor

    set error data

    @param[in] msg error message
    @param[in] file file name that error occurs
    @param[in] line line number of the file that error occurs
    
*/
LACoreInvalidData::LACoreInvalidData(const char_t*   msg, const char_t*   file, 
                                                    unsigned int line)
:LACoreAppError(msg, file, line)
{
}

/*!
    @brief copy constructor
    @param[in] e original error object
*/
LACoreInvalidData::LACoreInvalidData(const LACoreInvalidData& e)
:LACoreAppError(e)
{
}

/*!
    @brief constructor

    set error data

    @param[in] msg error message
    @param[in] file file name that error occurs
    @param[in] line line number of the file that error occurs
    
*/
LACoreNumericalError::LACoreNumericalError(const char_t*   msg, const char_t*   file, 
                                   unsigned int line)
:LACoreAppError(msg, file, line)
{
}

/*!
    @brief copy constructor
    @param[in] e original error object
*/
LACoreNumericalError::LACoreNumericalError(const LACoreNumericalError& e)
:LACoreAppError(e)
{
}
