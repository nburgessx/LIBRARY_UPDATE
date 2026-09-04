//
// @Description: This is a test program


// Includes: Standard Library
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

#include <string>
#include <iostream>
#include <cstdlib>
#include <boost/algorithm/string.hpp>

// Includes: This Library
#include "UserUtilities.h"

namespace etrading
{

    std::string getUserName()
    {
#if defined(_WIN32) || defined(_WIN64)
        char usernameChar[100];
        DWORD usernameSize = sizeof( usernameChar );
        return GetUserName( usernameChar, &usernameSize ) ? boost::to_upper_copy<std::string>( std::string( usernameChar ) ) : "UNKNOWN";
#else
        return boost::to_upper_copy<std::string>( getEnvironmentVariable( "USER" ) );
#endif
    }

    /* returns the value of an environment variable, if the environment variable does not exist, it returns an empty string */
    std::string getEnvironmentVariable( const std::string& varName )
    {
        const char* valueOfVarName = getenv( varName.c_str() );
        return ( valueOfVarName == nullptr ) ? std::string( "" ) : std::string( valueOfVarName );
    }

    void logToConsole( const std::string& text )
    {
#if defined(_DEBUG)
        boost::lock_guard<boost::mutex> guard( consoleMtx );
        std::cout << text.c_str() << std::endl;
#endif
    }


}

