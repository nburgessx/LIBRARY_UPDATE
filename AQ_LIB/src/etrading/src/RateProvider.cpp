#include "RateProvider.h"
#include "CommonConstants.h"
#include "CurveUtilities.h"
#include "CurveValidation.h"
#include "LACurvePricingObject.h"
#include "SwapValidation.h"
#include "ParameterValidation.h"
#include "CurveInstrumentPricing.h"
#include "LACurveForwardRateHelpers.h"

namespace etrading
{

	RateProvider::RateProvider(const std::string& curveCollection, const std::shared_ptr<FixingTable>& fixingTable) : curveCollection_(curveCollection), fixingTable_(fixingTable)
	{
		if (!curveCollection.empty())
		{
			checkIfCurveExists(getDataInstance(), curveCollection.c_str());
		}
	}

    RateProvider::RateProvider(const RateProvider& rhs) : curveCollection_(rhs.curveCollection_)
	{
		if (rhs.fixingTable_ != nullptr)
		{
			fixingTable_ = rhs.fixingTable_->clone();
		}
	}

	const AQLDate RateProvider::asOfDate() const
	{
		return getCurveAsOfDate(curveCollection_.c_str());
	}

	//From payment dates to asOfDate of the curve
	const DoubleVector RateProvider::discountFactors(const std::shared_ptr<ScheduleParameters>& schParams, const DateVector& paymentDates) const
	{
		//The method will throw exception if the curve has not been built.
		return getCurveDiscountFactors(paymentDates, curveCollection_.c_str(), schParams->discountCurve().c_str());
	}

