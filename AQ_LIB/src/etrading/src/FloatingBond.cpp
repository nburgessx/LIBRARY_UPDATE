#include "FloatingBond.h"
#include "FloatBondSchedule.h"
#include "BondAccrualPeriods.h"
#include "BondYields.h"
#include "SwapValidation.h"
#include "Solvers.h"
#include "FixedBondCashflow.h"
#include "ExceptionMacros.h"
#include "ParameterValidation.h"
#include "BondUtilities.h"

namespace etrading
{
   
	FloatingBond::FloatingBond(const std::string& instanceName, const LabelValueBlock& bondLVB, const LabelValueBlock& scheduleLVB) : Bond( instanceName, bondLVB, scheduleLVB )
	{
        initializeBondDescriptionLVB(bondLVB);

        // Generate the schedule if nullptr
        schedule_ = BondSchedulePtr( new FloatBondSchedule( scheduleLVB, instanceName ) );

        // Set the Bond Yield Parameters Struct
        setBondYieldParameters();
	}

	FloatingBond::FloatingBond(const std::string& instanceName, const LabelValueBlock& bondLVB) : Bond(instanceName, bondLVB)
	{
		initializeBondDescriptionLVB(bondLVB);

		schedule_ = BondSchedulePtr(new FloatBondSchedule(bondLVB, instanceName));

		// Set the Bond Yield Parameters Struct
		setBondYieldParameters();
	}

	void FloatingBond::initializeBondDescriptionLVB(const LabelValueBlock& bondLVB)
	{
		// Initialize Bond Label Value Block Parameters
		quotedMargin_ = bondLVB.getCompulsoryValueAsDouble(BOND_KEY::QUOTED_MARGIN, "bondLVB");
	}

    void FloatingBond::initializeDataProvider( DataProvider& dataProvider, const double& yield, const YieldCalculationTypeEnum& yieldCalcType, const std::shared_ptr<BondActiveCouponDates>& activeCouponDatesPtr ) const
    {
        // Check the settlement Date is Valid
        Bond::checkSettlementDateValid(dataProvider.getValuationSettings().getValuationDate() );

        Bond::initializeDataProvider( dataProvider, yield, yieldCalcType );
        
        // Add Bond Input Parameters to the BondYield Struct, since cannot pass the abstract bond class by reference
        BondYieldParameters bondYieldParameters = getBondYieldParameters();
        
        // Populate DFs for cashflows
        schedule_->initializeDataProviderWithYieldData( dataProvider, bondYieldParameters, yield,  yieldCalcType, activeCouponDatesPtr );
     
    }

	FloatingBond::FloatingBond( const FloatingBond& rhs) : Bond( rhs )
	{}
	
    BondPtr FloatingBond::clone() const
    {
        BondPtr bond = BondPtr( new FloatingBond( *this ) );
        return bond;
	}

	// @returns The bond coupon spread / quoted margin over the reference index (in BPS).
	double FloatingBond::getQuotedMargin() const
	{
		return quotedMargin_;
	}

	/* @brief Calculates the bond accrued interest percent. This API takes in a dataProvider object.
	*  @param[in]	dataProvider	Contains the bond settlementDate, plus other market data required for the FRN.
	*  @returns		The bond accrued interest percent
	*/
    double FloatingBond::accruedInterestPercent( const DataProvider& dataProvider ) const
    {
        const double accruedInterestCashflow = FloatingBond::accruedInterest( dataProvider );
		const double faceValue               = std::fabs(getSchedule()->getNotional()); // Notional can be negative, and scaling should not change the sign so we make it absolute
        const double accruedInterestPercent  = accruedInterestCashflow / faceValue * 100.0;

        return accruedInterestPercent;
    }

    double FloatingBond::yield( const AQLDate& settlementDate, const double& price, const YieldCalculationTypeEnum& yieldCalcType) const
    {
		AQ_THROW( "yield() is not yet supported for FloatingBonds." );
    }

