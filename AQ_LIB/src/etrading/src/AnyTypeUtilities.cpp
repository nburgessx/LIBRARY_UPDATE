// AnyTypeUtilities.cpp

/*
 * @brief			Utility file for Boost Variant or Any Type Helpers
 */

#include "AnyTypeUtilities.h"
#include <sstream>

namespace etrading
{
    // Method to cast boost any variant type to a string
    // anyValue             The anyType variant to cast
    // doublePrecision      The double precision to use when casting from double AnyTypes to string - defualts to 10 d.p.
    std::string fromAnyTypeToString( const AnyType& anyValue, const int doublePrecision)
    {
        if ( anyValue.type() == typeid(int) )
        {
    		return std::to_string( static_cast<long long>( boost::get<int>(anyValue) ) );
        }
        else if ( anyValue.type() == typeid(double) )
        {
            const double doubleValue = boost::get<double>(anyValue);
            if ( std::isnan(doubleValue) )
            {
                return "";
            }
            else
            {
                // Cast doubles to string streams with doublePrecision specified, defaults to 10 d.p.
                std::ostringstream out;
                out.precision( doublePrecision );
                out << std::fixed << doubleValue;
                return out.str();
            }
        }
        else if( anyValue.type() == typeid(bool) )
        {
    		return boost::get<bool>(anyValue) ? "true" : "false";
        }
        else if( anyValue.type() == typeid(std::string) )
        {
    		return boost::get< std::string >(anyValue);
        }
        else if( anyValue.type() == typeid(AQLString) )
        {
    		return boost::get<AQLString>(anyValue).getCString();
        }
        else if( anyValue.type() == typeid(const char*) )
        {
    		return boost::get<const char*>(anyValue);
        }
        else
        {
            throw AQLCoreInvalidData( "#Error: Unable to display AnyTypeMatrix data.", __FILE__, __LINE__ );
        }
        return "";
    }
}
