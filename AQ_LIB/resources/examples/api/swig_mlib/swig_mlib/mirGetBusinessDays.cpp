/* 
 * @brief			Swig interface to Java for mirGetBusinessDays methods
 * @Created:		04 April 2016 
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#include "stdafx.h"

#include "mirGetBusinessDays.h"
#include "tryMirGetBusinessDays.h"


/* @brief			 swig interface for the mirGetBusinessDays method
*  @param [in]		fromDate	    From Date
*  @param [in]		toDate			To Date
*  @param [in]		calendar		Calendar
*  @return			Number of business days between fromDate and toDate 
*/
int mirGetBusinessDays(const std::string& fromDate, 
					const std::string& toDate,
					const std::string& calendar) throw(std::exception) 
{
	int ret = 0;
	try 
	{
		// Input marshalling
		LADate fromDt(fromDate.c_str(),"YYYYMMDD");
		LADate toDt (toDate.c_str(),"YYYYMMDD");
		LAString cal			(calendar.c_str());

		ret = validation_api::tryMirGetBusinessDays(fromDt, toDt, cal);		
	} 
	catch (MEError& mesx) 
	{
		throw std::exception(mesx.getMsg());
	} 
	
	return ret;

}