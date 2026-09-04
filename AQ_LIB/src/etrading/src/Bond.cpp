#include "Bond.h"
#include "SwapValidation.h"
#include "SwapUtilities.h"
#include <iomanip>
#include "StaticStructureStore.h"
#include "Variant.h"
#include "AQLString.h"
#include <boost/format.hpp>
#include <FixedBondCashflow.h>


namespace etrading
{
    Bond::Bond( const std::string& bondObjectName, const LabelValueBlock& bondLVB, const LabelValueBlock& scheduleLVB )
        : IsAQObject( bondObjectName, BOND ), schedule_(), bondParameters_( bondLVB ), scheduleParameters_( scheduleLVB ), bondObjectName_( bondObjectName ), bondYieldParameters_()
	{
		
		initializeBondDescriptionLVB(bondLVB);
	}

	Bond::Bond(const std::string& bondObjectName, const LabelValueBlock& bondLVB)
		: IsAQObject(bondObjectName, BOND), schedule_(), bondParameters_(bondLVB), scheduleParameters_(LabelValueBlock()), bondObjectName_(bondObjectName), bondYieldParameters_()
	{
		initializeBondDescriptionLVB(bondLVB);

	}

	void Bond::initializeBondDescriptionLVB(const LabelValueBlock& bondLVB)
	{
		// Initialize Bond Label Value Block Parameters
		bondISIN_ = bondLVB.getCompulsoryValueAsString(BOND_KEY::ISIN, "bondLVB");
		currency_ = toCCYEnum(bondLVB.getCompulsoryValueAsString(BOND_KEY::CURRENCY, "bondLVB"));
		bondType_ = toBondTypeEnum(bondLVB.getCompulsoryValueAsString(BOND_KEY::BOND_TYPE, "bondLVB"));
		yieldType_ = toYieldTypeEnum(bondLVB.getCompulsoryValueAsString(BOND_KEY::YIELD_TYPE, "bondLVB"));
		isCleanPrice_ = bondLVB.getCompulsoryValueAsBool(BOND_KEY::IS_CLEAN_PRICE, "bondLVB");

		//Optional:
		bondDescription_ = bondLVB.getOptionalValueAsString(BOND_KEY::BOND_DESCRIPTION, bondISIN_);
		bondQuoteConvention_ = toBondQuoteConventionEnum(bondLVB.getOptionalValueAsString(BOND_KEY::BOND_QUOTE_CONVENTION, "NONE"));
	}

    Bond::Bond( const Bond& rhs )
        : IsAQObject( rhs.getRefToName(), BOND ), bondParameters_( rhs.bondParameters_ ), scheduleParameters_( rhs.scheduleParameters_ ), bondObjectName_( rhs.bondObjectName_ )
	{
        // Initialize Bond Label Value Block Parameters
        bondDescription_            = rhs.bondDescription_;
        bondISIN_                   = rhs.bondISIN_;
        currency_                   = rhs.currency_;
        bondType_                   = rhs.bondType_;
        yieldType_                  = rhs.yieldType_;
        bondQuoteConvention_        = rhs.bondQuoteConvention_;
        isCleanPrice_               = rhs.isCleanPrice_;
        
		if (rhs.schedule_ != nullptr)
        {
			schedule_ = std::static_pointer_cast<BondSchedule> (rhs.schedule_->clone());
		}

        // Set the Bond Yield Parameters Struct
        setBondYieldParameters();
    }

    void Bond::initializeDataProvider( DataProvider& dataProvider, const double& yield, const YieldCalculationTypeEnum& yieldCalcType, const std::shared_ptr<BondActiveCouponDates>& activeCouponDatesPtr) const
    {
        // Update the Currency Parameters
		dataProvider.setCurrency( currency_ );
		dataProvider.setValuationCurrency( currency_ );
	}
    

	SchedulePtr Bond::getSchedule() const
	{
        return schedule_;
	}

	double Bond::price( const AQLDate& settlementDate, const double& yield, const YieldCalculationTypeEnum& yieldCalcType ) const
    {
        double price = 0.0;
        if (isCleanPrice_)
        {
            price = cleanPrice(settlementDate, yield, yieldCalcType);
        }
        else
        {
            price = dirtyPrice(settlementDate, yield, yieldCalcType);
        }

        return price;
    }

	/* @brief: Calculates the price of a bond using a BondCurve to discount coupons.
	* @param[in]	settlementDate	The bond settlement date
	* @param[in]	bondCurve		A calibrated bond curve
	* @returns		The bond price. This is clean or dirty, depending on the convention used by the bond
	*/
	double Bond::priceFromBondCurve( const AQLDate& settlementDate, const BondCurve& bondCurve ) const
	{
		double price = 0.0;
		if (isCleanPrice_)
        {
            price = cleanPriceFromBondCurve( settlementDate, bondCurve );
        }
        else
        {
            price = dirtyPriceFromBondCurve( settlementDate, bondCurve );
        }

        return price;
	}

	/* @brief: Calculates the price of a defaultable bond using a CreditModel to compute survival probabilities.
	* @param[in]	settlementDate	The bond settlement date
	* @param[in]	creditModel		The calibrated credit model
	* @returns		The bond price. This is clean or dirty, depending on the convention used by the bond
	*/
	double Bond::priceFromCreditModel( const AQLDate& settlementDate, const CreditModel& creditModel ) const
	{
		double price = 0.0;
		if (isCleanPrice_)
        {
            price = cleanPriceFromCreditModel(settlementDate, creditModel );
        }
        else
        {
            price = dirtyPriceFromCreditModel(settlementDate, creditModel );
        }

        return price;
	}

