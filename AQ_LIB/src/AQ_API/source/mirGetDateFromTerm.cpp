#include "mirGetDateFromTerm.h"
#include "AQLDate.h"
#include "AQLString.h"
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
		AQLDate AQLDate(FromDate.c_str(),"YYYYMMDD");
		AQLString mbDayCount(DayCount.c_str());
		ret = validation::tryMirGetDateFromTerm(AQLDate, TermY, mbDayCount, IncludeLast).getCString();
	} 
	catch (AQLCoreError& mesx) 
	{
		throw std::runtime_error(mesx.getMsg());
	} 

	return ret;

}