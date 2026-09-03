/* 
 * @brief			Swig interface to Java for meProductOISSwapPV method
 * @Created:		03 June 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#pragma once

#include <string>
#include <vector>

/* @brief			swig interface for meProductOISSwapPV
*  @param [in]		oisSwapLVB		A label value block defining the ois swap
*  @param [in]		validateKeys		True to validate the all keys provided are valid. Default to TRUE
*  @return			OIS PV
*/
double meProductOISPV(const std::vector<std::vector <std::string> >& oisSwapLVB, 
						bool validateKeys=true);


/* @brief			return a set of expected keys for swap pv label value block
*  @return			expected keys
*/
std::vector<std::string> meProductOISPVLVBKeys();