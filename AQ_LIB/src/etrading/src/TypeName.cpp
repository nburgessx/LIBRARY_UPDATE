#include "TypeName.h"

namespace etrading
{

	// a specialization of std::string (which is a container of type std::basic_string) to keep the display simple
    template <>
    typename std::enable_if<is_container<std::string>::value, std::string>::type TypeName::get<std::string>()
    {
		return std::string( "std::string" );
    }

}

