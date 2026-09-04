// APISetUp.h

/* 
 * @brief			Collection of SetUp Macros and Utilities for the AQ_CLIENT_API
 */

#pragma once

#include <cstring>

#define AQ_API_START  \
    try{


#define AQ_API_END                                                \
    }                                                               \
    catch(const AQLCoreError& e)	                                    \
    {						                                        \
        const char* cstr = e.getMsg();                              \
        const size_t msgSize = std::strlen( cstr );                 \
        throw std::runtime_error( cstr );                           \
    }                                                               \
    catch(const std::exception& e)	                                \
    {						                                        \
        throw std::runtime_error( e.what() );                       \
    }  

