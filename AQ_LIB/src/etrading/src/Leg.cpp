#include "Leg.h"
#include "SwapValidation.h"
#include "SwapUtilities.h"
#include <iomanip>
#include "StaticStructureStore.h"
#include "Variant.h"
#include "ExceptionMacros.h"


namespace etrading
{

	Leg::Leg(const std::string& instanceName) : IsAQObject(instanceName, LEG), legStaticData_(), schedule_(), inputParameters_(LabelValueBlock())
	{}
	
	Leg::Leg(const LabelValueBlock& legLVB, const std::string& instanceName, const SchedulePtr& schedule) : IsAQObject(instanceName, LEG), legStaticData_(), schedule_(schedule)
	{
        inputParameters_ = legLVB;
		
		// Do not add the bespoke schedule's input parameters as when loading it we will handle it in special case.
		if (schedule != nullptr && !schedule->getInputParameters().isEmpty())
		{
            inputParameters_ = LabelValueBlock( inputParameters_, schedule_->getInputParameters() );
		}
	}

    Leg::Leg(const std::string& instanceName, const LegStaticDataPtr& legStaticData, const SchedulePtr& schedule) : IsAQObject(instanceName, LEG), legStaticData_(legStaticData), schedule_(schedule)
    {
  		if (legStaticData_ == nullptr || schedule_ == nullptr)
		{
			throw AQLCoreInvalidData( "#Error: LegStaticData or Schedule has not been built", __FILE__, __LINE__ );
		}

        inputParameters_ = LabelValueBlock( legStaticData_->getInputParameters(), schedule_->getInputParameters() );
    }

    Leg::Leg( const Leg& rhs) : IsAQObject(rhs.getRefToName(), LEG), inputParameters_(rhs.inputParameters_)
	{
        if (rhs.legStaticData_ != nullptr)
        {
            legStaticData_ = rhs.legStaticData_->clone();
        }
        if (rhs.schedule_ != nullptr)
        {
            schedule_ = rhs.schedule_->clone();
        }
    }

	/*	@brief	Initialise the DataProvider object from the supplied modelName / curveCollection name parameter, and optional fixngtable
	*	@param[out]		dataProvider		A reference to the DataProvider object which will be populated
	*	@param[in]		isFloatRateRequired	Whether to calculate float rates
	*	@param[in]		updateCurveData		Whether to update the curve data inside the dataProvider
	*/
	void Leg::initializeDataProviderUsingModel( DataProvider& dataProvider, bool isFloatRateRequired, bool updateCurveData )
	{
		// The default behaviour is to simply delegate to initializeDataProvider() with the modelName specifying a curveCollection
		initializeDataProvider( dataProvider, updateCurveData );
	}

	/*	@brief	Initialise the DataProvider object from the supplied curveCollection and fixngtable
	*	@param[out]		dataProvider		A reference to the DataProvider object which will be populated
	*	@param[in]		updateCurveData		Whether to update the curve data inside the dataProvider
	*/
    void Leg::initializeDataProvider( DataProvider& dataProvider, bool updateCurveData)
    {
        //
        // TODO: Enhance refresh logic to allow curves to tick, must be based on the object build time rather than the object name changes
        // See Swap.cpp Swap::updateNotionalResetByFxLeg line # 257 and Leg.cpp Leg::updateCashflows method line #62
        // ------------------------------------------------------------------
        bool notSameCurve = true;
        bool notSameFixingTable = true;
        
        //bool notSameCurve = !same( legStaticData_->getCurveCollection(), curveCollection);
        //bool notSameFixingTable = (fixingTable != nullptr && !same( legStaticData_->getFixingTableName(), fixingTable->getName().c_str()));
        // ------------------------------------------------------------------

        if ((notSameCurve || notSameFixingTable) && legStaticData_ !=nullptr && updateCurveData)
        {

			const AQLString curveCollection  = dataProvider.getValuationSettings().getCurveCollection().c_str();
			
			legStaticData_->validateCurveInput(curveCollection);

            legStaticData_->setCurveDataLoaded(false);
            
            auto currency = legStaticData_->getCurrency();
            auto valuationCurrency = legStaticData_->getValuationCurrency();
            auto compoundType = legStaticData_->getCouponCompoundMethod();

			// compoundType: only use default value when the schedule is not Structured bespoked
			if (schedule_->getBespokeScheduleType() != BESPOKE_SCHEDULE)
			{
				//If accrualFreq is the same as paymentFreqEnum, no need to compound the coupon.
				//Note need to use the paymentFreqEnum here as for zero coupon swap, the paymentFreq is the same as accrualFreq but the paymentFreqEnum is diff
				if (schedule_->getAccrualFrequency() == schedule_->getPaymentFreqEnum())
				{
					compoundType = NONE_COMPOUNDING_METHOD;
				}
				// If the leg's accrual freq is diff than  payment freq
				else
				{
					if (compoundType == NONE_COMPOUNDING_METHOD)
					{

						bool isForeCastCurveOISorARR = true;
						if (schedule_->getScheduleType() == FLOAT_SCHEDULE_TYPE)
						{
							CurveTypeEnum curveType = toCurveTypeEnum(getCurveType(curveCollection, legStaticData_->getForecastCurveMarketName()).c_str());
							isForeCastCurveOISorARR = (curveType == OIS_CURVETYPE || curveType == ARR_CURVETYPE);
						}

						// We set a default couponType for normal leg (i.e. non-OIS leg)
						if (!isForeCastCurveOISorARR)
						{
							compoundType = FLAT_COMPOUNDING_METHOD;
						}
					}
				}
			}

			dataProvider.setCurrency( currency );
			dataProvider.setValuationCurrency( valuationCurrency );
			dataProvider.setCompoundType( compoundType );

		}
    }

