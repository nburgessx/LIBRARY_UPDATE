#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LADate.h"
#include "LACoreAppError.h"
#include "LACoreTemplateType.h"

enum RoundFunction
{
    ROUND,			// Round
    ROUND_UP,		// Round Up
    ROUND_DOWN		// Round Down
};



class LAString;
class LAObject;
class LAPriceDataSlidingRule;
class LAPriceDataCalendar;
///////////////////////////////////////////////////////////////////////
/*! 
    @brief calculate date of schedule

*/
class LAPriceCFGenUtility
{
public:
	// calculate dates by String input
	static void					generateSchedule(const LAObject& object,
										const LADate& start, const LADate& end,
										const LAString& data_frequency,
										const LAString& data_timing,
										const LAString& data_fodd, const LAString& data_lodd,
										const LAString& data_day,
										const LAString& data_slidingrule, 
										const LAString& data_calendar,
										DateVector& out,
										DateVector& out_unadjust); 
	// calculate both adjusted and unadjusted dates 
	static void					generateSchedule(const LAObject& object,
										const LADate& start, const LADate& end,
										const LAString& data_frequency,
										const LAString& data_timing,
										const LAString& data_fodd, const LAString& data_lodd,
										const LAString& data_day,
										const LAPriceDataSlidingRule& busdayrule,
										const LAPriceDataCalendar* pCal,
										DateVector& out,
										DateVector& out_unadjust); 

	// calculate dates
/*	static	void				generateSchedule(const LADate& start, const LADate& end,
										const LAString& data_frequency,
										const bool isarrear,
										const LADate* firstStubDate, const LADate* lastStubDate,
										const int* pday,
										DateVector& out,
										const LAPriceDataSlidingRule* pbusdayrule = NULL,
										const LAPriceDataCalendar* pcal = NULL
										);*/
	// Set term from string to integer 
//	static	void				termStrtoYMD(const LAString& term, int& y, int& m, int& d);
	
	// get slidingdate by specified SlidingRule and Calender
	static	LADate				getDate(const LADate& basedate, 
										const LAObject& object,
										const LAString& data_slidingrule, 
										const LAString& data_calendar); 
	// get date either term after or before from basedate  
/*	static	LADate				getDate(const LADate& basedate, const LAString& term, 
										bool rollForwards);
	// get date, which slidied by specified SlidingRule and Calender, either term after or before from basedate 
	static	LADate				getDate(const LADate& basedate, const LAString& term, 
										const LAPriceDataSlidingRule& busdayrule,
										const LAPriceDataCalendar* pCal,
										bool rollForwards);
*/
	// get date, which slidied by specified SlidingRule and Calender, either term after or before from basedate
	static	LADate				getDate(const LADate& basedate, const LAString& term, 
										const LAObject& object,
										const LAString& data_slidingrule, 
										const LAString& data_calendar, 
										bool rollForwards);

	// get date, which is special day and slidied by specified SlidingRule and Calender, before specified term from basedate
	static	LADate				getDate(const LADate& basedate, 
										const LAString& specialoffset,
										const IntArray& specialday,
										const LAPriceDataSlidingRule& busdayrule,
										const LAPriceDataCalendar* pCal = NULL);

	// get SlidingRule and Calender by Object Class
	static void					getBusDayRuleAndCalendar(const LAObject& object,
														const LAString& data_slidingrule, 
														const LAString& data_calendar,
														const LAObject& entity2,
														const LAString& data_slidingrule2, 
														const LAString& data_calendar2,
														const LAPriceDataSlidingRule*& pbusdayrule,
														const LAPriceDataCalendar*& pcal
														);
	// get rounded value by specified function and digit
	static	double				round(const double value, const LAString& roundfunction, const int rounddigit);
	// get rounded value by specified function and digit
	static	double				round(const double value, RoundFunction roundfunction, const int rounddigit);
	
	// check arrear or not  
	static	bool				isArrear(const LAString& timing);

protected:

private:

};
