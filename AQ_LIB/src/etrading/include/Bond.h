/*
 * @brief			Base Class for the Bond Analytics
 * @Created:		17th January 2017
 * @Author:			Nicholas Burgess
 * @Department:	    Quant Research & Analytics
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once

#include <string>
#include "BondSchedule.h"
#include "CoreEnumerations.h"
#include "BondEnumerations.h"
#include "BondYields.h"
#include "IsLWOObject.h"
#include "SchemaObject.h"
#include "BondUtilities.h"
#include "BondCurves.h"

namespace etrading
{
    // Declare Bond Shared Pointer Typedef
    class Bond;
    typedef std::shared_ptr< Bond > BondPtr;

    class Bond : public IsLWOObject 
    {
	public:
        
        Bond( const std::string& bondObjectName, const LabelValueBlock& bondLVB, const LabelValueBlock& scheduleLVB );
		Bond(const std::string& bondObjectName, const LabelValueBlock& bondLVB);
		Bond( const Bond& rhs );
        virtual ~Bond() {};

        virtual BondPtr clone() const                                                            = 0;

        virtual double yield( const LADate& settlementDate, const double& price, const YieldCalculationTypeEnum& yieldCalcType ) const = 0;
		
		/* @brief	Calculates the bond yield. This API takes in a dataProvider object
		*  @param[in]	dataProvider	Contains the bond settlement date, plus other market data required for more complex bonds such as FRNs
		*  @param[in]	price			The quoted bond price
		*  @param[in]	yieldCalcType	Specifies the yield calculation type required
		*  @returns		The bond yield
		*/
		virtual double yield( const DataProvider& dataProvider, const double price, const YieldCalculationTypeEnum& yieldCalcType ) const = 0;
        virtual double price( const LADate& settlementDate, const double& yield, const YieldCalculationTypeEnum& yieldCalcType ) const;
		
		/* @brief: Calculates the price of a bond using a BondCurve to discount coupons.
		* @param[in]	settlementDate	The bond settlement date
		* @param[in]	bondCurve		A calibrated bond curve
		* @returns		The bond price. This is clean or dirty, depending on the convention used by the bond
		*/
		virtual double priceFromBondCurve( const LADate& settlementDate, const BondCurve& bondCurve ) const;

		/* @brief Calculates the dirty price of a bond using a BondCurve to discount the coupons.
		* @param[in]	settlementDate	The bond settlement date
		* @param[in]	bondCurve		A calibrated BondCurve
		* @returns		The bond dirty price.
		*/
		virtual double dirtyPriceFromBondCurve( const LADate& settlementDate, const BondCurve& bondCurve ) const = 0;
		
		/* @brief Calculates the clean price of a bond using a BondCurve to discount the coupons.
		* @param[in]	settlementDate	The bond settlement date
		* @param[in]	bondCurve		A calibrated BondCurve
		* @returns		The bond clean price.
		*/
		virtual double cleanPriceFromBondCurve( const LADate& settlementDate, const BondCurve& bondCurve ) const = 0;

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
		virtual double yieldFromPriceAndBondCurve( const LADate& settlementDate,  const double& price, BondCurve& bondCurve ) const = 0;

		/* @brief	Calculates the bond yield-to-maturity from a bund curve
		*  @param[in]	settlementDate	The bond settlement date
		*  @param[in]	bondCurve		A bondCurve used for discounting coupons
		*  @returns		The bond yield to maturity
		*/
		virtual double yieldFromBondCurve( const LADate& settlementDate, const BondCurve& bondCurve ) const = 0;

		/* @brief: Calculates the price of a defaultable bond using a CreditModel to compute survival probabilities.
		* @param[in]	settlementDate	The bond settlement date
		* @param[in]	creditModel		The calibrated credit model
		* @returns		The bond price. This is clean or dirty, depending on the convention used by the bond
		*/
		virtual double priceFromCreditModel( const LADate& settlementDate, const CreditModel& creditModel ) const;

		//bondActiveCouponDates is populated when solving yields, as the activeCouponDate won't change
		virtual double dirtyPrice( const LADate& settlementDate, const double& yield, const YieldCalculationTypeEnum& yieldCalcType, bool isCompoundYield=false, const std::shared_ptr<BondActiveCouponDates>& activeCouponDatesPtr={}) const     = 0;

		/* @brief: Calculates the dirty price of a defaultable bond using a CreditModel to compute survival probabilities.
		* @param[in]	settlementDate	The bond settlement date
		* @param[in]	creditModel		The calibrated credit model
		* @returns		The bond dirty price.
		*/
		virtual double dirtyPriceFromCreditModel( const LADate& settlementDate, const CreditModel& creditModel ) const = 0;

		virtual double cleanPrice( const LADate& settlementDate, const double& yield, const YieldCalculationTypeEnum& yieldCalcType ) const     = 0;

		/* @brief: Calculates the clean price of a defaultable bond using a CreditModel to compute survival probabilities.
		* @param[in]	settlementDate	The bond settlement date
		* @param[in]	creditModel		The calibrated credit model
		* @returns		The bond clean price.
		*/
		virtual double cleanPriceFromCreditModel( const LADate& settlementDate, const CreditModel& creditModel ) const = 0;

		/* @brief: Solves for the hazard rate implied by the price of a defaultable bond.
		*		   Note: This modifies the hazard rate in the credit model. Used in CreditModel calibration.
		*
		* @param[in]	settlementDate			The bond settlement date
		* @param[in]	price					The bond price quote. Either clean or dirty, depending on the convention used by the bond
		* @param[inout]	creditModel				The calibrated credit model
		* @param[in]	useHullApproximation	Whether to use simple approximation: HazardRate = ( Yield - RiskFreeRate ) / (1-RecoveryRate), or to use a solver
		* @returns		The implied hazard rate
		*/
		virtual double hazardRateFromPrice( const LADate& settlementDate, const double price, CreditModel& creditModel, const bool useHullApproximation = false ) const = 0;

		/* @brief Calculates the bond accrued interest. This API takes in a dataProvider object.
		*  @param[in]	dataProvider	Contains the bond settlementDate for the calculation
		*  @returns		The bond accrued interest
		*/
        virtual double accruedInterest( const DataProvider& dataProvider ) const = 0;
        virtual unsigned int accruedInterestDays( const LADate& settlementDate ) const           = 0;

        virtual double compoundYieldFromQuotedYield( const LADate& settlementDate, const double& yield, const YieldCalculationTypeEnum& yieldCalcType ) const = 0;

        //Solve compound yield either 1) based on the cashflow calculation, or 2) use JGB Approximation
        virtual double compoundYieldFromQuotedPrice( const LADate& settlementDate, const double& price, const YieldCalculationTypeEnum& yieldCalcType ) const = 0;

		/* @brief	Utility functions to convert between clean to dirty price. The API takes in a dataProvider parameter which
		 * in addition to providing the settlementDate can provide further parameters used for more complex bonds such as FRNs.
		 * @param[in]	dirtyPrice/cleanPrice	The input price
		 * @param[in]	dataProvider			Input market data parameters. At minimum this should provide the settlementDate.
		 * @returns		The converted price.
		 */
        double priceFromDirtyToClean( const double dirtyPrice, const DataProvider& dataProvider ) const;
        double priceFromCleanToDirty( const double cleanPrice, const DataProvider& dataProvider ) const;

		/* @brief	Utility functions for simple fixed bonds which support conversion between clean and dirty price using only the settlementDate.
		 *			For more complex bonds (such as FRNs) please use the alternative APIs above which takes in a DataProvider parameter.
		 * @param[in]	dirtyPrice/cleanPrice	The input price
		 * @param[in]	settlementDate			The settlement date used for the valuation
		 * @returns		The converted price.
		 */
		double priceFromDirtyToClean( const double dirtyPrice, const LADate& settlementDate ) const;
        double priceFromCleanToDirty( const double cleanPrice, const LADate& settlementDate ) const;

	    LADate getBondLastCouponDate(const LADate& settlementDate) const;

        virtual ScheduleTypeEnum getScheduleType() const                                    = 0;

		virtual double dv01Numerical( const LADate& settlementDate, const double& yield, const YieldCalculationTypeEnum& yieldCalcType, const double bumpSize, const LAString& bumpMode ) const;
		virtual double dv01( const LADate& settlementDate, const double& yield, const YieldCalculationTypeEnum& yieldCalcType, bool isCompoundYield=false ) const = 0;
		virtual double modifiedDuration( const LADate& settlementDate, const double& yield, const YieldCalculationTypeEnum& yieldCalcType, bool isCompoundYield=false ) const = 0;

		//forward price from the given repo rate
		// 1) If the Actual repo rate is used, the result is the fairFwdPrice, where fairFwdPrice * ConversionFactor = Fair Future Price
		// 2) If the Implied repo rate is used, the result is the actualFwdPrice, where actualFwdPrice * ConversionFactor = Actual Future Price
		double forwardPrice(const double& price, const LADate& settleDate, const LADate& forwardSettleDate, const double& repoRate, const DayCountEnum& repoDayCount) const;

		/* @brief	Credit Risky forward dirty price calculated from a credit model and underlying discount curve
		*  @param[in]	forwardSettlementDate	The forward date on which to calculate the bond dirty price.
		*  @param[in]	creditModel				The credit model used to obtain survival probabilities and discount curve.
		*  @returns		The bond dirty price on the forward settle date
		*/
		virtual double forwardDirtyPriceFromCreditModel( const LADate& forwardSettleDate, const CreditModel& creditModel ) const = 0;

		/* @brief	Risk-free forward dirty price calculated from a discount curve
		*  @param[in]	forwardSettlementDate	The forward date on which to calculate the bond dirty price.
		*  @param[in]	curveCollection			The curve collection containing the discount curve
		*  @param[in]	discountCurve			The curve used to discount future coupons
		*  @returns		The bond dirty price on the forward settle date
		*/
		virtual double forwardDirtyPriceFromDiscountCurve( const LADate& forwardSettleDate, const std::string& curveCollection, const std::string& discountCurve ) const = 0;

		// implied/breakeven repo rate from forward price
		double impliedRepoRate(const double& price, const LADate& settleDate, const LADate& forwardSettleDate, const double& forwardPrice, const DayCountEnum& repoDayCount) const;

		// implied/breakeven repo rate from future price
		// If the Actual/Quoted future price is used, the result is implied repo rate (This is the meaning of implied repo rate in papers/BB)
		// If the Fair future price is used, the result  is actual repo rate
		double impliedRepoRateFromFuture(const double& price, const LADate& settleDate, const LADate& futureSettleDate, const double& futurePrice, const double& conversionFactor, const DayCountEnum& repoDayCount) const;

		// Future price from repo rate
		// 1) If the Actual repo rate is used, the result is the Fair future price
		// 2) If the Implied repo rate is used, the result is the Actual/Quoted Future price
		double futurePrice(const double& bondPrice, const LADate& settleDate, const LADate& futureSettleDate, const double& repoRate, const DayCountEnum& repoDayCount, const double& conversionFactor) const;

		// conversion factor of the bond against the future's first delivery date and notional coupon rate
		double conversionFactor(const LADate& firstFutureSettleDate, const double& notionalBondCouponRate) const;

		// Gross basis: currentCleanPrice - futurePrice * conversionFactor
		double grossBasis(const double& price, const LADate& settleDate, const double& futurePrice, const double& conversionFactor) const;

		// Net basis: forwardCleanPrice - futurePrice * conversionFactor
		double netBasis(const double& bondForwardPrice, const LADate& forwardSettleDate, const double& futurePrice, const double& conversionFactor) const;

		// Schema Helpers
        std::map<std::string, Variant> getDataMap() const;

		const DataSchema generateDataSchema(const std::string& schemaName) const;
		const SchemaObject toSchemaObject() const;
		//Append new schema to input schemaObject
		void toSchemaObject(SchemaObject& schemaObject) const; 	
		
		SchedulePtr getSchedule() const;
		
		// For cashflow display
		virtual std::unordered_set<CashflowHeaderEnum,EnumClassHash> allowedColumns() const;
		virtual void populateHeaderAndBody(const DataProvider& dataProvider, AnyTypeVector& headers, std::vector<AnyTypeVector>& bodyBlock, bool showColumnHeaders, const std::unordered_set<CashflowHeaderEnum,EnumClassHash>& columnList) const;
		AnyTypeMatrix view( const LADate& settlementDate, const double& yield, const YieldCalculationTypeEnum& yieldCalcType, bool showColumnHeaders = true, const std::unordered_set<CashflowHeaderEnum,EnumClassHash>& columnList=std::unordered_set<CashflowHeaderEnum,EnumClassHash>() ) const;
        

	   // Accessors
        LabelValueBlock getInputParameters() const;
        LabelValueBlock getBondParameters() const                   { return bondParameters_; };
        LabelValueBlock getScheduleParameters() const               { return scheduleParameters_; };
        
        std::string getBondObjectName() const                       { return bondObjectName_; };
        std::string getBondDescription() const                      { return bondDescription_; };
        std::string getBondISIN() const                             { return bondISIN_; };
        CCY getCurrency() const										{ return currency_; };
        
        BondTypeEnum getBondTypeEnum() const                        { return bondType_; };
        YieldTypeEnum getYieldTypeEmum() const                      { return yieldType_; };
        BondQuoteConventionEnum getBondQuoteConventionEnum() const  { return bondQuoteConvention_; };
        
        const BondYieldParameters & getBondYieldParameters() const  { return bondYieldParameters_; };
        void setBondYieldParameters();

        bool isCleanPrice() const                                   { return isCleanPrice_; };

		virtual void initializeDataProvider( DataProvider& dataProvider, const double& yield, const YieldCalculationTypeEnum& yieldCalcType, const std::shared_ptr<BondActiveCouponDates>& activeCouponDatesPtr={}) const = 0;

        // Check that the given settlement date is valid i.e. not before bond start date and not after bond maturity
        void checkSettlementDateValid( const LADate & settlementDate ) const;

        //Use the user specified calculation type, if not specified, use the default one to the bond 
        YieldCalculationTypeEnum getYieldCalulationType(const std::string& yieldCalcType) const;

        const double convertYieldFromYieldFreqToCouponFreq(const double yield) const;

		//the reinvest coupons' value between settleDate and forwardSettleDate for bond forward
		double forwardReinvestedCouponValue(const double& price, const LADate& settleDate, const LADate& forwardSettleDate, const DayCountEnum& repoDayCount, const double& repoRate) const;

	protected:
        
		BondSchedulePtr schedule_;
   		LabelValueBlock bondParameters_;
        LabelValueBlock scheduleParameters_;
        
        std::string bondObjectName_;
        std::string bondDescription_;
        std::string bondISIN_;
        CCY currency_;

        BondTypeEnum bondType_;                         // Default=FIXED, FLOATER, PERPETUAL, CALLABLE, INFLATION-LINKED, CONVERTIBLE, STRUCTURED
        YieldTypeEnum yieldType_;                       // Default=YIELD-TO-MATURITY, YIELD-TO-WORST, YIELD-TO-CALL
        BondQuoteConventionEnum bondQuoteConvention_;   // Default=NONE, QUOTE_IN_32NDS

        bool isCleanPrice_;
        
        BondYieldParameters bondYieldParameters_;
		
		/* @brief Calculates the bond accrued interest percent. This API takes in a dataProvider object.
		*  @param[in]	dataProvider	Contains the bond settlementDate for the calculation
		*  @returns		The bond accrued interest percent
		*/
        virtual double accruedInterestPercent( const DataProvider& dataProvider ) const = 0 ;

        //Solve compound yield based on the cashflow calculation
        virtual double compoundYield( const LADate& settlementDate, const double& price, const YieldCalculationTypeEnum& yieldCalcType) const = 0;
        virtual double compoundYieldJGBApproximation( const LADate& settlementDate, const double& price) const;
        virtual double cleanPriceJGBApproximation(const LADate& settlementDate, const double& yield) const;


	private:
		void initializeBondDescriptionLVB(const LabelValueBlock& bondLVB);

		//Utility function to get the reinvest coupons between settleDate and forwardSettleDate
		std::vector< BondFwdReinvestedCoupon > getBondFwdReinvestedCoupons(const double& price, const LADate& settleDate, const LADate& forwardSettleDate, const DayCountEnum& repoDayCount, const double& repoRate = std::numeric_limits<double>::quiet_NaN()) const;
	};

	typedef std::shared_ptr< Bond > BondPtr;
}
