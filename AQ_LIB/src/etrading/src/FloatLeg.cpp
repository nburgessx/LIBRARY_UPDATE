/*
 * @brief			Class the defines the float leg
 * @Created:		05 July 2016
 * @Author:			Yongyan Zheng
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */
#include "FloatLeg.h"
#include "FloatCashflow.h"
#include "LACurveForwardRateHelpers.h"
#include "CurveInstrumentPricing.h"
#include "LACurvePricingObject.h"
#include "SwapValidation.h"
#include "ParameterValidation.h"
#include "CurveBuildDefaults.h"
#include "LWOUtilities.h"
#include "LADateScheduleHelpers.h"
#include "ConvexityModel.h"

namespace etrading
{
	FloatLeg::FloatLeg(const std::string& instanceName) : Leg(instanceName), paymentTrigger_(PAY_ALWAYS)
    {}

    FloatLeg::FloatLeg(const LabelValueBlock& legLVB, const std::string& instanceName, const SchedulePtr& schedule) : Leg(legLVB, instanceName, schedule)
	{
        const std::string inputLVB = "legLVB";

        legStaticData_ = LegStaticDataPtr (new FloatStaticData(legLVB));

		// Control whether the coupons are credit risky
		paymentTrigger_ = toPaymentTriggerEnum(legLVB.getOptionalValueAsLAString(IRS_KEY::PAYMENT_TRIGGER, "PAYALWAYS").getCString());

    	//Schedule
		if (schedule==nullptr)
		{
			schedule_ = SchedulePtr(new FloatSchedule(legLVB, instanceName));
		}
		
		//If schedule is not from bespoke cashflow, then forcaseCurve cannot be empty
		if (schedule_->getBespokeScheduleType() != BESPOKE_SCHEDULE)
		{
			MLIB_REQUIRE(legStaticData_ ->getForecastCurve().size() != 0, "Float Leg's forcast curve must provided.");
		}
	}

    FloatLeg::FloatLeg(const std::string& instanceName, const LegStaticDataPtr& legStaticData, const SchedulePtr& schedule) : Leg(instanceName, legStaticData, schedule), paymentTrigger_(PAY_ALWAYS)
    {}
			

	/*	@brief	Initialise the DataProvider object from the supplied modelName / curveCollection name parameter, and optional fixngtable
	*	@param[out]		dataProvider		A reference to the DataProvider object which will be populated
	*	@param[in]		isFloatRateRequired	Whether to calculate float rates
	*	@param[in]		updateCurveData		Whether to update the curve data inside the dataProvider
	*/
	void FloatLeg::initializeDataProviderUsingModel( DataProvider& dataProvider, bool isFloatRateRequired, bool updateCurveData )
	{
		// The user may give us a curveCollection or a CreditModel
		// Depending on the leg paymentTrigger, we may need the CreditModel in order to calculate survival probabilities
		

		switch ( getPaymentTrigger() )
		{
			case PAY_ON_SURVIVAL:
			{
				auto creditModelPtr = getCreditModel(dataProvider.getValuationSettings().getCreditModelName(), false /* do not throw if credit model missing */);

				MLIB_REQUIRE( creditModelPtr != nullptr, "A credit model is required if PaymentTrigger is PayOnSurvival." );

				// Calculate Discount Factors and forward rates
				initializeDataProviderInternal( dataProvider, isFloatRateRequired, updateCurveData );

				// Calculate Survival Probabilities
				setSurvivalProbabilitiesUsingCreditModel(dataProvider.getValuationSettings().getValuationDate(), *creditModelPtr );
				break;
			}
			case PAY_ALWAYS:
				// Calculate Discount Factors and forward rates
				initializeDataProviderInternal( dataProvider, isFloatRateRequired, updateCurveData );
				
				// Set survival probabilities back to 1.0, default probabilities back to 0.0
				resetRiskFreeCashflows();
				break;

			default:
				MLIB_THROW( "Unsupported PaymentTrigger type: " + toString( getPaymentTrigger() ));
			}

	}

	/*	@brief	Initialise the DataProvider object from the supplied curveCollection and fixngtable
	*	@param[out]		dataProvider		A reference to the DataProvider object which will be populated
	*	@param[in]		updateCurveData		Whether to update the curve data inside the dataProvider
	*/
    void FloatLeg::initializeDataProvider( DataProvider& dataProvider, bool updateCurveData )
    {
        initializeDataProviderInternal( dataProvider, true, updateCurveData );
    }