	double Leg::pv( DataProvider& dataProvider, bool nativeCurrencyPV, bool updateCurveData)
    {
		throw AQLCoreInvalidData( ( boost::format( "#Error: pv() method is not implemented for swap leg with name (%s) ." )
                                   % getLegName() ).str().c_str(), __FILE__, __LINE__ );
	}

	AQLString Leg::getLegName() const
	{
   		if (legStaticData_ == nullptr)
		{
			throw AQLCoreInvalidData( "#Error: LegStaticData has not been built", __FILE__, __LINE__ );
		}

		return legStaticData_->getLegName();
	}

    LegStaticDataPtr Leg::getStaticData() const
	{
		return legStaticData_;
	}

	SchedulePtr Leg::getSchedule() const
	{
        return schedule_;
	}
	
  	double Leg::calculateAnnuityWithNotional( const DataProvider& dataProvider ) const 
	{
        double annuity = 0.0;
		if (schedule_ == nullptr)
		{
			throw AQLCoreInvalidData( "#Error: Schedule has not been built", __FILE__, __LINE__ );
		}

		for( size_t i = 0; i < schedule_->getCashflowSize(); i++ )
		{
			CashflowPtr cf = schedule_->getCashflow(i);
    		annuity += cf->getAnnuity( dataProvider.getCashflowDataExcludingUpfront( i ) );
		}
		return annuity;
	}

	double Leg::annuityWithNotional(DataProvider& dataProvider)
	{
		initializeDataProvider(dataProvider);

		return calculateAnnuityWithNotional(dataProvider);
	}

    void Leg::flipPayerReceiver()
    {
		if (schedule_ == nullptr)
		{
			throw AQLCoreInvalidData( "#Error: Schedule has not been built", __FILE__, __LINE__ );
		}

        schedule_->flipPayerReceiver();

        auto payReceive = schedule_->getPayerReceiver();

        // Add Pay/Receive to LVB
        inputParameters_ =  LabelValueBlock( inputParameters_, IRS_KEY::PAY_RECEIVE, toString(payReceive) );
    }

