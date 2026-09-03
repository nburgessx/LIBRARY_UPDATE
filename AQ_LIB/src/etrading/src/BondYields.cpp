// BondYields.cpp

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

#include "BondYields.h"
#include "BondUtilities.h"
#include "BondAccrualPeriods.h"
#include "LWOUtilities.h"
#include "CoreEnumerations.h"
#include "LADateScheduleHelpers.h"

namespace etrading
{

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
															  const YieldCalculationTypeEnum& yieldCalcType)
    {
        auto firstActiveDateToUse = ( yieldCalcType == TRUE_YIELD ) ? activeCouponDates.adjustedFirstActiveCouponDate_ : activeCouponDates.firstActiveCouponDate_;
        
        auto futureDays           = getBondActualCouponPeriodDays(settlementDate, firstActiveDateToUse, bondDaycount );     // Calculate the number of days from the settlementDate to the firstActivePaymentDate

        auto totalCouponDays      = getBondFullCouponPeriodDays(activeCouponDates.priorFirstActiveCouponDate_, activeCouponDates.firstActiveCouponDate_, bondDaycount,  bondCouponFrequency);   // Calculate the number of days from the priorfirstActivePaymentDate to the firstActivePaymentDate

        // Divide by Zero Guard
        if ( totalCouponDays == 0 )
        {
            throw AQLCoreInvalidData("#Error: Invalid Bond Coupon Dates. Cannot evaluate a Bond Coupon accruing for zero days.",__FILE__,__LINE__);
        }

        const double accrualRatio = futureDays / ( 1.0 * totalCouponDays ); // Cast to double
        return accrualRatio;
    }

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
    const double calculateSingleDiscountFactorFromBondYield( const BondYieldParameters & bondYieldParameters,
                                                             const AQLDate& settlementDate, 
                                                             unsigned int& firstActivePaymentIndex,
                                                             const AQLDate& couponPaymentDate, 
                                                             const std::vector< AQLDate>& bondPaymentlDatesForIndexation,
                                                             const double& yield,
                                                             const double& accruedAdjustmentFactor, 
                                                             const YieldCalculationTypeEnum& yieldCalcType,
                                                             const double& sumYearFractionsFrom2ndActiveToCurCashflow )
    {
        double discountFactor = 0.0;

        // Return Discount Factor = 0.0 if the cashflow payment is in the past, note today is considered in the past (as Bloomberg consider today in the past).
        if ( couponPaymentDate <= settlementDate )
        {
            return discountFactor;
        }

        const double simpleYearFraction = convertBondFrequencyToYearFraction( bondYieldParameters.couponFrequency_ );
 
        if (yieldCalcType == TRUE_YIELD)
        {
            // Since the previous cashflows' year fractions are scaled by the simpleYearFraction, we need to remove this scale in DF calculation
            double totalYearFraction = accruedAdjustmentFactor + sumYearFractionsFrom2ndActiveToCurCashflow / simpleYearFraction;
            
            discountFactor = std::pow( ( 1 + yield * simpleYearFraction ), (-1.0) * totalYearFraction );
        }
        else
        {

            // Exclude the upfront notional exchange from the nthCoupon calculation. The discount factor from the upfront notional exchange should not be 
            // included in the compounded discount factor calculation
            unsigned int nthCoupon = getBondRelativeCashflowIndex( firstActivePaymentIndex, couponPaymentDate, bondPaymentlDatesForIndexation );
    
            discountFactor = std::pow( ( 1 + yield * simpleYearFraction ), (-1.0) * (accruedAdjustmentFactor + nthCoupon) );
        }
        
        return discountFactor;
    }

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
	const double  calculateSingleDiscountFactorFromConventionsAndBondYield( const BondYieldParameters & bondYieldParameters,
                                                                            const AQLDate& settlementDate, 
                                                                            unsigned int& firstActivePaymentIndex,
                                                                            const AQLDate& currentPaymentDate, const AQLDate& previousPaymentDate, 
		                                                                    const std::vector< AQLDate>& paymentDates,
                                                                            const double& yield, const double& settleDateToFirstActivePaymentDateRatio, 
		                                                                    const YieldCalculationTypeEnum& yieldCalcType,
                                                                            const double& sumYearFractionsFromSecondActivePaymentToCurrentCashflow,
		                                                                    const double& simpleYearFraction,
                                                                            const DayCountEnum & bondDaycount,
                                                                            const bool& isLastCashflow,
                                                                            const bool& isSettleDateInLastCashflow )
	{
		double discountFactor = 0.0;

		// If it's the last cashflow of the Italian Bonds, and if settle date is in the last cashflow:
		if ( isSettleDateInLastCashflow && isItalianGovenmentBond(bondYieldParameters.calculationType_))
		{
			// Use simple / original yield for the discount factor
			double quotedYield = convertYield( yield, bondYieldParameters.couponFrequency_ , bondYieldParameters.yieldFrequency_ );

			discountFactor = 1.0 / (1.0 + quotedYield * settleDateToFirstActivePaymentDateRatio * simpleYearFraction);

		}
		// If it's the last cashflow of the Aussie Bond calType23, and if settle date is in the last cashflow:
		else if (isSettleDateInLastCashflow && bondYieldParameters.calculationType_ == TYPE23_AUSTRALIAN_GOVERNMENT_BONDS)
		{
			//--- DF formula: 1/(1 +(Days Remaining to Maturity/365)* Yield ---//

			//Days remaining to maturity
			const AQLDate maturityDate = currentPaymentDate;

			// If the maturity is a holiday, use the next business date
			const AQLDate adjustedMaturityDate = etrading::LADateScheduleHelpers::getDate(maturityDate, "0D", "FOLLOWING", bondYieldParameters.calendar_.c_str());


			const double remainingDays = getBondActualCouponPeriodDays(settlementDate, adjustedMaturityDate, bondDaycount);
			const double yearFraction = remainingDays / 365.0;

			discountFactor = 1.0 / (1.0 + yield * yearFraction);

		}
		//Special handling for SPAIN Govt
		else if ( isLastCashflow && bondYieldParameters.calculationType_ == TYPE1029_SPAIN_GOVERNMENT_BONDS)
		{
			const AQLDate maturityDate = currentPaymentDate;


			if (isSettleDateInLastCashflow)
			{
				discountFactor = std::pow((1 + yield * simpleYearFraction), (-1.0) * settleDateToFirstActivePaymentDateRatio);
			}
			else
			{
				// If the maturity is a holiday, use the next business date
				const AQLDate adjustedMaturityDate = etrading::LADateScheduleHelpers::getDate(maturityDate, "0D", "FOLLOWING", bondYieldParameters.calendar_.c_str());
				const double adjustedDays = previousPaymentDate.intervalDays(adjustedMaturityDate);
				const double unAdjustedDays = previousPaymentDate.intervalDays(maturityDate);

				const double adjustMaturityDateRatio = adjustedDays / unAdjustedDays;

				//number of coupons from settleDateCashflow (exclusive) to previusCashflow (inclusive)
				int nthCoupon = getBondRelativeCashflowIndex(firstActivePaymentIndex, previousPaymentDate, paymentDates);

				discountFactor = std::pow((1 + yield * simpleYearFraction), (-1.0) * (settleDateToFirstActivePaymentDateRatio + nthCoupon + adjustMaturityDateRatio));
			}

		}
		//Special handling for SPAIN T-Bill, Calculates simple interest for the 6- and 12-month bills and compound interest for the 18-month bills.
		else if (bondYieldParameters.calculationType_ == TYPE730_SPAIN_T_BILL)
		{
			const AQLDate accrualStartDate = paymentDates.front();
			const AQLDate maturityDate = currentPaymentDate;

			const double issueToMaturityYearFraction = getYearFraction(accrualStartDate, maturityDate, bondDaycount);

			//number of coupons from settleDateCashflow (exclusive) to currentCashflow (inclusive)
			int nthCoupon = getBondRelativeCashflowIndex(firstActivePaymentIndex, maturityDate, paymentDates);

			// 6 - and 12 - month bills, use simple yield for the discount factor, 1.25 is a proxy for 1 year to allow for holidays

			if (issueToMaturityYearFraction <= 1.25)
			{
				discountFactor = 1.0 / (1.0 + yield * ( settleDateToFirstActivePaymentDateRatio + nthCoupon) );
			}
			// 18-month bills, use compound yield for the discount factor
			else
			{
				discountFactor = std::pow((1 + yield *simpleYearFraction), (-1.0) * (settleDateToFirstActivePaymentDateRatio + nthCoupon));
			}

		}
		else
		{
			discountFactor = calculateSingleDiscountFactorFromBondYield(bondYieldParameters, settlementDate, firstActivePaymentIndex, currentPaymentDate, paymentDates, yield, settleDateToFirstActivePaymentDateRatio, yieldCalcType, sumYearFractionsFromSecondActivePaymentToCurrentCashflow);
		}                

		return discountFactor;
	}
    
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
																		const YieldCalculationTypeEnum& yieldCalcType, const DoubleVector& trueYieldYearFractions ) 

    {

        const double settleDateToFirstActivePaymentDateRatio  = calculateBondActiveCouponFutureAccrualRatio( settlementDate,
																											 activeCouponDates,
                                                                                                             bondDaycount,
																											 bondYieldParameters.couponFrequency_, 
																											 yieldCalcType );

        auto firstActivePaymentIndex       = getBondCashflowIndex( activeCouponDates.firstActiveCouponDate_, paymentDates );
        auto secondActivePaymentIndex      = firstActivePaymentIndex + 1;
        
        size_t paymentDatesSize = paymentDates.size();

        std::vector< double > discountFactors;
        discountFactors.reserve(paymentDatesSize);

		auto simpleYearFraction = convertBondFrequencyToYearFraction(bondYieldParameters.couponFrequency_);

        for( size_t i = 0; i < paymentDatesSize; ++i )
        {
            double sumYearFractionsFromSecondActivePaymentToCurrentCashflow = 0;
            if (yieldCalcType== TRUE_YIELD)
            {
                if (trueYieldYearFractions.size() == 0)
                {
                    throw AQLCoreInvalidData("#Error: For True Yield, trueYieldYearFractions cannot be empty.",__FILE__,__LINE__);
                }

                for( size_t k = secondActivePaymentIndex; k <= i; ++k )
                {
                    sumYearFractionsFromSecondActivePaymentToCurrentCashflow += trueYieldYearFractions[k];
                }

            }

            double discountFactor = 0.0;

            //Only calculate DF for the activeCashflows
            if ( i >= firstActivePaymentIndex)
            {
 
               auto previousPaymentDate = paymentDates[i - 1];
			   auto currentPaymentDate = paymentDates[i];

			   const bool isLastCashflow = (i == paymentDatesSize - 1);
			   const bool isSettleDateInLastCashflow = isLastCashflow && (settlementDate > previousPaymentDate);

			   discountFactor = calculateSingleDiscountFactorFromConventionsAndBondYield( bondYieldParameters,
																					settlementDate,
																					firstActivePaymentIndex,
																					currentPaymentDate,
																					previousPaymentDate,
																					paymentDates,
																					yield,
																					settleDateToFirstActivePaymentDateRatio,
																					yieldCalcType,
																					sumYearFractionsFromSecondActivePaymentToCurrentCashflow,
																					simpleYearFraction,
																					bondDaycount,
																					isLastCashflow,
																					isSettleDateInLastCashflow);
			                
            }

            discountFactors.push_back( discountFactor);

        }

        return discountFactors;
    }


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
																		const DoubleVector& trueYieldYearFractions )
	{
		const YieldCalculationTypeEnum yieldCalcType = bondCurve.getYieldCalculationTypeEnum();
        const double settleDateToFirstActivePaymentDateRatio  = calculateBondActiveCouponFutureAccrualRatio( settlementDate,
																											 activeCouponDates,
                                                                                                             bondDaycount,
																											 bondYieldParameters.couponFrequency_, 
																											 yieldCalcType );

        auto firstActivePaymentIndex       = getBondCashflowIndex( activeCouponDates.firstActiveCouponDate_, paymentDates );
        auto secondActivePaymentIndex      = firstActivePaymentIndex + 1;
        
        size_t paymentDatesSize = paymentDates.size();

        std::vector< double > discountFactors;
        discountFactors.reserve(paymentDatesSize);

		auto simpleYearFraction = convertBondFrequencyToYearFraction(bondYieldParameters.couponFrequency_);

        for( size_t i = 0; i < paymentDatesSize; ++i )
        {
            double sumYearFractionsFromSecondActivePaymentToCurrentCashflow = 0;
            if (yieldCalcType== TRUE_YIELD)
            {
                if (trueYieldYearFractions.size() == 0)
                {
                    throw AQLCoreInvalidData("#Error: For True Yield, trueYieldYearFractions cannot be empty.",__FILE__,__LINE__);
                }

                for( size_t k = secondActivePaymentIndex; k <= i; ++k )
                {
                    sumYearFractionsFromSecondActivePaymentToCurrentCashflow += trueYieldYearFractions[k];
                }

            }

            double discountFactor = 0.0;

            //Only calculate DF for the activeCashflows
            if ( i >= firstActivePaymentIndex)
            {

               auto previousPaymentDate = paymentDates[i - 1];
			   auto currentPaymentDate = paymentDates[i];
			   const double yield = bondCurve.getYield( currentPaymentDate );

			   const bool isLastCashflow = (i == paymentDatesSize - 1);

			   const bool isSettleDateInLastCashflow = isLastCashflow && (settlementDate > previousPaymentDate);

			   discountFactor = calculateSingleDiscountFactorFromConventionsAndBondYield( bondYieldParameters,
																					settlementDate,
																					firstActivePaymentIndex,
				   																	currentPaymentDate,
																					previousPaymentDate,
																					paymentDates,
																					yield,
																					settleDateToFirstActivePaymentDateRatio,
																					yieldCalcType,
																					sumYearFractionsFromSecondActivePaymentToCurrentCashflow,
																					simpleYearFraction,
																					bondDaycount,
																					isLastCashflow,
																					isSettleDateInLastCashflow);
            }

            discountFactors.push_back( discountFactor );

        }

        return discountFactors;
    }
    
}