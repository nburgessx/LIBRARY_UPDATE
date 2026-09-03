// BondAccrualPeriods.h

/*
 * @brief			Bond Methods to Calculate Bond Accrual Periods
 * @Created:		21st February 2017
 * @Author:			Nicholas Burgess
 * @Department:		MHI Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once

#include "LADate.h"
#include "BondEnumerations.h"
#include "CoreEnumerations.h"
#include <string>
#include "BondYields.h"

namespace etrading
{
    
	/* @brief			Calculate the bond accrued interest days
	*  @param [in]		settlementDate              Bond Settlement Date
	*  @param [in]		activeCouponDates			Bond active coupon dates
	*  @param [in]		bondYieldParameters         Bond Yield Parameters
	*  @return			Number of Bond Accrued Interest Days for a given coupon
	*/
	double calculateBondAccruedInterestDays(const LADate& settlementDate, const BondActiveCouponDates & activeCouponDates , const BondYieldParameters& bondYieldParameters);

	/* @brief			Calculate the bond accrued interest year fraction i.e. days between lastCouponDate and settleDate over days between lastCouponDate and currentCouponDate
	*  @param [in]		settlementDate                      Bond Settlement Date
	*  @param [in]		activeCouponDates					Bond active coupon dates
	*  @param [in]		bondYieldParameters                 Bond Yield Parameters
	*  @return			Bond Accrual Period as a double
	*/
	const double calculateBondAccruedInterestYearFraction(const LADate & settlementDate, const BondActiveCouponDates & activeCouponDates, const BondYieldParameters & bondYieldParameters);

}

