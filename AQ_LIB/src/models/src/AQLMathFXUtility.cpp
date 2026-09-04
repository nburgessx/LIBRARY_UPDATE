/*! @file
    @brief source code of utility class to calculate about FX.
*/
///

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "AQLMathFXUtility.h"


#include "AQLObject.h"
#include "AQLDataBasics.h"
//#include "AQLDataVector.h"
#include "AQLCoreTemplateType.h"
#include "AQLPriceDataCalendar.h"
#include "AQLPriceDataSlidingRule.h"
#include "AQLModelDynamicsCurve.h"


#include <cmath>

using namespace std;


/*!
    @brief calculate spot date case of USD included

    @param[in] cur currency(not USD)
    @param[in] today today
    @param[in] calname calendar name correspond to cur
    @param[in] calname_usd calendar name correspond to USD

    @return spot date
*/
AQLDate
AQLMathFXUtility::getSpotDate_IncludedUSD(const AQLString& cur,
												const AQLDate& today, 
												const AQLString& calname,
												const AQLString& calname_usd,
												unsigned int spotlag)
{
	AQLPriceDataCalendar cal;
	cal.convertFromString(calname);
	const AQLDate& spot = cal.getBusinessDay(today, spotlag);
	cal.convertFromString(calname_usd);
	AQLPriceDataSlidingRule srule(SLIDING_RULE_FOLLOWING);
	return srule.getDate(spot, cal);
}


/*!
    @brief calculate spot date case of USD not included

    @param[in] cur1 currency1(not USD)
    @param[in] cur2 currency2(not USD)
	@param[in] today today
    @param[in] calname1 calendar name correspond to cur1
    @param[in] calname2 calendar name correspond to cur1
	@param[in] calname_usd calendar name correspond to USD

    @return spot date
*/
AQLDate
AQLMathFXUtility::getSpotDate_NotIncludedUSD(const AQLString& cur1,
												const AQLString& cur2,
												const AQLDate& today, 
												const AQLString& calname1,
												const AQLString& calname2,
												const AQLString& calname_usd,
												unsigned int spotlag1,
												unsigned int spotlag2)
{
	const AQLDate& spot1 = getSpotDate_IncludedUSD(cur1, today, calname1, calname_usd, spotlag1); 
	const AQLDate& spot2 = getSpotDate_IncludedUSD(cur2, today, calname2, calname_usd, spotlag2); 
	if (spot1 > spot2) return spot1;
	else return spot2;
}
/*!
    @brief calculate forward rate

    @param[in] rate_base rate(ToCur/fromCur) of basedate
    @param[in] curve_fromCur curve of fromCurrency
    @param[in] curve_toCur curve of ToCurrency
	@param[in] forward_t forward time
 
    @return forward rate
	@note start time of curve_fromCur and curve_ToCur must be same, in this method no check. 
			rate_base is rate of start time.
			forward_t is not relative time from initial time but absolute time.
			DayCount of forward_t and both curve must be same, in this method no check. 
*/
double
AQLMathFXUtility::getForwardRate(double rate_base,
				const AQLRatesPathElementCurve& curve_fromCur,
				const AQLRatesPathElementCurve& curve_toCur,
				double forward_t)
{
	return rate_base * curve_fromCur.getP(forward_t) / curve_toCur.getP(forward_t);	
}


/*!
    @brief get currency pair

    @param[in] curpair currency pair with format like "JPY/USD"
  
    @return two currencys
*/
AQLStringVector
AQLMathFXUtility::getCurrencyPair(const AQLString& curpair)
{
    const AQLStringVector& curs = curpair.toToken('/');

	if (curs.size() != 2)
	{
	    //error
		AQLString msg = "Currency : ";
		msg += curpair;
		msg = " is invaid format(right format example is USD/JPY)";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);			
	}

	return curs;
}