	void Leg::populateHeaderAndBody(const DataProvider& dataProvider, AnyTypeVector& headers, std::vector<AnyTypeVector>& bodyBlock, bool showColumnHeaders, const std::unordered_set<CashflowHeaderEnum,EnumClassHash>& columnList) const
	{

		auto allowColumns = allowedColumns();

		bool includeFloatRate = includeCashflowColumn(FLOAT_RATE_HEADER, columnList, allowColumns);
		bool includeCoupon = includeCashflowColumn(COUPON_HEADER, columnList, allowColumns);
		bool includeDF = includeCashflowColumn(DISCOUNT_FACTOR_HEADER, columnList, allowColumns);
		bool includeCouponPV = includeCashflowColumn(COUPON_PV_HEADER, columnList, allowColumns);

		// For Float leg, Premium leg

		//By default, only show SurvivalProbability, MarginalDefaultProbability, RiskyCoupon for riskLeg leg
		bool riskyLeg = (getPaymentTrigger() != PAY_ALWAYS);
		bool includeSurvivalProbability = includeCashflowColumn(SURVIVAL_PROBABILITY_HEADER, columnList, allowColumns, riskyLeg);
		bool includeMarginalDefaultProbability = includeCashflowColumn(MARGINAL_DEFAULT_PROBABILITY_HEADER, columnList, allowColumns, riskyLeg);
		bool includeRiskyCoupon = includeCashflowColumn(RISKY_COUPON_HEADER, columnList, allowColumns, riskyLeg);

		// For Premium leg
		bool includePremiumCoupon = includeCashflowColumn(PREMIUM_COUPON_HEADER, columnList, allowColumns);
		bool includeAccrualOnDefaultCoupon = includeCashflowColumn(ACCRUAL_ON_DEFAULT_COUPON_HEADER, columnList, allowColumns);

		// For Float Leg and libor in Arrears
		bool isArrearFixing = (schedule_->getFixingAdvanceOrArrears() == IN_ARREARS_FIXING);
		bool includeConvexity = includeCashflowColumn(CONVEXITY_HEADER, columnList, allowColumns, isArrearFixing);
		bool includeUnadjustedFloatRate = includeCashflowColumn(UNADJUSTED_FLOAT_RATE_HEADER, columnList, allowColumns, isArrearFixing);
		
		auto allCashflows = schedule_->getAllCashflows();
		size_t cashflowSize = allCashflows.size();

		AQ_REQUIRE(cashflowSize != 0, "Cashflow size cannot be empty.");

		//Get the first non-upfront cashflow
		bool isNotionalReset = (schedule_->getCashflow(0)->getFxFixingDate() != AQLDate());
		bool includeFxFixingDate = includeCashflowColumn(FX_FIXING_DATE_HEADER, columnList, allowColumns, isNotionalReset);
		bool includeFxRate = includeCashflowColumn(FX_RATE_HEADER, columnList, allowColumns, isNotionalReset);

		//-----Header-----
		AnyTypeMatrix cashflowHeaderBlock;
		if (showColumnHeaders)
		{
			AQ_PUSH_BACK_IF(headers, toString(FLOAT_RATE_HEADER), includeFloatRate);
			AQ_PUSH_BACK_IF(headers, toString(CONVEXITY_HEADER), includeConvexity);
			AQ_PUSH_BACK_IF(headers, toString(UNADJUSTED_FLOAT_RATE_HEADER), includeUnadjustedFloatRate);

			AQ_PUSH_BACK_IF(headers, toString(FX_FIXING_DATE_HEADER), includeFxFixingDate);
			AQ_PUSH_BACK_IF(headers, toString(FX_RATE_HEADER), includeFxRate);

			// For Float leg, Premium leg
			AQ_PUSH_BACK_IF(headers, toString(SURVIVAL_PROBABILITY_HEADER), includeSurvivalProbability);
			AQ_PUSH_BACK_IF(headers, toString(MARGINAL_DEFAULT_PROBABILITY_HEADER), includeMarginalDefaultProbability);

			AQ_PUSH_BACK_IF(headers, toString(COUPON_HEADER), includeCoupon);

			// For Premium leg
			AQ_PUSH_BACK_IF(headers, toString(PREMIUM_COUPON_HEADER), includePremiumCoupon);
			AQ_PUSH_BACK_IF(headers, toString(ACCRUAL_ON_DEFAULT_COUPON_HEADER), includeAccrualOnDefaultCoupon);

			// For Float leg, Premium leg
			AQ_PUSH_BACK_IF(headers, toString(RISKY_COUPON_HEADER), includeRiskyCoupon);

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

			AQ_PUSH_BACK_IF(body, cashflowData.floatRateData.resetRate, includeFloatRate);
			AQ_PUSH_BACK_IF(body, cashflowData.floatRateData.convexity, includeConvexity);
			AQ_PUSH_BACK_IF(body, cashflowData.floatRateData.unadjustedResetRate, includeUnadjustedFloatRate);

			AQ_PUSH_BACK_IF(body, fromLADateToDouble(cashflow->getFxFixingDate()), includeFxFixingDate);
			AQ_PUSH_BACK_IF(body, cashflow->getFwdFxRate(), includeFxRate);

			AQ_PUSH_BACK_IF(body, roundToNearest(cashflow->getSurvivalProbability(), cashflowData.currency), includeSurvivalProbability);

			AQ_PUSH_BACK_IF(body, roundToNearest(cashflow->getMarginalDefaultProbability(), cashflowData.currency), includeMarginalDefaultProbability);

			AQ_PUSH_BACK_IF(body, roundToNearest(cashflow->getPremiumCoupon(), cashflowData.currency), includePremiumCoupon);

			AQ_PUSH_BACK_IF(body, roundToNearest(cashflow->getAccrualOnDefaultCoupon(), cashflowData.currency), includeAccrualOnDefaultCoupon);

			AQ_PUSH_BACK_IF(body, roundToNearest(cashflow->getCoupon(cashflowData), cashflowData.currency), includeCoupon);

			// Display coupon with survivalProbability factor
			AQ_PUSH_BACK_IF(body, roundToNearest(cashflow->getCoupon(cashflowData), cashflowData.currency), includeRiskyCoupon);

			AQ_PUSH_BACK_IF(body, cashflowData.discountFactor, includeDF);

			AQ_PUSH_BACK_IF(body, roundToNearest(cashflow->getCouponPv(cashflowData), cashflowData.valuationCurrency), includeCouponPV);

		}

	}