	/*	@brief	Initialise the DataProvider object from the supplied curveCollection and fixngtable
	*	@param[out]		dataProvider		A reference to the DataProvider object which will be populated
	*	@param[in]		isFloatRateRequired	Whether to calculate float rates
	*	@param[in]		updateCurveData		Whether to update the curve data inside the dataProvider
	*/
    void FloatLeg::initializeDataProviderInternal( DataProvider& dataProvider, bool isFloatRateRequired, bool updateCurveData)
    {
        Leg::initializeDataProvider( dataProvider, updateCurveData );

        if(!legStaticData_->isCurveDataLoaded() && updateCurveData)
        {

            //Populate DFs and FloatRates for cashflows
			if (isFloatRateRequired)
            {
				const LAString curveCollection = dataProvider.getValuationSettings().getCurveCollection().c_str();
				auto fixingTable = getFixingTable(dataProvider.getValuationSettings().getFixingTableName(getLegName().getCString(), getType()), false /* do not throw when missing*/);

				// First validate that the fixing table (if provided) is consistent with the forecast curve
                if ( fixingTable != nullptr &&  legStaticData_->getForecastCurveMarketName() != legStaticData_->getDiscountCurveMarketName() )
		        {


				    const LAString& curveCurrency = getCurveCurrency( curveCollection );
				    const LAString& curveFrequencyTenor = validateCurveAndGetCurveFrequency( curveCollection, legStaticData_->getForecastCurve() );
				    if (fixingTable->getCurrency() != toCCYEnum( curveCurrency.getCString() ))
				    {
					    throw LACoreInvalidData("#Error: FixingTable currency does not match the forecast curve currency. ", __FILE__, __LINE__ );
				    }
				    if (fixingTable->getCurveTenor() != toCurveTenorEnum( curveFrequencyTenor.getCString() ))
				    {
					    throw LACoreInvalidData("#Error: FixingTable curve frequency tenor does not match the forecast curve frequency tenor ", __FILE__, __LINE__ );
				    }
			    }

				std::vector<FloatRateData> floatRates;
				//Calculate float rates different for Bespoke Schedule built from Cashflow with forecastCurve
				if (schedule_->getBespokeScheduleType() == BESPOKE_SCHEDULE)
				{
					floatRates = calculateFloatRatesFromCashflows(curveCollection, dataProvider.getValuationSettings().getValuationDate(), fixingTable, dataProvider.getValuationSettings().getVolatilityModelName(), dataProvider.getValuationSettings().getConvexityMethod());
				} 
				else 
				{
					floatRates = calculateFloatRates(curveCollection, dataProvider.getValuationSettings().getValuationDate(), fixingTable, dataProvider.getValuationSettings().getVolatilityModelName(), dataProvider.getValuationSettings().getConvexityMethod());
				}

                schedule_->initializeDataProviderWithCurveData( dataProvider, legStaticData_->getDiscountCurve(), floatRates);
            }

            //Populate DFs for cashflows
            else
            {
                schedule_->initializeDataProviderWithCurveData( dataProvider, legStaticData_->getDiscountCurve());
            }

			/*
			 *  NOTE: The next line of code is commented out because we always want to enter this block of code
			 *  and recalculate the schedule discount factors. A better optimisation is to check
			 *  whether the discountCurve has changed (for example via a curve build time).
			 */
            // legStaticData_->setCurveDataLoaded(true);
        }

        if (isFloatRateRequired)
        {
            //When a leg with compounding coupons, the previous coupons need to be updated, as the spread may changed
            schedule_->updateCashflowsCompoundingCoupons( dataProvider );
        }
    }

    LegPtr FloatLeg::clone()
    {
        LegPtr leg = LegPtr(new FloatLeg(*this));
        return leg;
    }

    FloatLeg::FloatLeg( const FloatLeg& rhs) : Leg(rhs), paymentTrigger_(rhs.paymentTrigger_)
	{}


