// BondYields.h

/*
 * @brief			Bond Yields are used to calculate Bond Discount Factors. There are many conventions 
                    and methodologies on how to calculate a Bond Discount Factor from a Bond Yield. 
                    We manage this logic here

 * @Created:		14th February 2017
 * @Author:			Nicholas Burgess
 * @Department:		AlgoQuantHub Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#pragma once

#include "BondCurves.h"
#include "CoreEnumerations.h"
#include "BondEnumerations.h"
#include "AQLDate.h"
#include "LabelValueBlock.h"
#include <vector>

namespace etrading
{
    // Placeholder struct to centrally pass bond information to the yield helpers
    struct BondYieldParameters
    {
        FrequencyEnum               couponFrequency_;
        FrequencyEnum               yieldFrequency_;
        BondCalculationTypeEnum     calculationType_;
		DayCountEnum				dayCount_;
		std::string					calendar_;
	};

	struct BondActiveCouponDates
	{
		unsigned int firstActiveCashflowIndex_; // index in the cashflows (excluding the upfront cashflow for notional exchange)
		AQLDate firstActiveCouponDate_;
		AQLDate adjustedFirstActiveCouponDate_; // The adjustedFirstActiveCouponDate has business day adjustment if the yield is TRUE yield, otherwise it is the same as firstActivePaymentDate.
		AQLDate firstActiveCashflowExDividendDate_;
		
		bool isSettleDateExdividend_;

		AQLDate priorFirstActiveCouponDate_;
		AQLDate firstPriorVirtualCouponDate_;
		AQLDate secondPriorVirtualCouponDate_;

	};


    /* @brief			Calculates the Amount of the Bond Coupon is yet to accrue as a ratio in percent. Needed for upfront Bond Yield to discount factor calculations.
                        Example: A bond coupon with 43 days yet to accrue with a total number of coupon days as 181 would return 43/181 = 0.237569
    *  @param [in]		settlementDate                  Bond Settlement Date
    *  @param [in]		activeCouponDates				Specifies the coupon dates which bracket the settlement date.
    *  @param [in]		bondDaycount                    Bond day count
    *  @param [in]		bondAccrualFrequency            Bond Accrual Frequency
    *  @param [in]		yieldCalcType                   Yield calculation type
    *  @return			Bond coupon accrual ratio of future days versus total days in coupon period in percent
    */
	const double calculateBondActiveCouponFutureAccrualRatio( const AQLDate & settlementDate,
															  const BondActiveCouponDates& activeCouponDates,
															  const DayCountEnum & bondDaycount,
															  const FrequencyEnum & bondCouponFrequency,
															  const YieldCalculationTypeEnum& yieldCalcType);

    /* @brief			Caclulate a Bond Discount Factor from a Bond Yield
    *  @param [in]		bondYieldParameters                 Bond Yield Parameters Struct
    *  @param [in]		settlementDate                      Bond Settlement Date
    *  @param [in]		firstActivePaymentIndex              The first active coupon(payment) date index in the given bondPaymentlDatesForIndexation
    *  @param [in]		couponPaymentDate                   Current coupon(payment) date
    *  @param [in]		bondPaymentlDatesForIndexation      All Bond Cashflow Payment Dates, used to reference the active coupon
    *  @param [in]		yield                               Bond Yield in %
    *  @param [in]		accruedAdjustmentFactor             accruedAdjustmentFactor
    *  @param [in]		yieldCalcType                       Yield calculation type
    *  @param [in]		sumYearFractionsFrom2ndActiveToCurCashflow    Sum of Cashflows' YearFraction from second active cashflow to current cashflow, used for True Yield calculation type
    *  @return			Returns a single Bond Discount Factor
    */
    const double calculateSingleDiscountFactorFromBondYield( const BondYieldParameters & bondYieldParameters, const AQLDate& settlementDate, 
                                                            unsigned int& firstActivePaymentIndex, const AQLDate& couponPaymentDate, 
                                                            const std::vector< AQLDate>& bondPaymentlDatesForIndexation, const double& yield, const double& accruedAdjustmentFactor, 
                                                            const YieldCalculationTypeEnum& yieldCalcType, const double& sumYearFractionsFrom2ndActiveToCurCashflow=0);

    /* @brief			Caclulate a Bond Discount Factor from a Bond Yield. Observes the special conventions for
	*					bonds in the final coupon period.
    *  @param [in]		bondYieldParameters                 Bond Yield Parameters Struct
    *  @param [in]		settlementDate                      Bond Settlement Date
    *  @param [in]		firstActivePaymentIndex             The first active coupon(payment) date index in the given bondPaymentlDatesForIndexation
    *  @param [in]		currentPaymentDate                  Current coupon(payment) date
	*  @param [in]		previousPaymentDate                 Previous coupon(payment) date
    *  @param [in]		paymentDates                        All Bond Cashflow Payment Dates, used to reference the active coupon
    *  @param [in]		yield                               Bond Yield in %
    *  @param [in]		settleDateToFirstActivePaymentDateRatio       accruedAdjustmentFactor
    *  @param [in]		yieldCalcType                       Yield calculation type
    *  @param [in]		sumYearFractionsFrom2ndActiveToCurCashflow    Sum of Cashflows' YearFraction from second active cashflow to current cashflow, used for True Yield calculation type
	*  @param [in]		simpleYearFraction                  1 / (coupon frequency)
	*  @param [in]		bondDaycount                        The daycount used for bond coupon accrual
	*  @param [in]		isLastCashflow                      Whether current cashflow is the final cashflow of the bond
	*  @param [in]		isSettleDateInLastCashflow          Whether current cashflow is the final cashflow, and the settlementDate is in the final coupon period.
    *  @return			Returns a single Bond Discount Factor
    */
	const double  calculateSingleDiscountFactorFromConventionsAndBondYield( const BondYieldParameters & bondYieldParameters, const AQLDate& settlementDate, 
																			unsigned int& firstActivePaymentIndex, const AQLDate& currentPaymentDate, const AQLDate& previousPaymentDate,
																			const std::vector< AQLDate>& paymentDates, const double& yield, const double& settleDateToFirstActivePaymentDateRatio, 
																			const YieldCalculationTypeEnum& yieldCalcType, const double& sumYearFractionsFromSecondActivePaymentToCurrentCashflow,
																			const double& simpleYearFraction, const DayCountEnum & bondDaycount, const bool& isLastCashflow, const bool& isSettleDateInLastCashflow );


    /* @brief			Caclulate Bond Discount Factors from a Bond Yield
    *  @param [in]		bondYieldParameters                 Bond Yield Parameters Struct
    *  @param [in]		settlementDate                      Bond Settlement Date
    *  @param [in]		activeCouponDates					Specifies the coupon dates which bracket the settlement date.
    *  @param [in]		paymentDates                        All Bond Cashflow Payment Dates
    *  @param [in]		yield                               Bond Yield 
    *  @param [in]		bondDaycount                        Bond day count
    *  @param [in]		yieldCalcType                       Yield calculation type
    *  @param [in]		trueYieldYearFractions              The accrual year fractions using dates with budiness day adjusted
    *  @return			Returns a vector of Bond Discount Factors
    */
    const std::vector< double > calculateDiscountFactorsFromBondYield( const BondYieldParameters & bondYieldParameters, const AQLDate& settlementDate, const BondActiveCouponDates& activeCouponDates,
																		const std::vector< AQLDate>& paymentDates, const double& yield, const DayCountEnum & bondDaycount, 
																		const YieldCalculationTypeEnum& yieldCalcType, const DoubleVector& trueYieldYearFractions=DoubleVector()); 

	/* @brief			Caclulates Bond Discount Factors from a Bond Curve
    *  @param [in]		bondYieldParameters                 Bond Yield Parameters Struct
    *  @param [in]		settlementDate                      Bond Settlement Date
    *  @param [in]		activeCouponDates					Specifies the coupon dates which bracket the settlement date.
    *  @param [in]		paymentDates                        All Bond Cashflow Payment Dates
    *  @param [in]		bondCurve                           A calibrated bond curve, used for interpolating yields
    *  @param [in]		bondDaycount                        Bond day count
    *  @param [in]		trueYieldYearFractions              The accrual year fractions using dates with budiness day adjusted
    *  @return			Returns a vector of Bond Discount Factors
    */
	const std::vector< double > calculateDiscountFactorsFromBondCurve( const BondYieldParameters & bondYieldParameters, const AQLDate& settlementDate, const BondActiveCouponDates& activeCouponDates,
																		const std::vector< AQLDate>& paymentDates, const BondCurve& bondCurve, const DayCountEnum & bondDaycount, 
																		const DoubleVector& trueYieldYearFractions=DoubleVector() );


}