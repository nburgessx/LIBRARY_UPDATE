/*
 * @brief			Methods that assist in carrying out MLib tests
 * @Created:		23 Feb 2017
 * @Author:			Joseph Ye
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#include "LAString.h"
#include "ReadDataFile.h"
#include <iostream>

namespace google_test
{
    /* @brief			Find a value by a given key from a ReadDataFile object
    * @param [out]		val				Value of the given key
	* @param [in]		collection		ReadDataFile object
    * @param [in]		keys			All keys in the ReadDataFile object
    * @param [in]		key				Key that gives value
	* @param [in]		optional		Is optional key?
    */
	void findValByKey(LAString& val, const etrading::ReadDataFile& collection, const std::set<LAString>& keys, const LAString& key, bool optional = false);

	/* @brief			Return the short name of interpolation types
	* @param [in]		interpolation	Interpolation in long name
    * @output			interpolation short name
    */
	LAString interpolationShortName(const LAString& interpolation);

	/* @brief			Return the Bond Spread tolerance
	*/
	double getBondSpreadTolerance();

#define GTEST_WARNING( warn_message ) \
	std::cout << "\n#Warning: " << warn_message << "\n" << std::endl; \
 

#define GTEST_CONDITIONAL_WARNING( condition, warn_message ) \
	if (! ( condition ) ) \
	{ \
		std::cout << "\n#Warning: " << warn_message << "\n" << std::endl; \
    } \


}