	std::unordered_set<CashflowHeaderEnum,EnumClassHash> Bond::allowedColumns() const
	{
		const CashflowHeaderEnum arr[] =
		{
			FIXED_RATE_HEADER
			, COUPON_HEADER
			, DISCOUNT_FACTOR_HEADER
			, COUPON_PV_HEADER
		};

		std::unordered_set<CashflowHeaderEnum,EnumClassHash> expectedList(arr, arr + sizeof(arr) / sizeof(arr[0]));

		return expectedList;

	}

	void Bond::populateHeaderAndBody(const DataProvider& dataProvider, AnyTypeVector& headers, std::vector<AnyTypeVector>& bodyBlock, bool showColumnHeaders, const std::unordered_set<CashflowHeaderEnum,EnumClassHash>& columnList) const
	{

		auto allowColumns = allowedColumns();

		bool includeFixedRate = includeCashflowColumn(FIXED_RATE_HEADER, columnList, allowColumns);
		bool includeCoupon = includeCashflowColumn(COUPON_HEADER, columnList, allowColumns);
		bool includeDF = includeCashflowColumn(DISCOUNT_FACTOR_HEADER, columnList, allowColumns);
		bool includeCouponPV = includeCashflowColumn(COUPON_PV_HEADER, columnList, allowColumns);

		auto allCashflows = schedule_->getAllCashflows();
		size_t cashflowSize = allCashflows.size();

		AQ_REQUIRE(cashflowSize != 0, "Cashflow size cannot be empty.");

		//-----Header-----
		AnyTypeMatrix cashflowHeaderBlock;
		if (showColumnHeaders)
		{
			AQ_PUSH_BACK_IF(headers, toString(FIXED_RATE_HEADER), includeFixedRate);
			AQ_PUSH_BACK_IF(headers, toString(COUPON_HEADER), includeCoupon);
			AQ_PUSH_BACK_IF(headers, toString(DISCOUNT_FACTOR_HEADER), includeDF);
			AQ_PUSH_BACK_IF(headers, toString(COUPON_PV_HEADER), includeCouponPV);
		}


		AQ_REQUIRE(cashflowSize == bodyBlock.size(), "Cashflow sizes of leg and schedule not matched.");

		//-----Body-----
		for (size_t i = 0; i < cashflowSize; ++i)
		{
			// Continue populating the body block which have been populated by the schedule
			AnyTypeVector& body = bodyBlock[i];

			auto cashflow = allCashflows[i];
			auto cashflowData = dataProvider.getCashflowDataIncludingUpfront(i);

			AQ_PUSH_BACK_IF(body, cashflow->isUpfrontCashflow() ? std::numeric_limits<double>::quiet_NaN() : cashflow->getCompoundRate(cashflowData), includeFixedRate);
			AQ_PUSH_BACK_IF(body, roundToNearest(cashflow->getCoupon(cashflowData), cashflowData.currency), includeCoupon);
			AQ_PUSH_BACK_IF(body, cashflowData.discountFactor, includeDF);
			AQ_PUSH_BACK_IF(body, roundToNearest(cashflow->getCouponPv(cashflowData), cashflowData.valuationCurrency), includeCouponPV);
		}

	}

	AnyTypeMatrix Bond::view( const AQLDate& settlementDate, const double& yield, const YieldCalculationTypeEnum& yieldCalcType, bool showColumnHeaders, const std::unordered_set<CashflowHeaderEnum,EnumClassHash>& columnList ) const
	{

        double compoundYield = compoundYieldFromQuotedYield(settlementDate, yield, yieldCalcType);

		DataProvider dataProvider(settlementDate);
        initializeDataProvider( dataProvider, compoundYield, yieldCalcType );

		const size_t MAX_CASHFLOW_COLUMNS = 50;
		const size_t maxRows = schedule_->getAllCashflows().size();

		AnyTypeVector headers;
		headers.reserve(MAX_CASHFLOW_COLUMNS);

		AnyTypeMatrix bodyBlock;
		bodyBlock.reserve(maxRows);

		schedule_->populateHeaderAndBody(headers, bodyBlock, showColumnHeaders, columnList);

		populateHeaderAndBody(dataProvider, headers, bodyBlock, showColumnHeaders, columnList);

		// Validate the result size
		validateDisplayCashflowHeaderAndBody(showColumnHeaders, headers, bodyBlock);
		
		AnyTypeMatrix cashflowMatrix;
		cashflowMatrix.reserve(bodyBlock.size() + 1);

        //-----Header-----
		if( showColumnHeaders )
		{
            cashflowMatrix.push_back(headers);
		}

		//-----Body-----
		for(size_t i=0; i < bodyBlock.size(); ++i)
		{
			cashflowMatrix.push_back(bodyBlock[i]);
		}

		if ( cashflowMatrix.size() == 0 )
		{
			throw AQLCoreInvalidData( "#Error: Unable to display cashflows. There are no cashflows to display.", __FILE__, __LINE__ );
		}
		
		return cashflowMatrix;
	}

    std::map<std::string, Variant> Bond::getDataMap() const
    {
		std::map<std::string, Variant> dataMap;

		// For string variables, can be populated using inputParameters (LVB) directly
		LabelValueBlock inputParameters = getInputParameters();
		auto keys = inputParameters.getKeys();
		auto values = inputParameters.getValues();
		for (size_t i = 0; i < keys.size(); ++i)
		{
			dataMap[keys[i]] = values[i];
		}

        return dataMap;
    }

