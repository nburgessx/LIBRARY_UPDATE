/* 
 * @brief			Swig interface to Java for mirGetECBStartDate methods
 * @Created:		04 April 2016 
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#include "stdafx.h"

#include "mirGetECBStartDate.h"
#include "tryMirGetECBStartDate.h"


/* @brief			swig interfac for the mirGetECBStartDate method
*  @param [in]		ecbDate ECB(European Central Bank) date
*  @return			The start date of an ECB Swap base on the ecb date
*/
std::string mirGetECBStartDate(const std::string& ecbDate) throw(std::exception)
{

	std::string ret("");
	try 
	{
		// Input marshalling
		LADate ecbDt(ecbDate.c_str(),"YYYYMMDD");

		LADate date = validation_api::tryMirGetECBStartDate(ecbDt);
		ret = date.stringWithFormat("YYYYMMDD").getCString();

	} 
	catch (MEError& mesx) 
	{
		throw std::exception(mesx.getMsg());
	} 
	
	return ret;

}