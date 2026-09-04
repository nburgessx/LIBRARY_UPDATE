#pragma once
#include "AQLCoreTemplateType.h"

namespace validation
{

    /* @brief			Validate if swap has regular date schedule without stub coupons.
	* @param [in]		swapStart			    Swap start date
	* @param [in]		swapMaturity            Swap end date. Tenors are typically adjusted and end dates are not
	* @param [in]		isMaturityAdjusted      Swap end date business day adjusted. Maturities derived from Tenors are adjusted, whereas explicit maturity dates are unadjusted
	* @param [in]		frequency			    Swap floating frequency
	* @param [in]		busDayAdj			    Swap business date adjustment convention
	* @param [in]		calendar			    Swap calendar
	* @param [in]		rollDay				    Roll day
	* @param [in]		rollConvention		    Swap roll convention e.g. IMM, EOM
	* @output			Returns TRUE if the swap schedule is regular (with no stub) and FALSE otherwise
	*/
    bool tryAqDatesIsRegularSwapSchedule( const AQLDate& swapStart,
		                                 const AQLDate& swapMaturity,
		                                 bool isMaturityAdjusted,
		                                 const AQLString& frequency,
		                                 const AQLString& busDayAdj,
		                                 const AQLString& calendar,
		                                 int rollDay,
		                                 const AQLString& rollConvention = "" );
    
}
