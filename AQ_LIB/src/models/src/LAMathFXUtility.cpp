/*! @file
    @brief source code of utility class to calculate about FX.

*/
//  2006, AlgoQuantHub..
///

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "LAMathFXUtility.h"


#include "LAObject.h"
#include "LADataBasics.h"
//#include "LADataVector.h"
#include "LACoreTemplateType.h"
#include "LAPriceDataCalendar.h"
#include "LAPriceDataSlidingRule.h"
#include "LAModelDynamicsCurve.h"


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
LADate
LAMathFXUtility::getSpotDate_IncludedUSD(const LAString& cur,
												const LADate& today, 
												const LAString& calname,
												const LAString& calname_usd,
												unsigned int spotlag)
{
	LAPriceDataCalendar cal;
	cal.convertFromString(calname);
	const LADate& spot = cal.getBusinessDay(today, spotlag);
	cal.convertFromString(calname_usd);
	LAPriceDataSlidingRule srule(SLIDING_RULE_FOLLOWING);
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
LADate
LAMathFXUtility::getSpotDate_NotIncludedUSD(const LAString& cur1,
												const LAString& cur2,
												const LADate& today, 
												const LAString& calname1,
												const LAString& calname2,
												const LAString& calname_usd,
												unsigned int spotlag1,
												unsigned int spotlag2)
{
	const LADate& spot1 = getSpotDate_IncludedUSD(cur1, today, calname1, calname_usd, spotlag1); 
	const LADate& spot2 = getSpotDate_IncludedUSD(cur2, today, calname2, calname_usd, spotlag2); 
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
LAMathFXUtility::getForwardRate(double rate_base,
				const LARatesPathElementCurve& curve_fromCur,
				const LARatesPathElementCurve& curve_toCur,
				double forward_t)
{
	return rate_base * curve_fromCur.getP(forward_t) / curve_toCur.getP(forward_t);	
}


/*!
    @brief get currency pair

    @param[in] curpair currency pair with format like "JPY/USD"
  
    @return two currencys
*/
LAStringVector
LAMathFXUtility::getCurrencyPair(const LAString& curpair)
{
    const LAStringVector& curs = curpair.toToken('/');

	if (curs.size() != 2)
	{
	    //error
		LAString msg = "Currency : ";
		msg += curpair;
		msg = " is invaid format(right format example is USD/JPY)";
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);			
	}

	return curs;
}