	const DataSchema Bond::generateDataSchema(const std::string& schemaName) const
	{
		std::string schName = (schemaName.size() != 0) ? schemaName : toString(BOND);

		const DataSchema  dynamicSchema(schName, 2, boost::assign::list_of(STRING_VALUE)(VARIANT_VALUE), boost::assign::list_of("BOND_PARAMETER")("BOND_VALUE"));

		return dynamicSchema;
	}


    const SchemaObject Bond::toSchemaObject() const
	{
        SchemaObject schemaObject( BOND, getRefToName() );

		toSchemaObject(schemaObject);

		return schemaObject;
	}

	//Append new schema to input schemaObject
	void Bond::toSchemaObject(SchemaObject& schemaObject) const
	{
		const std::string schemaName = toString(BOND);

		//dynamic generate schema
		schemaObject.addDataSchema(generateDataSchema(schemaName));

		//add data to schema
		schemaObject.setDataForSchemaWithMap(schemaName, getDataMap());
		
	}

    LabelValueBlock Bond::getInputParameters() const
    {
        LabelValueBlock inputParameters( bondParameters_, scheduleParameters_ );
        return inputParameters;
    }

    // Check that the given settlement date is valid i.e. not before bond start date and not after bond maturity
    void Bond::checkSettlementDateValid( const AQLDate & settlementDate ) const
    {
        const unsigned int cashflowSize = schedule_->getCashflowSize();
        if ( cashflowSize == 0 )
		{
			throw AQLCoreInvalidData("#Error: Unable to evaluate the Bond Cashflows. The bond has no cashflows.", __FILE__, __LINE__ );
		}

        AQLDate bondStartDate = schedule_->getEffectiveDate();
        if ( settlementDate < bondStartDate )
        {
            throw AQLCoreInvalidData( ( boost::format( "#Error: Invalid Settlement Date: SettlementDate '%s'is before the Bond Start Date. " ) % settlementDate.convertDateToString().getCString() ).str().c_str() , __FILE__, __LINE__ );
        }

        AQLDate bondMaturityDate = schedule_->getUnadjustedMaturityDate();
        if ( settlementDate >= bondMaturityDate )
        {
            throw AQLCoreInvalidData( ( boost::format( "#Error: Invalid Settlement Date: SettlementDate  '%s' should be before Bond Maturity Date. " ) % settlementDate.convertDateToString().getCString() ).str().c_str() , __FILE__, __LINE__ );
        }
        
        return;
    }

    //Use the user specified calculation type, if not specified, use the default one to the bond 
    YieldCalculationTypeEnum Bond::getYieldCalulationType(const std::string& yieldCalcType) const
    {
        auto yieldCalcTypeEnum = toYieldCalculationTypeEnum(yieldCalcType);

        //If not specified, use the bond's default yield calculation type
        if (yieldCalcTypeEnum == NONE_YIELD)
        {
            if (isJapaneseGovenmentBond(bondYieldParameters_.calculationType_))
            {
                yieldCalcTypeEnum = SIMPLE_YIELD;
            }
            else if (isItalianGovenmentBond(bondYieldParameters_.calculationType_))
            {
                yieldCalcTypeEnum = TRUE_YIELD;
            }
            else
            {
                yieldCalcTypeEnum = ISMA_YIELD;
            }
        }
        
        return yieldCalcTypeEnum;
    }

    void Bond::setBondYieldParameters()
    {
		populateBondYieldParameters(bondYieldParameters_, schedule_->getPaymentFrequency(), schedule_->getYieldFrequency(), schedule_->getBondCalculationType(), schedule_->getAccrualDaycount(), schedule_->getAccrualCalendar().getCString(), schedule_->getExDividendTenor());
	}

	double Bond::dv01Numerical( const AQLDate& settlementDate, const double& yield, const YieldCalculationTypeEnum& yieldCalcType, const double bumpSize, const AQLString& bumpMode ) const
	{
		double dv01            = 0.0;
		const double yieldBump = bumpSize * 0.0001;

		AQLString uppercaseBumpMode( bumpMode );
		uppercaseBumpMode.toUpper();
		if ( uppercaseBumpMode == "UP" )
		{
			double basePrice = price( settlementDate, yield, yieldCalcType );
			double yieldUp   = yield + yieldBump;
			double priceUp   = price( settlementDate, yieldUp, yieldCalcType );
			dv01             = ( basePrice - priceUp ) / yieldBump;
		}
		else if (uppercaseBumpMode == "DOWN" )
		{
			double basePrice = price( settlementDate, yield, yieldCalcType );
			double yieldDown = yield - yieldBump;
			double priceDown = price( settlementDate, yieldDown, yieldCalcType );
			dv01             = ( priceDown - basePrice ) / yieldBump;
		}
		else if ( uppercaseBumpMode == "CENTRAL" )
		{
			double yieldUp   = yield + yieldBump;
			double priceUp   = price( settlementDate, yieldUp, yieldCalcType );
			double yieldDown = yield - yieldBump;
			double priceDown = price( settlementDate, yieldDown, yieldCalcType );
			dv01             = ( priceDown - priceUp ) / ( 2.0 * yieldBump );
		}
		else
		{
			throw AQLCoreInvalidData( "#Error: BumpMode must be either 'UP, or 'DOWN', or 'CENTRAL'", __FILE__, __LINE__ );
		}

		// Undo bond price() scaling
		const double faceValue = getSchedule()->getNotional(); 
		dv01 *= ( faceValue / 100. );

		// BBG DV01 is for a 1bp change in yield
		dv01 *= 0.0001;
		return dv01;
	}