	std::vector<FloatRateData> FloatLeg::calculateFloatRates(const LAString& curveCollection, const LADate& valuationDate, const std::shared_ptr<FixingTable>& fixingTable, const std::string& volatilityModelName, const ConvexityMethodEnum& convexityMethod) const
	{
		DateVector fixingDates  = schedule_->getFixingDates();
		DateVector fixingEndDates = schedule_->getFixingEndDates();
        DateVector paymentDates = schedule_->getPaymentDates();

		const FixingTypeEnum fixingType = schedule_->getFixingAdvanceOrArrears();
		const std::string OIS_CURVE = legStaticData_->getDiscountCurve().getCString();

        size_t firstNonpastFixingDateIndex = getFirstNonpastDateIndex(fixingDates, valuationDate);

		const LAString forecastCurveMarketName = legStaticData_->getForecastCurveMarketName();

		const CurveTypeEnum curveType = toCurveTypeEnum(getCurveType(curveCollection, forecastCurveMarketName).c_str());

		const bool isOISOrARR = (curveType == OIS_CURVETYPE || curveType == ARR_CURVETYPE);

		size_t fixingDatesSize = fixingDates.size();
		
		std::vector<FloatRateData> floatRates;
        floatRates.reserve(fixingDatesSize);

		// If it is OIS float leg, and CouponCompoundMethod is specified, call calculateOisFloatRates()
        if (isOISOrARR && legStaticData_->getCouponCompoundMethod() != NONE_COMPOUNDING_METHOD)
		{		
		    // If no fixing table is specified
	        fixingTableExistenceValidation(fixingTable, valuationDate, fixingDates, schedule_->getPaymentDates());

            // Get equivalent rate over accrual periods
			auto oisFloatRates = calculateOisFloatRates(firstNonpastFixingDateIndex, curveCollection, fixingTable);

			floatRates = toFloatRateDataVector(oisFloatRates);

		}
		// If 1) it is OIS float leg, and CouponCompoundMethod is NOT specified, or 2) a normal float leg
		else
		{
			// Get Fixing Information needed for Stub Calculations
			double firstFixingRate = legStaticData_->getFirstFixing();
			double lastFixingRate = legStaticData_->getLastFixing();

			bool hasFirstFixing = useFloatFixing(firstFixingRate);
			bool hasLastFixing = useFloatFixing(lastFixingRate);

			auto firstStubCurveIndex = legStaticData_->getFirstStubCurveIndex();
			auto lastStubCurveIndex = legStaticData_->getLastStubCurveIndex();

			LAString crvFreqTenor = validateCurveAndGetCurveFrequency(curveCollection, legStaticData_->getForecastCurve());

			if (!isOISOrARR && getFrequencyOrTenorMonth(crvFreqTenor) != getFrequencyOrTenorMonth(toString(schedule_->getAccrualFrequency()).c_str()))
			{
				throw LACoreInvalidData("#Error: Accrual Frequency is not the same as Curve Frequency.", __FILE__, __LINE__);
			}

			// *** Special treatment for OIS leg without compoundMethod: use leg's accrual freq instead of curve's freq to calculate Forward Rates
			CurveTenorEnum curveFreqTenorOverride = NONE_CURVE_TENOR;
			if (isOISOrARR)
			{
				//For long-end OIS, it does not use annual compounding on daily rate(i.e. CouponCompoundMethod is NOT specified), so accrualFreq rather than curveFreq is used to calculat the fixingEndDate.
				curveFreqTenorOverride = toCurveTenorEnum(getFrequencyTenor(schedule_->getAccrualFrequency()).getCString());
			}

			//Both past and non-past forward rates from the curve
			// Calculate fwdRates using precalculated fixingStartDates and fixingEndDates.
			const DoubleVector unadjustedFwdRates = getCurveForwardRatesFromForwardDates(fixingDates, fixingEndDates, curveCollection, legStaticData_->getForecastCurve(), legStaticData_->getFwdInter(), schedule_->getFixingBusinessDayAdj());

            std::shared_ptr<Volatility> volatilityModel;
			if (convexityMethod != NONE_CONVEXITY && fixingType == IN_ARREARS_FIXING)
            {
                volatilityModel = getVolatility( volatilityModelName, false ); // false = don't throw general error message
                MLIB_REQUIRE( volatilityModel != nullptr, "Libor fixing In-Arrears requires a convexity adjustment - Please provide a volatility model.")
    	        MLIB_REQUIRE( legStaticData_->getCurrency() == volatilityModel->currency(), "Invalid Volatility Model: Volatility model and trade leg " + getLegName() + " currency must match." )
            }

			// Convexity Adjustment using fixingDates and fixingEnd dates, BB is using AccrualEndDates
			const std::vector<FloatRateData> fwdRates = getCurveForwardRatesWithConvexityAdjustment(unadjustedFwdRates, fixingDates, fixingEndDates, valuationDate, schedule_->getAccrualDaycount(), fixingType, volatilityModel, convexityMethod, curveCollection.getCString(), OIS_CURVE);

			// update the leg's fwdInter flag so that we/the test will know what is actually used
			bool isFwdInter = getfwdInterInfo( curveCollection, forecastCurveMarketName, legStaticData_->getFwdInter()).isFwdInter;
			legStaticData_->setFwdInter(isFwdInter? TRUE_BOOL : FALSE_BOOL);

            std::pair<LAStringVector, LAStringVector> curvesInfo = getStubRateCurveIndicesTenors(curveCollection);
            auto curveIndices = curvesInfo.first;
            auto curveTenors = curvesInfo.second;

            //Check if it's front stub or end stub
            bool frontStub = schedule_->isFrontStub();

            //Only call stub rate when the leg has irregular tenor
            bool irregularStub = schedule_->isIrregularStub();

            for( size_t i = 0; i < fixingDatesSize; ++i )
			{
				FloatRateData floatRate;
				auto fixingDate     = fixingDates[i];
				auto fixingEndDate = fixingEndDates[i];
				auto paymentDate    = paymentDates[i];

                //
                // First Fixing Logic:
                // The first fixing should be applied to the current or latest fixing. However in some cases we have a more than one fixing date in the past
                // with a corresponding payment date in the future. This code block ensures that when we have multiple fixings that we apply the first fix override
                // to the most recent fixing. Note when we have mutliple fixings the previous fixings will need to come from the fixing table.
                //
                bool isCurrentFixing = false;
                
                //This make sure the first fixing field is always applied when the fixingDate is not in the past
                if (fixingDate > valuationDate)
                {
                   isCurrentFixing = true;
                }
                else if ( fixingDate <= valuationDate && paymentDate >= valuationDate)
                {
                    // Check for mutliple fixings
                    if ( i < fixingDatesSize - 1 )
                    {
                        auto nextFixingDate     = fixingDates[i+1];
                        auto nextPaymentDate    = paymentDates[i];
                        
                        // Check if the next fixing is a more recent fixing
                        if ( nextFixingDate <= valuationDate && nextPaymentDate >= valuationDate)
                        {
                            isCurrentFixing = false;
                        }
                        else
                        {
                            isCurrentFixing = true;
                        }
                    }
                    // If we don't have mutliple fixings then this fixing is the current one
                    else
                    {
                        isCurrentFixing = true;
                    }

                }

                // Priorities & Logic
                // -------------------------------------------------------------------------------------------------------------------------------------------
                // 1. First Fixing Rate ( = Latest Fixing )     - The first fixing is an override for the current fixing, which may be a stub
                // 2. Last Fixing Rate                          - The last fixing is also an override but only for the final fixing, which may be a stub
                // 3. Stub Rate (can be a front or back stub)   - For the first or last fixing if a stub
                // 4. Fixing Table                              - For historic fixings
                // 5. Yield Curve Libor Forecast Table          - For future fixings
                // 
                // Note: In the case where we have a single cashflow the first fixing should be used as the fixing override
                // -------------------------------------------------------------------------------------------------------------------------------------------


                //This is to check if there are more than one curve indices available, do not call the stub calculation function if the size is 1, intead use the forward rates from the curve
                size_t curveIndicesSize = curveIndices.size();

                // 1. First Fixing (=Latest or Current Fixing) - only apply to the current cashflow don't apply the first fixing to the final cashflow, use the last fixing for that
                // Exception1: If there is only one cashflow, apply the first fixing to that cashflow
                // Exception2: Allow fixings today to be overridden to allow the 11:00am Libor reset
                if ( hasFirstFixing && isCurrentFixing )
                {
                    // Use the first fixing only once, after that the last fixing wil be used, if present
                    hasFirstFixing = false; 
                    floatRate = legStaticData_->getFirstFixing();
                }
                // 2. Last Fixing - only apply to the final cashflow
                // Exception: If there is only one cashflow, apply the first fixing to that cashflow
                else if ( hasLastFixing && i==fixingDatesSize-1 )
                {
                    // Use the last fixing only once
                    hasLastFixing = false;
                    floatRate = legStaticData_->getLastFixing();
                }
                // 3a) Front Stubs - note for regular swaps there is no stub and the front stub will match the first float rate 
                // Front stubs are always applied to the first cashflow i.e. i=0
                // Do not call the stub calculation function if the curveIndicesSize is one or less
                else if ( frontStub && irregularStub && i==0 && curveIndicesSize > 1)
                { 
                    // If a front stub exists, has fixed already, is the current fixing and has not yet been paid out then the firstFixingRate must be provided
                    if ( fixingDate < valuationDate && paymentDate >= valuationDate && !hasFirstFixing )
                    {
                    	throw LACoreInvalidData("#Error: The float leg 'firstFixing' is required for the front stub rate.", __FILE__, __LINE__ );
                    }
                    
                    auto stubRate = calculateStubRate(curveCollection, firstStubCurveIndex, curveIndices, curveTenors, crvFreqTenor);

					floatRate = getCurveForwardRateWithConvexityAdjustment(stubRate, fixingDate, fixingEndDate, valuationDate, schedule_->getAccrualDaycount(), fixingType, volatilityModel, convexityMethod, curveCollection.getCString(), OIS_CURVE);
                }
                // 3b) Back Stubs - note for regular swaps there is no stub and the back stub will match the first float rate 
                // Back stubs are always applied to the last cashflow i.e. i=fixingDatesSize-1
                // Do not call the stub calculation function if the curveIndicesSize is one or less
                else if (!frontStub && irregularStub && i==fixingDatesSize-1 && curveIndicesSize > 1)
                { 
                    // Back-Stub Issue:
                    // ----------------
                    // We can't evaluate back stubs exactly on the fixing date because of the artifical extra fixing date in the FloatLeg.cpp 
                    // calculateStubRate function. Adding the extra day with fixing.addDays(-1) inserts a date in the past and we have to get a 
                    // discount factor on that date, which is not possible. This is done for back stubs only.
                    // Therefore throw and error to request the fix be input by the end user. T
                    
                    // If a back stub exists, has fixed already, is the current fixing and has not yet been paid out then the lastFixingRate must be provided
                    if ( fixingDate <= valuationDate && paymentDate >= valuationDate && !hasLastFixing && !hasFirstFixing )
                    {
                    	throw LACoreInvalidData("#Error: The float leg 'lastFixing' is required for the back stub rate.", __FILE__, __LINE__ );
                    }
                    
					auto stubRate = calculateStubRate(curveCollection, lastStubCurveIndex, curveIndices, curveTenors, crvFreqTenor);
				
					floatRate = getCurveForwardRateWithConvexityAdjustment(stubRate, fixingDate, fixingEndDate, valuationDate, schedule_->getAccrualDaycount(), fixingType, volatilityModel, convexityMethod, curveCollection.getCString(), OIS_CURVE);
				}
                // 4 and 5. Fixing Table for Past Fixings and Regular Libor Forecast Rate from Yield Curve for Future Fixings
                else
                {
					floatRate = getForwardRate(fwdRates[i], valuationDate, fixingDate, paymentDate, fixingTable);
                }
				
                floatRates.push_back(floatRate);
            }
		}

		return floatRates;
	}