	const DoubleVector RateProvider::liborRates(const std::shared_ptr<ScheduleParameters>& schParams, const std::shared_ptr<EnrichedSchedule>& schOutput) const
	{
		const DateVector fixingDates = schOutput->fixingDates();
		const DateVector paymentDates = schOutput->paymentDates();
		const auto forecastCurve = schParams->forecastCurve();

		const AQLDate asOfDt = asOfDate();
		size_t firstNonpastFixingDateIndex = getFirstNonpastDateIndex(fixingDates, asOfDt);

		const auto forecastCurveMarketName = (forecastCurve.size() != 0) ? getCurveStaticDataTableName(curveCollection_.c_str(), forecastCurve.c_str()) : "";
		const std::string interpolation = getCurveInterpolation(curveCollection_.c_str(), forecastCurveMarketName).getCString();
		
		CurveTypeEnum curveType = toCurveTypeEnum(getCurveType(curveCollection_.c_str(), forecastCurveMarketName).c_str());

		const bool isOISOrARR = (curveType == OIS_CURVETYPE || curveType == ARR_CURVETYPE);

		DoubleVector floatRates;
		size_t fixingDatesSize = fixingDates.size();
		floatRates.reserve(fixingDatesSize);

		// If it is OIS float leg, and CouponCompoundMethod is specified, call calculateOisFloatRates()
		if (isOISOrARR && schParams->couponCompoundMethod() != NONE_COMPOUNDING_METHOD)
		{
			// If no fixing table is specified
			fixingTableExistenceValidation(fixingTable_, asOfDt, fixingDates, paymentDates);

			// Get equivalent rate over accrual periods
			firstNonpastFixingDateIndex = getFirstNonpastDateIndex(fixingDates, asOfDt);

			floatRates = calculateOisFloatRates(firstNonpastFixingDateIndex, interpolation, schParams, schOutput);

		}
		// If 1) it is OIS float leg, and CouponCompoundMethod is NOT specified, or 2) a normal float leg
		else
		{
			// Get Fixing Information needed for Stub Calculations

			bool hasFirstFixing = useFloatFixing(schParams->firstFixing());
			bool hasLastFixing = useFloatFixing(schParams->lastFixing());

			const auto firstStubCurveIndex = schParams->firstStubCurveIndex();
			const auto lastStubCurveIndex = schParams->lastStubCurveIndex();

			const std::string crvFreqTenor = validateCurveAndGetCurveFrequency(curveCollection_.c_str(), forecastCurve.c_str()).getCString();

			if (!isOISOrARR && getFrequencyOrTenorMonth(crvFreqTenor.c_str()) != getFrequencyOrTenorMonth(toString(schParams->accrualFrequency()).c_str()))
			{
				throw AQLCoreInvalidData("#Error: Accrual Frequency is not the same as Curve Frequency.", __FILE__, __LINE__);
			}

			// *** Special treatment for OIS leg without compoundMethod: use leg's accrual freq instead of curve's freq to calculate Forward Rates
			CurveTenorEnum curveFreqTenorOverride = NONE_CURVE_TENOR;
			if (isOISOrARR)
			{
				//For long-end OIS, it does not use annual compounding on daily rate(i.e. CouponCompoundMethod is NOT specified), so accrualFreq rather than curveFreq is used to calculat the fixingEndDate.
				curveFreqTenorOverride = toCurveTenorEnum(getFrequencyTenor(schParams->accrualFrequency()).getCString());
			}

			//Both past and non-past forward rates from the curve
			const DoubleVector fwdRates = getCurveForwardRates(fixingDates, curveCollection_.c_str(), forecastCurve.c_str(),
				schParams->fixingbusinessDayAdj(), schParams->fixingCalendar().c_str(), schParams->fwdInter(), curveFreqTenorOverride);

			//// update the leg's fwdInter flag so that we/the test will know what is actually used
			//bool isFwdInter = getfwdInterInfo(curveCollection, forecastCurveMarketName_, schParams->fwdInter_).isFwdInter;
			//schParams->fwdInter_ = (isFwdInter ? TRUE_BOOL : FALSE_BOOL);

			const std::pair<AQLStringVector, AQLStringVector> curvesInfo = getStubRateCurveIndicesTenors(curveCollection_.c_str());
			const auto curveIndices = curvesInfo.first;
			const auto curveTenors = curvesInfo.second;

			//Check if it's front stub or end stub
			bool frontStub = isFrontStub(schParams->stubType());

			//Only call stub rate when the leg has irregular tenor
			const bool irregularStub = schOutput->isIrregularStub();

			for (size_t i = 0; i < fixingDatesSize; ++i)
			{
				double floatRate = 0.0;
				const auto fixingDate = fixingDates[i];
				const auto paymentDate = paymentDates[i];

				//
				// First Fixing Logic:
				// The first fixing should be applied to the current or latest fixing. However in some cases we have a more than one fixing date in the past
				// with a corresponding payment date in the future. This code block ensures that when we have multiple fixings that we apply the first fix override
				// to the most recent fixing. Note when we have mutliple fixings the previous fixings will need to come from the fixing table.
				//
				bool isCurrentFixing = false;

				//This make sure the first fixing field is always applied when the fixingDate is not in the past
				if (fixingDate > asOfDt)
				{
					isCurrentFixing = true;
				}
				else if (fixingDate <= asOfDt && paymentDate >= asOfDt)
				{
					// Check for mutliple fixings
					if (i < fixingDatesSize - 1)
					{
						const auto nextFixingDate = fixingDates[i + 1];
						const auto nextPaymentDate = paymentDates[i];

						// Check if the next fixing is a more recent fixing
						if (nextFixingDate <= asOfDt && nextPaymentDate >= asOfDt)
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
				if (hasFirstFixing && isCurrentFixing)
				{
					// Use the first fixing only once, after that the last fixing wil be used, if present
					hasFirstFixing = false;
					floatRate = schParams->firstFixing();
				}
				// 2. Last Fixing - only apply to the final cashflow
				// Exception: If there is only one cashflow, apply the first fixing to that cashflow
				else if (hasLastFixing && i == fixingDatesSize - 1)
				{
					// Use the last fixing only once
					hasLastFixing = false;
					floatRate = schParams->lastFixing();
				}
				// 3a) Front Stubs - note for regular swaps there is no stub and the front stub will match the first float rate 
				// Front stubs are always applied to the first cashflow i.e. i=0
				// Do not call the stub calculation function if the curveIndicesSize is one or less
				else if (frontStub && irregularStub && i == 0 && curveIndicesSize > 1)
				{
					// If a front stub exists, has fixed already, is the current fixing and has not yet been paid out then the firstFixingRate must be provided
					if (fixingDate < asOfDt && paymentDate >= asOfDt && !hasFirstFixing)
					{
						throw AQLCoreInvalidData("#Error: The float leg 'firstFixing' is required for the front stub rate.", __FILE__, __LINE__);
					}

					floatRate = calculateStubRate(firstStubCurveIndex, curveIndices, curveTenors, crvFreqTenor, interpolation, schParams, schOutput);
				}
				// 3b) Back Stubs - note for regular swaps there is no stub and the back stub will match the first float rate 
				// Back stubs are always applied to the last cashflow i.e. i=fixingDatesSize-1
				// Do not call the stub calculation function if the curveIndicesSize is one or less
				else if (!frontStub && irregularStub && i == fixingDatesSize - 1 && curveIndicesSize > 1)
				{
					// Back-Stub Issue:
					// ----------------
					// We can't evaluate back stubs exactly on the fixing date because of the artifical extra fixing date in the FloatLeg.cpp 
					// calculateStubRate function. Adding the extra day with fixing.addDays(-1) inserts a date in the past and we have to get a 
					// discount factor on that date, which is not possible. This is done for back stubs only.
					// Therefore throw and error to request the fix be input by the end user. T

					// If a back stub exists, has fixed already, is the current fixing and has not yet been paid out then the lastFixingRate must be provided
					if (fixingDate <= asOfDt && paymentDate >= asOfDt && !hasLastFixing && !hasFirstFixing)
					{
						throw AQLCoreInvalidData("#Error: The float leg 'lastFixing' is required for the back stub rate.", __FILE__, __LINE__);
					}

					floatRate = calculateStubRate(lastStubCurveIndex, curveIndices, curveTenors, crvFreqTenor, interpolation, schParams, schOutput);
				}
				// 4 and 5. Fixing Table for Past Fixings and Regular Libor Forecast Rate from Yield Curve for Future Fixings
				else
				{
					floatRate = getForwardRate(fwdRates[i], asOfDt, fixingDate, paymentDate, fixingTable_).resetRate;
				}

				floatRates.push_back(floatRate);
			}
		}

		return floatRates;
	}

	// Produce a compounding rate for each of the accrual periods
	const DoubleVector RateProvider::calculateOisFloatRates(size_t firstNonpastFixingDateIndex, const std::string& interpolation, const std::shared_ptr<ScheduleParameters>& schParams, const std::shared_ptr<EnrichedSchedule>& schOutput) const
	{
		// Get equivalent rates over accrual periods

		const auto forecastCurve = schParams->forecastCurve();

		const auto fixingEndDates = schOutput->fixingEndDates();

		// Get the equivalent rate of each accrual period. 
		// The equivalent rate is obtained either through daily compounding over this period or finding the arithmetic average.
		if (schParams->accrualbusinessDayAdj() == NONE_BUSINESS_DAY_ADJ || schParams->accrualCalendar().size() == 0)
		{
			throw AQLCoreInvalidData("#Error: OIS average rate cannot be calculated without AccrualbusinessDayAdj or AccrualCalendar, please update the float leg schedule", __FILE__, __LINE__);
		}

		const size_t expectedSize = fixingEndDates.size();

		//Get float rates using compounding method:
		DoubleVector floatRates(expectedSize);

		const std::string compoundFrequency = "Business_Days";
		// const bool isStartRoll = false; <--- Unused Variable

		const auto internalCompoundMethodType = validateCompoundingMethod(toString(schParams->couponCompoundMethod()));

		const double oneBasisPoint = 0.0001;

		//Call compoundRate function once when all cashflows have same spread 
		if (!schOutput->isVariableSpread())
		{
			const double spread = schParams->spread() * oneBasisPoint;

			floatRates = calculateCompoundRateWithFixingTable(schOutput->fixingDates(),
				fixingEndDates,
				curveCollection_.c_str(),
				forecastCurve.c_str(),	// The forecast curve given by user is expected to be an OIS curve
				compoundFrequency.c_str(),
				spread,
				schParams->stubType(),
				schParams->rollDayInput().c_str(),
				schParams->fixingCalendar().c_str(),
				schParams->fixingbusinessDayAdj(),
				schParams->accrualDaycount(),
				interpolation.c_str(),
				internalCompoundMethodType.c_str(),
				schParams->firstStub().c_str(),
				schParams->lastStub().c_str(),
				schParams->fwdInter(),
				fixingTable_,
				true);

			////Set spread to zero, as the calculated compoundFloatRate includes the spread
			//for (size_t i = 0; i < schOutput->spreads_.size(); ++i)
			//{
			//	schOutput->spreads_[i] = 0.0;
			//}
		}
		//Call the compoundRate function multiple times when cashflows have different spreads 
		else
		{
			for (size_t i = 0; i < expectedSize; ++i)
			{
				const double spread = schOutput->spreads()[i] * oneBasisPoint;

				const AQLDate fixingDate = schOutput->fixingDates()[i];
				const AQLDate fixingEndDate = fixingEndDates[i];

				const double floatRate = calculateCompoundRateWithFixingTable(boost::assign::list_of(fixingDate),
					boost::assign::list_of(fixingEndDate),
					curveCollection_.c_str(),
					forecastCurve.c_str(),	// The forecast curve given by user is expected to be an OIS curve
					compoundFrequency.c_str(),
					spread,
					schParams->stubType(),
					schParams->rollDayInput().c_str(),
					schParams->fixingCalendar().c_str(),
					schParams->fixingbusinessDayAdj(),
					schParams->accrualDaycount(),
					interpolation.c_str(),
					internalCompoundMethodType.c_str(),
					schParams->firstStub().c_str(),
					schParams->lastStub().c_str(),
					schParams->fwdInter(),
					fixingTable_,
					true).at(0);

				floatRates[i] = floatRate;

				//schOutput->spreads_.emplace_back(0.0);

			}
		}

		return floatRates;
	}


	const double RateProvider::calculateStubRate(const std::string& stubCurveIndex, const AQLStringVector& curveIndices, const AQLStringVector& curveTenors, const std::string& indexFrequency, const std::string& interpolation,
										const std::shared_ptr<ScheduleParameters>& schParams, const std::shared_ptr<EnrichedSchedule>& schOutput) const
	{

		// Choose the nearest curve when no explicit curve has been chosen to use 
		bool useNearestCurve = same(stubCurveIndex, "NATURAL") ? true : false;

		std::string useStubCurveIndex = stubCurveIndex;
		if (!useNearestCurve)
		{
			// The user specified stubCurveIndex may not be the same name as curveIndices, so we need to find the matching one within the curveIndices, by matching the staticDataTable
			useStubCurveIndex = getMatchingCurveIndex(curveCollection_.c_str(), stubCurveIndex.c_str(), curveIndices).getCString();
		}

		//Default the tolerance to 0D
		const std::string toleranceTenor = "0D";

		//For stub rate, fwdInter always false
		const FwdInterInfo info = etrading::getfwdInterInfo("", "", etrading::FALSE_BOOL);
		const bool isFwdInter = info.isFwdInter;
		const bool useFwdData = info.useFwdData;

		auto fixingDates = schOutput->fixingDates();

		const bool INCLUDE_LAST_FIXING = true;
		auto lastFixingEndDate = validateAndGenerateFixingSchedule(boost::assign::list_of(schOutput->accrualStartDates().back())(schOutput->accrualEndDates().back()),
			toString(schParams->fixingbusinessDayAdj()).c_str(),
			schParams->fixingCalendar(),
			schParams->fixLag(),
			"advance",
			INCLUDE_LAST_FIXING).at(1);

		fixingDates.push_back(lastFixingEndDate);

		auto effectiveDate = schOutput->accrualStartDates().front();
		auto unadjustedMaturityDate = validateMaturityDate(effectiveDate, schParams->accrualEndDateOrTenor());
		AQLString rollConv = getRollConvection(effectiveDate, unadjustedMaturityDate, schParams->rollDayInput());

		const double ret = CurveInstrumentPricing::getStubRate( fixingDates,
																curveIndices,
																curveTenors,
																DoubleVector(), //tenorCurveFixings, remove from user input so that the fixing of the curves will be used
																curveCollection_.c_str(),
																toString(schParams->stubType()).c_str(),
																interpolation.c_str(),
																toString(schParams->accrualDaycount()).c_str(),
																schParams->fixingCalendar().c_str(),
																toString(schParams->fixingbusinessDayAdj()).c_str(),
																rollConv,
																useNearestCurve,
																isFwdInter,
																useFwdData,
																toleranceTenor.c_str(),
																useStubCurveIndex.c_str(),
																getFrequencyString(indexFrequency.c_str()) ).stubRate_;

		return ret;
	}

	

}

