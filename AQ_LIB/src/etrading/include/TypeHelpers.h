/*
 * @brief			Helper mehtods that are to do with data types
 * @Created:		26 April 2016
 * @Author:			Joseph Ye
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once

#include "LACoreTemplateType.h"

namespace etrading
{
    /* @brief	Template method that converts a number to a string
    *  @param [in]	number		Input of a certain numeric data type
    *  @return		String output
    */
    template <typename T>
    std::string NumberToString ( T number )
    {
        std::stringstream ss;
        ss << number;
        return ss.str();
    }

    /* @brief	Template method that converts a string to a number
    *  @param [in]	text	String to be converted
    *  @return		Output of a certain numeric data type
    */
    template <typename T>
    T StringToNumber ( const std::string& text )
    {
        std::stringstream ss( text );
        T result;
        return ss >> result ? result : 0;
    }
}