	// Porduce a compounding rate for each of the accrual periods
	DoubleVector FloatLeg::calculateOisFloatRates(size_t firstNonpastFixingDateIndex, const LAString& curveCollection, const std::shared_ptr<FixingTable>& fixingTable) const
	{
		// Get equivalent rates over accrual periods
        // Populate fixingEndDates
		auto fixingEndDates = schedule_->calculateAndPopulateOISFixingEndDates();

		const LAString interpolation = getCurveInterpolation( curveCollection, legStaticData_->getForecastCurveMarketName() );

		// Get the equivalent rate of each accrual period. 
		// The equivalent rate is obtained either through daily compounding over this period or finding the arithmetic average.
		if (schedule_->getAccrualbusinessDayAdj() == NONE_BUSINESS_DAY_ADJ || schedule_->getAccrualCalendar().size() == 0)
		{
			throw LACoreInvalidData( "#Error: OIS average rate cannot be calculated without AccrualbusinessDayAdj or AccrualCalendar, please update the float leg schedule", __FILE__, __LINE__ );
		}

		size_t expectedSize = schedule_->getCashflowSize();

		//Get float rates using compounding method:
        DoubleVector floatRates( expectedSize );

        LAString compoundFrequency = "Business_Days";
		bool isStartRoll = false;
        
        auto internalCompoundMethodType = validateCompoundingMethod(toString(legStaticData_->getCouponCompoundMethod()));

        double oneBasisPoint = 0.0001;

		//Call compoundRate function once when all cashflows have same spread 
        if (!schedule_->isVariableSpread())
        {
			double spread = schedule_->getCashflow(0)->getSpread() * oneBasisPoint;

            floatRates = calculateCompoundRateWithFixingTable(schedule_->getFixingDates(),
													        fixingEndDates,
													        curveCollection,
													        legStaticData_->getForecastCurve(),	// The forecast curve given by user is expected to be an OIS curve
													        compoundFrequency,			
													        spread,
													        schedule_->getStubType(),						
													        schedule_->getRollDayInput(),
													        schedule_->getFixingCalendar(),
													        schedule_->getFixingBusinessDayAdj(), 
													        schedule_->getAccrualDaycount(),
													        interpolation,
													        internalCompoundMethodType.c_str(),
                                                            schedule_->getFirstStub(),
                                                            schedule_->getLastStub(),
															legStaticData_->getFwdInter(),
                                                            fixingTable,
                                                            true);


			//Set spread to zero, as the calculated compoundFloatRate includes the spread
            for( size_t i = 0; i < expectedSize; ++i )
		    {
	            auto floatCf = schedule_->getCashflow(i);
				floatCf->setSpread(0.0);
			}
		}
        //Call the compoundRate function multiple times when cashflows have different spreads 
        else
        {
            for( size_t i = 0; i < expectedSize; ++i )
		    {
	            auto floatCf = schedule_->getCashflow(i);
				
				double spread = floatCf->getSpread() * oneBasisPoint;

				double floatRate =  calculateCompoundRateWithFixingTable(boost::assign::list_of(floatCf->getFixingDate()),
												        boost::assign::list_of(floatCf->getFixingEndDate()),
												        curveCollection,
												        legStaticData_->getForecastCurve(),	// The forecast curve given by user is expected to be an OIS curve
												        compoundFrequency,			
												        spread,
												        schedule_->getStubType(),						
												        schedule_->getRollDayInput(),
												        schedule_->getFixingCalendar(),
												        schedule_->getFixingBusinessDayAdj(), 
												        schedule_->getAccrualDaycount(),
												        interpolation,
												        internalCompoundMethodType.c_str(),
                                                        schedule_->getFirstStub(),
                                                        schedule_->getLastStub(),
														legStaticData_->getFwdInter(),
														fixingTable,
                                                        true).at(0);

		        floatRates[i] = floatRate;

				//Set spread to zero, as the calculated compoundFloatRate includes the spread
				floatCf->setSpread(0.0);

			}
        }
		
		return floatRates;
	}

