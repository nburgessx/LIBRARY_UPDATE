/*
 * @brief			Class that Defines a Fixed Coupon Bond
 * @Created:		17th January 2017
 * @Author:			Nicholas Burgess
 * @Department:	    Quant Research & Analytics
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#include "FixedBond.h"
#include "FixedBondSchedule.h"
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
   
	FixedBond::FixedBond(const std::string& instanceName, const LabelValueBlock& bondLVB, const LabelValueBlock& scheduleLVB) : Bond( instanceName, bondLVB, scheduleLVB )
	{
        const std::string inputLVB = "bondLVB";
        
        // Generate the schedule if nullptr
        schedule_ = BondSchedulePtr( new FixedBondSchedule( scheduleLVB, instanceName ) );

        // Set the Bond Yield Parameters Struct
        setBondYieldParameters();
	}

	FixedBond::FixedBond(const std::string& instanceName, const LabelValueBlock& bondLVB) : Bond(instanceName, bondLVB)
	{
		const std::string inputLVB = "bondLVB";

		schedule_ = BondSchedulePtr(new FixedBondSchedule(bondLVB, instanceName));

		// Set the Bond Yield Parameters Struct
		setBondYieldParameters();
	}

    void FixedBond::initializeDataProvider( DataProvider& dataProvider, const double& yield, const YieldCalculationTypeEnum& yieldCalcType, const std::shared_ptr<BondActiveCouponDates>& activeCouponDatesPtr) const
    {
        // Check the settlement Date is Valid
        Bond::checkSettlementDateValid(dataProvider.getValuationSettings().getValuationDate());

        Bond::initializeDataProvider( dataProvider, yield, yieldCalcType, activeCouponDatesPtr);
        
        // Add Bond Input Parameters to the BondYield Struct, since cannot pass the abstract bond class by reference
        BondYieldParameters bondYieldParameters = getBondYieldParameters();

        // Populate DFs for cashflows
        schedule_->initializeDataProviderWithYieldData( dataProvider, bondYieldParameters, yield,  yieldCalcType, activeCouponDatesPtr);
     
    }

	/* @brief: Initializes the dataProvider using discount factors calculated using yield points obtained from a BondCurve.
	*
	* @param[out]	dataProvider			The dataProvider to initialize
	* @param[in]	bondCurve				The bond curve
	* @param[in]	activeCouponDatesPtr	Specifies the coupon dates which bracket the settlement date.
	*/
	void FixedBond::initializeDataProviderWithBondCurve( DataProvider& dataProvider, const BondCurve& bondCurve, const std::shared_ptr<BondActiveCouponDates>& activeCouponDatesPtr ) const
	{
		// Check the settlement Date is Valid
        Bond::checkSettlementDateValid( dataProvider.getValuationSettings().getValuationDate() );

		const double dummyYield = 0.0;
		Bond::initializeDataProvider( dataProvider, dummyYield, bondCurve.getYieldCalculationTypeEnum() );

        // Add Bond Input Parameters to the BondYield Struct, since cannot pass the abstract bond class by reference
        BondYieldParameters bondYieldParameters = getBondYieldParameters();

        // Populate DFs for cashflows
        schedule_->initializeDataProviderWithBondCurve( dataProvider, bondYieldParameters, bondCurve, activeCouponDatesPtr );
	}

	/* @brief: Initializes the dataProvider using discount factors from the specified discount curve and curveCollection.
	*
	* @param[out]	dataProvider	The dataProvider to initialize
	* @param[in]	discountCurve	The discount curve name
	*/
	void FixedBond::initializeDataProviderWithCurveData( DataProvider& dataProvider, const LAString& discountCurve ) const
	{
		const double dummyYield = 0.0;
		const std::string dummyYieldCalcType("");
		const YieldCalculationTypeEnum yieldCalcTypeEnum = getYieldCalulationType( dummyYieldCalcType );
		Bond::initializeDataProvider( dataProvider, dummyYield, yieldCalcTypeEnum );
		schedule_->initializeDataProviderWithCurveData( dataProvider, discountCurve );
	}

    BondPtr FixedBond::clone() const
    {
        BondPtr bond = BondPtr( new FixedBond( *this ) );
        return bond;
    }

	FixedBond::FixedBond( const FixedBond& rhs) : Bond( rhs )
	{}

	/* @brief Calculates the bond accrued interest percent. This API takes in a dataProvider object.
	*  @param[in]	dataProvider	Contains the bond settlementDate for the calculation
	*  @returns		The bond accrued interest
	*/
    double FixedBond::accruedInterestPercent( const DataProvider& dataProvider ) const
    {
        const double accruedInterestCashflow    = FixedBond::accruedInterest( dataProvider );
		const double faceValue = std::fabs(getSchedule()->getNotional()); // Notional can be negative, and scaling should not change the sign so we make it absolute
        const double accruedInterestPercent     = accruedInterestCashflow / faceValue * 100.0;

        return accruedInterestPercent;
    }

    //Helper function to avoid duplication
    double FixedBond::getYearFractionFromSettleToMaturityDtForSimpleYieldCalType(const LADate& settlementDate) const
    {
        auto maturityDate = schedule_->getMaturityDate();

        int years = settlementDate.intervalYears(maturityDate) ;

        // When settle to maturity is greater or equal to one calendar year, the dayCount is NL/365; otherwise it is Act/365
        auto dayCount = (years >= 1) ? schedule_->getAccrualDaycount() : ACT_365_DAYCOUNT;

        double tao = getYearFraction(settlementDate, maturityDate, dayCount);  

        return tao;

    }

    double FixedBond::getCleanPriceFromSimpleYield(const LADate& settlementDate, double simpleYield) const
    {
        //The input yield is the SIMPLE yield
        // Simple yield and clean price relation formula: simpleYieldInPercent = [couponRateInPercent + (100 - cleanPrice)/tao]/cleanPrice*100, where tao is the number of days from settle to maturity divided by 365
        // So clean price can be cal from Simple yield by" cleanPrice = (couponRateInPercent * tao + 100)/(simpleYieldInPercent * tao/100 + 1)  

        double tao = getYearFractionFromSettleToMaturityDtForSimpleYieldCalType(settlementDate);  
            
        //For instance if couponRate is 0.1%, the couponRateInPercent is 0.1
        double couponRateInPercent = schedule_->getFixedRate() * 100; 
        
        auto yieldInPercent = simpleYield * 100;
        
        double cleanPrice= (couponRateInPercent * tao + 100) / (yieldInPercent * tao / 100.0 + 1);

        return cleanPrice;
    }

    double FixedBond::getSimpleYieldFromCleanPrice(const LADate& settlementDate, double cleanPrice) const
    {
        // Simple yield and clean price relation formula: simpleYieldInPercent = [couponRateInPercent + (100 - cleanPrice)/tao]/cleanPrice*100, where tao is the number of days from settle to maturity divided by 365

        double tao = getYearFractionFromSettleToMaturityDtForSimpleYieldCalType(settlementDate);  

        //For instance if couponRate is 0.1%, the couponRateInPercent is 0.1
        double couponRateInPercent = schedule_->getFixedRate() * 100.0; 

        auto simpleYieldInPercent = (couponRateInPercent + (100.0 - cleanPrice) / tao) / cleanPrice * 100;

        double yield = simpleYieldInPercent * 0.01;

        return yield;
    }

    double FixedBond::yield( const LADate& settlementDate, const double& price, const YieldCalculationTypeEnum& yieldCalcType) const
    {
        double yield = 0.0;

        if (isCleanPrice_)
        {
            if (yieldCalcType == SIMPLE_YIELD)
		    {
                yield = getSimpleYieldFromCleanPrice(settlementDate, price);
            }
            else
            {
                yield = compoundYield(settlementDate, price, yieldCalcType);
            }
        }
        else
        {
            if (yieldCalcType == SIMPLE_YIELD)
		    {
                double cleanPrice = priceFromDirtyToClean(price, settlementDate);
                yield = getSimpleYieldFromCleanPrice(settlementDate, cleanPrice);
            }
            else
            {
                yield = compoundYield(settlementDate, price, yieldCalcType);
            }
         }

        // convert yield from coupon frequency to yield frequency, if they are different  
        auto couponFreq = schedule_->getAccrualFrequency();
        auto yieldFreq = schedule_->getYieldFrequency();

        yield = convertYield( yield, couponFreq, yieldFreq);


        return yield;
    }
	
	/* @brief	Calculates the bond yield. This API takes in a dataProvider object
	*  @param[in]	dataProvider	Contains the bond settlement date
	*  @param[in]	price			The quoted bond price
	*  @param[in]	yieldCalcType	Specifies the yield calculation type required
	*  @returns		The bond yield
	*/
	double FixedBond::yield( const DataProvider& dataProvider, const double price, const YieldCalculationTypeEnum& yieldCalcType ) const
	{
		// Check the settlement Date is Valid
		LADate settlementDate = dataProvider.getValuationSettings().getSettlementDate();
        Bond::checkSettlementDateValid( settlementDate );

		const double impliedYield = yield( settlementDate, price, yieldCalcType );
		return impliedYield;
	}

    //The price can be either dirtyPrice or cleanPrice
    double FixedBond::compoundYield( const LADate& settlementDate, const double& price, const YieldCalculationTypeEnum& yieldCalcType) const
    {
		// Check the settlement date is valid
		Bond::checkSettlementDateValid(settlementDate);
		
		//get the dirty price from clean price
        double targetDirtyPrice = price;
        if (isCleanPrice_)
        {
            targetDirtyPrice = priceFromCleanToDirty(price, settlementDate);
        }

        // Newton-Raphson Solver Settings

		// Initial guess formula: Approximate YTM = annualCouponRate + (parValue - bondPrice )/ yearsToMaturity) / ((parValue + bondPrice)/2)
		// Intuition: Each year, you earn interest PLUS a gain/loss on the bond price, and you earn that amount on the "average" between the initial bond price and the amount you get back on maturity

		const double couponRate = schedule_->getFixedRate();
		const double years = getYearFraction(settlementDate, schedule_->getMaturityDate(), ACT_365_DAYCOUNT);
		const double cleanPrice = isCleanPrice_ ? price : priceFromDirtyToClean(price, settlementDate);

		const double initialGuessForYield = (couponRate + ((100.0 - cleanPrice) * 0.01) / years) / ((100.0 + cleanPrice) * 0.01 * 0.5);

		const double tolerance            = 0.0000000001;
        const unsigned int maxIterations  = 1000;
        const double shiftSize            = 0.0000001;

		auto activeCouponDates = schedule_->getBondFirstActiveCouponDates(settlementDate, true);

		auto activeCouponDatesPtr = std::make_shared<BondActiveCouponDates>(activeCouponDates);

		// One-dimensional objective function used by the solver:
		// This lambda function captures the settlementDate and yieldCalcType as fixed parameters.
		// The inputYield is the variable which the solver will adjust in order to obtain the targetDirtyPrice.
		auto function = [settlementDate, yieldCalcType, activeCouponDatesPtr, this] ( const double inputYield )
		{
			return dirtyPrice( settlementDate, inputYield, yieldCalcType, true, activeCouponDatesPtr);
		};

        // Solver Results Contain: Solution, nInterations and Jacobian
		auto result = solvers::newtonRaphson( function, targetDirtyPrice, initialGuessForYield, tolerance, maxIterations, shiftSize );
		
		double yield = result.solution;

		return yield;
    }

    double FixedBond::cleanPriceJGBApproximation(const LADate& settlementDate, const double& inputYield) const
    {
        // ***JGB Approximation ComppoundYield formula: cleanPrice = coupon*100/y * (1- factor) + 100 * factor, 
        // where factor=(1+y/200))^(-2T), and T is the time from settle to maturity using JGB day count 
        // This formula is from BB discussion, which is based on http://market.jsda.or.jp/shiraberu/saiken/wi/files/wi_guideline0310.pdf 
 
        double tao = getYearFractionFromSettleToMaturityDtForSimpleYieldCalType(settlementDate);  

        double inputYieldInPercent = inputYield * 100.0;

        double coupongInPercent = schedule_->getFixedRate() * 100.0;

        double factor = std::pow(( 1.0 + inputYieldInPercent/200.0 ), (-2.0 * tao));     

        double cleanPrice = coupongInPercent * 100.0/ inputYieldInPercent * (1.0 - factor) + 100 * factor;

        return cleanPrice;
    }

    double FixedBond::compoundYieldJGBApproximation( const LADate& settlementDate, const double& price) const
    {
        //get the dirty price from clean price
        double targetCleanPrice = price;
        if (!isCleanPrice_)
        {
            targetCleanPrice = priceFromDirtyToClean(price, settlementDate);
        }
        // Check the settlement date is valid
        Bond::checkSettlementDateValid( settlementDate );

        // Newton-Raphson Solver Settings
        const double initialGuessForYield = getSimpleYieldFromCleanPrice(settlementDate, targetCleanPrice);
        const double tolerance            = 0.0000000001;
        const unsigned int maxIterations  = 1000;
        const double shiftSize            = 0.0000001;

		// One-dimensional objective function used by the solver:
		// This lambda function captures the settlementDate and yieldCalcType as fixed parameters.
		// The inputYield is the variable which the solver will adjust in order to obtain the targetCleanPrice.
		auto function = [settlementDate, this] ( const double inputYield )
		{
            return cleanPriceJGBApproximation(settlementDate, inputYield);
		};

        // Solver Results Contain: Solution, nInterations and Jacobian
		double yield = solvers::newtonRaphson( function, targetCleanPrice, initialGuessForYield, tolerance, maxIterations, shiftSize ).solution;
		return yield;
    }

    double FixedBond::compoundYieldFromQuotedYield( const LADate& settlementDate, const double& yield, const YieldCalculationTypeEnum& yieldCalcType ) const
    {

        double compoundYd = 0.0;

        if(yieldCalcType == SIMPLE_YIELD)
		{
            double bondPrice = price(settlementDate, yield, yieldCalcType);
            double compoundYd = compoundYieldFromQuotedPrice(settlementDate, bondPrice, yieldCalcType );
        }
        else
        {
           compoundYd = convertYieldFromYieldFreqToCouponFreq(yield);
        }

        return compoundYd;
    }

    double FixedBond::compoundYieldFromQuotedPrice( const LADate& settlementDate, const double& price, const YieldCalculationTypeEnum& yieldCalcType ) const
    {
        double compoundYd = 0.0;

        if(yieldCalcType == SIMPLE_YIELD)
		{
            if (isJapaneseGovenmentBond(schedule_->getBondCalculationType()))
            {
                //Solve compoundYield using JGB Approximation fomula
                compoundYd = compoundYieldJGBApproximation(settlementDate, price);
            }
            else
            {
                //Solve compoundYield from price
                compoundYd = compoundYield( settlementDate, price, yieldCalcType);
            }
        }
        else
        {
           double inputYield = yield(settlementDate, price, yieldCalcType);
           compoundYd = convertYieldFromYieldFreqToCouponFreq(inputYield);
        }

        return compoundYd;
    }

    double FixedBond::dirtyPrice( const LADate& settlementDate, const double& yield, const YieldCalculationTypeEnum& yieldCalcType, bool isCompoundYield, const std::shared_ptr<BondActiveCouponDates>& activeCouponDatesPtr) const
    {
        double compoundYield = isCompoundYield ? yield : compoundYieldFromQuotedYield(settlementDate, yield, yieldCalcType);

        // Update Cashflows checks for a valid settlement date i.e. not before bond start and not after bond maturity
        // No need to repeat that check here i.e. don't need to check for a valid settlement date here
		DataProvider dataProvider(settlementDate);
        initializeDataProvider( dataProvider, compoundYield, yieldCalcType, activeCouponDatesPtr);

        // Get all the bond cashflows excluding the upfrontCashflow
        auto cashflows = schedule_->getAllCashflowsExcludingUpfrontNotional();
		MLIB_REQUIRE( ! cashflows.empty(), "Unable to calculate the dirty price. The bond has no cashflows." );

        // Get the Bond PV
        double pv = 0;
		const size_t cashflowSize = cashflows.size();

 		for( size_t i = 0; i < cashflowSize; i++ )
		{			
			auto cf = cashflows[i];
			pv += cf->getCouponPv( dataProvider.getCashflowDataExcludingUpfront( i ) );
		}

        // Convert the Bond PV into the Dirty Bond Price in Percent, note that the sign of the notional and the pv will cancel out
        const double faceValue            = getSchedule()->getNotional();

		MLIB_REQUIRE(!MLIB_IS_EQUAL_ZERO(faceValue), "Bond notional cannot be zero.");

        double dirtyPrice                 = pv / faceValue * 100.0;

		// Handle price rounding, TRUE isCompundYield means this function is used to calculate yield from price, so no rounding
		// *** Note that we do not round the coupon or couponPV in cashflows, only the bond price
		if (!isCompoundYield)
		{
			if (bondYieldParameters_.calculationType_ == TYPE23_AUSTRALIAN_GOVERNMENT_BONDS)
			{
				bool isSettleDateInLastCashflow = (settlementDate >= cashflows[cashflowSize - 1]->getAccrualStartDate());
				
                //If it is NOT near maturing bond, rounded to 3 decimals
				if (!isSettleDateInLastCashflow && bondQuoteConvention_ == QUOTE_TO_3_DECIMAL_PLACES)
				{
                    dirtyPrice = formatBondPrice( dirtyPrice, QUOTE_TO_3_DECIMAL_PLACES );
				}
			}
		}

        return dirtyPrice;
    }
 

    double FixedBond::cleanPrice( const LADate& settlementDate, const double& yield, const YieldCalculationTypeEnum& yieldCalcType ) const
    {
        
        checkSettlementDateValid(settlementDate );

        double cPrice  = 0.0; 
        if ( yieldCalcType == SIMPLE_YIELD )
		{
            double yieldToUse = convertYieldFromYieldFreqToCouponFreq(yield);
            cPrice= getCleanPriceFromSimpleYield(settlementDate, yieldToUse); 
        }
        else
        {
            const double dirtyPrice  = FixedBond::dirtyPrice( settlementDate, yield, yieldCalcType);
            cPrice  = priceFromDirtyToClean(dirtyPrice, settlementDate);
        }

        return cPrice;
    }

	/* @brief Calculates the bond accrued interest. This API takes in a dataProvider object.
	*  @param[in]	dataProvider	Contains the bond settlementDate for the calculation
	*  @returns		The bond accrued interest percent
	*/
    double FixedBond::accruedInterest( const DataProvider& dataProvider ) const
    {
        // Check the settlement Date is Valid
		LADate settlementDate = dataProvider.getValuationSettings().getSettlementDate();
        Bond::checkSettlementDateValid( settlementDate );

        double accruedInterest = schedule_->calculateBondAccruedInterest(settlementDate, bondYieldParameters_);

        return accruedInterest;
    }
    
    unsigned int FixedBond::accruedInterestDays( const LADate& settlementDate ) const
    {
        const unsigned int accruedInterestDays = schedule_->calculateBondAccruedInterestDays(settlementDate, bondYieldParameters_);
        return accruedInterestDays;
    }

    double FixedBond::annuity( const double& yield, const YieldCalculationTypeEnum& yieldCalcType ) const
    {
        return 0.0;
    }

	double FixedBond::annuityWithNotional( const double& yield, const YieldCalculationTypeEnum& yieldCalcType ) const
    {
        return 0.0;
    }

    double FixedBond::pv01( const double& yield, const YieldCalculationTypeEnum& yieldCalcType ) const
    {
        return 0.0;
    }
    
    ScheduleTypeEnum FixedBond::getScheduleType() const
    {
        return FIXED_SCHEDULE_TYPE;
    }

	double FixedBond::dv01( const LADate& settlementDate, const double& yield, const YieldCalculationTypeEnum& yieldCalcType, bool isCompoundYield ) const
	{

		double compoundYield = isCompoundYield ? yield : compoundYieldFromQuotedYield(settlementDate, yield, yieldCalcType);

        // Update Cashflows checks for a valid settlement date i.e. not before bond start and not after bond maturity
        // No need to repeat that check here i.e. don't need to check for a valid settlement date here
		DataProvider dataProvider(settlementDate);
        initializeDataProvider( dataProvider, compoundYield, yieldCalcType );

        // Get all the bond cashflows excluding the upfrontCashflow
        auto cashflows = schedule_->getAllCashflowsExcludingUpfrontNotional();
		MLIB_REQUIRE( ! cashflows.empty(), "Unable to calculate the dv01. The bond has no cashflows." );
		
		auto activeCouponDates = schedule_->getBondFirstActiveCouponDates(settlementDate, true);
		LADate firstActivePaymentDate = activeCouponDates.firstActiveCouponDate_;

		DayCountEnum dayCount                = schedule_->getAccrualDaycount();
		FrequencyEnum accrualFrequency         = schedule_->getAccrualFrequency();
		const double accruedAdjustmentFactor = calculateBondActiveCouponFutureAccrualRatio( settlementDate, activeCouponDates, dayCount, accrualFrequency, yieldCalcType );

		auto bondPaymentDates        = schedule_->getPaymentDates();
		auto firstActivePaymentIndex = getBondCashflowIndex( firstActivePaymentDate, bondPaymentDates );

		BondYieldParameters bondYieldParameters = getBondYieldParameters();
		const double simpleYearFraction         = convertBondFrequencyToYearFraction(  bondYieldParameters.couponFrequency_ );

        double dv01 = 0;
		for( size_t i = 0; i < cashflows.size(); i++ )
		{			
			auto cf = cashflows[i];
			CashflowData cashflowData = dataProvider.getCashflowDataExcludingUpfront( i );

			LADate paymentDate = cf->getPaymentDate();
			if ( paymentDate > settlementDate )
			{
				unsigned int nthCoupon = getBondRelativeCashflowIndex( firstActivePaymentIndex, paymentDate, bondPaymentDates );
				double ti = simpleYearFraction * ( accruedAdjustmentFactor + nthCoupon );

				dv01 += ti * cf->getCouponPv( cashflowData );
			}
		}
		dv01 /= ( 1 + compoundYield * simpleYearFraction );

		// Scale to a 1bp move
		dv01 *= 0.0001;
		return dv01;
	}

	double FixedBond::modifiedDuration( const LADate& settlementDate, const double& yield, const YieldCalculationTypeEnum& yieldCalcType , bool isCompoundYield ) const
	{
		double price            = dirtyPrice( settlementDate, yield, yieldCalcType );
		double dv01sensitivity  = dv01( settlementDate, yield, yieldCalcType );
		double modifiedDuration = dv01sensitivity / price;

		return modifiedDuration;
	}

	/* @brief Calculates the dirty price of a bond using a BondCurve to discount the coupons.
	* @param[in]	settlementDate	The bond settlement date
	* @param[in]	bondCurve		A calibrated BondCurve
	* @returns		The bond dirty price.
	*/
	double FixedBond::dirtyPriceFromBondCurve( const LADate& settlementDate, const BondCurve& bondCurve ) const
	{
		// Update Cashflows checks for a valid settlement date i.e. not before bond start and not after bond maturity
        // No need to repeat that check here i.e. don't need to check for a valid settlement date here
		DataProvider dataProvider(settlementDate);
		initializeDataProviderWithBondCurve( dataProvider, bondCurve );


		// Get all the bond cashflows excluding the upfrontCashflow
        auto cashflows = schedule_->getAllCashflowsExcludingUpfrontNotional();
		MLIB_REQUIRE( ! cashflows.empty(), "Unable to calculate the dirty price from bond curve. The bond has no cashflows." );

		const double faceValue = getSchedule()->getNotional();
		MLIB_REQUIRE(!MLIB_IS_EQUAL_ZERO(faceValue), "Bond notional cannot be zero.");
		
		double pv = 0;

 		for( size_t i = 0; i < cashflows.size(); i++ )
		{			
			auto cf = cashflows[i];
			const LADate paymentDate = cf->getPaymentDate();
			if ( paymentDate > settlementDate )
			{
				const CashflowData& cashflowData = dataProvider.getCashflowDataExcludingUpfront( i );
				const double couponPv = cf->getCouponPv( cashflowData );
				//const double couponPv = 1.0;

				pv += couponPv;
			}
		}

		// Convert the Bond PV into the Dirty Bond Price in Percent, note that the sign of the notional and the pv will cancel out
        double dirtyPrice = pv / faceValue * 100.0;
		return dirtyPrice;
	}
	
	/* @brief Calculates the clean price of a bond using a BondCurve to discount the coupons.
	* @param[in]	settlementDate	The bond settlement date
	* @param[in]	bondCurve		A calibrated BondCurve
	* @returns		The bond clean price.
	*/
	double FixedBond::cleanPriceFromBondCurve( const LADate& settlementDate, const BondCurve& bondCurve ) const
	{
        const double dirtyPrice  = FixedBond::dirtyPriceFromBondCurve( settlementDate, bondCurve );
        const double cleanPrice  = priceFromDirtyToClean( dirtyPrice, settlementDate );
        return cleanPrice;
	}

	/* @brief Calculates the discount factor for the final cashflow of a bond using a BondCurve to discount the coupons.
	* @param[in]	settlementDate	The bond settlement date
	* @param[in]	bondCurve		A calibrated BondCurve
	* @returns		The discount factor at maturity.
	*/
	double FixedBond::discountFactorAtMaturityFromBondCurve( const LADate& settlementDate, const BondCurve& bondCurve ) const
	{
		DataProvider dataProvider(settlementDate);
		initializeDataProviderWithBondCurve( dataProvider, bondCurve );

		// Get all the bond cashflows excluding the upfrontCashflow
        auto cashflows = schedule_->getAllCashflowsExcludingUpfrontNotional();
		size_t nCashflows = cashflows.size();

		// Get the data associated with the final cashflow
		const CashflowData& cashflowData = dataProvider.getCashflowDataExcludingUpfront( nCashflows-1 );
		const double discountFactor = cashflowData.discountFactor;
		return discountFactor;
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
	double FixedBond::yieldFromPriceAndBondCurve( const LADate& settlementDate, const double& price, BondCurve& bondCurve ) const
	{
		Bond::checkSettlementDateValid( settlementDate );
		const LADate& maturityDate = getSchedule()->getMaturityDate();
		
		double yield = std::numeric_limits<double>::quiet_NaN();

		// Initial guess formula: Approximate YTM = annualCouponRate + (parValue - bondPrice )/ yearsToMaturity) / ((parValue + bondPrice)/2)
		// Intuition: Each year, you earn interest PLUS a gain/loss on the bond price, and you earn that amount on the "average" between the initial bond price and the amount you get back on maturity

		const double couponRate = schedule_->getFixedRate();
		const double years = getYearFraction(settlementDate, schedule_->getMaturityDate(), ACT_365_DAYCOUNT);
		const double cleanPrice = isCleanPrice_ ? price : priceFromDirtyToClean(price, settlementDate);

		const double initialGuessForYield = (couponRate + ((100.0 - cleanPrice) * 0.01) / years) / ((100.0 + cleanPrice) * 0.01 * 0.5);

		// Use solver approach
		// Newton-Raphson Solver Settings
		const double tolerance					= 1.0e-10;
		const unsigned int maxIterations		= 1000;
		const double shiftSize					= 0.0000001;
		const double targetPrice				= price;

		// One-dimensional objective function used by the solver:
		// This lambda function captures the valuationSettings and recoveryRate as fixed parameters.
		// The inputHazardRate is the variable which the solver will adjust in order to obtain the targeCdsSpread.
		auto function = [&, this] ( const double yieldEstimate ) -> double
		{
			bondCurve.setCalibrationPoint( maturityDate, yieldEstimate );
			const double price = priceFromBondCurve( settlementDate, bondCurve );
			return price;
		};

		// Solver Results Contain: Solution, nInterations and Jacobian
		yield = solvers::newtonRaphson( function, targetPrice, initialGuessForYield, tolerance, maxIterations, shiftSize ).solution;		
		
		// Set the discountFactor at this calibrated point into the bond curve.
		const double discFactAtMaturity = discountFactorAtMaturityFromBondCurve( settlementDate, bondCurve );
		bondCurve.setDiscountFactorAtCalibrationPoint( maturityDate, discFactAtMaturity );
			
		return yield;
	}

	/* @brief	Calculates the bond yield-to-maturity from a bund curve
	*  @param[in]	settlementDate	The bond settlement date
	*  @param[in]	bondCurve		A bondCurve used for discounting coupons
	*  @returns		The bond yield to maturity
	*/
	double FixedBond::yieldFromBondCurve( const LADate& settlementDate, const BondCurve& bondCurve ) const
	{
		/* The function works as follows:
		   a) calculate the bond price from the bond curve
		   b) calculate the implied yield to maturity using the bond price from step a).
		*/
		const double price = priceFromBondCurve( settlementDate, bondCurve );
		const double yieldFromBondCurve = yield( settlementDate, price, bondCurve.getYieldCalculationTypeEnum() );

		return yieldFromBondCurve;
	}

	/* @brief: Calculates the accrued interest of a defaultable bond using a CreditModel to compute survival probabilities.
	*  @param[in]	settlementDate	The bond settlement date
	*  @param[in]	creditModel		The calibrated credit model
	*  @returns		The bond accrued interest, scaled by the probability of survival to the next coupon.
	*/
	double FixedBond::accruedInterestPercentFromCreditModel( const LADate& settlementDate, const CreditModel& creditModel ) const
	{
		ValuationSettings valuationSettings;
		valuationSettings.setSettlementDate( settlementDate );
		DataProvider dataProvider( valuationSettings );

		const double accruedInterest = accruedInterestPercent( dataProvider );

		double riskyAccruedInterest = accruedInterest;

		if ( creditModel.bondHasRiskyAccruedInterest() )
		{
			const bool isPriorDtWithFullCouponPeriod = false;
			const BondActiveCouponDates activeCouponDates = schedule_->getBondFirstActiveCouponDates( settlementDate, isPriorDtWithFullCouponPeriod );
			const LADate& nextCouponDate = activeCouponDates.firstActiveCouponDate_;

			const double survivalProbability = creditModel.getSurvivalProbability( nextCouponDate );
			riskyAccruedInterest = accruedInterest * survivalProbability;
		}
		
		return riskyAccruedInterest;
	}

	
	/* @brief: Calculates the dirty price of a defaultable bond using a CreditModel to compute survival probabilities
	* @param[in]	settlementDate	The bond settlement date
	* @param[in]	creditModel		The calibrated credit model
	* @returns		The bond dirty price.
	*/
	double FixedBond::dirtyPriceFromCreditModel( const LADate& settlementDate, const CreditModel& creditModel ) const
	{
		Bond::checkSettlementDateValid( settlementDate );
		const std::string curveCollection = creditModel.getBondCurveCollection();
		const ValuationSettings valuationSettings( settlementDate, curveCollection );
		DataProvider dataProvider( valuationSettings );
		const LAString& discountCurve = creditModel.getBondDiscountCurve().c_str();
		initializeDataProviderWithCurveData( dataProvider, discountCurve );

		// Get all the bond cashflows excluding the upfrontCashflow
        auto cashflows = schedule_->getAllCashflowsExcludingUpfrontNotional();
		MLIB_REQUIRE( ! cashflows.empty(), "Unable to calculate the dirty price from credit model. The bond has no cashflows." );

		const double faceValue = getSchedule()->getNotional();
		const double recoveryRate = creditModel.getRecoveryRate();
		const double recoveryAmount = recoveryRate * faceValue;

		double pv = 0;
		double prevSurvivalProbability = 1.0;
 		for( size_t i = 0; i < cashflows.size(); i++ )
		{			
			auto cf = cashflows[i];
			const LADate paymentDate = cf->getPaymentDate();
			if ( paymentDate > settlementDate )
			{
				const double survivalProbability = creditModel.getSurvivalProbability( paymentDate );

				const CashflowData& cashflowData = dataProvider.getCashflowDataExcludingUpfront( i );
				const double couponPv = cf->getCouponPv( cashflowData );
				const double riskyCouponPv = couponPv * survivalProbability;

				const double marginalDefaultProbability = prevSurvivalProbability - survivalProbability;
				const double recoveryPv = recoveryAmount * marginalDefaultProbability * cashflowData.discountFactor * survivalProbability;
				prevSurvivalProbability = survivalProbability;

				pv += ( riskyCouponPv + recoveryPv );
			}
		}

		// Convert the Bond PV into the Dirty Bond Price in Percent, note that the sign of the notional and the pv will cancel out
		MLIB_REQUIRE(!MLIB_IS_EQUAL_ZERO(faceValue), "Bond notional cannot be zero.");

        double dirtyPrice = pv / faceValue * 100.0;
		return dirtyPrice;
	}

	/* @brief: Calculates the clean price of a defaultable bond using a CreditModel to compute survival probabilities
	* @param[in]	settlementDate	The bond settlement date
	* @param[in]	creditModel		The calibrated credit model
	* @returns		The bond clean price.
	*/
	double FixedBond::cleanPriceFromCreditModel( const LADate& settlementDate, const CreditModel& creditModel ) const
    {
        const double dirtyPrice  = FixedBond::dirtyPriceFromCreditModel( settlementDate, creditModel );
        const double cleanPrice  = priceFromDirtyToClean( dirtyPrice, settlementDate );
        return cleanPrice;
    }

	/* @brief: Solves for the hazard rate implied by the price of a defaultable bond.
	*		   Note: This modifies the hazard rate in the credit model. Used in CreditModel calibration.
	*
	* @param[in]	settlementDate			The bond settlement date
	* @param[in]	price					The bond price quote. Either clean or dirty, depending on the convention used by the bond
	* @param[inout]	creditModel				The calibrated credit model
	* @param[in]	useHullApproximation	Whether to use simple approximation: HazardRate = ( Yield - RiskFreeRate ) / (1-RecoveryRate), to to use a solver
	* @returns		The implied hazard rate
	*/
	double FixedBond::hazardRateFromPrice( const LADate& settlementDate, const double price, CreditModel& creditModel, const bool useHullApproximation ) const
	{
		Bond::checkSettlementDateValid( settlementDate );
		const LADate& maturityDate = getSchedule()->getMaturityDate();
		
		double hazardRate = std::numeric_limits<double>::quiet_NaN();

		if ( useHullApproximation )
		{
			Bond::checkSettlementDateValid( settlementDate );
			const std::string& curveCollection = creditModel.getBondCurveCollection();
			ValuationSettings valuationSettings( settlementDate, curveCollection );
			DataProvider dataProvider( valuationSettings );
			const LAString& discountCurve   = creditModel.getBondDiscountCurve().c_str();
			initializeDataProviderWithCurveData( dataProvider, discountCurve );

			auto cashflows = schedule_->getAllCashflowsExcludingUpfrontNotional();
			const size_t nCashflows = cashflows.size();
			const CashflowData& cashflowDataAtMaturity = dataProvider.getCashflowDataExcludingUpfront( nCashflows - 1 );
			const double discountFactorAtMaturity = cashflowDataAtMaturity.discountFactor;

			const LADate maturityDate = getSchedule()->getMaturityDate();
			const double yearFraction = getYearFraction( settlementDate, maturityDate, getSchedule()->getAccrualDaycount() );
			const double discountRate = -1.0 * log( discountFactorAtMaturity ) / yearFraction;

			std::string yieldCalcType("");
			YieldCalculationTypeEnum yieldCalcTypeEnum = toYieldCalculationTypeEnum( yieldCalcType );
			const double bondYield = yield( settlementDate, price, yieldCalcTypeEnum );

			const double recoveryRate = creditModel.getRecoveryRate();
			hazardRate = ( bondYield - discountRate ) / ( 1 - recoveryRate );
			creditModel.setCalibrationPoint( maturityDate, hazardRate );		
		}
		else
		{
			// User solver approach
			// Newton-Raphson Solver Settings
			const double initialGuessForHazardRate	= 0.;
			const double tolerance					= 1.0e-10;
			const unsigned int maxIterations		= 1000;
			const double shiftSize					= 0.0000001;
			const double targetPrice				= price;

			// One-dimensional objective function used by the solver:
			// This lambda function captures the valuationSettings and recoveryRate as fixed parameters.
			// The inputHazardRate is the variable which the solver will adjust in order to obtain the targeCdsSpread.
			auto function = [&, this] ( const double inputHazardRate ) -> double
			{
				creditModel.setCalibrationPoint( maturityDate, inputHazardRate );
				const double price = priceFromCreditModel( settlementDate, creditModel );
				return price;
			};

			// Solver Results Contain: Solution, nInterations and Jacobian
			hazardRate = solvers::newtonRaphson( function, targetPrice, initialGuessForHazardRate, tolerance, maxIterations, shiftSize ).solution;		
		}

		return hazardRate;
	}

	/* @brief	Credit Risky forward dirty price calculated from a credit model and underlying discount curve
	*  @param[in]	forwardSettlementDate	The forward date on which to calculate the bond dirty price.
	*  @param[in]	creditModel				The credit model used to obtain survival probabilities and discount curve.
	*  @returns		The bond dirty price on the forward settle date
	*/
	double FixedBond::forwardDirtyPriceFromCreditModel( const LADate& forwardSettleDate, const CreditModel& creditModel ) const
	{
		Bond::checkSettlementDateValid( forwardSettleDate );
		const std::string curveCollection = creditModel.getBondCurveCollection();

		// Calculate forward discount factors as of forwardSettleDate
		const ValuationSettings valuationSettings( forwardSettleDate, curveCollection );
		DataProvider dataProvider( valuationSettings );
		const LAString& discountCurve = creditModel.getBondDiscountCurve().c_str();
		initializeDataProviderWithCurveData( dataProvider, discountCurve );

		// Get all the bond cashflows excluding the upfrontCashflow
        auto cashflows = schedule_->getAllCashflowsExcludingUpfrontNotional();
		MLIB_REQUIRE( ! cashflows.empty(), "Unable to calculate the forward dirty price. The bond has no cashflows." );

		const double faceValue = getSchedule()->getNotional();
		const double recoveryRate = creditModel.getRecoveryRate();
		const double recoveryAmount = recoveryRate * faceValue;

		double pv = 0;
		double prevSurvivalProbability = 1.0;
 		for( size_t i = 0; i < cashflows.size(); i++ )
		{			
			auto cf = cashflows[i];
			const LADate paymentDate = cf->getPaymentDate();
			if ( paymentDate > forwardSettleDate )
			{
				// This is the survival probability calculated from the creditModel asOfDate to paymentDate
				const double survivalProbability = creditModel.getSurvivalProbability( paymentDate ); 

				const CashflowData& cashflowData = dataProvider.getCashflowDataExcludingUpfront( i );
				const double couponPv = cf->getCouponPv( cashflowData );
				const double riskyCouponPv = couponPv * survivalProbability;

				const double marginalDefaultProbability = prevSurvivalProbability - survivalProbability;
				const double recoveryPv = recoveryAmount * marginalDefaultProbability * cashflowData.discountFactor * survivalProbability;
				prevSurvivalProbability = survivalProbability;

				pv += ( riskyCouponPv + recoveryPv );
			}
		}

		// Adjust pv by survival probability to forwardSettleDate
		const double survivalProbabilityToForwardSettleDate = creditModel.getSurvivalProbability( forwardSettleDate ); 
		pv /= survivalProbabilityToForwardSettleDate;

		// Convert the Bond PV into the Dirty Bond Price in Percent, note that the sign of the notional and the pv will cancel out
		MLIB_REQUIRE(!MLIB_IS_EQUAL_ZERO(faceValue), "Bond notional cannot be zero.");

        const double fwdDirtyPrice = pv / faceValue * 100.0;
	
		return fwdDirtyPrice;
	}

	/* @brief	Risk-free forward dirty price calculated from a discount curve
	*  @param[in]	forwardSettlementDate	The forward date on which to calculate the bond dirty price.
	*  @param[in]	curveCollection			The curve collection containing the discount curve
	*  @param[in]	discountCurve			The curve used to discount future coupons
	*  @returns		The bond dirty price on the forward settle date
	*/
	double FixedBond::forwardDirtyPriceFromDiscountCurve( const LADate& forwardSettleDate, const std::string& curveCollection, const std::string& discountCurve ) const
	{
		Bond::checkSettlementDateValid( forwardSettleDate );

		// Calculate forward discount factors as of forwardSettleDate
		const ValuationSettings valuationSettings( forwardSettleDate, curveCollection );
		DataProvider dataProvider( valuationSettings );
		initializeDataProviderWithCurveData( dataProvider, discountCurve.c_str() );

		// Get all the bond cashflows excluding the upfrontCashflow
        auto cashflows = schedule_->getAllCashflowsExcludingUpfrontNotional();
		MLIB_REQUIRE( ! cashflows.empty(), "Unable to calculate the forward dirty price. The bond has no cashflows." );

		double pv = 0;
 		for( size_t i = 0; i < cashflows.size(); i++ )
		{			
			auto cf = cashflows[i];
			const LADate paymentDate = cf->getPaymentDate();
			if ( paymentDate > forwardSettleDate )
			{
				const CashflowData& cashflowData = dataProvider.getCashflowDataExcludingUpfront( i );

				const double couponPv = cf->getCouponPv( cashflowData );
				pv += couponPv;
			}
		}

		// Convert the Bond PV into the Dirty Bond Price in Percent, note that the sign of the notional and the pv will cancel out
		const double faceValue = getSchedule()->getNotional();
		MLIB_REQUIRE(!MLIB_IS_EQUAL_ZERO(faceValue), "Bond notional cannot be zero.");

        const double fwdDirtyPrice = pv / faceValue * 100.0;
	
		return fwdDirtyPrice;	
	}
}

