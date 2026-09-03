#pragma once

#include "Bond.h"
#include "FixedSchedule.h"


namespace etrading
{
    class FloatingBond : public Bond
    {
    public:

        FloatingBond( const std::string& instanceName, const LabelValueBlock& bondLVB, const LabelValueBlock& scheduleLVB);
		FloatingBond(const std::string& bondObjectName, const LabelValueBlock& bondLVB);
		FloatingBond( const FloatingBond& rhs) ;
		virtual ~FloatingBond() {}
        
        BondPtr clone() const;
        
        double yield( const AQLDate& settlementDate, const double& price, const YieldCalculationTypeEnum& yieldCalcType ) const;
		
		/* @brief	Calculates the bond yield. This API takes in a dataProvider object
		*  @param[in]	dataProvider	Contains the bond settlement date, plus other market data required to calculate the FRN yield
		*  @param[in]	price			The quoted bond price
		*  @param[in]	yieldCalcType	Specifies the yield calculation type required
		*  @returns		The bond yield
		*/
		double yield( const DataProvider& dataProvider, const double price, const YieldCalculationTypeEnum& yieldCalcType ) const;

		double dirtyPrice( const AQLDate& settlementDate, const double& yield, const YieldCalculationTypeEnum& yieldCalcType , bool isCompoundYield=false, const std::shared_ptr<BondActiveCouponDates>& activeCouponDatesPtr={} ) const;
        double cleanPrice( const AQLDate& settlementDate, const double& yield, const YieldCalculationTypeEnum& yieldCalcType  ) const;

		/* @brief Calculates the dirty price of a bond using a BondCurve to discount the coupons.
		* @param[in]	settlementDate	The bond settlement date
		* @param[in]	bondCurve		A calibrated BondCurve
		* @returns		The bond dirty price.
		*/
		double dirtyPriceFromBondCurve( const AQLDate& settlementDate, const BondCurve& bondCurve ) const;

		/* @brief Calculates the clean price of a bond using a BondCurve to discount the coupons.
		* @param[in]	settlementDate	The bond settlement date
		* @param[in]	bondCurve		A calibrated BondCurve
		* @returns		The bond clean price.
		*/
		double cleanPriceFromBondCurve( const AQLDate& settlementDate, const BondCurve& bondCurve ) const;
		
		/* @brief	calculate the bond yield from the price quote and a bondCurve for discounting coupons
		*			NOTE: Used when CALIBRATING the bondCurve. A node point is added to the bondCurve
		*			corresponding to the maturity of this bond, and the yield of that point is adjusted
		*			until the bond matches the input price quote.
		*  @param[in]		settlementDate	The bond settlement date
		*  @param[in]		price			The bond inpiut price quote
		*  @param[inout]	bondCurve		A bondCurve used for discounting coupons
		*									NOTE: the bondCurve is UPDATED to include the yield point for this bond.
		*  @returns		The bond yield to maturity
		*/
		double yieldFromPriceAndBondCurve( const AQLDate& settlementDate,  const double& price, BondCurve& bondCurve ) const;

		/* @brief	Calculates the bond yield-to-maturity from a bund curve
		*  @param[in]	settlementDate	The bond settlement date
		*  @param[in]	bondCurve		A bondCurve used for discounting coupons
		*  @returns		The bond yield to maturity
		*/
		double yieldFromBondCurve( const AQLDate& settlementDate, const BondCurve& bondCurve ) const;

		/* @brief: Calculates the dirty price of a defaultable bond using a CreditModel to compute survival probabilities.
		* @param[in]	settlementDate	The bond settlement date
		* @param[in]	creditModel		The calibrated credit model
		* @returns		The bond dirty price.
		*/
		double dirtyPriceFromCreditModel( const AQLDate& settlementDate, const CreditModel& creditModel ) const;