	double FloatLeg::annuityWithNotional(DataProvider& dataProvider)
	{
		initializeDataProviderUsingModel( dataProvider, false /* float rates are not required */ );

        return calculateAnnuityWithNotional( dataProvider );
	}

	const double FloatLeg::pvCalc( DataProvider& dataProvider, const bool& nativeCurrencyPV ) const
	{
		double floaterPV = 0.;

		//get all the cashflows including the upfrontCashflow
		auto cashflows = schedule_->getAllCashflows();

		const LAString curveCollection = dataProvider.getValuationSettings().getCurveCollection().c_str();

		const bool usingExternalCurve = (curveCollection.size() == 0);

		CurveTypeEnum curveType = toCurveTypeEnum(getCurveType(curveCollection, legStaticData_->getForecastCurveMarketName()).c_str());
		
		const bool isOISorARR = !usingExternalCurve ? (curveType == OIS_CURVETYPE || curveType == ARR_CURVETYPE) : false;
	
		if (usingExternalCurve || isOISorARR || legStaticData_->getForecastCurveMarketName() != legStaticData_->getDiscountCurveMarketName())
		{
			for (size_t i = 0; i < cashflows.size(); i++)
			{
				CashflowPtr cf = cashflows[i];
				floaterPV += cf->getCouponPv(dataProvider.getCashflowDataIncludingUpfront( i ), nativeCurrencyPV);
			}
		}
		else
			// Note: Past Historic Cashflows not supported i.e. negative terms or yearFractions will return an error message.
		{
			// Libor Discounting Scenario, do not include upfrontCashflow
			LADate valuationDate = dataProvider.getValuationSettings().getValuationDate();
			LADate floatAccrualStartDate = schedule_->getCashflow(0)->getAccrualStartDate();
			LADate floatAccrualEndDate = schedule_->getCashflow(schedule_->getCashflowSize() - 1)->getAccrualEndDate();

			/* Call CurveUtilities getCurveDiscountFactors() API method.
			* This allows CurveResults to be used, if available.
			*/
			const LAString discountCurve = legStaticData_->getDiscountCurveMarketName();
			const DateVector paymentDates = { floatAccrualStartDate, floatAccrualEndDate };
			
			const DoubleVector discountFactors = getCurveDiscountFactors( valuationDate, paymentDates, curveCollection, discountCurve );
			MLIB_REQUIRE( discountFactors.size() == 2, "Internal Error: Expected two discount factors." );
			
			const double df1 = discountFactors[0];
			const double df2 = discountFactors[1];

			floaterPV = (df1 - df2) * schedule_->getNotional() * schedule_->getPayRecIndicator();

			if (!nativeCurrencyPV)
			{
				bool isCurrencySameAsValuationCurrency = (legStaticData_->getCurrency() == legStaticData_->getValuationCurrency());
				if (!isCurrencySameAsValuationCurrency)
				{
					floaterPV *= dataProvider.getValuationSettings().getFXAsOfDateRate();
				}
			}

		}

		double pv = floaterPV;

		// If need to exclude accruedInterest, when there is accruedInterest, we need to minus the accrued interest.
		// For instance, when calculating parRate/parSpread, assetSwap par, we need to exclude accruedInterest
		if ( !dataProvider.getValuationSettings().getIncludeAccruedInterest() && schedule_->hasAccruedInterest(dataProvider.getValuationSettings().getValuationDate()))
		{
			double accruedInterest = schedule_->calculateAccruedInterest(dataProvider, nativeCurrencyPV);
			//PV minus accrualInterest
			pv -= accruedInterest;
		}

		return pv;

	}

