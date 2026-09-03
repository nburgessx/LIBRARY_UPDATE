#include "TypeUtilities.h"
#include "mirGetDate.h"
#include "AQLDate.h"
#include "AQLString.h"
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
		AQLDate baseDate(BaseDate.c_str(),"YYYYMMDD");
		AQLString term(Term.c_str());
		AQLString slidingRule(SlidingRule.c_str());
		AQLString calendar(Calendar.c_str());
		AQLString rollConvention(RollConvention.c_str());

		ret = validation::tryMirGetDate(baseDate, term, slidingRule, calendar, rollConvention).getCString();
	} 
	catch (AQLCoreError& mesx) 
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

		AQLString term(Term.c_str());
		AQLString slidingRule(SlidingRule.c_str());
		AQLString calendar(Calendar.c_str());
		AQLString rollConvention(RollConvention.c_str());

		DateVector dates = validation::tryMirGetDate(baseDateVec, term, slidingRule, calendar, rollConvention);
		
		for(size_t i=0; i<dates.size(); ++i)
		{
			ret.push_back(dates[i].stringWithFormat("YYYYMMDD").getCString());
		}		
	} 
	catch (AQLCoreError& mesx) 
	{
		throw std::runtime_error(mesx.getMsg());
	} 
	
	return ret;
}