		/* @brief: Calculates the clean price of a defaultable bond using a CreditModel to compute survival probabilities.
		* @param[in]	settlementDate	The bond settlement date
		* @param[in]	creditModel		The calibrated credit model
		* @returns		The bond clean price.
		*/
		double cleanPriceFromCreditModel( const AQLDate& settlementDate, const CreditModel& creditModel ) const;

		/* @brief: Solves for the hazard rate implied by the price of a defaultable bond.
		*		   Note: This modifies the hazard rate in the credit model. Used in CreditModel calibration.
		*
		* @param[in]	settlementDate			The bond settlement date
		* @param[in]	price					The bond price quote. Either clean or dirty, depending on the convention used by the bond
		* @param[inout]	creditModel				The calibrated credit model
		* @param[in]	useHullApproximation	Whether to use simple approximation: HazardRate = ( Yield - RiskFreeRate ) / (1-RecoveryRate), to to use a solver
		* @returns		The implied hazard rate
		*/
		double hazardRateFromPrice( const AQLDate& settlementDate, const double price, CreditModel& creditModel, const bool useHullApproximation = false ) const;

		/* @brief	Credit Risky forward dirty price calculated from a credit model and underlying discount curve
		*  @param[in]	forwardSettlementDate	The forward date on which to calculate the bond dirty price.
		*  @param[in]	creditModel				The credit model used to obtain survival probabilities and discount curve.
		*  @returns		The bond dirty price on the forward settle date
		*/
		double forwardDirtyPriceFromCreditModel( const AQLDate& forwardSettleDate, const CreditModel& creditModel ) const;

		/* @brief	Risk-free forward dirty price calculated from a discount curve
		*  @param[in]	forwardSettlementDate	The forward date on which to calculate the bond dirty price.
		*  @param[in]	curveCollection			The curve collection containing the discount curve
		*  @param[in]	discountCurve			The curve used to discount future coupons
		*  @returns		The bond dirty price on the forward settle date
		*/
		double forwardDirtyPriceFromDiscountCurve( const AQLDate& forwardSettleDate, const std::string& curveCollection, const std::string& discountCurve ) const;

		/* @brief Calculates the bond accrued interest. This API takes in a dataProvider object.
		*  @param[in]	dataProvider	Contains the bond settlement date, plus other market data required to calculate the FRN accrued interest
		*  @returns		The bond accrued interest
		*/
        double accruedInterest( const DataProvider& dataProvider ) const;
 
		unsigned int accruedInterestDays( const AQLDate& settlementDate ) const;

        double compoundYieldFromQuotedYield( const AQLDate& settlementDate, const double& yield, const YieldCalculationTypeEnum& yieldCalcType ) const;

        //Solve compound yield either 1) based on the cashflow calculation, or 2) use JGB Approximation
        double compoundYieldFromQuotedPrice( const AQLDate& settlementDate, const double& price, const YieldCalculationTypeEnum& yieldCalcType ) const;

        ScheduleTypeEnum getScheduleType() const;

        double annuity( const double& yield, const YieldCalculationTypeEnum& yieldCalcType  ) const;
		double annuityWithNotional( const double& yield, const YieldCalculationTypeEnum& yieldCalcType  ) const;
        double pv01( const double& yield, const YieldCalculationTypeEnum& yieldCalcType  ) const;
        double dv01( const AQLDate& settlementDate, const double& yield, const YieldCalculationTypeEnum& yieldCalcType , bool isCompoundYield=false ) const;
		double modifiedDuration( const AQLDate& settlementDate, const double& yield, const YieldCalculationTypeEnum& yieldCalcType , bool isCompoundYield=false ) const;

		static std::vector<std::string> lvbKeys()
		{
			std::vector< std::string > scheduleKeys = FixedSchedule::lvbKeys();
			return scheduleKeys;
		}

		virtual void initializeDataProvider( DataProvider& dataProvider, const double& yield, const YieldCalculationTypeEnum& yieldCalcType, const std::shared_ptr<BondActiveCouponDates>& activeCouponDatesPtr={} ) const;

