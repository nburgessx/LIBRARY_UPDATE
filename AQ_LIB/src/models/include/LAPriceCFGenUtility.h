#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLDate.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"

enum RoundFunction
{
    ROUND,			// Round
    ROUND_UP,		// Round Up
    ROUND_DOWN		// Round Down
};



class AQLString;
class AQLObject;
class AQLPriceDataSlidingRule;
class AQLPriceDataCalendar;
///////////////////////////////////////////////////////////////////////
/*! 
    @brief calculate date of schedule

*/
class LAPriceCFGenUtility
{
public:
	// calculate dates by String input
	static void					generateSchedule(const AQLObject& object,
										const AQLDate& start, const AQLDate& end,
										const AQLString& data_frequency,
										const AQLString& data_timing,
										const AQLString& data_fodd, const AQLString& data_lodd,
										const AQLString& data_day,
										const AQLString& data_slidingrule, 
										const AQLString& data_calendar,
										DateVector& out,
										DateVector& out_unadjust); 
	// calculate both adjusted and unadjusted dates 
	static void					generateSchedule(const AQLObject& object,
										const AQLDate& start, const AQLDate& end,
										const AQLString& data_frequency,
										const AQLString& data_timing,
										const AQLString& data_fodd, const AQLString& data_lodd,
										const AQLString& data_day,
										const AQLPriceDataSlidingRule& busdayrule,
										const AQLPriceDataCalendar* pCal,
										DateVector& out,
										DateVector& out_unadjust); 

	// calculate dates
/*	static	void				generateSchedule(const AQLDate& start, const AQLDate& end,
										const AQLString& data_frequency,
										const bool isarrear,
										const AQLDate* firstStubDate, const AQLDate* lastStubDate,
										const int* pday,
										DateVector& out,
										const AQLPriceDataSlidingRule* pbusdayrule = NULL,
										const AQLPriceDataCalendar* pcal = NULL
										);*/
	// Set term from string to integer 
//	static	void				termStrtoYMD(const AQLString& term, int& y, int& m, int& d);
	
	// get slidingdate by specified SlidingRule and Calender
	static	AQLDate				getDate(const AQLDate& basedate, 
										const AQLObject& object,
										const AQLString& data_slidingrule, 
										const AQLString& data_calendar); 
	// get date either term after or before from basedate  
/*	static	AQLDate				getDate(const AQLDate& basedate, const AQLString& term, 
										bool rollForwards);
	// get date, which slidied by specified SlidingRule and Calender, either term after or before from basedate 
	static	AQLDate				getDate(const AQLDate& basedate, const AQLString& term, 
										const AQLPriceDataSlidingRule& busdayrule,
										const AQLPriceDataCalendar* pCal,
										bool rollForwards);
*/
	// get date, which slidied by specified SlidingRule and Calender, either term after or before from basedate
	static	AQLDate				getDate(const AQLDate& basedate, const AQLString& term, 
										const AQLObject& object,
										const AQLString& data_slidingrule, 
										const AQLString& data_calendar, 
										bool rollForwards);

	// get date, which is special day and slidied by specified SlidingRule and Calender, before specified term from basedate
	static	AQLDate				getDate(const AQLDate& basedate, 
										const AQLString& specialoffset,
										const IntArray& specialday,
										const AQLPriceDataSlidingRule& busdayrule,
										const AQLPriceDataCalendar* pCal = NULL);

	// get SlidingRule and Calender by Object Class
	static void					getBusDayRuleAndCalendar(const AQLObject& object,
														const AQLString& data_slidingrule, 
														const AQLString& data_calendar,
														const AQLObject& entity2,
														const AQLString& data_slidingrule2, 
														const AQLString& data_calendar2,
														const AQLPriceDataSlidingRule*& pbusdayrule,
														const AQLPriceDataCalendar*& pcal
														);
	// get rounded value by specified function and digit
	static	double				round(const double value, const AQLString& roundfunction, const int rounddigit);
	// get rounded value by specified function and digit
	static	double				round(const double value, RoundFunction roundfunction, const int rounddigit);
	
	// check arrear or not  
	static	bool				isArrear(const AQLString& timing);

protected:

private:

};