    double FloatLeg::pv( DataProvider& dataProvider, bool nativeCurrencyPV, bool updateCurveData)
	{
		initializeDataProviderUsingModel( dataProvider, true /* isFloatRateRequired */ , updateCurveData );

		return pvCalc( dataProvider, nativeCurrencyPV );
	}

    ScheduleTypeEnum FloatLeg::getType() const
    {
        return FLOAT_SCHEDULE_TYPE;
    }

    LabelValueBlock FloatLeg::getInputParameters() const
	{
        std::vector<std::string> unchangedKeys;
		unchangedKeys.push_back(IRS_KEY::FLOAT_SPREAD);
		return removeKeyPrefix(inputParameters_, "FLOAT", unchangedKeys);
	}

    double FloatLeg::calculateStubRate(const LAString& curveCollection, const LAString& stubCurveIndex, const LAStringVector& curveIndices, const LAStringVector& curveTenors, const LAString& indexFrequency) const
    {

        // Choose the nearest curve when no explicit curve has been chosen to use 
        bool useNearestCurve = same(stubCurveIndex, "NATURAL" ) ? true : false;

		const LAString interpolation = getCurveInterpolation( curveCollection, legStaticData_->getForecastCurveMarketName() );

        auto useStubCurveIndex = stubCurveIndex;
        if (!useNearestCurve)
        {
           // The user specified stubCurveIndex may not be the same name as curveIndices, so we need to find the matching one within the curveIndices, by matching the staticDataTable
           useStubCurveIndex = getMatchingCurveIndex(curveCollection, stubCurveIndex, curveIndices);
        }

        // Default the tolerance to 3D - Requested by Cam Gilbert MHI Euro Swaps Trader 31-Jan-2019
        // This is so that regular swaps with effective dates starting on weekends do not imply a stub rate
        LAString toleranceTenor = "3D";

		//For stub rate, fwdInter always false
        FwdInterInfo info = etrading::getfwdInterInfo( "", "", etrading::FALSE_BOOL );
        bool isFwdInter = info.isFwdInter;
        bool useFwdData = info.useFwdData;

        auto fixingDates = schedule_->getFixingDates();      

		const bool INCLUDE_LAST_FIXING = true;
		auto lastFixingEndDate = validateAndGenerateFixingSchedule(boost::assign::list_of(schedule_->getAccrualStartDates().back())(schedule_->getAccrualEndDates().back()),
																toString(schedule_->getFixingBusinessDayAdj()).c_str(),
																schedule_->getFixingCalendar(),
																schedule_->getFixLag(),
																"advance",
																INCLUDE_LAST_FIXING).at(1);

        fixingDates.push_back(lastFixingEndDate);

		LAString rollConv = getRollConvection(schedule_->getEffectiveDate(), schedule_->getUnadjustedMaturityDate(), schedule_->getRollDayInput());

		LAString indexFreqToUse = (indexFrequency.size() != 0) ? getFrequencyString(indexFrequency) : "";

        double stubRate = CurveInstrumentPricing::getStubRate( fixingDates,
															   curveIndices,
															   curveTenors,
															   DoubleVector(), //tenorCurveFixings, remove from user input so that the fixing of the curves will be used
															   curveCollection,
															   toString(schedule_->getStubType()).c_str(), 
															   interpolation,
															   toString(schedule_->getAccrualDaycount()).c_str(),
															   schedule_->getFixingCalendar(), 
															   toString(schedule_->getFixingBusinessDayAdj()).c_str(), 
															   rollConv,
															   useNearestCurve,
															   isFwdInter,
															   useFwdData,
															   toleranceTenor,
															   useStubCurveIndex,
															   indexFreqToUse).stubRate_;

        return stubRate;
    }

	double FloatLeg::calculateStubRateFromFixingStartEnd(const LADate& fixingDate, const LADate& fixingEndDate, const LAString& curveCollection, const LAString& stubCurveIndex, const LAStringVector& curveIndices, const LAStringVector& curveTenors, 
														const DayCountEnum& accrualDayCount, const LAString& fixingCalendar, const BusinessDayAdjustmentEnum& fixingBusinessDayAdj) const
	{

		// Choose the nearest curve when no explicit curve has been chosen to use 
		bool useNearestCurve = same(stubCurveIndex, "NATURAL") ? true : false;

		auto useStubCurveIndex = stubCurveIndex;
		if (!useNearestCurve)
		{
			// The user specified stubCurveIndex may not be the same name as curveIndices, so we need to find the matching one within the curveIndices, by matching the staticDataTable
			useStubCurveIndex = getMatchingCurveIndex(curveCollection, stubCurveIndex, curveIndices);
		}

		// Default the tolerance to 3D - Requested by Cam Gilbert MHI Euro Swaps Trader 31-Jan-2019
		// This is so that regular swaps with effective dates starting on weekends do not imply a stub rate
		LAString toleranceTenor = "3D";

		//For stub rate, fwdInter always false
		FwdInterInfo info = etrading::getfwdInterInfo("", "", etrading::FALSE_BOOL);
		bool isFwdInter = info.isFwdInter;
		bool useFwdData = info.useFwdData;

		// Note that we use the interpolation of the higher curve. Is it right?
		//const LAString interpolation = getCurveInterpolation(curveCollection, getCurveStaticDataTableName(curveCollection, curveIndices.back()));

		double ret = CurveInstrumentPricing::getStubRateFromFixingStartEnd(fixingDate, 
																	fixingEndDate,
																	curveIndices,
																	curveTenors,
																	DoubleVector(), //tenorCurveFixings, remove from user input so that the fixing of the curves will be used
																	curveCollection,
																	toString(accrualDayCount).c_str(),
																	fixingCalendar,
																	toString(fixingBusinessDayAdj).c_str(),
																	useNearestCurve,
																	isFwdInter,
																	useFwdData,
																	toleranceTenor,
																	useStubCurveIndex);

		return ret;
	}

