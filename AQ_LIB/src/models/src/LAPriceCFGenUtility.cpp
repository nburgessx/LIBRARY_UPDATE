/*! @file
    @brief source code of utility class to calculate dates of schedule.

*/
//  2006, AlgoQuantHub..
///
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "LAPriceCFGenUtility.h"

#include "LAMathDateCalculations.h"
#include "LAObject.h"
#include "LADataBasics.h"
#include "LADataVector.h"
#include "LACoreTemplateType.h"
#include "LAMathDefine.h"
#include "LAPriceDataCalendar.h"
#include "LAPriceDataSlidingRule.h"
#include "LABasic.h"
#include "LAAlgorithm.h"

#include <cmath>

using namespace std;

#define ROUND_STR		"ROUND"
#define ROUND_UP_STR	"ROUNDUP"
#define ROUND_DOWN_STR	"ROUNDDOWN"

/*#define	BUSINESS_DAYS	"Business_Days"
#define	WEEKLY			"Weekly"
#define	MONTHLY			"Monthly"
#define	QUARTERLY		"Quarterly"
#define	SEMI_ANNUAL		"Semi-Annual"
#define	ANNUAL			"Annual"
#define	NONE			"None"*/

#define ADVANCE			"ADVANCE"
#define ARREAR			"ARREAR"



