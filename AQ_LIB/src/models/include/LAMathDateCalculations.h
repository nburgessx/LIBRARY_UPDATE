#pragma once

#ifdef __GNUG__
#pragma interface
#endif


#include "LADate.h"
#include "LACoreAppError.h"
#include "LACoreTemplateType.h"
#include "LAMathCentralBank.h"


class LAString;
class LAObject;
class LAPriceDataSlidingRule;
class LAPriceDataCalendar;

enum RollConventionEnum
{
	ROLLCONV_ENUM_NONE,
	ROLLCONV_ENUM_IMM,
	ROLLCONV_ENUM_NORMAL,
	ROLLCONV_ENUM_STARTDATE,
	ROLLCONV_ENUM_ENDDATE,
	ROLLCONV_ENUM_EOM,
	ROLLCONV_ENUM_LUNAR
};

///////////////////////////////////////////////////////////////////////
/*! 
    @brief calculate date of schedule

*/

class LAMathDateCalculations
{
public:

	// calculate dates
	static	void				generateSchedule( const LADate& start, 
												  const LADate& end,
												  const LAString& data_frequency,
												  const bool isarrear,
												  const LADate* firstStubDate, 
												  const LADate* lastStubDate,
												  const int* pday,
												  DateVector& out,
												  const LAPriceDataSlidingRule* pbusdayrule = NULL,
												  const LAPriceDataCalendar* pcal = NULL,
												  const bool isstartroll = true,
												  const LAString* roll_convention=NULL);
	// Set term from string to integer 
	static	void				termStrtoYMDW(const LAString& term, int& y, int& m, int& d, int& w);

	// get date and roll (day is actual day)  
	static	LADate				getDate(const LADate& basedate, 
                                        const LAString& term, 
										bool rollForwards, 
                                        const LAString* roll_conv = 0);
	// get date, roll and slide (if roll convention is set, day is actual day, otherwise business day.)
	static	LADate				getDate(const LADate& basedate, 
                                        const LAString& term, 
										const LAPriceDataSlidingRule& busdayrule,
										const LAPriceDataCalendar* pCal,
										bool rollForwards,
										const LAString* roll_conv = 0);

	// get IMM date
	static	LADate				getIMMDate(const int& y, const int& m, bool isOddMonth = false);
	// get IMM date
	static	LADate				getImmEndDate(const LADate& startDate, const LAString& strTerm, int lag = 0);

    /*!
	@brief get futures contract start date

	@param[in]      month   futures contract month
	@param[in]      year    futures contract year
    @param[out]     returns the futures contract start date
    */
    static LADate               getFuturesContractStartDate(const unsigned int& month, const unsigned int& year );

	// get IMM date from future term
	static	LADate				getIMMDateFromTerm(const LADate& baseDate, const LAString& futureTerm);
	// calc FX spot date
	static  LADate				getFXSpotDate(const LAString& keyFX,
											  const LADate& basedate,
											  const LAString& calStr,
											  int spotlag,
											  bool rollForwards);

	// return true iff there is a scheduled Governing Council of the ECB monetary policy meeting 
	// on or after baseDate
	static bool					haveNextECBDate(const LADate& baseDate, bool strictlyAfter = true);

	// get the date of the next Governing Council of the ECB monetary policy meeting 
	// on or after baseDate; throw an exception after the last scheduled meeting
	static LADate				getNextECBDate(const LADate& baseDate, bool strictlyAfter = true);

	// get the start date of an ECB Swap with a given ECB date;
	// the start date is always the Wednesday strictly after the ECB date, 
	// even if the latter happens to be a Wednesday (GC, 12-10-2015).
	// note that the end date of an ECB Swap is the start date of the next ECB Swap;
	static LADate				getECBStartDate(const LADate& ecbDate);

	// return true iff there is a scheduled central bank meeting on or after baseDate
	static bool haveNextCBDate(
		const LAString& centralBankId, const LADate& baseDate, bool strictlyAfter);

	// get date of next central bank meeting on or after baseDate; 
	// throw MAAppError after last scheduled meeting
	static LADate getNextCBDate(
		const LAString& centralBankId, const LADate& baseDate, bool strictlyAfter);

	// get month from future term
	static	unsigned int		changeFutureMonthFormat(const LAString& futureMonth);
	
	// contert from double grid to dates grid
	static void convertToDateGrid(const LADate &asofDate, const DoubleArray &terms, DateVector &dates);
	
	// Return the frequency in months for comparing two frequencies, not for accurate calculations
	// *** Duplicate method in LADateHelpers.cpp ***
	static double getPeriodFrequencyInMonths(const LAString& freq);
	
	// calc compounding times
	static int calcCompoundingTimes(const LAString& freq_rst, const LAString& freq_pay);

	// get fedfund future dates from fedfundfuter term
	static DateVector			getFFDatesFromTerm(const LADate& baseDate, const LAString& fedfundTerm);

	// return next date with given weekday (e.g., Wednesday) on of after given baseDate
	static LADate getNextWeekdayDate(LADayOfWeekEnum weekday, const LADate& baseDate, bool strictlyAfter); 

protected:
  

private:
	// calculate dates based basic pattern
	static DateVector			generateRegularSchedule(const LADate& start, 
		                                        const LADate& end,
								                const LAString& data_frequency,
												const bool isarrear,
												const int* pday,
												const LAPriceDataSlidingRule* pbusdayrule,
												const LAPriceDataCalendar* pcal,
												const bool isstartroll,
												const LAString* roll_convention=NULL);

	// get adjust date
	static LADate getAdjDate(const LAPriceDataSlidingRule* pbusdayrule, const LAPriceDataCalendar* pcal, const LADate &date);
	// set date
	static void setDate(const int* pday,  LADate &date, const RollConventionEnum rollConventionEnum);
	// slide date
	static LADate slideDate(const LADate& date, const LAPriceDataSlidingRule& srule, const LAPriceDataCalendar* pCal);
	// roll date
	static LADate rollDate(const LADate& date, const LAString* roll_conv);
	// common implementation of have-/getNext [ECB/Fed/BoE] Date

	// return true iff there is a scheduled central bank meeting on or after baseDate,
	// in which case result gets updated with the meeting date
	static bool getIfExistsNextCBDate(
		const LAString& centralBankId, const LADate& baseDate, bool strictlyAfter, LADate& result);
};

//
// inline method implementation
//

inline bool	LAMathDateCalculations::haveNextECBDate(const LADate& baseDate, bool strictlyAfter)
{
	return haveNextCBDate("ECB", baseDate, strictlyAfter);
}

inline LADate LAMathDateCalculations::getNextECBDate(const LADate& baseDate, bool strictlyAfter)
{
	return getNextCBDate("ECB", baseDate, strictlyAfter);
}

inline LADate LAMathDateCalculations::getECBStartDate(const LADate& ecbDate)
{
	return getNextWeekdayDate(WED, ecbDate, true);
}
