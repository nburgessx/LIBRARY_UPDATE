#include "mirGetDateFromTerm.h"
#include "LADate.h"
#include "LAString.h"
#include "tryMirGetDateFromTerm.h"

/* @brief			swig interface for mirGetDateFromTerm
*  @param [in]		FromDate			The curve collection ID
*  @param [in]		TermY		Name of the curve constructed by this method
*  @param [in]		DayCount	General curve properties such as asofdate, ccy, interp, etc
*  @param [in]		IncludeLast			The OIS curve configuration info
*  @return			The end date derived from the FromDate and given term
*/
const std::string mirGetDateFromTerm(const std::string& FromDate, 
									double TermY, 
									const std::string& DayCount, 
									bool IncludeLast) 
{
	std::string ret;
	try 
	{
		// Data type marshalling
		LADate LADate(FromDate.c_str(),"YYYYMMDD");
		LAString mbDayCount(DayCount.c_str());
		ret = validation_api::tryMirGetDateFromTerm(LADate, TermY, mbDayCount, IncludeLast).getCString();
	} 
	catch (LACoreError& mesx) 
	{
		throw std::runtime_error(mesx.getMsg());
	} 

	return ret;

}