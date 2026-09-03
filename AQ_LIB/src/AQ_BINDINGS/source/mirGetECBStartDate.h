/* 
 * @brief			Swig interface to Java for mirGetECBStartDate methods
 * @Created:		04 April 2016 
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once

#include <string>

/* @brief			swig interfac for the mirGetECBStartDate method
*  @param [in]		ecbDate ECB(European Central Bank) date
*  @return			The start date of an ECB Swap base on the ecb date
*/
std::string mirGetECBStartDate(const std::string& ecbDate);