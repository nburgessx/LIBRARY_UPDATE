// APISetUp.h

/* 
 * @brief			Collection of SetUp Macros and Utilities for the MLIB_CLIENT_API
 * @Created:	    21 September 2018
 * @Author:			Nicholas Burgess
 * @Department:		AlgoQuantHub Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#pragma once

#include <cstring>

#define MLIB_API_START  \
    try{


#define MLIB_API_END                                                \
    }                                                               \
    catch(const LACoreError& e)	                                    \
    {						                                        \
        const char* cstr = e.getMsg();                              \
        const size_t msgSize = std::strlen( cstr );                 \
        throw std::runtime_error( cstr );                           \
    }                                                               \
    catch(const std::exception& e)	                                \
    {						                                        \
        throw std::runtime_error( e.what() );                       \
    }  

