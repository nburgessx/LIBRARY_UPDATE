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
		AQLDate fromDt(fromDate.c_str(),"YYYYMMDD");
		AQLDate toDt (toDate.c_str(),"YYYYMMDD");
		AQLString cal			(calendar.c_str());

		ret = validation::tryMirGetBusinessDays(fromDt, toDt, cal);		
	} 
	catch (AQLCoreError& mesx) 
	{
		throw std::runtime_error(mesx.getMsg());
	} 
	
	return ret;

}