	/* @brief	Calculates the bond yield. This API takes in a dataProvider object
	*  @param[in]	dataProvider	Contains the bond settlement date, plus other market data required to calculate the FRN yield
	*  @param[in]	price			The quoted bond price
	*  @param[in]	yieldCalcType	Specifies the yield calculation type required
	*  @returns		The bond yield
	*/
	double FloatingBond::yield( const DataProvider& dataProvider, const double price, const YieldCalculationTypeEnum& yieldCalcType ) const
	{
		// Check the settlement Date is Valid
		const ValuationSettings& valuationSettings = dataProvider.getValuationSettings();
		AQLDate settlementDate = valuationSettings.getSettlementDate();
        Bond::checkSettlementDateValid( settlementDate );

		const double assumedRate = valuationSettings.getFloatBondAssumedRate();
		const double indexToNextCoupon = valuationSettings.getFloatBondIndexToNextCoupon();
		const double annualizedNextCouponRate = valuationSettings.getFloatBondCurrentCouponRate();
		
		const double yield = yieldFromPrice( settlementDate, price, assumedRate, indexToNextCoupon, annualizedNextCouponRate );
		return yield;
	}

    //The price can be either dirtyPrice or cleanPrice
    double FloatingBond::compoundYield( const AQLDate& settlementDate, const double& price, const YieldCalculationTypeEnum& yieldCalcType) const
    {
		AQ_THROW( "compoundYield() is not yet supported for FloatingBonds." );
    }

    double FloatingBond::dirtyPrice( const AQLDate& settlementDate, const double& yield, const YieldCalculationTypeEnum& yieldCalcType, bool isCompoundYield, const std::shared_ptr<BondActiveCouponDates>& activeCouponDatesPtr ) const
    {
		AQ_THROW( "dirtyPrice() is not yet supported for FloatingBonds." );
    }

    double FloatingBond::cleanPrice( const AQLDate& settlementDate, const double& yield, const YieldCalculationTypeEnum& yieldCalcType ) const
    {
         AQ_THROW( "cleanPrice() is not yet supported for FloatingBonds." );
    }

	/* @brief Calculates the bond accrued interest. This API takes in a dataProvider object.
	*  @param[in]	dataProvider	Contains the bond settlement date, plus other market data required to calculate the FRN accrued interest
	*  @returns		The bond accrued interest
	*/
    double FloatingBond::accruedInterest( const DataProvider& dataProvider ) const
    {
		AQLDate settlementDate = dataProvider.getValuationSettings().getSettlementDate();
        Bond::checkSettlementDateValid( settlementDate );	

		const double annualizedNextCouponRate = dataProvider.getValuationSettings().getFloatBondCurrentCouponRate();
		if (boost::math::isnan(annualizedNextCouponRate))
		{
			AQ_THROW( "Missing FloatBondCoupon for floating bond." );
		}

		const BondActiveCouponDates activeCouponDates = schedule_->getBondFirstActiveCouponDates( settlementDate, false );

		const double accruedInterest = accruedInterestFromAnnualizedCouponRate( settlementDate, activeCouponDates, annualizedNextCouponRate );
        return accruedInterest;
	}

	/* @brief Calculates the dirty price of a bond using a BondCurve to discount the coupons.
	* @param[in]	settlementDate	The bond settlement date
	* @param[in]	bondCurve		A calibrated BondCurve
	* @returns		The bond dirty price.
	*/
	double FloatingBond::dirtyPriceFromBondCurve( const AQLDate& settlementDate, const BondCurve& bondCurve ) const
	{
		AQ_THROW( "dirtyPriceFromBondCurve() is not yet supported for FloatingBonds." );
	}
	
	/* @brief Calculates the clean price of a bond using a BondCurve to discount the coupons.
	* @param[in]	settlementDate	The bond settlement date
	* @param[in]	bondCurve		A calibrated BondCurve
	* @returns		The bond clean price.
	*/
	double FloatingBond::cleanPriceFromBondCurve( const AQLDate& settlementDate, const BondCurve& bondCurve ) const
	{
		AQ_THROW( "cleanPriceFromBondCurve() is not yet supported for FloatingBonds." );
	}

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
	double FloatingBond::yieldFromPriceAndBondCurve( const AQLDate& settlementDate, const double& price, BondCurve& bondCurve ) const
	{
		AQ_THROW( "yieldFromPriceAndBondCurve() is not yet supported for FloatingBonds." );
	}
	
	/* @brief	Calculates the bond yield-to-maturity from a bund curve
	*  @param[in]	settlementDate	The bond settlement date
	*  @param[in]	bondCurve		A bondCurve used for discounting coupons
	*  @returns		The bond yield to maturity
	*/
	double FloatingBond::yieldFromBondCurve( const AQLDate& settlementDate, const BondCurve& bondCurve ) const
	{
		AQ_THROW( "yieldFromBondCurve() is not yet supported for FloatingBonds." );
	}