	/* @brief Updates the cashflow survival / default probabilities using the provided credit model.
	*
	* @param[in]	asOfDate				The valuation date of the leg
	* @param[in]	creditModel				The calibrated credit model
	*/
	void FloatLeg::setSurvivalProbabilitiesUsingCreditModel( const LADate& asOfDate, const CreditModel& creditModel  )
	{
		double prevSurvivalProbability = 1.0;
		double survivalProbability = 1.0;
		LADate prevPaymentDate = asOfDate;

		auto paymentDates = schedule_->getAllPaymentDates();
		auto cashflows = schedule_->getAllCashflows();

		size_t cashflowSize = cashflows.size();
		for (size_t i=0; i<cashflowSize; i++ )
		{
			CashflowPtr curCashflow = cashflows[i];

			const LADate& paymentDate = paymentDates[i];
			double paymentYearFraction = getYearFraction( prevPaymentDate, paymentDate, schedule_->getAccrualDaycount(), false );

			double hazardRate = creditModel.getHazardRate( paymentDate );

			const double survivalFactor = exp( -hazardRate * paymentYearFraction );
			survivalProbability *= survivalFactor;
			
			double marginalDefaultProbability = prevSurvivalProbability - survivalProbability;
			
			// It is OK for this dynamic cast to fail because the FloatLeg may contain other cashflow types, for example FraCashflow.
			std::shared_ptr<FloatCashflow> floatCashflow = std::dynamic_pointer_cast<FloatCashflow>( curCashflow );
			if ( floatCashflow )
			{
				floatCashflow->setSurvivalProbability( survivalProbability );
				floatCashflow->setMarginalDefaultProbability( marginalDefaultProbability );
			}
			else
			{
				MLIB_THROW( "Unexpected cashflow type found in FloatSchedule. Expecting FloatCashflows." );
			}

			prevSurvivalProbability = survivalProbability;
			prevPaymentDate = paymentDate;
		}
	}

	/*
	* @brief	Helper method which ensures that all cashflow survivalProbabilities are initialised to 1.0
	*/
	void FloatLeg::resetRiskFreeCashflows()
	{
		auto cashflows = schedule_->getAllCashflows();
		size_t cashflowSize = cashflows.size();
		for (size_t i=0; i<cashflowSize; i++ )
		{
			CashflowPtr curCashflow = cashflows[i];

			// It is OK for this dynamic cast to fail because the FloatLeg may contain other cashflow types, for example FraCashflow.
			std::shared_ptr<FloatCashflow> floatCashflow = std::dynamic_pointer_cast<FloatCashflow>( curCashflow );
			if ( floatCashflow )
			{
				floatCashflow->setSurvivalProbability( 1.0 );
				floatCashflow->setMarginalDefaultProbability( 0.0 );
			}
		}
	}