/*!
	@brief calculate dates by String input

    @param[in] object			LAObject Class
    @param[in] start			start date
	@param[in] end				end date
	@param[in] data_frequency	stirng of payment frequency
	@param[in] data_timing		stirng of data_timing
	@param[in] data_fodd		stirng of front oddday
	@param[in] data_lodd		stirng of last oddday
	@param[in] data_day			stirng of payment day
	@param[in] data_slidngrule	stirng of SlidingRule
	@param[in] data_calendar	stirng of Calender

*/
void
LAPriceCFGenUtility::generateSchedule(const LAObject& object,
							const LADate& start, const LADate& end,
							const LAString& data_frequency,
							const LAString& data_timing,
							const LAString& data_fodd, const LAString& data_lodd,
							const LAString& data_day,
							const LAString& data_slidingrule, 
							const LAString& data_calendar,
							DateVector& out,
							DateVector& out_unadjust)
{
	const LADataHolder* dh;
	//sliding rule
	dh = &(object.getData(data_slidingrule, ISNOTNULL));
	const LAPriceDataSlidingRule& srule = dynamic_cast<const LAPriceDataSlidingRule&>(dh->get());

	//calendar
	const LAPriceDataCalendar* pCal = NULL;
	dh = &(object.getData(data_calendar, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
		pCal = &dynamic_cast<const LAPriceDataCalendar&>(dh->get());


	LAPriceCFGenUtility::generateSchedule(object, start, end, data_frequency, data_timing,
								data_fodd, data_lodd, data_day, srule, pCal, 
								out, out_unadjust);

}

/*!
	@brief calculate both adjusted and unadjusted dates 

    @param[in] object			LAObject Class
    @param[in] start			start date
	@param[in] end				end date
	@param[in] data_frequency	stirng of payment frequency
	@param[in] data_timing		stirng of data_timing
	@param[in] data_fodd		stirng of front oddday
	@param[in] data_lodd		stirng of last oddday
	@param[in] data_day			stirng of payment day
	@param[in] pbusdayrule		pointer of SlidingRule
	@param[in] pcal				pointer of Calender

*/
void
LAPriceCFGenUtility::generateSchedule(const LAObject& object,
							const LADate& start, const LADate& end,
							const LAString& data_frequency,
							const LAString& data_timing,
							const LAString& data_fodd, const LAString& data_lodd,
							const LAString& data_day,
							const LAPriceDataSlidingRule& busdayrule,
							const LAPriceDataCalendar* pCal,
							DateVector& out,
							DateVector& out_unadjust)
{
	const LADataHolder* dh;
	//frequency
	dh = &(object.getData(data_frequency, ISNOTNULL));
	const LAString& freq = dynamic_cast<const LADataString&>(dh->get()).get();
	
	//isArrear
	dh = &(object.getData(data_timing, ISNOTNULL));
	const LAString& timing = dynamic_cast<const LADataString&>(dh->get()).get();
	bool isarrear = LAPriceCFGenUtility::isArrear(timing);


	//first odd date
	const LADate* firstStubDate = NULL;
	dh = &(object.getData(data_fodd, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
		firstStubDate = &dynamic_cast<const LADataDate&>(dh->get()).get();

	//last odd date
	const LADate* lastStubDate = NULL;
	dh = &(object.getData(data_lodd, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
		lastStubDate = &dynamic_cast<const LADataDate&>(dh->get()).get();

	//coupon day
	int day;
	const int* pday = NULL;
	dh = &(object.getData(data_day, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		day = dynamic_cast<const LADataInt&>(dh->get()).get();
		pday = &day;
	}

	
	///////////////////////////////
	//dates before holiday adjust//
	///////////////////////////////
	LAMathDateCalculations::generateSchedule(start, end, freq, isarrear,
							firstStubDate, lastStubDate, pday, out_unadjust);

	//////////////////////////////////////
	//payment dates after holiday adjust//
	//////////////////////////////////////
	
	out = out_unadjust;
	//sliding rule
	if (busdayrule.getSlidingRule() != SLIDING_RULE_NO_CHANGE)
	{
		if (pCal == NULL)
		{
			//error
			LAString msg = "Calendar is need";
			throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);			
		}
		for (unsigned int i = 0; i < out_unadjust.size(); i++)	
		{
			out[i] = busdayrule.getDate(out_unadjust[i], *pCal);
			if (i > 0 && out[i] == out[i - 1])
			{
				out.erase(out.begin() + i);
				out_unadjust.erase(out_unadjust.begin() + i);
				i--;
			}
		}
	}
}




/*!
	@brief get slidingdate by specified SlidingRule and Calender
	
	@param[in] basedate			base date
	@param[in] object			LAObject Class
	@param[in] data_slidngrule	stirng of SlidingRule
	@param[in] data_calendar	stirng of Calender

*/
LADate
LAPriceCFGenUtility::getDate(const LADate& basedate, 
							const LAObject& object,
							const LAString& data_slidingrule, 
							const LAString& data_calendar)
{
	LADate date = basedate;
	//sliding rule
	const LADataHolder *dh = &(object.getData(data_slidingrule, ISNOTNULL));
	const LAPriceDataSlidingRule& srule = dynamic_cast<const LAPriceDataSlidingRule&>(dh->get());
	if (srule.getSlidingRule() != SLIDING_RULE_NO_CHANGE)
	{
		//calendar
		dh = &(object.getData(data_calendar, ISNOTNULL));
		const LAPriceDataCalendar& cal = dynamic_cast<const LAPriceDataCalendar&>(dh->get());
		date = srule.getDate(date, cal);
	}
	return date;

}





/*!
	@brief get date, which slidied by specified SlidingRule and Calender,
		   either term after or before from basedate 
	
	@param[in] basedate			base date
	@param[in] term				stirng of term
	@param[in] data_slidngrule	stirng of SlidingRule
	@param[in] data_calendar	stirng of Calender
	@param[in] rollForwards			true:after, false:before(bool)
	
*/
LADate
LAPriceCFGenUtility::getDate(const LADate& basedate, const LAString& term, 
							const LAObject& object,
							const LAString& data_slidingrule, 
							const LAString& data_calendar, 
							bool rollForwards)
{
	int y, m, d, w;
	LAMathDateCalculations::termStrtoYMDW(term, y, m, d, w);
	LADate date = basedate;
	if (!rollForwards)
	{
		y = -y;
		m = -m;
		d = -d;
	}
	date.addYears(y);
	date.addMonths(m);

	date = getDate(date, object, data_slidingrule, data_calendar);
	if (d == 0) return date;

	//sliding rule
	const LADataHolder *dh = &(object.getData(data_slidingrule, ISNOTNULL));
	const LAPriceDataSlidingRule& srule = dynamic_cast<const LAPriceDataSlidingRule&>(dh->get());
	if (srule.getSlidingRule() != SLIDING_RULE_NO_CHANGE)
	{
		//calendar
		dh = &(object.getData(data_calendar, ISNOTNULL));
		const LAPriceDataCalendar& cal = dynamic_cast<const LAPriceDataCalendar&>(dh->get());
		date = cal.getBusinessDay(date, d);
	}
	else 
		date.addDays(d);

	return date;

}

/*!
	@brief get date, which is special day and slidied by specified SlidingRule and Calender,
		   before specified term from basedate
	
	@param[in] basedate			base date
	@param[in] term				stirng of term
	@param[in] specialoffset	special offset term
	@param[in] specialday		special day of each month
	@param[in] busdayrule		SlidingRule
	@param[in] pCal				pointer of Calender

*/

LADate
LAPriceCFGenUtility::getDate(const LADate& basedate, 
							const LAString& specialoffset,
							const IntArray& specialday,
							const LAPriceDataSlidingRule& busdayrule,
							const LAPriceDataCalendar* pCal)
{
	LADate date = basedate;
	int y, m, d, w;
	LAMathDateCalculations::termStrtoYMDW(specialoffset, y, m, d, w);
	
	date.addYears(-y);
	date.addMonths(-m);
	date.addDays(-d);
	
	m = date.monthOfYear() - 1;
	if (specialday.at(m) == 99)
		date.addDays(date.intervalToEndOfMonth());
	else
		date.setDay(specialday.at(m));

	if (busdayrule.getSlidingRule() == SLIDING_RULE_NO_CHANGE) 
		return date;

	if (pCal == NULL)
	{
		//error
		LAString msg = "Calendar is need";
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);			
	}
	return busdayrule.getDate(date, *pCal);
}

/*!
	@brief check arrear or not 
	
	@param[in] timing
	
	@return true arrear
    @return false not arrear
*/

bool
LAPriceCFGenUtility::isArrear(const LAString& timing)
{
	LAString str = timing;
	str.toUpper();
	if (str == ARREAR) return true;
	else if (str == ADVANCE) return false;
	else
	{
		//error
		LAString msg = str;
		msg += " is wrong input";
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);				
	}
}

/*!
	@brief get SlidingRule and Calender by Object Class



    @param[in] object			first priority LAObject Class
	@param[in] data_slidngrule	stirng of SlidingRule
	@param[in] data_calendar	stirng of Calender
	@param[in] entity2			sedond priority LAObject Class
	@param[in, out] pbusdayrule		pointer of SlidingRule
	@param[in, out] pcal			pointer of Calender


	@note 	First, search in object, and if not find search entity2. 
			if slidingrule is SLIDING_RULE_NO_CHANGE, set null pointer to pcal.
*/

void
LAPriceCFGenUtility::getBusDayRuleAndCalendar(const LAObject& object,
											const LAString& data_slidingrule, 
											const LAString& data_calendar,
											const LAObject& entity2,
											const LAString& data_slidingrule2, 
											const LAString& data_calendar2,
											const LAPriceDataSlidingRule*& pbusdayrule,
											const LAPriceDataCalendar*& pcal
														)
{
	const LADataHolder* dh;
	pcal = NULL;
	//sliding rule
	dh = &(object.getData(data_slidingrule, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
		pbusdayrule = &dynamic_cast<const LAPriceDataSlidingRule&>(dh->get());
	else
	{
		dh = &(entity2.getData(data_slidingrule2, ISNOTNULL));
		pbusdayrule = &dynamic_cast<const LAPriceDataSlidingRule&>(dh->get());
	}

	if (pbusdayrule->getSlidingRule() == SLIDING_RULE_NO_CHANGE) return;
		
		
	dh = &(object.getData(data_calendar, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
		pcal = &dynamic_cast<const LAPriceDataCalendar&>(dh->get());
	else
	{
		dh = &(entity2.getData(data_calendar2, ISNOTNULL));
		pcal = &dynamic_cast<const LAPriceDataCalendar&>(dh->get());
	}
	
}

/*!
	@brief get rounded value by specified function and digit
	
	@param[in] value			value	
	@param[in] roundfunction	string of roundfunction
	@param[in] rounddigit		rounddigit		
	
	@return rounded value

*/

double
LAPriceCFGenUtility::round(const double value, const LAString& roundfunction, const int rounddigit)
{
	LAString roundstr = roundfunction;
	roundstr.toUpper();
	

	if (roundstr == ROUND_STR)
		return round(value, ROUND, rounddigit);
	else if (roundstr == ROUND_UP_STR)
		return round(value, ROUND_UP, rounddigit);
	else if (roundstr == ROUND_DOWN_STR)
		return round(value, ROUND_DOWN, rounddigit);
	else
	{
		//error
		LAString msg = "RoundFunction : " + roundfunction;
		msg += " is not support";
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);		
	
	}

}

/*!
	@brief get rounded value by specified function and digit
	
	@param[in] value			value	
	@param[in] roundfunction	roundfunction
	@param[in] rounddigit		rounddigit		
	
	@return rounded value

*/

double
LAPriceCFGenUtility::round(const double value, RoundFunction roundfunction, const int rounddigit)
{
	
	int a = 1;
	for (int i = 0; i < LAMath::abs(rounddigit); i++)
		a *= 10;

	if (roundfunction == ROUND)
	{
		if (rounddigit >=0)
			return floor(value / a + 0.5) * a;
		else
			return floor(value * a + 0.5) / a;
	}
	else if (roundfunction == ROUND_UP)
	{
		if (rounddigit >=0)
			return ceil(value / a) * a;
		else
			return ceil(value * a) / a;
	}
	else 
	{
		if (rounddigit >=0)
			return floor(value / a) * a;
		else
			return floor(value * a) / a;	
	}
	

}