	double FloatingBond::dirtyPriceFromCreditModel( const AQLDate& settlementDate, const CreditModel& creditModel ) const
	{
		AQ_THROW( "dirtyPriceFromCreditModel() is not yet supported for FloatingBonds." );
	}

	double FloatingBond::cleanPriceFromCreditModel( const AQLDate& settlementDate, const CreditModel& creditModel ) const
	{
		AQ_THROW( "cleanPriceFromCreditModel() is not yet supported for FloatingBonds." );
	}

	double FloatingBond::hazardRateFromPrice( const AQLDate& settlementDate, const double price, CreditModel& creditModel, const bool useHullApproximation ) const
	{
		AQ_THROW( "hazardRateFromPrice() is not yet supported for FloatingBonds." );
	}

	/* @brief	Credit Risky forward dirty price calculated from a credit model and underlying discount curve
	*  @param[in]	forwardSettlementDate	The forward date on which to calculate the bond dirty price.
	*  @param[in]	creditModel				The credit model used to obtain survival probabilities and discount curve.
	*  @returns		The bond dirty price on the forward settle date
	*/
	double FloatingBond::forwardDirtyPriceFromCreditModel( const AQLDate& forwardSettleDate, const CreditModel& creditModel ) const
	{
		AQ_THROW( "forwardDirtyPriceFromCreditModel() is not yet supported for FloatingBonds." );
	}

	/* @brief	Risk-free forward dirty price calculated from a discount curve
	*  @param[in]	forwardSettlementDate	The forward date on which to calculate the bond dirty price.
	*  @param[in]	curveCollection			The curve collection containing the discount curve
	*  @param[in]	discountCurve			The curve used to discount future coupons
	*  @returns		The bond dirty price on the forward settle date
	*/
	double FloatingBond::forwardDirtyPriceFromDiscountCurve( const AQLDate& forwardSettleDate, const std::string& curveCollection, const std::string& discountCurve ) const
	{
		AQ_THROW( "forwardDirtyPriceFromDiscountCurve() is not yet supported for FloatingBonds." );
	}

    unsigned int FloatingBond::accruedInterestDays( const AQLDate& settlementDate ) const
    {
        const unsigned int accruedInterestDays = schedule_->calculateBondAccruedInterestDays(settlementDate, bondYieldParameters_);
        return accruedInterestDays;
    }

    double FloatingBond::annuity( const double& yield, const YieldCalculationTypeEnum& yieldCalcType ) const
    {
        AQ_THROW( "annuity() is not yet supported for FloatingBonds.");
    }

	double FloatingBond::annuityWithNotional( const double& yield, const YieldCalculationTypeEnum& yieldCalcType ) const
    {
        AQ_THROW( "annuityWithNotional() is not yet supported for FloatingBonds.");
    }

    double FloatingBond::pv01( const double& yield, const YieldCalculationTypeEnum& yieldCalcType ) const
    {
        AQ_THROW( "pv01() is not yet supported for FloatingBonds.");
    }
    
    ScheduleTypeEnum FloatingBond::getScheduleType() const
    {
        return FLOAT_SCHEDULE_TYPE;
    }

	double FloatingBond::dv01( const AQLDate& settlementDate, const double& yield, const YieldCalculationTypeEnum& yieldCalcType, bool isCompoundYield ) const
	{
		AQ_THROW( "dv01() is not yet supported for FloatingBonds.");
	}

