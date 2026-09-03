#include "InitializeAQETrading.h"
#include "mirGetTerm.h"
#include "tryMirGetTerm.h"


/* @brief			 swig interface for the mirGetTerm method
*  @param [in]		fromDate	    From Date
*  @param [in]		toDate			To Date
*  @param [in]		dayCount		Day count convention
*  @param[in]		includelast		True(default):include the last day and not include start day; False:include start day and not include last day 
*  @return			Term between fromDate and toDate 
*/
double mirGetTerm(const std::string& fromDate, 
			const std::string& toDate,
			const std::string& dayCount,
			bool includeLast) 
{
	double ret = 0.0;
	try 
	{
		// Input marshalling
		LADate fromDt(fromDate.c_str(), "YYYYMMDD");
		LADate toDt(toDate.c_str(), "YYYYMMDD");
		LAString dayC(dayCount.c_str());

		ret = validation::tryMirGetTerm(fromDt, toDt, dayC, includeLast);		
	} 
	catch (LACoreError& mesx) 
	{
		throw std::runtime_error(mesx.getMsg());
	} 
	
	return ret;

}