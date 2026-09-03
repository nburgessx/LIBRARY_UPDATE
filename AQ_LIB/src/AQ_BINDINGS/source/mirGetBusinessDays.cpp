/* 
 * @brief			Swig interface to Java for mirGetBusinessDays methods
 * @Created:		04 April 2016 
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

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
					const std::string& calendar) 
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
	catch (LACoreError& mesx) 
	{
		throw std::runtime_error(mesx.getMsg());
	} 
	
	return ret;

}