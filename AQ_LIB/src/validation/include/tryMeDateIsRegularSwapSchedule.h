/*
 * @brief			validation interface for meDateIsRegularSwapSchedule method(s)
 * @Created:		30th May 2018
 * @Author:			Nicholas Burgess
 * @Department:		ISD Quant Research & Analytics
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once
#include "LACoreTemplateType.h"

namespace validation_api
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
    bool tryMeDateIsRegularSwapSchedule( const LADate& swapStart,
		                                 const LADate& swapMaturity,
		                                 bool isMaturityAdjusted,
		                                 const LAString& frequency,
		                                 const LAString& busDayAdj,
		                                 const LAString& calendar,
		                                 int rollDay,
		                                 const LAString& rollConvention = "" );
    
}
