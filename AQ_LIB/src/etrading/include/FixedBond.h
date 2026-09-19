#pragma once

#include "Bond.h"
#include "FixedSchedule.h"


namespace etrading
{
    class FixedBond : public Bond
    {
    public:

        FixedBond( const std::string& instanceName, const LabelValueBlock& bondLVB, const LabelValueBlock& scheduleLVB);
		FixedBond(const std::string& bondObjectName, const LabelValueBlock& bondLVB);
		FixedBond( const FixedBond& rhs) ;
		virtual ~FixedBond() {}
        
        BondPtr clone() const;
        
        double yield( const AQLDate& settlementDate, const double& price, const YieldCalculationTypeEnum& yieldCalcType ) const;

		/* @brief	Calculates the bond yield. This API takes in a dataProvider object
		*  @param[in]	dataProvider	Contains the bond settlement date
		*  @param[in]	price			The quoted bond price
		*  @param[in]	yieldCalcType	Specifies the yield calculation type required
		*  @returns		The bond yield
		*/
		double yield( const DataProvider& dataProvider, const double price, const YieldCalculationTypeEnum& yieldCalcType ) const;

		//bondActiveCouponDates is populated when solving yields, as the activeCouponDate won't change
		double dirtyPrice( const AQLDate& settlementDate, const double& yield, const YieldCalculationTypeEnum& yieldCalcType , bool isCompoundYield=false, const std::shared_ptr<BondActiveCouponDates>& activeCouponDatesPtr={}) const;

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
		double yieldFromPriceAndBondCurve( const AQLDate& settlementDate, const double& price, BondCurve& bondCurve ) const;

		/* @brief	Calculates the bond yield-to-maturity from a bund curve
		*  @param[in]	settlementDate	The bond settlement date
		*  @param[in]	bondCurve		A bondCurve used for discounting coupons
		*  @returns		The bond yield to maturity
		*/
		double yieldFromBondCurve( const AQLDate& settlementDate, const BondCurve& bondCurve ) const;

		/* @brief: Calculates the accrued interest of a defaultable bond using a CreditModel to compute survival probabilities.
		*  @param[in]	settlementDate	The bond settlement date
		*  @param[in]	creditModel		The calibrated credit model
		*  @returns		The bond accrued interest, scaled by the probability of survival to the next coupon.
		*/
		double accruedInterestPercentFromCreditModel( const AQLDate& settlementDate, const CreditModel& creditModel ) const;

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
		*  @param[in]	dataProvider	Contains the bond settlementDate for the calculation
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

		/* @brief: Initializes the dataProvider using discount factors calculated using yield points obtained from a BondCurve.
		*
		* @param[out]	dataProvider			The dataProvider to initialize
		* @param[in]	bondCurve				The bond curve
		* @param[in]	activeCouponDatesPtr	Specifies the coupon dates which bracket the settlement date.
		*/
		void initializeDataProviderWithBondCurve( DataProvider& dataProvider, const BondCurve& bondCurve, const std::shared_ptr<BondActiveCouponDates>& activeCouponDatesPtr={} ) const;

		/* @brief: Initializes the dataProvider using discount factors from the specified discount curve and curveCollection.
		*
		* @param[out]	dataProvider	The dataProvider to initialize
		* @param[in]	discountCurve	The discount curve name
		*/
		void initializeDataProviderWithCurveData( DataProvider& dataProvider, const AQLString& discountCurve ) const;

   	protected:
		/* @brief Calculates the bond accrued interest percent. This API takes in a dataProvider object.
		*  @param[in]	dataProvider	Contains the bond settlementDate for the calculation
		*  @returns		The bond accrued interest percent
		*/
        double accruedInterestPercent( const DataProvider& dataProvider ) const;
        double getCleanPriceFromSimpleYield(const AQLDate& settlementDate, double simpleYield) const;
        double getSimpleYieldFromCleanPrice(const AQLDate& settlementDate, double cleanPrice) const;
        //Helper function to avoid duplication
        double getYearFractionFromSettleToMaturityDtForSimpleYieldCalType(const AQLDate& settlementDate) const;
        
        //Solve compound yield based on the cashflow calculation
        double compoundYield( const AQLDate& settlementDate, const double& price, const YieldCalculationTypeEnum& yieldCalcType) const;
        double compoundYieldJGBApproximation( const AQLDate& settlementDate, const double& price) const;
        double cleanPriceJGBApproximation(const AQLDate& settlementDate, const double& yield) const;

    };

}