    const double Bond::convertYieldFromYieldFreqToCouponFreq(const double yield) const
    {
      // convert yield from yield frequency to coupon frequency, if they are different  
        auto couponFreq = schedule_->getAccrualFrequency();
        auto yieldFreq = schedule_->getYieldFrequency();

        double toYield = convertYield( yield, yieldFreq, couponFreq);

        return toYield;
    }

	/* @brief	Utility function to convert from dirty to clean price. The API takes in a dataProvider parameter which
	 * in addition to providing the settlementDate can provide further parameters used for more complex bonds such as FRNs.
	 * @param[in]	dirtyPrice		The input price
	 * @param[in]	dataProvider	Input market data parameters. At minimum this should provide the settlementDate.
	 * @returns		The converted price.
	 */
    double Bond::priceFromDirtyToClean( const double dirtyPrice, const DataProvider& dataProvider ) const
    {        
		double cleanPrice  = dirtyPrice - accruedInterestPercent( dataProvider );
        return cleanPrice;
    }
	/* @brief	Utility function to convert from clean to dirty price. The API takes in a dataProvider parameter which
	 * in addition to providing the settlementDate can provide further parameters used for more complex bonds such as FRNs.
	 * @param[in]	cleanPrice		The input price
	 * @param[in]	dataProvider	Input market data parameters. At minimum this should provide the settlementDate.
	 * @returns		The converted price.
	 */
    double Bond::priceFromCleanToDirty( const double cleanPrice, const DataProvider& dataProvider ) const
    {
        double dirtyPrice  = cleanPrice + accruedInterestPercent( dataProvider );
        return dirtyPrice;
    }

	/* @brief	Utility function for simple fixed bonds which support conversion between clean and dirty price using only the settlementDate.
	 *			For more complex bonds (such as FRNs) please use the alternative APIs which takes in a DataProvider parameter.
	 * @param[in]	dirtyPrice		The input price
	 * @param[in]	settlementDate	The settlement date used for the valuation
	 * @returns		The converted price.
	 */
	double Bond::priceFromDirtyToClean( const double dirtyPrice, const AQLDate& settlementDate ) const
	{
		ValuationSettings valuationSettings;
		valuationSettings.setSettlementDate( settlementDate );
		DataProvider dataProvider( valuationSettings );
		const double cleanPrice = priceFromDirtyToClean( dirtyPrice, dataProvider );
		return cleanPrice;
	}

	/* @brief	Utility function for simple fixed bonds which support conversion between clean and dirty price using only the settlementDate.
	 *			For more complex bonds (such as FRNs) please use the alternative APIs which takes in a DataProvider parameter.
	 * @param[in]	cleanPrice		The input price
	 * @param[in]	settlementDate	The settlement date used for the valuation
	 * @returns		The converted price.
	 */
    double Bond::priceFromCleanToDirty( const double cleanPrice, const AQLDate& settlementDate ) const
	{
		ValuationSettings valuationSettings;
		valuationSettings.setSettlementDate( settlementDate );
		DataProvider dataProvider( valuationSettings );
		const double dirtyPrice = priceFromCleanToDirty( cleanPrice, dataProvider );
		return dirtyPrice;
	}


    AQLDate Bond::getBondLastCouponDate(const AQLDate& settlementDate) const
    {
		auto activeCouponDates = schedule_->getBondFirstActiveCouponDates(settlementDate, false);
		AQLDate lastCouponDate = activeCouponDates.priorFirstActiveCouponDate_;

		return lastCouponDate;
    }

