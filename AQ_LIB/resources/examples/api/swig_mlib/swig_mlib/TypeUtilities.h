/* 
 * @brief			Collection of utility methods to do with data types
 * @Created:		14 March 2016 
 * @Author:			Joseph Ye
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once

#include "MBTemplateType.h"

class LAString;

namespace swig
{
	/* @brief			build StringMatrix from a vector of string vectors
	*  @param [out]		sMatrix			the string matrix object being built
	*  @param [in]		rhs				a vector of string vectors
	*/
	void buildStringMatrix(StringMatrix& sMatrix, const std::vector<std::vector <std::string> >& rhs);

	/* @brief			build DateVector from a vector of strings 
	*  @param [out]		dVector			A vector of dates
	*  @param [in]		rhs				a vector of strings
	*/
	void buildDateVector(DateVector& dVector, const std::vector<std::string>& rhs);

	/* @brief			build StringVector from a vector of strings 
	*  @param [out]		sVector			A StringVector object
	*  @param [in]		rhs				a vector of strings
	*/
	void buildStringVector(StringVector& sVector, const std::vector<std::string>& rhs);

	/* @brief			build a vector of strings from a DoubleVector object
	*  @param [in]		dVector			A DoubleVector object
	*  @output			date with expected format
	*/
	LAString fromStringToLAString(const std::string& inVal);

	/* @brief			build a matrix of strings from a StringMatrix object
	*  @param [in]		inVal	a StringMatrix object
	*  @output			output a matrix of strings
	*/
	std::vector<std::vector<std::string>> fromStringMatrixToMatrixOfString(const StringMatrix& inVal);
		
}