	double FloatingBond::modifiedDuration( const AQLDate& settlementDate, const double& yield, const YieldCalculationTypeEnum& yieldCalcType , bool isCompoundYield ) const
	{
		AQ_THROW( "modifiedDuration() is not yet supported for FloatingBonds.");
	}

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
	double FloatingBond::priceFromDiscountMargin( const AQLDate& settlementDate, const double& discountMargin, const double& assumedRate, const double& indexToNextCoupon, const double& annualizedNextCouponRate ) const
	{
		Bond::checkSettlementDateValid(settlementDate);

		if ( schedule_->getBondCalculationType() != TYPE21_FLOATING_RATE_NOTE )
		{
			AQ_THROW( "Discount margin calculation is only supported for Bond Calc Type 21.")	
		}

		/*
		 * Use simple compounding to calculate discount factor to next coupon.
		 * The discount rate is specified as: The underlying floating index rate to the next fixing date ( indexToNextCoupon ) + discountMargin.
		 */
		const DayCountEnum dayCount = schedule_->getAccrualDaycount();

		const BondActiveCouponDates activeCouponDates = schedule_->getBondFirstActiveCouponDates( settlementDate, false );
		
		const double yearFractionToNextCoupon = getYearFraction( settlementDate, activeCouponDates.firstActiveCouponDate_, dayCount );

		const double discountFactorToNextCoupon =  1.0 / ( 1.0 + yearFractionToNextCoupon * ( indexToNextCoupon + discountMargin ) );

		/*
		 * Compute the first floating coupon amount. This coupon rate has already fixed.
		 */
		const double faceValue        = getSchedule()->getNotional();
		AQ_REQUIRE(!AQ_IS_EQUAL_ZERO(faceValue), "Bond notional cannot be zero.");
		
        const double exactCouponYearFraction = getYearFraction( activeCouponDates.priorFirstActiveCouponDate_, activeCouponDates.firstActiveCouponDate_, dayCount); 

		const double firstCouponAmount = annualizedNextCouponRate * exactCouponYearFraction * faceValue;
		const double firstCouponAmountWithFloor = std::max(0.0, firstCouponAmount );
		/*
		 * Compute projected coupon amounts. Bond Calc Type 21 makes the assumption that all future coupons have equal value and are equally spaced.
		 *
		 * i) In order to make all coupons equal and to spread out the effect of leap years, the projected coupons are scaled up by a leap year Factor.
		 *    Note that BBG CSHF screen shows the projected coupons prior to this leapYear adjustment.
		 *
		 * ii)  Since these are floating coupons ( based on an assumedRate + quotedMargin ), we must floor the coupons at 0.0.
		 */
		const FrequencyEnum couponFrequency = schedule_->getAccrualFrequency();
		const double couponYearFraction = convertBondFrequencyToYearFraction( couponFrequency );

		const double leapYearFactor = leapYearAdjustmentFactor( dayCount );

		 // BBG CSHF screen shows this unadjusted cashflow value
		const double projectedCouponAmount             = ( assumedRate + quotedMargin_ * 0.0001 )  * faceValue * couponYearFraction;
		const double projectedCouponWithLeapYearFactor = projectedCouponAmount * leapYearFactor;
		const double projectedCouponWithFloor          = std::max( 0.0, projectedCouponWithLeapYearFactor );

		/*
		 * Compute the compound discount factor for each coupon period.  The coupons are discounted at ( assumedRate + discountMargin ).
		 * The year fraction is scaled up by the leap year adjustment factor.
		 */
		const double compoundDiscountFactor = 1.0 / ( 1.0 + ( assumedRate + discountMargin ) * couponYearFraction * leapYearFactor );

        auto cashflows = schedule_->getAllCashflowsExcludingUpfrontNotional();
		const size_t nCoupons = cashflows.size();
		AQ_REQUIRE( nCoupons > 0, "Unable to calculate the price. The bond has no cashflows." )

	     /*
		 *  Compute the PV of all future cashflows to the first coupon date:
		 *  PV = Sum_i ( projectedRate * tau * DF_i )
		 *
		 *  Assuming all coupons have the same projectedRate and tau,
		 *  PV = projectedRate * tau * Sum_i ( DF_i )
		 */
		double sumDF = 0.0;
		double couponDiscountFactor = 1.0;
		for ( size_t i=activeCouponDates.firstActiveCashflowIndex_ + 1; i < nCoupons; i++ )
		{
			couponDiscountFactor *= compoundDiscountFactor;
			sumDF += couponDiscountFactor;
		}
		const double projectedCouponPV = sumDF * projectedCouponWithFloor;

		const double redemptionAmount =  getSchedule()->getNotional();
		const double redemptionPV = redemptionAmount * couponDiscountFactor;

		const double totalPVToSettleDate = discountFactorToNextCoupon * ( projectedCouponPV  + redemptionPV + firstCouponAmountWithFloor );

		/*
		 * Finally calculate bond price from PV.
		 */
		double price = 0.0;
		if ( isCleanPrice_ )
		{
			const double accruedInterest = accruedInterestFromAnnualizedCouponRate(  settlementDate, activeCouponDates, annualizedNextCouponRate );
			price = ( totalPVToSettleDate - accruedInterest ) / faceValue * 100.;
		}
		else
		{
			price = totalPVToSettleDate / faceValue * 100.;
		}

		return price;
	}

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
	double FloatingBond::priceFromYield( const AQLDate& settlementDate, const double& yield, const double& assumedRate, const double& indexToNextCoupon, const double& annualizedNextCouponRate ) const
	{
		Bond::checkSettlementDateValid(settlementDate);

		if ( schedule_->getBondCalculationType() != TYPE21_FLOATING_RATE_NOTE )
		{
			AQ_THROW( "Price from Yield calculation is only supported for Bond Calc Type 21.")	
		}

		/*
		 * The price calculation works by first calculating the implied Discount Margin,
		 * and then invoking the priceFromDiscountMargin() method.
		 *
		 * The Discount Margin is calculated using the following relationship from
		 * Stignum & Robinson "Money Market and Bond Calculations", p270:
		 *
		 * discount margin = yield to maturity - reference index level
		 *
		 * Because the floating bond has a different reference index level for the first coupon (rateToNextFix),
		 * vs subsequent projected coupons (assumedRate), we decide the reference index level based on
		 * how many coupons are remaining.
		 */
		const double discountMargin = discountMarginFromYield( settlementDate, yield, assumedRate, indexToNextCoupon );

		/*
		 * Finally, calculate the price from the implied Discount Margin
		 */
		return priceFromDiscountMargin( settlementDate, discountMargin, assumedRate, indexToNextCoupon, annualizedNextCouponRate );

	}

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
	double FloatingBond::yieldFromPrice( const AQLDate& settlementDate, const double& targetPrice, const double& assumedRate, const double& indexToNextCoupon, const double& annualizedNextCouponRate ) const
	{
		Bond::checkSettlementDateValid(settlementDate);
		
		const BondActiveCouponDates activeCouponDates = schedule_->getBondFirstActiveCouponDates( settlementDate, false );
		double accruedInterest = accruedInterestFromAnnualizedCouponRate(  settlementDate, activeCouponDates, annualizedNextCouponRate );


        // Newton-Raphson Solver Settings

		// Initial guess formula: Approximate YTM = annualCouponRate + (parValue - bondPrice )/ yearsToMaturity) / ((parValue + bondPrice)/2)
		// Intuition: Each year, you earn interest PLUS a gain/loss on the bond price, and you earn that amount on the "average" between the initial bond price and the amount you get back on maturity

		const double couponRate = annualizedNextCouponRate;
		const double years = getYearFraction(settlementDate, schedule_->getMaturityDate(), ACT_365_DAYCOUNT);
		const double cleanPrice = isCleanPrice_ ? targetPrice : targetPrice - accruedInterest;

		const double initialGuessForYield = (couponRate + ((100.0 - cleanPrice) * 0.01) / years) / ((100.0 + cleanPrice) * 0.01 * 0.5);
		
		const double tolerance            = 1.0e-10;
        const unsigned int maxIterations  = 1000;
        const double shiftSize            = 0.0000001;

		// One-dimensional objective function used by the solver:
		// This lambda function captures the settlementDate and yieldCalcType as fixed parameters.
		// The inputYield is the variable which the solver will adjust in order to obtain the targetDirtyPrice.
		auto function = [&] ( const double inputYield )
		{
			// Calculating activeCouponDates is expensive. Consider passing through as activeCouponDatesPtr
			// as is done for fixed bonds
			return priceFromYield( settlementDate, inputYield, assumedRate, indexToNextCoupon, annualizedNextCouponRate );
		};

        // Solver Results Contain: Solution, nInterations and Jacobian
		auto result = solvers::newtonRaphson( function, targetPrice, initialGuessForYield, tolerance, maxIterations, shiftSize );
		
		double yield = result.solution;

		return yield;
	}

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
	double FloatingBond::discountMarginFromPrice( const AQLDate& settlementDate, const double& targetPrice, const double& assumedRate, const double& indexToNextCoupon, const double& annualizedNextCouponRate ) const
	{

		// Invoke Yield from Price, and then convert the Yield to Discount Margin
		const double yield = yieldFromPrice( settlementDate, targetPrice, assumedRate, indexToNextCoupon, annualizedNextCouponRate );

		const double discountMargin = discountMarginFromYield( settlementDate, yield, assumedRate, indexToNextCoupon );
		return discountMargin;	
	}

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
	double FloatingBond::discountMarginFromYield( const AQLDate& settlementDate, const double& yield, const double& assumedRate, const double& indexToNextCoupon ) const
	{
		auto cashflows = schedule_->getAllCashflowsExcludingUpfrontNotional();
		const size_t nCoupons = cashflows.size();
		const BondActiveCouponDates activeCouponDates = schedule_->getBondFirstActiveCouponDates( settlementDate, false );
		const size_t remainingCoupons = nCoupons - activeCouponDates.firstActiveCashflowIndex_;

		if ( remainingCoupons == 0 )
		{
			return std::numeric_limits<double>::quiet_NaN();
		}

		/* If more than one coupon is yet to be paid out, calculate the discount margin from the assumedRate.
		 * Otherwise calculate from the indexToNextCoupon.
		 */
		const double discountMargin = remainingCoupons > 1 ? ( yield - assumedRate ) : ( yield - indexToNextCoupon );
		return discountMargin;
	}

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
	double FloatingBond::yieldFromDiscountMargin( const AQLDate& settlementDate, const double& discountMargin, const double& assumedRate, const double& indexToNextCoupon ) const
	{
		auto cashflows = schedule_->getAllCashflowsExcludingUpfrontNotional();
		const size_t nCoupons = cashflows.size();
		const BondActiveCouponDates activeCouponDates = schedule_->getBondFirstActiveCouponDates( settlementDate, false );
		const size_t remainingCoupons = nCoupons - activeCouponDates.firstActiveCashflowIndex_;

		if ( remainingCoupons == 0 )
		{
			return std::numeric_limits<double>::quiet_NaN();
		}

		double yield = remainingCoupons > 1 ? ( discountMargin + assumedRate ) : ( discountMargin + indexToNextCoupon );
		return yield;
	}


