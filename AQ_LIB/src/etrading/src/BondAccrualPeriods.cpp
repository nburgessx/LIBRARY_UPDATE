// BondFactory.cpp

/*
 * @brief			Bond Factory to Create Bond AQO objects
 * @Created:		3rd February 2017
 * @Author:			Nicholas Burgess
 * @Department:		AlgoQuantHub Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#include "BondAccrualPeriods.h"
#include "AQOUtilities.h"
#include "BondUtilities.h"
#include "ScheduleValidation.h"

namespace etrading
{

    /* @brief			Calculate the bond accrued interest days, i.e. days between lastCouponDate and settleDate 
	*  @param [in]		settlementDate              Bond Settlement Date
	*  @param [in]		activeCouponDates			Bond active coupon dates
	*  @param [in]		bondYieldParameters         Bond Yield Parameters
	*  @return			Number of Bond Accrued Interest Days for a given coupon
    */
    double calculateBondAccruedInterestDays(const AQLDate & settlementDate, const BondActiveCouponDates & activeCouponDates, const BondYieldParameters & bondYieldParameters)
    {

		//First accrualStartDate for the first cashflow, or the payment date before the first active coupon(payment) date for the non - first cashflow
		AQLDate firstActivePaymentDate = activeCouponDates.firstActiveCouponDate_;
		AQLDate priorFirstActivePaymentDate = activeCouponDates.priorFirstActiveCouponDate_;
		AQLDate  firstActiveCashflowExDividendDate = activeCouponDates.firstActiveCashflowExDividendDate_;

		// For JGB Bond, Accrued interest is on ACT/365 basis
        auto bondDaycount = isJapaneseGovenmentBond(bondYieldParameters.calculationType_) ? ACT_365_DAYCOUNT : bondYieldParameters.dayCount_;

        // Return accrued interest days = zero if the from priorPaymentDate is later than the settlementDate
        double accruedInterestDays = 0;

		if (excludeCouponInterest(settlementDate, firstActiveCashflowExDividendDate))
		{
			accruedInterestDays = getBondActualCouponPeriodDays(firstActivePaymentDate, settlementDate, bondDaycount);
		}
		else
		{
			accruedInterestDays = getBondActualCouponPeriodDays(priorFirstActivePaymentDate, settlementDate, bondDaycount);
		}

        return accruedInterestDays;
    }

	/* @brief			Calculate the bond accrued interest year fraction i.e. days between lastCouponDate and settleDate over days between lastCouponDate and currentCouponDate
	*  @param [in]		settlementDate                      Bond Settlement Date
	*  @param [in]		activeCouponDates					Bond active coupon dates
	*  @param [in]		bondYieldParameters                 Bond Yield Parameters
	*  @return			Bond Accrual Period as a double
	*/
	const double calculateBondAccruedInterestYearFraction(const AQLDate & settlementDate, const BondActiveCouponDates & activeCouponDates, const BondYieldParameters & bondYieldParameters)
	{

		AQLDate firstActivePaymentDate = activeCouponDates.firstActiveCouponDate_;
		AQLDate priorFirstActivePaymentDate = activeCouponDates.priorFirstActiveCouponDate_;
		AQLDate  firstActiveCashflowExDividendDate = activeCouponDates.firstActiveCashflowExDividendDate_;
		AQLDate  firstPriorVirtualPaymentDate = activeCouponDates.firstPriorVirtualCouponDate_;
		AQLDate  secondPriorVirtualPaymentDate = activeCouponDates.secondPriorVirtualCouponDate_;

		double accruedInterestYearFraction = 0.0;

		// For JGB Bond, Accrued interest is on ACT/365 basis
		auto bondDaycount = bondYieldParameters.dayCount_;

		if (isJapaneseGovenmentBond(bondYieldParameters.calculationType_))
		{
			bondDaycount = ACT_365_DAYCOUNT;
		}

		//Special treatment for the accrual interest when that settleDate greater than the currentCouponDate's exDividendDate 
		if (excludeCouponInterest(settlementDate, firstActiveCashflowExDividendDate))
		{
			// (settleDt - curCouponDate)/(curCouponDate - lastCouponDate)
			auto accruedDays = getBondActualCouponPeriodDays(firstActivePaymentDate, settlementDate, bondDaycount);

			double fullCouponPeriodDays = getBondFullCouponPeriodDays(firstPriorVirtualPaymentDate, firstActivePaymentDate, bondDaycount, bondYieldParameters.couponFrequency_);

			accruedInterestYearFraction = accruedDays * 1.0 / fullCouponPeriodDays; // Cast to double

			return accruedInterestYearFraction;
		}

		//No Stub or ShortStart 
		if (priorFirstActivePaymentDate >= firstPriorVirtualPaymentDate)
		{

			// (settleDt - lastCouponDate)/(curCouponDate - 1stVirtualLastCouponDate)
			auto shortCouponPeriodDays = getBondActualCouponPeriodDays(priorFirstActivePaymentDate, settlementDate, bondDaycount);

			auto fullCouponPeriodDays = getBondFullCouponPeriodDays(firstPriorVirtualPaymentDate, firstActivePaymentDate, bondDaycount, bondYieldParameters.couponFrequency_);

			accruedInterestYearFraction = shortCouponPeriodDays * 1.0 / fullCouponPeriodDays;
		}
		//LongStart (but based on priorFirstActivePaymentDate)
		else
		{
			double normalYearFraction = convertBondFrequencyToYearFraction(bondYieldParameters.couponFrequency_);

			// (1stPriorVirtualDt - priorFirstActivePaymentDate)/(1stPriorVirtualDt-2stPriorVirtualDt)  
			auto shortCouponPeriodDays1 = getBondActualCouponPeriodDays(priorFirstActivePaymentDate, firstPriorVirtualPaymentDate, bondDaycount);

			double fullCouponPeriodDays1 = getLongStartStubFirstFullCouponDays(firstActivePaymentDate, firstPriorVirtualPaymentDate, secondPriorVirtualPaymentDate, bondYieldParameters.calculationType_, bondDaycount, bondYieldParameters.couponFrequency_);

			double yearFraction1 = shortCouponPeriodDays1 * 1.0 / fullCouponPeriodDays1;

			//  (settleDt - 1stPriorVirtualDt)/ (1stCouponDt -1stPriorVirtualDt)
			auto shortCouponPeriodDays2 = getBondActualCouponPeriodDays(firstPriorVirtualPaymentDate, settlementDate, bondDaycount);
			auto fullCouponPeriodDays2 = getBondFullCouponPeriodDays(firstPriorVirtualPaymentDate, firstActivePaymentDate, bondDaycount, bondYieldParameters.couponFrequency_);

			double yearFraction2 = shortCouponPeriodDays2 * 1.0 / fullCouponPeriodDays2;

			accruedInterestYearFraction = yearFraction1 + yearFraction2;
		}

		return accruedInterestYearFraction;

	}



}