	//Utility function to get the reinvest coupons between settleDate and forwardSettleDate
	std::vector< BondFwdReinvestedCoupon > Bond::getBondFwdReinvestedCoupons(const double& price, const AQLDate& settleDate, const AQLDate& forwardSettleDate, const DayCountEnum& repoDayCount, const double& repoRate) const
	{

		// Data provider is based on settleDate
		DataProvider dataProvider(settleDate);
		auto yieldCalcType = getYieldCalulationType("");
		double cpYield = compoundYield(settleDate, price, yieldCalcType);
		initializeDataProvider(dataProvider, cpYield, yieldCalcType);

		// Get the cashflow index where the settleDate is on 
		auto settleActiveCouponDates = schedule_->getBondFirstActiveCouponDates(settleDate, true);

		const int settleCashflowIndex = settleActiveCouponDates.firstActiveCashflowIndex_;
		//const bool isSettleDateExDividend = settleActiveCouponDates.isSettleDateExdividend_;

		// Get the cashflow index where the forwardSettleDate is on 
		auto fwdSettleFirstActiveCouponDates = schedule_->getBondFirstActiveCouponDates(forwardSettleDate, true);
		const int fwdSettleCashflowIndex = fwdSettleFirstActiveCouponDates.firstActiveCashflowIndex_;

		// coupons between settleDate and forwardSettleDate
		std::vector< BondFwdReinvestedCoupon > reinvestedCoupons;

		AQ_REQUIRE(!AQ_IS_EQUAL_ZERO(schedule_->getNotional()), "Bond notional cannot be zero.");
		const double couponScalingFactor = 100.0 / std::fabs(schedule_->getNotional());

		// first coupon factor FCF
		const double firstCouponFactor = settleActiveCouponDates.priorFirstActiveCouponDate_.intervalDays(settleActiveCouponDates.firstActiveCouponDate_) / settleActiveCouponDates.firstPriorVirtualCouponDate_.intervalDays(settleActiveCouponDates.firstActiveCouponDate_);

		double lastCpnFwdValue = 1.0;

		//k is the index of the coupon payment date before or on fwdSettleDate
		int k = fwdSettleCashflowIndex - 1;

		// *** From fwdSettleCashflowIndex (inclusive) to settleCashflowIndex (exclusive)
		for (int i = k; i >= settleCashflowIndex; --i)
		{
			auto cashflow = schedule_->getCashflow(i);
			auto curCouponDate = cashflow->getPaymentDate();

			BondFwdReinvestedCoupon receivedcoupon;

			const double paidCoupon = cashflow->getCoupon(dataProvider.getCashflowDataExcludingUpfront(i));
			
			receivedcoupon.couponValue = firstCouponFactor * couponScalingFactor * paidCoupon;

			double fwdYearFraction = 0.0;

			// last coupon date before fwdSettleDate
			if (i == k)
			{
				fwdYearFraction = getYearFraction(curCouponDate, forwardSettleDate, repoDayCount);
		
				// Update the last coupon rate C_i
				lastCpnFwdValue = receivedcoupon.couponValue;

			}
			else
			{
				// a_i is the year fraction from current payment date to next paymentDate, here max of (i + 1) if k 
				auto nextCouponDate = schedule_->getCashflow(i + 1)->getPaymentDate();
				fwdYearFraction = getYearFraction(curCouponDate, nextCouponDate, repoDayCount);
			}

			receivedcoupon.couponFwdYearFraction = fwdYearFraction;

			// Formula: FirstCouponFactor * CouponRate * (1 + r * a_1) * (1 + r * a_2)...*(1+ r * b) + CouponRate * (1 + r * a_2)...*(1+ r * b) +...+ CouponRate *(1+ r * b) 
			if (!boost::math::isnan(repoRate))
			{
				// lastCpnFwdValue * (1+ r * a_i) 
				receivedcoupon.couponFwdValue = lastCpnFwdValue * (1.0 + repoRate * fwdYearFraction);

				lastCpnFwdValue = receivedcoupon.couponFwdValue;
			}

			reinvestedCoupons.push_back(receivedcoupon);
		}

		//If fwdSettleDate is exdividend, then the paid coupon at this cashflow needs to be discount back to the fwdSettleDate
		//Formula: CouponRate / (1+ r * yf), where yf is the year fraction from fwdSettleDate to coupon paymentDate
		if (fwdSettleFirstActiveCouponDates.isSettleDateExdividend_)
		{
			BondFwdReinvestedCoupon receivedcoupon;

			auto cashflow = schedule_->getCashflow(fwdSettleCashflowIndex);
			const double paidCoupon = cashflow->getCoupon(dataProvider.getCashflowDataExcludingUpfront(fwdSettleCashflowIndex));
			auto fwdYF = getYearFraction(forwardSettleDate, schedule_->getCashflow(fwdSettleCashflowIndex)->getPaymentDate(), repoDayCount);

			receivedcoupon.couponValue = couponScalingFactor * paidCoupon;
			receivedcoupon.couponFwdYearFraction = fwdYF;

			if (!boost::math::isnan(repoRate))
			{
				const double interest = 1.0 + repoRate * fwdYF;

				AQ_REQUIRE(!AQ_IS_EQUAL_ZERO(interest), "denominator cannot be zero.");

				// couponRate * (1+ r * yf) 
				receivedcoupon.couponFwdValue = receivedcoupon.couponValue / interest;

			}

			reinvestedCoupons.push_back(receivedcoupon);
		}

		return reinvestedCoupons;
	}


	//the reinvest coupons' value between settleDate and forwardSettleDate for bond forward
	double Bond::forwardReinvestedCouponValue(const double& price, const AQLDate& settleDate, const AQLDate& forwardSettleDate, const DayCountEnum& repoDayCount, const double& repoRate) const
	{
		// coupons between settleDate and forwardSettleDate
		auto sumOfReceivedCoupons = calculateReinvestedCouponsFwdValue(getBondFwdReinvestedCoupons(price, settleDate, forwardSettleDate, repoDayCount, repoRate));

		return sumOfReceivedCoupons;

	}

	//forward price from the given repo rate
	// 1) If the Actual repo rate is used, the result is the fairFwdPrice, where fairFwdPrice * ConversionFactor = Fair Future Price
	// 2) If the Implied repo rate is used, the result is the actualFwdPrice, where actualFwdPrice * ConversionFactor = Actual Future Price
	double Bond::forwardPrice(const double& price, const AQLDate& settleDate, const AQLDate& forwardSettleDate, const double& repoRate, const DayCountEnum& repoDayCount) const
	{
		double dirtyPrice = price;
		if (isCleanPrice_)
		{
			dirtyPrice = priceFromCleanToDirty(price, settleDate);
		}

		// coupons between settleDate and forwardSettleDate
		auto sumOfReceivedCoupons = forwardReinvestedCouponValue(price, settleDate, forwardSettleDate, repoDayCount, repoRate);

		const double fwdDirtyPrice = calculateForwardDirtyPrice(dirtyPrice, settleDate, forwardSettleDate, repoRate, repoDayCount, sumOfReceivedCoupons);

		double bondFwdPrice = fwdDirtyPrice;
		if (isCleanPrice_)
		{
			bondFwdPrice = priceFromDirtyToClean(fwdDirtyPrice, forwardSettleDate);
		}

		return bondFwdPrice;
	}

