// StringUtilities.h

#ifndef ETRADING_HANDLES_STRING_UTILITIES_H
#define ETRADING_HANDLES_STRING_UTILITIES_H

// Includes: Standard Library
#include <string>

// Namespaces
using namespace std;

namespace ETrading
{
    namespace Utilities
    {
        string          uppercase( string s );
        string          intToString( const unsigned int& i );
        unsigned int    stringToInt( const string& s );
    }
}

#endif ETRADING_HANDLES_STRING_UTILITIES_H