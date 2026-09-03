// StringUtilities.cpp

// Includes: This Library
#include "StringUtilities.h"

// Includes: Standard Library
#include <sstream>

namespace ETrading
{
    namespace Utilities
    {
        string uppercase( string s )
        {
            for( unsigned int i = 0; i < s.length(); ++i )
            {
                s[i] = toupper( s[i] );
            }
            return s;
        }

        string intToString( const unsigned int& i )
        {
            stringstream ss;
            ss << i;
            return ss.str();
        }

        unsigned int stringToInt( const string& s )
        {
            stringstream ss( s );
            unsigned int result;
            return ss >> result ? result : 0;
        }

    }
}