	// implied/breakeven repo rate from forward price
	double Bond::impliedRepoRate(const double& price, const AQLDate& settleDate, const AQLDate& forwardSettleDate, const double& forwardPrice, const DayCountEnum& repoDayCount) const
	{
		double dirtyPrice = price;
		double fwdDirtyPrice = forwardPrice;
		if (isCleanPrice_) 
		{
			dirtyPrice = priceFromCleanToDirty(price, settleDate);
			fwdDirtyPrice = priceFromCleanToDirty(forwardPrice, forwardSettleDate);
		}

		// coupons between settleDate and forwardSettleDate
		auto reinvestedCoupons = getBondFwdReinvestedCoupons(price, settleDate, forwardSettleDate, repoDayCount);

		const double impliedRepoRate = calculateRepoRate(dirtyPrice, fwdDirtyPrice, settleDate, forwardSettleDate, repoDayCount, reinvestedCoupons);

		return impliedRepoRate;
	}

	// implied/breakeven repo rate from future price
	// If the Actual/Quoted future price is used, the result is implied repo rate (This is the meaning of implied repo rate in papers/BB)
	// If the Fair future price is used, the result  is actual repo rate
	double Bond::impliedRepoRateFromFuture(const double& price, const AQLDate& settleDate, const AQLDate& futureSettleDate, const double& futurePrice, const double& conversionFactor, const DayCountEnum& repoDayCount) const
	{
		// The future contract is traded on exchange and the underlying bond is 'standardized' (theoretical). The real bonds that can be delivered into the contract are translated into units of the standardized bond through conversion factors.

		// Aussie future is special, there is no CTD bond
		if (bondYieldParameters_.calculationType_ == TYPE23_AUSTRALIAN_GOVERNMENT_BONDS)
		{
			throw AQLCoreInvalidData("#Error: Implied repo rate is not supported for Australian Bond Future.", __FILE__, __LINE__);
		}

		// *** Note that FUTURE is a theoretical bond, so the forward price from the future price is always CLEAN price
		double impliedFwdCleanPrice = toImpliedForwardCleanPrice(futurePrice, conversionFactor);

		double impliedFwdDirtyPrice = priceFromCleanToDirty(impliedFwdCleanPrice, futureSettleDate);

		double dirtyPrice = price;
		if (isCleanPrice_)
		{
			dirtyPrice = priceFromCleanToDirty(price, settleDate);
		}

		// coupons between settleDate and forwardSettleDate
		auto reinvestedCoupons = getBondFwdReinvestedCoupons(price, settleDate, futureSettleDate, repoDayCount);

		const double impliedRepoRate = calculateRepoRate(dirtyPrice, impliedFwdDirtyPrice, settleDate, futureSettleDate, repoDayCount, reinvestedCoupons);

		return impliedRepoRate;

	}

	// Future price from repo rate
	// 1) If the Actual repo rate is used, the result is the Fair future price
	// 2) If the Implied repo rate is used, the result is the Actual/Quoted Future price
	double Bond::futurePrice(const double& bondPrice, const AQLDate& settleDate, const AQLDate& futureSettleDate, const double& repoRate, const DayCountEnum& repoDayCount, const double& conversionFactor) const
	{
		// Aussie future is special, there is no CTD bond
		if (bondYieldParameters_.calculationType_ == TYPE23_AUSTRALIAN_GOVERNMENT_BONDS)
		{
			throw AQLCoreInvalidData("#Error: This function is not supported for Australian Bond Future.", __FILE__, __LINE__);
		}

		const double fwdPrice = forwardPrice(bondPrice, settleDate, futureSettleDate, repoRate, repoDayCount);

		double fwdCleanPrice = fwdPrice;

		if (!isCleanPrice_)
		{
			fwdCleanPrice = priceFromDirtyToClean(fwdPrice, futureSettleDate);
		}

		AQ_REQUIRE(!AQ_IS_EQUAL_ZERO(conversionFactor), "Conversion factor cannot be zero.");

		const double futPrice = fwdCleanPrice / conversionFactor;

		return futPrice;

	}

