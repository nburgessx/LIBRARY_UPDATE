// HandleEnums.cpp

// Includes: This Library
#include "HandleEnums.h"
#include "StringUtilities.h"

// Includes: Standard Library
#include <vector>
#include <map>

namespace ETrading
{
    namespace Enums
    {

        /*
        const HandleType toHandleType( const string & handleTypeString )
        {
            ETrading::Utilities::uppercase( handleTypeString );

            map< string, HandleType > enumTable;
            map< string, HandleType >::iterator it;
            typedef pair< string, HandleType > enumPair;

            // Populate the Enum Table
            enumTable.insert( enumPair( "INVALID",      INVALID     ) );
            enumTable.insert( enumPair( "GRID",         GRID        ) );
            enumTable.insert( enumPair( "LOOKUP",       LOOKUP      ) );
            enumTable.insert( enumPair( "TABLE",        TABLE       ) );
            enumTable.insert( enumPair( "MATRIX",       MATRIX      ) );
            enumTable.insert( enumPair( "CURVE",        CURVE       ) );
            enumTable.insert( enumPair( "SWAP",         SWAP        ) );

            // Convert the String to an Enum
            it = enumTable.find( handleTypeString );

            // Return enum type or INVALID enum if not found
            return ( it == enumTable.end() ) ? INVALID : it->second;
        }

        const string toString( const HandleType & type )
        {
            return HandleTypeStrings[ type ];
        }

        const string listHandleTypes()
        {
            string result = string("");
            if ( HandleTypeStrings == NULL ) return result;

            const string COMMA = string(",");
            const string SPACE = string(" ");

            // Check size of string array, note we exit early above if there are zero elements to avoid divide by undefined
            unsigned int nHandleTypeStrings = sizeof( HandleTypeStrings ) / sizeof( HandleTypeStrings[0] );

            for( size_t i = 0; i < nHandleTypeStrings; ++i )
            {
                ( i == 0 ) ? result += HandleTypeStrings[i] : result += COMMA + SPACE + HandleTypeStrings[i];
            }

            return result;
        }

        */
    }
}