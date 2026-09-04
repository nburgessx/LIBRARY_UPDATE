//
// @Description: This is a test program

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

