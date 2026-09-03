#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "LACoreAppError.h"
#include "LACoreTemplateType.h"


class LADate;
class LAString;
class LAMathCalendar;
class LARatesPathElementCurve;
/*! 
    @brief utility class to calculate about FX. 
*/
class LAMathFXUtility
{
public:

	// calculate spot date case of USD included
	static LADate		getSpotDate_IncludedUSD(const LAString& cur,
												const LADate& today, 
												const LAString& calname,
												const LAString& calname_usd,
												unsigned int spotlag = 0);

	//// calculate spot date case of USD not included
	static LADate		getSpotDate_NotIncludedUSD(const LAString& cur1,
													const LAString& cur2,
												const LADate& today, 
												const LAString& calname1,
												const LAString& calname2,
												const LAString& calname_usd,
												unsigned int spotlag1 = 0,
												unsigned int spotlag2 = 0);

	// calculate forward rate
	static double		getForwardRate(double rate_base,
										const LARatesPathElementCurve& curve_fromCur,
										const LARatesPathElementCurve& curve_toCur,
										double forward_t);
	// get currency pair
	static LAStringVector getCurrencyPair(const LAString& curpair);
protected:
	

private:
	

};
