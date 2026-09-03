/*
 * @brief			TypeName Template specializations
 * @Created:		2 Aug 2018
 * @Author:			Ian Castleton
 * @Department:		Quant Research & Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

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