	// conversion factor of the bond against the future's first delivery date and notional coupon rate
	double Bond::conversionFactor(const AQLDate& firstFutureSettleDate, const double& notionalBondCouponRate) const
	{
		double convfactor = 1.0;
		// Special Calcuation for AUD future: there is no CTD bond, so conversionFactor is always 1.

		switch (bondYieldParameters_.calculationType_)
		{
		case TYPE23_AUSTRALIAN_GOVERNMENT_BONDS:
		{
			convfactor = 1.0;
			break;
		}
		// Special Formula for US Treasury Future:
		case TYPE1_STREET_CONVENTION:
		case TYPE102_STREET_CONVENTION:
		{
			// A bond's conversion factor is defined as :
			// factor = a x[(coupon / 2) + c + d] - b, where factor is rounded to 4 decimal places,
			// coupon is the bond's annual coupon in decimals.
			//	n is the number of whole years from the first day of the delivery month to the maturity(or call) date of the bond or note.
			//	z is the number of whole months between n and the maturity(or call) date rounded down to the nearest quarter for the 10 - Year U.S.Treasury Note
			//	and 30 - Year U.S.Treasury Bond futures contracts, and to the nearest month for the 2 - Year, 3 - Year and 5 - Year U.S.Treasury Note futures contracts.
			//	v: 
			//     -- v = z, if z < 7; 
			//     -- v = 3, if z >= 7 for 10Y, 30Y bonds; 
			//     -- v = z-6,  if z >= 7 for 2Y, 3Y, 5Y bonds;
			//	a = (1 / 1.03)^(v / 6)
			//	b = (coupon / 2) x(6 - v) / 6
			//	c = { 1 / 1.03^(2n)........ if z < 7 or { 1 / 1.03^(2n + 1)........ if otherwise
			//	d = (coupon / 0.06) x(1 - c)
			// Formula ref: https://www.cmegroup.com/trading/interest-rates/files/Calculating_U.S.Treasury_Futures_Conversion_Factors.pdf
			// --- 1) n Calculation ---

			//Delivery Date to use: First day of the delivery Month, i.e. 01 Dec 2008
			AQLDate futureDateToUse = firstFutureSettleDate;
			futureDateToUse.setDay(1);

			const AQLDate bondMaturityDate = schedule_->getMaturityDate();
			const DayCountEnum dayCount = schedule_->getAccrualDaycount();

			const double years = getYearFraction(futureDateToUse, bondMaturityDate, dayCount);

			//round down
			const int n = floor(years);

			// --- 2) z Calculation ---

			const double months = (years - n) * 12.0;

			const double effectiveToMaturityYearFraction = getYearFraction(schedule_->getEffectiveDate(), bondMaturityDate, dayCount);
			const bool isLongTermBond = (effectiveToMaturityYearFraction > 9.9);

			// For 10Y, 30Y bonds, round down to nearest quarter:
			int z = 0;
			if (isLongTermBond)
			{
				z = floor(months / 3.0) * 3;
			}
			// For 2Y, 3Y, 5Y bonds, round down to nearest month:
			else
			{
				z = floor(months);
			}

			// --- 3) v calculation ---
			//     -- v = z, if z < 7; 
			//     -- v = 3, if z >= 7 for 10Y, 30Y bonds; 
			//     -- v = z-6,  if z >= 7 for 2Y, 3Y, 5Y bonds;

			int v = 0;
			if (z < 7)
			{
				v = z;
			}
			else if (isLongTermBond)
			{
				v = 3;
			}
			else
			{
				v = z - 6;
			}

			//	--- 4) a, b, c, d calculation --- 

			// a = (1 / 1.03)^(v / 6)
			const double a = std::pow(1.0 / 1.03, v * 1.0 / 6.0);

			//	b = (coupon / 2) x(6 - v) / 6
			const double coupon = schedule_->getFixedRate();
			const double b = (coupon / 2.0) * (6.0 - v) / 6.0;

			//	c = { 1 / 1.03^(2n)........ if z < 7 or { 1 / 1.03^(2n + 1)........ if otherwise
			double c = 0.0;
			if (z < 7)
			{
				c = std::pow(1.03, -(2.0 * n));
			}
			else
			{
				c = std::pow(1.03, -(2.0 * n + 1));
			}

			//	d = (coupon / 0.06) x(1 - c)
			const double d = coupon / 0.06 * (1.0 - c);

			//	--- 5) factor calculation --- 
			// factor = a x[(coupon / 2) + c + d] - b, where factor is rounded to 4 decimal places.
			convfactor = a * (coupon / 2.0 + c + d) - b;

			convfactor = roundToDecimal(convfactor, 4);
		
			break;
		}
		case TYPE51_JAPANESE_GOVERNMENT_BONDS:
		case TYPE235_JAPANESE_GOVERNMENT_BONDS:
		{
			// ConversionFactor = (a/x * ((1 + x/2)^b - 1) + 100) / ((1 + x/2)^(c/6)*100) - a*(6-d)/1200
			// a = amount of interest payable per year
			// b = number of payment from delivery day to the redemption date
			// c = number of months from delivery day to the redemption date
			// d = number of months from delivery day to the next coupon payment date
			// X =(i) 0.03 (5 - year JGB Futures & 20 - year JGB Futures), (ii) 0.06 (10 - year JGB Futures)
			// Formula ref: https://www.jpx.co.jp/english/derivatives/products/jgb/jgb-futures/tvdivq0000003ncd-att/formula.pdf

			//  Special case: The above b & d shall be changed to the following calculation, if the bond to the delivered for the settlement of 10Y/20Y JGB Futures has maturity of more than 10/20 years and yet to have the first coupon payment on the delivery date.
			//	b = aforementioned b + 1
			//	d = number of months from the delivery day to the first payment date - 6
			//  *** Note that we have not implement this special case, because in BB, an example bond with ISIN JP1103511J74 that matches such criteria still use the non-special b and d *** 

			//1 ) a: coupon rate in percent format
			const double a = schedule_->getFixedRate() * 100.0;

			//Delivery Date to use: First day of the delivery Month, i.e. 01 Dec 2008
			AQLDate futureDateToUse = firstFutureSettleDate;
			futureDateToUse.setDay(1);
			
			const auto couponDates = schedule_->getPaymentDates();
			const AQLDate bondMaturityDate = schedule_->getMaturityDate();

			// 2) b: number of coupons from delivery date to maturity
			auto activeCouponDates = schedule_->getBondFirstActiveCouponDates(futureDateToUse, true);

			const int b = couponDates.size() - activeCouponDates.firstActiveCashflowIndex_;

			// 3) c: number of months from delivery date to maturity
			const DayCountEnum dayCount = schedule_->getAccrualDaycount();

			const double yearsToMaturity = getYearFraction(futureDateToUse, bondMaturityDate, dayCount);

			const double c = floor(yearsToMaturity * 12.0);

			// 4) d: number of months from delivery date to next coupon date
			// LOWER_BOND: Find First Element in GREATER THAN OR EQUAL to the futureDateToUse
			auto it = std::lower_bound(couponDates.begin(), couponDates.end(), futureDateToUse);
			const AQLDate nextCouponDate = (*it);
			const double yearsToNextCoupon = getYearFraction(futureDateToUse, nextCouponDate, dayCount);

			const int d = floor(yearsToNextCoupon * 12.0);

			// 5) x: yield - is the notional bond coupon rate
			const double x = notionalBondCouponRate;

			// 5) CF = (a/x * ((1 + x/2)^b - 1) + 100) / ((1 + x/2)^(c/6)*100) - a*(6-d)/1200

			const double temp = 1.0 + x * 0.5;

			AQ_REQUIRE(!AQ_IS_EQUAL_ZERO(temp), "(1.0 + x * 0.5) cannot be zero.");

			convfactor = (a / x * (std::pow(temp, b) - 1.0) + 100.0) / (std::pow(temp, c / 6.0) * 100.0) - a *(6.0 - d) / 1200.0;

			convfactor = truncateToDecimal(convfactor, 6);

			break;
		}
		default:
		{
			// ConversionFactor: the clean price of a deliveryable bond to yield the coupon rate on the notional bond by the first futures delviery date,
			// i.e. using notionalBondCouponRate as the yield to price the bond at firstFutureSettleDate

			AQLDate futureDateToUse = firstFutureSettleDate;

			if (bondYieldParameters_.calculationType_ == TYPE26_UK_GILT)
			{
				//First day of the delivery Month, i.e. 01 Dec 2008
				futureDateToUse.setDay(1);
			}

			const double forwardCleanPrice = cleanPrice(futureDateToUse, notionalBondCouponRate, getYieldCalulationType(""));

			convfactor = forwardCleanPrice / 100.0;

			break;
		}
		}


		return convfactor;
	}