		/* @brief	Computes the price of a floating rate bond without requiring discount and forecast curves.
		*			The price is found given the following assumed values:  the discountMargin, assumedIndex, indexToNextCoupon, annualizedNextCouponRate.
		*			Assume: Future projected coupons are all the same, and future coupon periods are the same.
		*			See BBG CalcType 21 for calculation details.
		*			Also see: Stignum and Robinson "Money Market and Bond Calculations" pp259-271
		*
		* @param[in]	settlementDate				settlement date
		* @param[in]	discountMargin				Additional discount rate for projected coupons
		* @param[in]	assumedRate					Assumed index rate used for projected coupons
		* @param[in]	indexToNextCoupon			The index discount rate for the next coupon period. i.e. the Libor rate from settlement date to next coupon
		* @param[in]	annualizedNextCouponRate	The rate of the next coupon, already fixed.

		* @returns	The clean / dirty floating bond price	
		*/
		double priceFromDiscountMargin( const AQLDate& settlementDate, const double& discountMargin, const double& assumedRate, const double& indexToNextCoupon, const double& annualizedNextCouponRate ) const;

		/* @brief	Computes the price of a floating rate bond from yield, without requiring discount and forecast curves.
		*			The price is found given the following assumed values:  the assumedIndex, indexToNextCoupon, annualizedNextCouponRate.
		*			Assume: Future projected coupons are all the same, and future coupon periods are the same.
		*			See BBG CalcType 21 for calculation details.
		*			Also see: Stignum and Robinson "Money Market and Bond Calculations" pp259-271
		*
		* @param[in]	settlementDate				settlement date
		* @param[in]	yield						Yield to maturity
		* @param[in]	assumedRate					Assumed index rate used for projected coupons
		* @param[in]	indexToNextCoupon			The index discount rate for the next coupon period. i.e. the Libor rate from settlement date to next coupon
		* @param[in]	annualizedNextCouponRate	The rate of the next coupon, already fixed.
		* @returns	The clean / dirty floating bond price	
		*/
		double priceFromYield( const AQLDate& settlementDate, const double& yield, const double& assumedRate, const double& indexToNextCoupon, const double& annualizedNextCouponRate ) const;

		/* @brief	Computes the yield of a floating rate bond from quoted price, without requiring discount and forecast curves.
		*			The yield is found given the following assumed values:  the assumedIndex, indexToNextCoupon, annualizedNextCouponRate.
		*			Assume: Future projected coupons are all the same, and future coupon periods are the same.
		*			See BBG CalcType 21 for calculation details.
		*			Also see: Stignum and Robinson "Money Market and Bond Calculations" pp259-271
		*
		* @param[in]	settlementDate				settlement date
		* @param[in]	targetPrice					The price of the bond, in the quoted convention (clean / dirty )
		* @param[in]	assumedRate					Assumed index rate used for projected coupons
		* @param[in]	indexToNextCoupon			The index discount rate for the next coupon period. i.e. the Libor rate from settlement date to next coupon
		* @param[in]	annualizedNextCouponRate	The rate of the next coupon, already fixed.
		* @returns	The bond yield	
		*/
		double yieldFromPrice( const AQLDate& settlementDate, const double& targetPrice, const double& assumedRate, const double& indexToNextCoupon, const double& annualizedNextCouponRate ) const;

		/* @brief	Computes the discount margin of a floating rate bond from quoted price, without requiring discount and forecast curves.
		*			The discount margin is found given the following assumed values:  the assumedIndex, indexToNextCoupon, annualizedNextCouponRate.
		*			Assume: Future projected coupons are all the same, and future coupon periods are the same.
		*			See BBG CalcType 21 for calculation details.
		*			Also see: Stignum and Robinson "Money Market and Bond Calculations" pp259-271
		*
		* @param[in]	settlementDate				settlement date
		* @param[in]	targetPrice					The price of the bond, in the quoted convention (clean / dirty )
		* @param[in]	assumedRate					Assumed index rate used for projected coupons
		* @param[in]	indexToNextCoupon			The index discount rate for the next coupon period. i.e. the Libor rate from settlement date to next coupon
		* @param[in]	annualizedNextCouponRate	The rate of the next coupon, already fixed.
		* @returns	The bond discount margin
		*/
		double discountMarginFromPrice( const AQLDate& settlementDate, const double& targetPrice, const double& assumedRate, const double& indexToNextCoupon, const double& annualizedNextCouponRate ) const;

