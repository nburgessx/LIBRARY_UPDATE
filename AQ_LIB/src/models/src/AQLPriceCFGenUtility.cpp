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

#include "AQLPriceCFGenUtility.h"

#include "AQLMathDateCalculations.h"
#include "AQLObject.h"
#include "AQLDataBasics.h"
#include "AQLDataVector.h"
#include "AQLCoreTemplateType.h"
#include "AQLMathDefine.h"
#include "AQLPriceDataCalendar.h"
#include "AQLPriceDataSlidingRule.h"
#include "AQLBasic.h"
#include "AQLAlgorithm.h"

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

    @param[in] object			AQLObject Class
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
AQLPriceCFGenUtility::generateSchedule(const AQLObject& object,
							const AQLDate& start, const AQLDate& end,
							const AQLString& data_frequency,
							const AQLString& data_timing,
							const AQLString& data_fodd, const AQLString& data_lodd,
							const AQLString& data_day,
							const AQLString& data_slidingrule, 
							const AQLString& data_calendar,
							DateVector& out,
							DateVector& out_unadjust)
{
	const AQLDataHolder* dh;
	//sliding rule
	dh = &(object.getData(data_slidingrule, ISNOTNULL));
	const AQLPriceDataSlidingRule& srule = dynamic_cast<const AQLPriceDataSlidingRule&>(dh->get());

	//calendar
	const AQLPriceDataCalendar* pCal = NULL;
	dh = &(object.getData(data_calendar, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
		pCal = &dynamic_cast<const AQLPriceDataCalendar&>(dh->get());


	AQLPriceCFGenUtility::generateSchedule(object, start, end, data_frequency, data_timing,
								data_fodd, data_lodd, data_day, srule, pCal, 
								out, out_unadjust);

}

/*!
	@brief calculate both adjusted and unadjusted dates 

    @param[in] object			AQLObject Class
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
AQLPriceCFGenUtility::generateSchedule(const AQLObject& object,
							const AQLDate& start, const AQLDate& end,
							const AQLString& data_frequency,
							const AQLString& data_timing,
							const AQLString& data_fodd, const AQLString& data_lodd,
							const AQLString& data_day,
							const AQLPriceDataSlidingRule& busdayrule,
							const AQLPriceDataCalendar* pCal,
							DateVector& out,
							DateVector& out_unadjust)
{
	const AQLDataHolder* dh;
	//frequency
	dh = &(object.getData(data_frequency, ISNOTNULL));
	const AQLString& freq = dynamic_cast<const AQLDataString&>(dh->get()).get();
	
	//isArrear
	dh = &(object.getData(data_timing, ISNOTNULL));
	const AQLString& timing = dynamic_cast<const AQLDataString&>(dh->get()).get();
	bool isarrear = AQLPriceCFGenUtility::isArrear(timing);


	//first odd date
	const AQLDate* firstStubDate = NULL;
	dh = &(object.getData(data_fodd, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
		firstStubDate = &dynamic_cast<const AQLDataDate&>(dh->get()).get();

	//last odd date
	const AQLDate* lastStubDate = NULL;
	dh = &(object.getData(data_lodd, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
		lastStubDate = &dynamic_cast<const AQLDataDate&>(dh->get()).get();

	//coupon day
	int day;
	const int* pday = NULL;
	dh = &(object.getData(data_day, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
	{
		day = dynamic_cast<const AQLDataInt&>(dh->get()).get();
		pday = &day;
	}

	
	///////////////////////////////
	//dates before holiday adjust//
	///////////////////////////////
	AQLMathDateCalculations::generateSchedule(start, end, freq, isarrear,
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
			AQLString msg = "Calendar is need";
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);			
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
	@param[in] object			AQLObject Class
	@param[in] data_slidngrule	stirng of SlidingRule
	@param[in] data_calendar	stirng of Calender

*/
AQLDate
AQLPriceCFGenUtility::getDate(const AQLDate& basedate, 
							const AQLObject& object,
							const AQLString& data_slidingrule, 
							const AQLString& data_calendar)
{
	AQLDate date = basedate;
	//sliding rule
	const AQLDataHolder *dh = &(object.getData(data_slidingrule, ISNOTNULL));
	const AQLPriceDataSlidingRule& srule = dynamic_cast<const AQLPriceDataSlidingRule&>(dh->get());
	if (srule.getSlidingRule() != SLIDING_RULE_NO_CHANGE)
	{
		//calendar
		dh = &(object.getData(data_calendar, ISNOTNULL));
		const AQLPriceDataCalendar& cal = dynamic_cast<const AQLPriceDataCalendar&>(dh->get());
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
AQLDate
AQLPriceCFGenUtility::getDate(const AQLDate& basedate, const AQLString& term, 
							const AQLObject& object,
							const AQLString& data_slidingrule, 
							const AQLString& data_calendar, 
							bool rollForwards)
{
	int y, m, d, w;
	AQLMathDateCalculations::termStrtoYMDW(term, y, m, d, w);
	AQLDate date = basedate;
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
	const AQLDataHolder *dh = &(object.getData(data_slidingrule, ISNOTNULL));
	const AQLPriceDataSlidingRule& srule = dynamic_cast<const AQLPriceDataSlidingRule&>(dh->get());
	if (srule.getSlidingRule() != SLIDING_RULE_NO_CHANGE)
	{
		//calendar
		dh = &(object.getData(data_calendar, ISNOTNULL));
		const AQLPriceDataCalendar& cal = dynamic_cast<const AQLPriceDataCalendar&>(dh->get());
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

AQLDate
AQLPriceCFGenUtility::getDate(const AQLDate& basedate, 
							const AQLString& specialoffset,
							const IntArray& specialday,
							const AQLPriceDataSlidingRule& busdayrule,
							const AQLPriceDataCalendar* pCal)
{
	AQLDate date = basedate;
	int y, m, d, w;
	AQLMathDateCalculations::termStrtoYMDW(specialoffset, y, m, d, w);
	
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
		AQLString msg = "Calendar is need";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);			
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
AQLPriceCFGenUtility::isArrear(const AQLString& timing)
{
	AQLString str = timing;
	str.toUpper();
	if (str == ARREAR) return true;
	else if (str == ADVANCE) return false;
	else
	{
		//error
		AQLString msg = str;
		msg += " is wrong input";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);				
	}
}

/*!
	@brief get SlidingRule and Calender by Object Class



    @param[in] object			first priority AQLObject Class
	@param[in] data_slidngrule	stirng of SlidingRule
	@param[in] data_calendar	stirng of Calender
	@param[in] entity2			sedond priority AQLObject Class
	@param[in, out] pbusdayrule		pointer of SlidingRule
	@param[in, out] pcal			pointer of Calender


	@note 	First, search in object, and if not find search entity2. 
			if slidingrule is SLIDING_RULE_NO_CHANGE, set null pointer to pcal.
*/

void
AQLPriceCFGenUtility::getBusDayRuleAndCalendar(const AQLObject& object,
											const AQLString& data_slidingrule, 
											const AQLString& data_calendar,
											const AQLObject& entity2,
											const AQLString& data_slidingrule2, 
											const AQLString& data_calendar2,
											const AQLPriceDataSlidingRule*& pbusdayrule,
											const AQLPriceDataCalendar*& pcal
														)
{
	const AQLDataHolder* dh;
	pcal = NULL;
	//sliding rule
	dh = &(object.getData(data_slidingrule, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
		pbusdayrule = &dynamic_cast<const AQLPriceDataSlidingRule&>(dh->get());
	else
	{
		dh = &(entity2.getData(data_slidingrule2, ISNOTNULL));
		pbusdayrule = &dynamic_cast<const AQLPriceDataSlidingRule&>(dh->get());
	}

	if (pbusdayrule->getSlidingRule() == SLIDING_RULE_NO_CHANGE) return;
		
		
	dh = &(object.getData(data_calendar, NOCHECK));
	if (dh->isDefined() && !dh->isNull())
		pcal = &dynamic_cast<const AQLPriceDataCalendar&>(dh->get());
	else
	{
		dh = &(entity2.getData(data_calendar2, ISNOTNULL));
		pcal = &dynamic_cast<const AQLPriceDataCalendar&>(dh->get());
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
AQLPriceCFGenUtility::round(const double value, const AQLString& roundfunction, const int rounddigit)
{
	AQLString roundstr = roundfunction;
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
		AQLString msg = "RoundFunction : " + roundfunction;
		msg += " is not support";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);		
	
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
AQLPriceCFGenUtility::round(const double value, RoundFunction roundfunction, const int rounddigit)
{
	
	int a = 1;
	for (int i = 0; i < AQLMath::abs(rounddigit); i++)
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