	// Gross basis: currentCleanPrice - futurePrice * conversionFactor
	double Bond::grossBasis(const double& price, const AQLDate& settleDate, const double& futurePrice, const double& conversionFactor) const
	{
		// Aussie future is special, there is no CTD bond
		if (bondYieldParameters_.calculationType_ == TYPE23_AUSTRALIAN_GOVERNMENT_BONDS)
		{
			throw AQLCoreInvalidData("#Error: This function is not supported for Australian Bond Future.", __FILE__, __LINE__);
		}

		// Gross basis = currentCleanPrice - futurePrice * conversionFactor
		double cleanPrice = price;
		if (!isCleanPrice_)
		{
			cleanPrice = priceFromDirtyToClean(price, settleDate);
		}

		const double grossBas = cleanPrice - toImpliedForwardCleanPrice(futurePrice, conversionFactor);

		return grossBas;
	}

	// Net basis: bondForwardCleanPrice - futurePrice * conversionFactor
	double Bond::netBasis(const double& bondForwardPrice, const AQLDate& forwardSettleDate, const double& futurePrice, const double& conversionFactor) const
	{
		// Aussie future is special, there is no CTD bond
		if (bondYieldParameters_.calculationType_ == TYPE23_AUSTRALIAN_GOVERNMENT_BONDS)
		{
			throw AQLCoreInvalidData("#Error: This function is not supported for Australian Bond Future.", __FILE__, __LINE__);
		}

		// Formula derivation:
		//GrossBasis = cleanPrice - futurePrice * CF
		//CouponIncome = (AI_FutureSettleDt - AI_SettleDt)
		//repoCost = currentDirtyPrice * actualRepoRate * repoYearFraction

		//NetBasis = GrossBasis - CouponIncome + repoCost
		//= > = (cleanPrice - futurePrice * CF) - (AI_FutureSettleDt - AI_SettleDt) + (currentDirtyPrice * actualRepoRate* repoYearFraction)
		//= > = -futurePrice * CF - AI_FutureSettleDt + currentDirtyPrice * (1 + actualRepoRate* repoYearFraction)
		//= > = -futurePrice * CF - AI_FutureSettleDt + forwardDirtyPrice
		//= > = -futurePrice * CF + forwardCleanPrice
		//= > = forwardCleanPrice - futurePrice * CF, where futurePrice * CF is normally called fairCleanForwardPrice

		double fwdCleanPrice = bondForwardPrice;

		if (!isCleanPrice_)
		{
			fwdCleanPrice = priceFromDirtyToClean(bondForwardPrice, forwardSettleDate);
		}

		const double netBas = fwdCleanPrice - toImpliedForwardCleanPrice(futurePrice, conversionFactor);

		return netBas;
	}
    double Bond::cleanPriceJGBApproximation(const AQLDate& settlementDate, const double& inputYield) const
    {
		AQ_THROW("cleanPriceJGBApproximation() is not supported for this bond type");
	}


    double Bond::compoundYieldJGBApproximation( const AQLDate& settlementDate, const double& price) const
    {
		AQ_THROW("compoundYieldJGBApproximation() is not supported for this bond type");
	}
}