		/* @brief	Computes the discount margin of a floating rate bond from quoted yield.
		*			The discount margin is found given the following assumed values:  the assumedIndex, indexToNextCoupon.
		*
		*			See Stignum and Robinson "Money Market and Bond Calculations" p270
		*
		* @param[in]	settlementDate				settlement date
		* @param[in]	yield						Yield to maturity
		* @param[in]	assumedRate					Assumed index rate used for projected coupons
		* @param[in]	indexToNextCoupon			The index discount rate for the next coupon period. i.e. the Libor rate from settlement date to next coupon
		* @returns	The bond discount margin
		*/
		double discountMarginFromYield( const AQLDate& settlementDate, const double& yield, const double& assumedRate, const double& indexToNextCoupon ) const;

		/* @brief	Computes the yield to maturity of a floating rate bond from quoted discount margin.
		*			The yield is found given the following assumed values:  the assumedIndex, indexToNextCoupon.
		*
		*			See Stignum and Robinson "Money Market and Bond Calculations" p270
		*
		* @param[in]	settlementDate				settlement date
		* @param[in]	discountMargin				Additional discount rate for projected coupons
		* @param[in]	assumedRate					Assumed index rate used for projected coupons
		* @param[in]	indexToNextCoupon			The index discount rate for the next coupon period. i.e. the Libor rate from settlement date to next coupon
		* @returns	The bond yield to maturity
		*/
		double yieldFromDiscountMargin( const AQLDate& settlementDate, const double& discountMargin, const double& assumedRate, const double& indexToNextCoupon ) const;

		// @returns The bond coupon spread / quoted margin over the reference index (in BPS).
		double getQuotedMargin() const;
   	protected:

		/* @brief Compute a year fraction adjustment factor, to spread the effect of leap years across all coupons.
		 *        BBG Calc Type 21 specifies that the adjustment factor be applied to all ACT/XX daycounts.
		 *
		 * @param[in]	dayCount	The bond schedule dayCount convention
		 * @returns	The calculated leap year adjustment factor
		 */
		double leapYearAdjustmentFactor( const DayCountEnum& dayCount ) const;

		/* @brief Determine the accrued interest for a floating bond, in a discount margin calculation.
		 *        This calculation uses the supplied annualizedCouponRate rather than the coupon rate from the bond schedule
		 *
		 * @param[in]	settlementDate				The settlement Date for the calculation
		 * @param[in]	activeCouponDates			A structure containing the prior and next coupon dates, relative to settlement date
		 * @param[in]	annualizedNextCouponRate	The annualized rate to use for the next (first) coupon.
		 *
		 * @returns	The accrued interest
		*/
		double accruedInterestFromAnnualizedCouponRate( const AQLDate& settlementDate, const BondActiveCouponDates& activeCouponDates,  const double& annualizedNextCouponRate ) const;
		
		/* @brief Calculates the bond accrued interest percent. This API takes in a dataProvider object.
		*  @param[in]	dataProvider	Contains the bond settlementDate, plus other market data required for the FRN.
		*  @returns		The bond accrued interest percent
		*/
		double accruedInterestPercent( const DataProvider& dataProvider ) const;

		//Solve compound yield based on the cashflow calculation
        double compoundYield( const AQLDate& settlementDate, const double& price, const YieldCalculationTypeEnum& yieldCalcType) const;

	private:
		void initializeBondDescriptionLVB(const LabelValueBlock& bondLVB);

		// The bond coupon spread / quoted margin over the reference index (in BPS).
		double quotedMargin_;
    };

}