	AnyTypeMatrix Leg::view(DataProvider& dataProvider, bool showColumnHeaders, const std::unordered_set<CashflowHeaderEnum,EnumClassHash>& columnList)
	{

	   initializeDataProviderUsingModel( dataProvider, true /* isFloatRateRequired */  );

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

       //-----Header-----
		AnyTypeMatrix ret;
		if (showColumnHeaders)
		{
			ret.push_back(headers);
		}

		ret.insert(ret.end(), bodyBlock.begin(), bodyBlock.end());
		return ret;
	}

	LabelValueBlock Leg::getInputParameters() const
	{
		return inputParameters_;
	}

	//--- Start of caching related methods -- //

	const SchemaObject Leg::toSchemaObject() const

	{
        SchemaObject schemaObject(LEG, getRefToName());

		//pass -1 to disable the index adding
		toSchemaObject(schemaObject, -1);

		return schemaObject;
	}

	void Leg::toSchemaObject(SchemaObject& schemaObject, const int& index) const
	{

        std::string legSchemaName = addIndexToSchemaName(toString(LEG), index);

		//dynamic generate schema
		schemaObject.addDataSchema(generateDataSchema(legSchemaName));

		//add data to schema
        schemaObject.setDataForSchemaWithMap(legSchemaName, getDataMap());

        if (schedule_ == nullptr)
        {
            throw AQLCoreInvalidData( "#Error: Missing leg schedule.", __FILE__, __LINE__ );
        }

		//only add the schema of Bespoke schedule, and Fee Schedule
		if (schedule_->isBespoke() || schedule_->getScheduleType() == FEE_SCHEDULE_TYPE)
		{
			std::string bespokeScheduleSchemaName = addIndexToSchemaName(schedule_->getDataSchemaName(), index);
			schedule_->addInputParametersToSchemaObject(schemaObject, bespokeScheduleSchemaName);

			std::string bespokeSCashflowSchemaName = addIndexToSchemaName(schedule_->getCashflowSchemaName(), index);
			schedule_->addBespokeCashflowsToSchemaObject(schemaObject, bespokeSCashflowSchemaName);
		}
	}

	const DataSchema Leg::generateDataSchema(const std::string& schemaName) const
    {
        std::string schName = (schemaName.size()!=0) ? schemaName : toString(LEG);
	    const DataSchema  dynamicSchema(    schName,2,
                                            boost::assign::list_of(STRING_VALUE)(VARIANT_VALUE),
		                                    boost::assign::list_of("VARIABLE_NAME")("VARIABLE_VALUE"));
        return dynamicSchema;
    }

	std::map<std::string, Variant> Leg::getDataMap() const
	{
        std::map<std::string, Variant> dataMap;
        auto keys = inputParameters_.getKeys();
        auto values = inputParameters_.getValues();
        for(size_t i=0; i < keys.size(); ++i) 
        {
			dataMap[keys[i]] = values[i];
        }
        return dataMap;
	}

	//--- End of caching related methods -- //

	// Default implementation for the case where coupons are always paid.
	// Override this in subclasses in where the cashflows are risky
	PaymentTriggerEnum Leg::getPaymentTrigger() const
	{
		return PAY_ALWAYS;
	}


}

