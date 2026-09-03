/* 
 * @brief			Swig interface to Java for mirGetNextECBDate methods
 * @Created:		04 April 2016 
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#include "stdafx.h"

#include "mirGetNextECBDate.h"
#include "tryMirGetNextECBDate.h"


/* @brief			swig interface for the mirGetNextECBDate method
*  @param [in]		baseDate		Base date
*  @param [in]		strictlyAfter	True if strictly after the base date
*  @return			Date of the next ECB (European Central Bank) meeting on/after the base date
*/
std::string mirGetNextECBDate(const std::string& baseDate,
							bool strictlyAfter) throw(std::exception)
{

	std::string ret("");
	try 
	{
		// Input marshalling
		LADate baseDt(baseDate.c_str(),"YYYYMMDD");

		LADate date = validation_api::tryMirGetNextECBDate(baseDt, strictlyAfter);
		
		ret = date.stringWithFormat("YYYYMMDD").getCString();

	} 
	catch (MEError& mesx) 
	{
		throw std::exception(mesx.getMsg());
	} 
	
	return ret;

}