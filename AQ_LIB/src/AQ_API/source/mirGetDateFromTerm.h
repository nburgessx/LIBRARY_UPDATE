#pragma once

#include <string>
#include <vector>

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
									bool IncludeLast);