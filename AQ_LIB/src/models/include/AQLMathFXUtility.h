#pragma once

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"


class AQLDate;
class AQLString;
class AQLMathCalendar;
class AQLRatesPathElementCurve;
/*! 
    @brief utility class to calculate about FX. 
*/
class AQLMathFXUtility
{
public:

	// calculate spot date case of USD included
	static AQLDate		getSpotDate_IncludedUSD(const AQLString& cur,
												const AQLDate& today, 
												const AQLString& calname,
												const AQLString& calname_usd,
												unsigned int spotlag = 0);

	//// calculate spot date case of USD not included
	static AQLDate		getSpotDate_NotIncludedUSD(const AQLString& cur1,
													const AQLString& cur2,
												const AQLDate& today, 
												const AQLString& calname1,
												const AQLString& calname2,
												const AQLString& calname_usd,
												unsigned int spotlag1 = 0,
												unsigned int spotlag2 = 0);

	// calculate forward rate
	static double		getForwardRate(double rate_base,
										const AQLRatesPathElementCurve& curve_fromCur,
										const AQLRatesPathElementCurve& curve_toCur,
										double forward_t);
	// get currency pair
	static AQLStringVector getCurrencyPair(const AQLString& curpair);
protected:
	

private:
	

};
