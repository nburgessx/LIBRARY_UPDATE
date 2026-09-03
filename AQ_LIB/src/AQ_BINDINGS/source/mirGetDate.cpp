/* 
 * @brief			Swig interface to Java for function mirGetDate
 * @Created:		16 March 2016 
 * @Author:			Joseph Ye
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#include "TypeUtilities.h"
#include "mirGetDate.h"
#include "LADate.h"
#include "LAString.h"
#include "tryMirGetDate.h"



/* @brief			swig interface for mirGetDate
*  @param [in]		BaseDate		The from date
*  @param [in]		Term			Term added to the from date
*  @param [in]		SlidingRule		Rule that decides if end date should roll forward or backward etc, ie, MF
*  @param [in]		Calendar		Calendar
*  @param [in]		RollConvention	Roll conventions, ie, Normal, IMM, EOM, Lunar, etc
*  @return			The end date
*/
std::string mirGetDate(const std::string& BaseDate, 
						const std::string& Term, 
						const std::string& SlidingRule, 
						const std::string& Calendar, 
						const std::string& RollConvention)
{
	std::string ret;
	try 
	{
		// Input marshalling
		LADate baseDate(BaseDate.c_str(),"YYYYMMDD");
		LAString term(Term.c_str());
		LAString slidingRule(SlidingRule.c_str());
		LAString calendar(Calendar.c_str());
		LAString rollConvention(RollConvention.c_str());

		ret = validation_api::tryMirGetDate(baseDate, term, slidingRule, calendar, rollConvention).getCString();
	} 
	catch (LACoreError& mesx) 
	{
		throw std::runtime_error(mesx.getMsg());
	} 
	
	return ret;
}


/* @brief			swig interface for mirGetDate
*  @param [in]		BaseDates		A vector of from dates
*  @param [in]		Term			Term added to the from date
*  @param [in]		SlidingRule		Rule that decides if end date should roll forward or backward etc, ie, MF
*  @param [in]		Calendar		Calendar
*  @param [in]		RollConvention	Roll conventions, ie, Normal, IMM, EOM, Lunar, etc
*  @return			A vector of end dates
*/
std::vector<std::string> mirGetDate(const std::vector<std::string>& BaseDates, 
									const std::string& Term, 
									const std::string& SlidingRule, 
									const std::string& Calendar, 
									const std::string& RollConvention)
{
	std::vector<std::string> ret;
	try 
	{
		// Input marshalling
		DateVector baseDateVec;
		swig::buildDateVector(baseDateVec, BaseDates);

		LAString term(Term.c_str());
		LAString slidingRule(SlidingRule.c_str());
		LAString calendar(Calendar.c_str());
		LAString rollConvention(RollConvention.c_str());

		DateVector dates = validation_api::tryMirGetDate(baseDateVec, term, slidingRule, calendar, rollConvention);
		
		for(size_t i=0; i<dates.size(); ++i)
		{
			ret.push_back(dates[i].stringWithFormat("YYYYMMDD").getCString());
		}		
	} 
	catch (LACoreError& mesx) 
	{
		throw std::runtime_error(mesx.getMsg());
	} 
	
	return ret;
}