	/* @brief Compute a year fraction adjustment factor, to spread the effect of leap years across all coupons.
	 *        BBG Calc Type 21 specifies that the adjustment factor be applied to all ACT/XX daycounts.
	 *
	 * @param[in]	dayCount	The bond schedule dayCount convention
	 * @returns	The calculated leap year adjustment factor
	 */
	double FloatingBond::leapYearAdjustmentFactor( const DayCountEnum& dayCount ) const
	{
		switch ( dayCount )
		{
		case ACT_360_DAYCOUNT:
			return 365.25 / 360.;

		case ACT_365_DAYCOUNT:
		case ACT_ACT_DAYCOUNT:
			return 365.25 / 365.;

		default:
			AQ_THROW( "Unsupported daycount: " + toString( dayCount ));
		}
	}

	/* @brief Determine the accrued interest for a floating bond, in a discount margin calculation.
	 *        This calculation uses the supplied annualizedCouponRate rather than the coupon rate from the bond schedule
	 *
	 * @param[in]	settlementDate				The settlement Date for the calculation
	 * @param[in]	activeCouponDates			A structure containing the prior and next coupon dates, relative to settlement date
	 * @param[in]	annualizedNextCouponRate	The annualized rate to use for the next (first) coupon.
	 *
	 * @returns	The accrued interest
	 */
	double FloatingBond::accruedInterestFromAnnualizedCouponRate( const AQLDate& settlementDate, const BondActiveCouponDates& activeCouponDates,  const double& annualizedNextCouponRate ) const
	{

		const double accruedInterestYearFraction = calculateBondAccruedInterestYearFraction(settlementDate,	activeCouponDates, bondYieldParameters_ );
		const double annualizationFactor = convertBondFrequencyToYearFraction( schedule_->getAccrualFrequency() );
		const double faceValue       = getSchedule()->getNotional();
		const double accruedInterest = accruedInterestYearFraction *  annualizationFactor * annualizedNextCouponRate * faceValue;
		const double accruedInterestWithFloor = std::max(0.0, accruedInterest );

		// For testing only
		const double accruedDays = calculateBondAccruedInterestDays( settlementDate, activeCouponDates, bondYieldParameters_ );

		return accruedInterestWithFloor;
	}

    double FloatingBond::compoundYieldFromQuotedYield( const AQLDate& settlementDate, const double& yield, const YieldCalculationTypeEnum& yieldCalcType ) const
	{
		double compoundYield = 0.;
		return compoundYield;
	}

    //Solve compound yield either 1) based on the cashflow calculation, or 2) use JGB Approximation
    double FloatingBond::compoundYieldFromQuotedPrice( const AQLDate& settlementDate, const double& price, const YieldCalculationTypeEnum& yieldCalcType ) const
	{
		double compoundYield = 0.;
		return compoundYield;
	}


}