	//floatRate from individual cashflows (bespoke cashflows)
	std::vector<FloatRateData> FloatLeg::calculateFloatRatesFromCashflows(const LAString& curveCollection, const LADate& valuationDate, const std::shared_ptr<FixingTable>& fixingTable, const std::string& volatilityModelName, const ConvexityMethodEnum& convexityMethod) const
	{

		auto cashflowSize = schedule_->getCashflowSize();

		std::vector<FloatRateData> floatRates(cashflowSize);

		std::pair<LAStringVector, LAStringVector> curvesInfo = getStubRateCurveIndicesTenors(curveCollection);
		auto curveIndices = curvesInfo.first;
		auto curveTenors = curvesInfo.second;

		const std::string fixingDateToleranceTenor = "1W";

		const std::string OIS_CURVE = legStaticData_->getDiscountCurve().getCString();

		std::shared_ptr<Volatility> volatilityModel = getVolatility(volatilityModelName, false); // false = don't throw general error message

		if (volatilityModel)
		{
			MLIB_REQUIRE(legStaticData_->getCurrency() == volatilityModel->currency(), "Invalid Volatility Model: Volatility model and trade leg " + getLegName() + " currency must match.")
		}

		for (size_t i = 0; i < cashflowSize; ++i)
		{
			// Priorities & Logic
			// -------------------------------------------------------------------------------------------------------------------------------------------
			// 1) Overrided Fixing Rate						- It is an override for the current fixing
			// 2) Stub rate									- For fixing rate of a stub
			// 3) Libor rate from forecast curve			- For fixing rate fixings
			// -------------------------------------------------------------------------------------------------------------------------------------------

			FloatRateData floatRate;

			auto cashflow = schedule_->getCashflow(i);

			auto forecastCurve = cashflow->bespokeInfo().forecastCurve;

			// 1) First priority, use the overrided floatRate from user
			if (isValid< double >(forecastCurve))
			{
				// Use the overrided floatRate from user
				floatRate = std::stod(forecastCurve);

				floatRates[i] = floatRate;

				continue;
			}

			auto fixingDate = cashflow->getFixingDate();
			auto accrualDayCount = cashflow->bespokeInfo().accrualDaycount;
			const std::string fixingCalendar = schedule_->getFixingCalendar().getCString();
			auto fixingBusinessDayAdj = schedule_->getFixingBusinessDayAdj();

			auto fixingType = getFixingTypeByFixingDate(fixingDate, cashflow->getAccrualStartDate(), cashflow->getAccrualEndDate(), fixingBusinessDayAdj, fixingCalendar, fixingDateToleranceTenor);

			if (convexityMethod != NONE_CONVEXITY && fixingType != IN_ADVANCE_FIXING)
			{
				MLIB_REQUIRE(volatilityModel != nullptr, "Unnatural Fixing requires a convexity adjustment - Please provide a volatility model.")
			}

			auto accrualYearFraction = cashflow->getAccrualYearFraction();

			//Only call stub rate when the cashflow's ForcastCurve is NATURAL
			bool irregularStub = same(forecastCurve, "NATURAL") || same(forecastCurve, "UNNATURAL");

			// 2) use stub rate if the forecastCurve is NATRUAL, 
			if (irregularStub)
			{
				// For non-last cashflows 
				LADate fixingEndDate;

				// The forcaseCurve can be 'UNNATURAL', 'NATURAL', or a standar curveIndex. 
				// *** Note the 'UNNATURAL' is only a flag to identify whether to use AccrualEnd or nextFixingDate ***
				if (i < cashflowSize - 1)
				{
					auto nextCashflow = schedule_->getCashflow(i + 1);
				
					//When current cashflow and next cashflow have different frequencies, e.g. changing from MONTHLY to QUARTLY, the accrualEnd is used.
					if (same(forecastCurve, "UNNATURAL"))
					{
						fixingEndDate = cashflow->getAccrualEndDate();
					}
					//When current cashflow and next cashflow have the same frequency, e.g. MONTHLY, then next fixing date is used
					else
					{
						fixingEndDate = nextCashflow->getFixingDate();
					}
				}
				// For the last cashflow, use MaturityDate;
				else
				{
					fixingEndDate = cashflow->getAccrualEndDate();
				}

				// The core stub rate function only accept forcastCurve as either 'NATURAL', or a standard curveIndex. 
				// Since when forecastCurve is a standard curveIndex, we will use the libor rate from curve instead, so we 'NATURAL' for stub rate here
				std::string forecastCurveForStubRate = "NATURAL";

				auto stubRate = calculateStubRateFromFixingStartEnd(fixingDate, fixingEndDate, curveCollection, forecastCurveForStubRate.c_str(), curveIndices, curveTenors, accrualDayCount, fixingCalendar.c_str(), fixingBusinessDayAdj);

				// Convexity Adjustment using fixingDates and fixingEnd dates, BB is using AccrualEndDates
				floatRate = getCurveForwardRateWithConvexityAdjustment(stubRate, fixingDate, fixingEndDate, valuationDate, accrualDayCount, fixingType, volatilityModel, convexityMethod, curveCollection.getCString(), OIS_CURVE);
			}
			// 3) use libor rate if the forecastCurve is a valid curve index
			else
			{
				CurveTypeEnum curveType = toCurveTypeEnum(getCurveType(curveCollection, getCurveStaticDataTableName(curveCollection, forecastCurve.c_str())).c_str());

				MLIB_REQUIRE( !(curveType == OIS_CURVETYPE || curveType == ARR_CURVETYPE), "VNS from cashflow does not support OIS, ARR as the ForecastCurve.")

				LAString crvFreqTenor = validateCurveAndGetCurveFrequency(curveCollection, forecastCurve.c_str());

				LADate fixingEndDate = LADateScheduleHelpers::getDate(fixingDate, crvFreqTenor, toString(fixingBusinessDayAdj).c_str(), fixingCalendar.c_str());

				// get floatRate from the curve
				const double unadjustedFwdRate = getCurveForwardRatesFromForwardDates(boost::assign::list_of(fixingDate), boost::assign::list_of(fixingEndDate), curveCollection, forecastCurve.c_str(), legStaticData_->getFwdInter(), fixingBusinessDayAdj)[0];

				// Convexity Adjustment using fixingDates and fixingEnd dates, BB is using AccrualEndDates
				const FloatRateData fwdRate = getCurveForwardRateWithConvexityAdjustment(unadjustedFwdRate, fixingDate, fixingEndDate, valuationDate, accrualDayCount, fixingType, volatilityModel, convexityMethod, curveCollection.getCString(), OIS_CURVE);
					
				//Handle rates in the past fixingTable
				floatRate = getForwardRate(fwdRate, valuationDate, fixingDate, cashflow->getPaymentDate(), fixingTable);
			}

			floatRates[i] = floatRate;
		}

		return floatRates;
	}


	std::unordered_set<CashflowHeaderEnum,EnumClassHash> FloatLeg::allowedColumns() const
	{
			std::unordered_set<CashflowHeaderEnum,EnumClassHash> expectedList
			{
				FLOAT_RATE_HEADER
				, FX_FIXING_DATE_HEADER
				, FX_RATE_HEADER
				, COUPON_HEADER
				, DISCOUNT_FACTOR_HEADER
				, COUPON_PV_HEADER

				, SURVIVAL_PROBABILITY_HEADER
				, MARGINAL_DEFAULT_PROBABILITY_HEADER
				, RISKY_COUPON_HEADER

				, CONVEXITY_HEADER
				, UNADJUSTED_FLOAT_RATE_HEADER
			};

			return expectedList;
	}

	PaymentTriggerEnum FloatLeg::getPaymentTrigger() const
	{
		return paymentTrigger_;
	}



}


