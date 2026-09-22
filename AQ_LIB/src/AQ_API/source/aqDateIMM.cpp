#include "TypeUtilities.h"
#include "aqDateIMM.h"
#include "tryAqDateIMM.h"
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros
#include "ParameterValidation.h"


/* @brief			swig interface for aqDateIMMFromMonth. 
*  @param [in]		year			The year of the IMM date
*  @param [in]		month			The month of the IMM date
*  @param [in]		calendar		Calendar
*  @param [in]		businessDayAdj	Rule that decides if end date should roll forward or backward etc, ie, MF
*  @return			The IMM date on the given month
*/
std::string aqDateIMMFromMonth(int year, 
							int month, 
							const std::string& calendar, 
							const std::string& businessDayAdj)
{
	AQ_API_START
    std::string ret;
	
	AQLString tmp_calendar(calendar.c_str());
	AQLString tmp_businessDayAdj(businessDayAdj.c_str());

	ret = validation::tryAqDateIMMFromMonth(year, month, tmp_calendar, tmp_businessDayAdj).stringWithFormat("YYYYMMDD").getCString();
	return ret;
    AQ_API_END
}


/* @brief			swig interface for aqDateNthIMM 
*  @param [in]		year			The year of the IMM date
*  @param [in]		nth				The n'th IMM date of the year
*  @param [in]		calendar		Calendar
*  @param [in]		businessDayAdj	Rule that decides if end date should roll forward or backward etc, ie, MF
*  @return			The nth IMM date of the year
*/
std::string aqDateNthIMM(int year, 
						int nth, 
						const std::string& calendar, 
						const std::string& businessDayAdj)
{
    AQ_API_START
	std::string ret;
	
	AQLString tmp_calendar(calendar.c_str());
	AQLString tmp_businessDayAdj(businessDayAdj.c_str());

	ret = validation::tryAqDateNthIMM(year, nth, tmp_calendar, tmp_businessDayAdj).stringWithFormat("YYYYMMDD").getCString();
	return ret;
    AQ_API_END
}


/* @brief			swig interface for aqDateNthIMMFromStartDate
*  @param [in]		startDate		The start (reference) date
*  @param [in]		nth				The nth IMM date from the start date, starting from the one closest to the start date
*  @param [in]		calendar		Calendar
*  @param [in]		businessDayAdj	Rule that decides if end date should roll forward or backward etc, ie, MF
*  @return			The nth IMM date from the base date, starting from the one closest to the base date
*/
std::string aqDateNthIMMFromStartDate(const std::string& startDate, 
									int nth, 
									const std::string& calendar, 
									const std::string& businessDayAdj)
{
    AQ_API_START
	std::string ret;

	AQLDate tmp_startDate( etrading::stringToDate( startDate ) );
	AQLString tmp_calendar(calendar.c_str());
	AQLString tmp_businessDayAdj(businessDayAdj.c_str());

	ret = validation::tryAqDateNthIMMFromStartDate(tmp_startDate, nth, tmp_calendar, tmp_businessDayAdj).stringWithFormat("YYYYMMDD").getCString();
	return ret;
    AQ_API_END
}

/* @brief			swig interface for aqDateIMMCurrent
*  @param [in]		valuationDate	The valuation date
*  @param [in]		includeToday	Optional. Default FALSE. TRUE lets an IMM date on the valuation date count as current
*  @return			The current (most recent) IMM start date, which may be in the past
*/
std::string aqDateIMMCurrent( const std::string& valuationDate,
                              const bool includeToday )
{
    AQ_API_START
	std::string ret;

	AQLDate tmp_valuationDate( etrading::stringToDate( valuationDate ) );

	ret = validation::tryAqDateIMMCurrent( tmp_valuationDate, includeToday ).stringWithFormat("YYYYMMDD").getCString();
	return ret;
    AQ_API_END
}

/* @brief			swig interface for aqDateIMMNext
*  @param [in]		referenceDate	The IMM reference date
*  @return			The next IMM start date relative to the reference date
*/
std::string aqDateIMMNext( const std::string& referenceDate )
{
    AQ_API_START
	std::string ret;

	AQLDate tmp_referenceDate( etrading::stringToDate( referenceDate ) );

	ret = validation::tryAqDateIMMNext( tmp_referenceDate ).stringWithFormat("YYYYMMDD").getCString();
	return ret;
    AQ_API_END
}

/* @brief			swig interface for aqDateIMMPrevious
*  @param [in]		referenceDate	The IMM reference date
*  @return			The previous IMM start date relative to the reference date
*/
std::string aqDateIMMPrevious( const std::string& referenceDate )
{
    AQ_API_START
	std::string ret;

	AQLDate tmp_referenceDate( etrading::stringToDate( referenceDate ) );

	ret = validation::tryAqDateIMMPrevious( tmp_referenceDate ).stringWithFormat("YYYYMMDD").getCString();
	return ret;
    AQ_API_END
}

/* @brief			swig interface for aqDateIMMNth
*  @param [in]		valuationDate	The valuation date
*  @param [in]		nthIMM			Which IMM date; positive is forward, negative is backward
*  @param [in]		includeToday	Optional. Default FALSE. TRUE lets an IMM date on the valuation date count
*  @return			The nth IMM date relative to the valuation date
*/
std::string aqDateIMMNth( const std::string& valuationDate,
                         const int nthIMM,
                         const bool includeToday )
{
    AQ_API_START
	std::string ret;

	AQLDate tmp_valuationDate( etrading::stringToDate( valuationDate ) );

	ret = validation::tryAqDateIMMNth( tmp_valuationDate, nthIMM, includeToday ).stringWithFormat("YYYYMMDD").getCString();
	return ret;
    AQ_API_END
}