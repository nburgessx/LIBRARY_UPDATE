//
// @File: UserUtilities.h
// @Description: This is a test program
// @Created: 27 Feb 2016
// @Author: Hans Roggeman
// @Department: ISO Front Office Development
//
// The copyright to the computer program(s) herein
// is the property of Mizuho International.

#pragma once

// Includes: Standard Library
#include <string>
#include <boost/thread.hpp>

namespace etrading
{
    /* returns the value of the user name*/
    std::string getUserName();
    /* returns the value of an environment variable, if the environment variable does not exist, it returns an empty string */
    std::string getEnvironmentVariable( const std::string& varName );

    static boost::mutex consoleMtx;
    void logToConsole( const std::string& text );
}

