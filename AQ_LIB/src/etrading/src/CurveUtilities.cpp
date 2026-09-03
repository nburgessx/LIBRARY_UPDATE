#include "CurveUtilities.h"
#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>     // Include for boost::iequals case insensitive comparison

#include "CurveCalibrationData.h"
#include "ConvexityModel.h"
#include "LACoreComponentManager.h"
#include "CurveBuildDefaults.h"
#include "CurveResultsContainer.h"  // Curve Results and Discount Factor Methods
#include "CurveValidation.h"
#include "InitializeMLibETrading.h"
#include "LACurveForwardRateHelpers.h"
#include "LACurvePricingObject.h"
#include "LADateHelpers.h"
#include "LADateScheduleHelpers.h"
#include "ParameterValidation.h"
#include "LWOUtilities.h"
#include "CommonConstants.h"

#include "LADefinitions.h"
#include "LADataBasics.h"
#include "LADataReference.h"
#include "LADataMatrix.h"
#include "LAPriceDataCalendar.h"
#include "LAPriceDataSlidingRule.h"
#include "LAMathDefine.h"
#include "LAMathInterpolationUtilities.h"
#include "LACompoundingFunc.h"
#include "LAMathDateUtilities.h"
#include "LAEnumConversion.h"				// include toLADaycount and toLARateConvention

namespace etrading
{
	// These are schedule parameters which are not yet part of the curve configuration
	namespace SCHEDULE_CONSTANTS
	{
		const std::string LAG_0D = "0D";
		const std::string LAG_2D = "2D";
		const std::string SHORT_END_STUB = "SHORTEND";
	}

	// Enum for Integration Methods
	enum IntegrationDaycountAdjustment { UNCHANGED, ADJUST_INPUTS, ADJUST_OUTPUTS };
	
    /* @brief			return the day count used in the core library
    */
    LAString getDiscountFactorDayCount()
    {
        return LAString( "ACT/365" );
    }

    bool isBasisFlagForDiscountFactor()
    {
        return false;
    }

    /* @brief			populate and setting default values
    *  @param [in]		curveCollection	curve collection
    *  @param [inout]	crvIndex		curve index
    *  @param [inout]	interpolation	interpolation
    */
    void populateDiscountFactorConventions( const LAString& curveCollection, const LAString& crvIndex, LAString& interpolation )
    {
        LAString curveIndex = getDefaultValueForEmptyString( crvIndex, "OIS" );

        //Throw exception if the curve has not been built.
        LAString staticDataTable = getCurveStaticDataTableName( curveCollection, curveIndex );

        interpolation = getCurveInterpolation( curveCollection, staticDataTable );
    }

    /* @brief			populate and setting default values
    *  @param [in]		curveCollection	curve collection
    *  @param [inout]	curveIndex		curve index
    *  @param [inout]	interpolation	interpolation
    *  @param [inout]	businessDayAdj	business day adjustment
    *  @param [inout]	calendar		calendar
    */
    void populateDiscountFactorConventions( const LAString& curveCollection, const LAString& curveIndex, LAString& interpolation, LAString& businessDayAdj, LAString& calendar )
    {
        populateDiscountFactorConventions( curveCollection, curveIndex, interpolation );

        businessDayAdj = getDefaultValueForEmptyString( businessDayAdj, "NO_CHANGE" );
        calendar = getDefaultCalendarForEmptyString( calendar, curveCollection );
    }

	// Function to get the curve Daycount as an LAString
	LAString getCurveDaycountAsLAString( const LAString& curveCollection, const LAString& curveIndex )
	{
		LAString daycount = validateCurveAndGetFloatDaycount(curveCollection, curveIndex);
		daycount = getDefaultValueForEmptyString( daycount, "ACT/360" ); // USD forward rates are based on Act/360, most markets follow suit
		return daycount;
	}

	// Function to get the curve Daycount as a std::string
	std::string getCurveDaycountAsStandardString( const std::string& curveCollection, const std::string& curveIndex )
	{
		std::string daycount = getCurveDaycountAsLAString( curveCollection.c_str(), curveIndex.c_str() ).getCString();
		return daycount;
	}

	
	/* @brief			Get the curve compound freq when calculating forward rates from the curve
	*  @return			The curve compound freq
	*/
	CompoundingFrequencyEnum getForwardRateCurveCompoundFrequency()
	{
		//SIMPLE as default for forward rates (including OIS forwards)
		return SIMPLE_COMPOUNDING;
	}


    /* @brief			populate and setting default values
    *  @param [in]		curveCollection	Curve collection
    *  @param [in]		crvIndex		Curve index
    *  @param [in]		fwdInter		User specified fwdInter flag
    *  @param [out]		interpolation	Interpolation
    *  @param [inout]	businessDayAdj	Business day adjustment
    *  @param [inout]	calendar		Calendar
    *  @param [inout]	dayCount		Day count
    *  @param [out]		isStdCurve		True if the curve is STD
    */
    void populateForwardRateConventions( const LAString& curveCollection,
                                         const LAString& crvIndex,
										 const BooleanEnum& fwdInter,
                                         LAString& interpolation,
                                         LAString& businessDayAdj,
                                         LAString& calendar,
                                         LAString& dayCount,
                                         bool& isFwdInter,
                                         bool& useFwdData )
    {
        LAString curveIndex = getDefaultValueForEmptyString( crvIndex, "STD" );

        //Throw exception if the curve has not been built.
        LAString staticDataTable    = getCurveStaticDataTableName( curveCollection, curveIndex );
        
        // If user has not specified the fwdInter flag, then use the default one
		FwdInterInfo info   = getfwdInterInfo( curveCollection, staticDataTable, fwdInter );
		isFwdInter          = info.isFwdInter;
		useFwdData          = info.useFwdData;

        interpolation       = getCurveInterpolation( curveCollection, staticDataTable );
		businessDayAdj      = getDefaultValueForEmptyString( businessDayAdj, "MOD_FOLLOWING" );
        calendar            = getDefaultCalendarForEmptyString( calendar, curveCollection );
        dayCount            = getCurveDaycountAsLAString( curveCollection, curveIndex );
    }

	/* @brief	Generate a fixing schedule with conventions obtained from the curve.
	*  @param[in]	curveCollection				The curve collection
	*  @param[in]	curveIndex					The curveIndex used to get conventions used to build the schedule
	*  @param[out]	curveTenorEnum				Output: The curve tenor e.g. 3M
	*  @param[out]	asOfDate					Output: The curve asOfDate
	*  @param[out]	curveDayCount				Output: The curve dayCount
	*  @param[out]	schedulePaymentDates		Output: The generated schedule payment dates
	*  @param[out]	scheduleAccrualStartDates	Output: The generated schedule accrual start dates
	*  @param[out]	scheduleAccrualEndDates		Output: The generated schedule accrual end dates
	*  @param[out]	scheduleFixingStartDates	Output:	The generated schedule fixing start dates
	*  @param[out]	scheduleFixingEndDates		Output:	The generated schedule fixing end dates
	*/
	void generateScheduleForZeroDiscountMargin( const std::string & curveCollection,
												const std::string & curveIndex,
												CurveTenorEnum &curveTenorEnum,
												LADate &asOfDate,
												std::string &curveDayCount,
												DateVector &schedulePaymentDates,
												DateVector &scheduleAccrualStartDates,
												DateVector &scheduleAccrualEndDates,
												DateVector &scheduleFixingStartDates,
												DateVector &scheduleFixingEndDates )
	{

		std::shared_ptr<CurveResults> curveResults = CurveResultsContainer::getInstance().getCurveResults(curveCollection, curveIndex );
		MLIB_REQUIRE( curveResults != nullptr, "CurveResults missing for curveCollection: " << curveCollection << " and curveIndex: " << curveIndex );

		std::shared_ptr<DiscountFactorResults> discountFactorResults = curveResults->discountFactorResults();
		MLIB_REQUIRE( discountFactorResults != nullptr, "DiscountFactor results missing for curveCollection: " << curveCollection << " and curveIndex: " << curveIndex );

		VectorDate discountFactorResultsPaymentDates = discountFactorResults->paymentDates();
		MLIB_REQUIRE( discountFactorResultsPaymentDates.size() > 0, "No payment date pillar points in curve." );

		// Read curve description parameters from curveResults object
		const std::shared_ptr<CurveDescription> curveDescription = curveResults->curveDescription();
		CurveTypeEnum curveType = curveDescription->curveTypeEnum();
		MLIB_REQUIRE( curveType == SWAP_CURVETYPE || curveType == TENORBASIS_CURVETYPE, "Only Swap / TenorBasis Libor curves are currently supported for the zero-discount margin calculation." );

		curveTenorEnum                   = curveDescription->curveTenorEnum();
		const std::string curveTenor     = curveDescription->curveTenor();
		const std::string curveFrequency = toFrequencyFromCurveTenor( curveTenor );

		asOfDate = discountFactorResults->asOfDate();

		/* Construct a schedule endDate which an exact whole number of years after asOfDate.
		* Use the last payment date in the discount factors as the approximate curve end date
		*/
		LADate curveLastPaymentDate = discountFactorResultsPaymentDates.back();
		const double curveTerm      = convertCurveDateToTerm( asOfDate, curveLastPaymentDate );
		const int curveTenorYears   = roundToNearest( curveTerm );
		LADate endDate = asOfDate;
		endDate.addYears( curveTenorYears );

		// Read conventions from curveResults object
		const LabelValueBlock& swapConventions = curveResults->curveConventionsAndMarketData()->swapCurveData()->swapConvLVB_;
		curveDayCount                          = swapConventions.getCompulsoryValueAsString( CURVEGENERATOR_SWAPS_KEY::DAYCOUNT_FLOAT ); // e.g. ACT/360
		const std::string businessDayAdj       = swapConventions.getCompulsoryValueAsString( CURVEGENERATOR_SWAPS_KEY::SLIDING_RULE );   // e.g. MOD_FOLLOWING
		const std::string calendar             = swapConventions.getCompulsoryValueAsString( CURVEGENERATOR_SWAPS_KEY::CALENDAR );       // e.g. TGT

		// Hard coded parameters. These parameters are not yet part of the curve configuration.
		const std::string payLag   = SCHEDULE_CONSTANTS::LAG_0D;
		const std::string fixLag   = SCHEDULE_CONSTANTS::LAG_2D;
		const std::string stubType = SCHEDULE_CONSTANTS::SHORT_END_STUB;  // Place any stub in our artificial schedule out of the way, at the end of the curve

		// Unset schedule parameters.
		const std::string firstStub;
		const std::string lastStub;
		const std::string rollDayInput;
		const std::string fixingAdvanceOrArrears;

		/*  Generate an accrual schedule with fixing start and end dates
		 *  First generate accrual and payment dates matching the curve frequency.
		 */
		DateVector scheduleAccrualDates;
		validateAndGenerateAccrualAndPaymentSchedules(  scheduleAccrualDates,
														schedulePaymentDates,
														asOfDate,
														endDate,
														curveFrequency,
														businessDayAdj,
														calendar,
														curveFrequency,
														businessDayAdj,
														calendar,
														payLag,
														stubType,
														firstStub,
														lastStub,
														rollDayInput,
														fixingAdvanceOrArrears );

		/* Construct vectors of accrual start and end dates for each accrual period
		 */
		validateAndGenerateAccrualStartAndEndDates( scheduleAccrualStartDates, scheduleAccrualEndDates, scheduleAccrualDates );

		/* Construct vectors of fixing start and end dates
		 */
		scheduleFixingStartDates = validateAndGenerateFixingSchedule( scheduleAccrualDates,
																	  businessDayAdj,
																	  calendar,
																	  fixLag,
																	  fixingAdvanceOrArrears );

		scheduleFixingEndDates = LADateScheduleHelpers::getMultiDate( scheduleFixingStartDates, curveTenor, businessDayAdj, calendar, nullptr ); // rollconvention* = nullptr
	}

	/* @brief			Calculates the discount factors from a curve and a spread.
	*					The discount factors are calculated using the zero discount margin approach
	*					described by O'Kane in "Credit Spreads Explained".
	*
	*  @param [in]		paymentDates	A vector of payment dates for which discount factors are required
	*  @param [in]		curveCollection	Curve set ID
	*  @param [in]		curveIndex		The curve index for which to calculate Discount Factors. Default to OIS
	*  @param [in]		spread			Spread to be added to the curve forward-rate when calculating the discount factors.
	*  @param [in]		fixingTableName An optional fixing table name. This field may be blank if no fixings are required.
	*  @return			A vector of discount factors
	*/
	DoubleVector calculateDiscountFactorsUsingZeroDiscountMarginApproach( const DateVector& paymentDates,
																		  const std::string& curveCollection,
																		  const std::string& curveIndex,
																		  const double& spread,
																		  const std::string& fixingTableName )
	{

		/* Generate a fixing schedule with conventions obtained from the curve
		 */
		LADate asOfDate;
		CurveTenorEnum curveTenorEnum;
		std::string curveDayCount;

		DateVector schedulePaymentDates;
		DateVector scheduleAccrualStartDates;
		DateVector scheduleAccrualEndDates;
		DateVector scheduleFixingStartDates;
		DateVector scheduleFixingEndDates;
		generateScheduleForZeroDiscountMargin(  curveCollection,
												curveIndex,
												curveTenorEnum,
												asOfDate,
												curveDayCount,
												schedulePaymentDates,
												scheduleAccrualStartDates,
												scheduleAccrualEndDates,
												scheduleFixingStartDates,
												scheduleFixingEndDates );

		/* Validate that the fixing table (if provided) is consistent with the forecast curve
		*/
		auto fixingTable = getFixingTable( fixingTableName, false /* do not throw when missing*/);
		if ( fixingTable != nullptr )
		{
			const LAString& curveCurrency = getCurveCurrency( curveCollection );

			MLIB_REQUIRE( fixingTable->getCurrency() == toCCYEnum(curveCurrency.getCString() ), "FixingTable currency does not match the forecast curve currency." );
			MLIB_REQUIRE( fixingTable->getCurveTenor() == curveTenorEnum, "FixingTable curve frequency tenor does not match the curve frequency tenor." );
		}

		LAPriceDataDayCount priceDataDayCount;
		priceDataDayCount.convertFromString( curveDayCount );
		LAPriceDataConvention convention( priceDataDayCount.getDayCount(), SIMPL );

		double prevDiscountFactorWithSpread = 1.0;      // At curve asOfDate, the discount factor is 1.0;
		LAString tmpDayCount( curveDayCount.c_str() );  // Non-const LAString Needed for LAMathDateUtilities::getTerm()

		/* Iterate over the schedule payment dates and calculate discount factors by applying a spread to the forward rate in each schedule period
		 */
		std::vector<double> scheduleDiscountFactors(schedulePaymentDates.size());

		std::shared_ptr<CurveResults> curveResults = CurveResultsContainer::getInstance().getCurveResults(curveCollection, curveIndex );
		std::shared_ptr<DiscountFactorResults> discFactorResults = curveResults->discountFactorResults();
		for (size_t i = 0; i < scheduleFixingStartDates.size(); ++i)
		{
			const LADate fixingStartDate = scheduleFixingStartDates[i];
			const LADate fixingEndDate   = scheduleFixingEndDates[i];

			double impliedFwdRate = 0.0;
			if ( fixingStartDate < asOfDate )
			{
				MLIB_REQUIRE( fixingTable != nullptr, "Fixing Table required for fixing date " << fixingStartDate.convertDateToString() );
				impliedFwdRate = fixingTable->getFixingValue( toGregorianDateFromLADate( fixingStartDate ) );
			}
			else
			{
				// Legacy Parameter: isFwdInter = false
				impliedFwdRate = discFactorResults->implyForwardRate( fixingStartDate, fixingEndDate, false, SIMPLE_COMPOUNDING );
			}

			/*
			* Here we apply the zero discount-margin spread to the forward rate
			*/
			const double forwardRatePlusSpread = impliedFwdRate + spread;

			// Calculate the accrual period year fraction
			const LADate accrualStartDate = scheduleAccrualStartDates[i];
			const LADate accrualEndDate = scheduleAccrualEndDates[i];
			const double yearFraction = LAMathDateUtilities::getTerm( accrualStartDate, accrualEndDate, tmpDayCount );

			/* Calculate the period rate of return implied from ( forward rate + spread )
			*  and then calculate the total discount factor
			*/
			const double returnWithSpread = LAPriceDataConvention::rateToRet( forwardRatePlusSpread, yearFraction, convention );

			// Guard against division by zero
			MLIB_REQUIRE(returnWithSpread != 0.0, "Period return is zero at fixing date: " << fixingStartDate.convertDateToString() );

			const double discountFactorWithSpread = prevDiscountFactorWithSpread / returnWithSpread;
			scheduleDiscountFactors[i] = discountFactorWithSpread;

			prevDiscountFactorWithSpread = discountFactorWithSpread;
		}

		/* Now create a discountFactorResults object with the newly calculated grid of schedule payment dates and discount factors.
		 * Use this to interpolate a discount factor for each supplied date.
		 */
		std::shared_ptr<DiscountFactorResults> spreadDiscountFactorResults( new DiscountFactorResults( *discFactorResults, schedulePaymentDates, scheduleDiscountFactors ) );

		DoubleVector discountFactors( paymentDates.size() );
		for ( size_t i = 0; i < paymentDates.size(); i++ )
		{
			const LADate paymentDate = paymentDates[i];
			discountFactors[i] = spreadDiscountFactorResults->getDiscountFactor( paymentDate );
		}

		return discountFactors;
	}
	

    /* @brief			validation method for meCurveDiscountFactors
	*                   NOTE: Payment dates have already been adjusted for business days and holidays
	*                   so we do not apply the same adjustments again. Hence we do not accept
	*                   a businessDayAdjustment or Calendar as input to this function.
    *  @param [in]		paymentDates	A vector of payment dates for which discount factors are required
    *  @param [in]		curveCollection	Curve set ID
    *  @param [in]		curveIndex		The curve index for which to calculate Discount Factors. Default to OIS
	*  @param [in]		spread			Spread to be added to the curve forward-rate when calculating the discount factors,
	*									following the zero discount margin approach described by O'Kane in "Credit Spreads Explained".
	*  @param [in]		fixingTableName An optional fixing table name. This field may be blank if no fixings are required.
	*  @return			A vector of discount factors
    */
    DoubleVector getCurveDiscountFactors( const DateVector& paymentDates,
                                          const LAString& curveCollection,
                                          const LAString& curveIndex,
										  const double& spread,
										  const std::string& fixingTableName )
    {
        MLIB_REQUIRE( paymentDates.size() > 0, "Unable to evaluate Discount Factors - No payment dates have been provided")
        
        LAString interp;
        LAString bdAdj( "NO_CHANGE" );
        LAString cal;
        populateDiscountFactorConventions( curveCollection, curveIndex, interp, bdAdj, cal );

        DoubleVector discountFactors;
        
		const bool isEnabledCurveResults = etrading::isEnabledCurveResults();
		const bool doesExistDiscountFactors = etrading::doesExistCurveResultsDiscountFactors( curveCollection.getCString(), curveIndex.getCString() );

		if ( isEnabledCurveResults && doesExistDiscountFactors )
		{
			// New: Get Discount Factors from the Thread-Safe Curve Results Object
			discountFactors = getDiscountFactorsFromCurveResultsObject( curveCollection.getCString(), curveIndex.getCString(), paymentDates );
		}
		else
		{
			// LEGACY: Get Discount Factors from the Object Pool
			discountFactors = etrading::LACurveForwardRateHelpers::getMultiSpotDiscountFactors( paymentDates, getDataInstance(), curveCollection, getDiscountFactorDayCount(), bdAdj, cal, interp, isBasisFlagForDiscountFactor(), curveIndex );
		}

		MLIB_REQUIRE( discountFactors.size() == paymentDates.size(), "Discount Factor Results should be same size as Payment Dates." );

		// Check if a spread parameter has been provided. The default parameter value = 0.0
		if ( spread != 0.0 )
		{
			/* A spread has been provided. The approach here is to calculate the forward rate from the curve,
			 * apply the spread and then recalculate the corresponding discount factor with spread.
			 * This follows the approach in the paper "Credit Spreads Explained"
			 * by Dominic O'Kane and Saurav Sen p15 Zero Discount Margin method.
			 */
			MLIB_REQUIRE( isEnabledCurveResults && doesExistDiscountFactors, "CurveResults are not enabled. Spread Discount Margin calculation only supported with CurveResults enabled." );

			discountFactors = calculateDiscountFactorsUsingZeroDiscountMarginApproach( paymentDates, curveCollection.getCString(), curveIndex.getCString(), spread, fixingTableName );
		}

        return discountFactors;
    }

    /* @brief			Get float forward rate 
    *  @param [in]		curveFowardRate   forward rate from the curve
    *  @param [in]		asOfDate	      asOfDate
    *  @param [in]		fixingDate	      fixingDate
    *  @param [in]		paymentDate		  paymentDate
    *  @param [in]		fixingTable		        Fixing table object name
    *  @return			The forward rate
    */
    FloatRateData getForwardRate(const FloatRateData& curveFowardRate,
                                 const LADate& asOfDate,
                                 const LADate& fixingDate, 
                                 const LADate& paymentDate,
                                 const std::shared_ptr<FixingTable>& fixingTable)
    {
		FloatRateData forwardRate;

		forwardRate.resetRate= 0.0;

        if (fixingDate < asOfDate)
        {
            if (fixingTable == nullptr)
            {
                // If fixingDate in the past and paymentDate is in the future, a fixing table is required
                if (paymentDate >= asOfDate)
                {
                    LAString erroMsg = "#Error: FixingTable required for fixingDate: " + fixingDate.stringWithFormat("DD-MM-YYYY");
        		    throw LACoreInvalidData( erroMsg.getCString(), __FILE__, __LINE__ );
                }
                else
                {
                    forwardRate.resetRate = 0.0;
                }
            }
            else
            {
                try
                {
                    forwardRate.resetRate = fixingTable->getFixingValue(toGregorianDateFromLADate(fixingDate));
                }
                catch( ETradingException &e )
                {
                    // Fixng Rates: Only throw an error if the fixing date is needed for valuation
                    if ( paymentDate < asOfDate )
                    {
                        // Fixings for payments in the past are only needed for information purposes to view historic
                        // cashflow information, which is optional and not mandatory. Such fixings should be set to zero
                        // if missing
                        forwardRate.resetRate = 0;
                    }
                    else
                    {
                        // Fixings that are required for valuation are mandatory i.e. when paymentDate >= asOfDate
                        throw e;
                    }
                }
            }
        }
        else if ( fixingDate == asOfDate && fixingTable != nullptr )
        {
            // Allow today's 11:00am Libor fixings to be taken from the fixing table
            // If provided use the fixing table otherwise use the curveForwardRate
            try
            {
                forwardRate.resetRate = fixingTable->getFixingValue(toGregorianDateFromLADate(fixingDate));
            }
            catch( ETradingException& )
            {
                forwardRate = curveFowardRate;
            }
        }
        else
        {
            forwardRate = curveFowardRate;
        }

        return forwardRate;
    }

    std::string validateCompoundingMethod( const std::string& compoundingMethod )
    {
        std::string compound = compoundingMethod;
        if ( compoundingMethod.size() != 0 )
        {
            if ( boost::iequals( compoundingMethod.c_str(), "Arithmetic" ) )
            {
                compound = "AVERAGE";
            }
            else if ( boost::iequals( compoundingMethod.c_str(), "Geometric" ) )
            {
                compound = "NORMAL";
            }
            else if ( !( boost::iequals( compoundingMethod.c_str(), "FLAT" ) || boost::iequals( compoundingMethod.c_str(), "SIMPLE" ) ) )
            {
                throw LACoreInvalidData( "#Error: Invalid compounding method, 'CompMethod' should either be 'Arithmetic', 'Geometric', 'FLAT', or 'SIMPLE'", __FILE__, __LINE__ );
            }
        }
        return compound;
    }

    // Find the index for the next cashflow, so that we can ignore cashflows in the past
    size_t getFirstNonpastDateIndex( const DateVector& dates, const LADate& asOf )
    {
        size_t nextCashflowIndex = 0;
        for ( size_t i = 0; i < dates.size(); ++i )
        {
            if ( dates[i] < asOf )
            {
                nextCashflowIndex++;
            }
        }
        return nextCashflowIndex;
    }

    /* @brief			validation method for meCurveDiscountFactors
    *  @param [in]		toDates			A single or an array of to-date in YYYYMMDD formate
    *  @param [in]		curveCollection	Curve set ID
    *  @param [in]		curveIndex		Index of the curve set. Default to OIS
    *  @param [in]		businessDayAdj	Rule that decides if end date should roll forward or backward etc, ie, MF. Default to 'NO_CHANGE'.
    *  @param [in]		calendar		Calendar
    *  @return			A array of discount factors
    */
    DoubleVector getCurveDiscountFactors( size_t firstNonpastCashflowIndex,
                                          const DateVector& paymentDates,
                                          const LAString& curveCollection,
                                          const LAString& curveIndex )
    {
        DoubleVector dfs = getCurveDiscountFactors( paymentDates, curveCollection, curveIndex );
        
        DoubleVector ret( dfs.size() );
        for( size_t i = 0; i < dfs.size(); ++i )
        {
            ret[i] = dfs[i];
            if ( i  < firstNonpastCashflowIndex )
            {
                ret[i] = 0.0;
            }
        }
        return ret;
    }

	/* @brief			validation method for meCurveDiscountFactors
	*  @param [in]		toDates			A single or an array of to-date in YYYYMMDD formate
	*  @param [in]		curveCollection	Curve set ID
	*  @param [in]		curveIndex		Index of the curve set. Default to OIS
	*  @param [in]		businessDayAdj	Rule that decides if end date should roll forward or backward etc, ie, MF. Default to 'NO_CHANGE'.
	*  @param [in]		calendar		Calendar
	*  @return			A array of discount factors
	*/
	DoubleVector getCurveDiscountFactorsFromBaseDate(const size_t& firstNonpastCashflowIndex,
													const LADate& baseDate,
													const DateVector& paymentDates,
													const LAString& curveCollection,
													const LAString& curveIndex)
	{
		DoubleVector dfs = getCurveDiscountFactors(baseDate, paymentDates, curveCollection, curveIndex);

		DoubleVector ret(dfs.size());
		for (size_t i = 0; i < dfs.size(); ++i)
		{
			ret[i] = dfs[i];
			if (i  < firstNonpastCashflowIndex)
			{
				ret[i] = 0.0;
			}
		}
		return ret;
	}

    /* @brief			Get the curve forward rate given a year fraction
    *  @param [in]		fromDate			forward rate start or from date
    *  @param [in]		daycount	        daycount fraction
    *  @param [in]		curveCollection	    Curve collection or set ID
    *  @param [in]		curveIndex		    Index of the curve set.
    *  @param [in]		fwdInter			User input fwdInter flag
	*  @param [in]		businessDayAdjust	The business day adjustment
	*  @return			forward rate
    */
    DoubleVector getCurveForwardRatesFromYearFraction( const DateVector& fromDates,
                                                       double yearFraction,
                                                       const LAString& dayCount,
                                                       const LAString& curveCollection,
                                                       const LAString& curveIndex,
													   const BooleanEnum& fwdInter,
													   const BusinessDayAdjustmentEnum& businessDayAdj)
    {

		const size_t fromDateSize = fromDates.size();

		if (fromDateSize == 0 )
		{
			throw LACoreInvalidData("#Error: fromDates and toDates must have the same size and not empty", __FILE__, __LINE__);
		}

		LAString curIndex( curveIndex );
        LAString interp;
		LAString bdAdj = toString(businessDayAdj).c_str();
		LAString cal( "" );
        LAString dayC( "" );

        bool isFwdInter;
        bool useFwdData;

        populateForwardRateConventions( curveCollection, curIndex, fwdInter, interp, bdAdj, cal, dayC, isFwdInter, useFwdData );

		CompoundingFrequencyEnum curveCompoundingfreq = getForwardRateCurveCompoundFrequency();

		DoubleVector forwardRates = etrading::LACurveForwardRateHelpers::getMultiForwardRate(fromDates, yearFraction, getDataInstance(), curveCollection, toString(curveCompoundingfreq).c_str(), dayC, bdAdj, cal, interp, curIndex, isFwdInter, useFwdData);

		return forwardRates;
    }


    /* @brief			Get the curve forward rate given a set of to- and fromDates
    *  @param [in]		fromDates	    A vector of from dates
    *  @param [in]		toDates			A vector of to dates
    *  @param [in]		curveCollection	ID of the yield curve
    *  @param [in]		curveIndex		Index of the curve set. Default to STD
	*  @param [in]		fwdInter		User input fwdInter flag
	*  @param [in]		businessDayAdjust	The business day adjustment
	*  @return			The forward rates based on fromDates and toDates
    */
    DoubleVector getCurveForwardRatesFromForwardDates( const DateVector& fromDates,
                                                       const DateVector& toDates,
                                                       const LAString& curveCollection,
                                                       const LAString& curveIndex,
													   const BooleanEnum& fwdInter,
													   const BusinessDayAdjustmentEnum& businessDayAdj)
    {
		MLIB_REQUIRE( fromDates.size() > 0, "Forward Rate Error: Fixing Start Dates are empty")
		MLIB_REQUIRE( fromDates.size() == toDates.size(), "Forward Rate Error: Fixing Start and End Dates must be the same size")
        
		// Get Curve Conventions
		LAString dayC		= "";
		LAString bdAdj		= toString(businessDayAdj).c_str();
		LAString cal		= "";
		LAString interp;
		bool isFwdInter;
		bool useFwdData;
		populateForwardRateConventions( curveCollection, curveIndex, fwdInter, interp, bdAdj, cal, dayC, isFwdInter, useFwdData );

		// This method in curve utilities always returns SIMPLE_COMPOUNDING
		CompoundingFrequencyEnum curveCompoundingfreq = getForwardRateCurveCompoundFrequency();
        
        DoubleVector forwardRates;
        const bool isEnabledCurveResults = etrading::isEnabledCurveResults();
        const bool doesExistDiscountFactors = etrading::doesExistCurveResultsDiscountFactors( curveCollection.getCString(), curveIndex.getCString() );
        
		if ( isEnabledCurveResults && doesExistDiscountFactors && !isFwdInter )
		{
            // 1. Use New Curve Object Look-Up when isFwdInter = FALSE
            // NEW: Get Forward Rates from the Thread-Safe Curve Results Object
            forwardRates = implyForwardRatesFromCurveResultsObject( curveCollection.getCString(), curveIndex.getCString(), fromDates, toDates, isFwdInter, curveCompoundingfreq );
        }
        else
        {
            // 2. Use Object Pool when isFwdInter = TRUE
            // LEGACY: Get Forward Rates from the Object Pool
			forwardRates = LACurveForwardRateHelpers::getMultiForwardRate( fromDates, toDates, getDataInstance(), curveCollection, toString(curveCompoundingfreq).c_str(), dayC, bdAdj, cal, interp, curveIndex, isFwdInter, useFwdData );
        }

        return forwardRates;
    }

	/* @brief			Helper function to transform resetRates from double list of FloatRateData list
	*  @param [in]		resetRates	    A vector of resetRates in doubles
	*  @return			The forward rates based on fromDates and toDates
	*/
	std::vector<FloatRateData> toFloatRateDataVector(const DoubleVector& resetRates)
	{
		const size_t expectedSize = resetRates.size();

		std::vector<FloatRateData> floatRates(expectedSize);

		for (size_t i = 0; i < expectedSize; ++i)
		{
			floatRates[i] = resetRates[i];
		}

		return floatRates;
	}

	
	/* @brief			Get the Fixing type
	*  @param [in]		fixingDate				Fixing start date
	*  @param [in]		accrualStartDate		Accrual start date
	*  @param [in]		accrualEndDate			Accrual end date
	*  @param [in]		fixingBusinessDayAdj	Fixing BusinessDayAdj
	*  @param [in]		fixingCalendar			Fixing Calendar
	*  @param [in]		ToleranceTenor			Fixing date tolerance tenor
	*  @return			Fixing type, i.e. Advance, Arrears, or Abitrary
	*/
	FixingTypeEnum getFixingTypeByFixingDate(const LADate& fixingDate,
											const LADate& accrualStartDate,
											const LADate& accrualEndDate,
											const BusinessDayAdjustmentEnum& fixingBusinessDayAdj,
											const std::string& fixingCalendar,
											const std::string& fixingDateToleranceTenor) // Positive Tenor 
	{

		std::string minusFixingDateToleranceTenor = "-" + fixingDateToleranceTenor;

		auto lowerBoundDates = getDateFromTenor(boost::assign::list_of(accrualStartDate)(accrualEndDate), minusFixingDateToleranceTenor.c_str(), toString(fixingBusinessDayAdj).c_str(), fixingCalendar.c_str(), "");
		auto upperBoundDates = getDateFromTenor(boost::assign::list_of(accrualStartDate)(accrualEndDate), fixingDateToleranceTenor.c_str(), toString(fixingBusinessDayAdj).c_str(), fixingCalendar.c_str(), "");

		const LADate accrualStartLowerBound = lowerBoundDates[0];
		const LADate accrualStartUpperBound = upperBoundDates[0];

		const LADate accrualEndLowerBound = lowerBoundDates[1]; 
		const LADate accrualEndUpperBound = upperBoundDates[1];

		// Default to fixing in advance
		FixingTypeEnum fixingType = IN_ADVANCE_FIXING;

		// Fixing earlier than accrualStart (outside tolerance)
		if (fixingDate < accrualStartLowerBound)
		{
			throw LACoreInvalidData("#Error: Invalid Fixing Date: The Fixing Date is before the Accrual Start Date and outside tolerance", __FILE__, __LINE__);
		}
		// Fixing later than accrualEnd (outside tolerance)
		else if (fixingDate > accrualEndUpperBound)
		{
			throw LACoreInvalidData("#Error: Invalid Fixing Date: The fixing date is after the accrual end date and outside tolerance", __FILE__, __LINE__);
		}
		// Fixing at accrualStart (within tolerance)
		else if (fixingDate >= accrualStartLowerBound && fixingDate <= accrualStartUpperBound)
		{
			fixingType = IN_ADVANCE_FIXING;
		}
		// *** Fixing at accrualEnd till accrualEnd uppperbound
		else if (fixingDate >= accrualEndDate && fixingDate <= accrualEndUpperBound)
		{
			fixingType = IN_ARREARS_FIXING;
		}
		// *** Fixing between accrualStart uppperbound  and accrualEnd 
		else if (fixingDate > accrualStartUpperBound && fixingDate < accrualEndDate)
		{
			fixingType = ARBITRARY_FIXING;
		}

		return fixingType;
	}

	/* @brief			Get the curve single forward rate with consideration of convexity adjustment
	*  @param [in]		fromDate				unadjustedForwardRate's date
	*  @param [in]		toDate					adjustedForwardRate's date
	*  @param [in]		valuationDate			Valuation date
	*  @param [in]		dayCount				Day count of the curve
	*  @param [in]		fixingType				Fixing in Advance or in Arrears
	*  @param [in]		volObject				Volatility object
	*  @param [in]		convexityMethod			Convexity method
	*  @param [in]		curveCollection			Curve collection, e.g. EURYC
	*  @param [in]		curveIndex				OIS Curve index
	*  @return			The adjusted forward rate
	*/
	FloatRateData getCurveForwardRateWithConvexityAdjustment(const double& unadjustedForwardRate,
														const LADate& fixingDate,
														const LADate& fixingEndDate,
														const LADate& valuationDate,
														const DayCountEnum& dayCount,
														const FixingTypeEnum& fixingType,
														const VolatilityPtr& volObject,
														const ConvexityMethodEnum& convexityMethod,
														const std::string& curveCollection,
														const std::string& curveIndex)
	{

		//If user don't want any convexity, just return the unadjusted rate
		if (convexityMethod == NONE_CONVEXITY || fixingType == IN_ADVANCE_FIXING)
		{
			return unadjustedForwardRate;
		}

		FloatRateData floatRate;

		switch (fixingType)
		{
		case IN_ARREARS_FIXING:
		{
			MLIB_REQUIRE(volObject, "Volatility Model is Missing")
				
			bool useHullApproximation = (convexityMethod == HULL_APPROX_CONVEXITY);

			//year fraction between fixingStart and fixingEnd
			const double fixingYearFraction = getYearFraction(fixingDate, fixingEndDate, dayCount);

			// year fraction from valuationDate to fixingDate
			const double yfValuationToFixing = getYearFraction(valuationDate, fixingDate, dayCount);

			ConvexityModel convexityModel(Convexity::VolatilityParameters(volObject->vol(), volObject->volType(), volObject->shiftSize()));

			floatRate.unadjustedResetRate = unadjustedForwardRate;

			floatRate.convexity = convexityModel.liborConvexityAdjustmentInArrears(unadjustedForwardRate, fixingYearFraction, yfValuationToFixing, useHullApproximation);

			floatRate.resetRate = unadjustedForwardRate + floatRate.convexity;

			break;
		}
		case ARBITRARY_FIXING:
		{
			MLIB_REQUIRE(volObject, "Volatility Model is Missing")
				
			bool useHullApproximation = (convexityMethod == HULL_APPROX_CONVEXITY);

			//year fraction between fixingStart and fixingEnd
			const double fixingYearFraction = getYearFraction(fixingDate, fixingEndDate, dayCount);

			// year fraction from valuationDate to fixingDate
			const double yfValuationToFixing = getYearFraction(valuationDate, fixingDate, dayCount);

			// Discount factors from OIS CURVE
			auto discountFactors = getCurveDiscountFactors(boost::assign::list_of(fixingDate)(fixingEndDate), curveCollection.c_str(), curveIndex.c_str());
			MLIB_REQUIRE(discountFactors.size() == 2, "Two discountFactors required: FixingStartDate and FixingEndDate.")

			ConvexityModel convexityModel(Convexity::VolatilityParameters(volObject->vol(), volObject->volType(), volObject->shiftSize()));

			floatRate.unadjustedResetRate = unadjustedForwardRate;

			floatRate.resetRate = convexityModel.liborRateForArbitraryFixingDate(unadjustedForwardRate, fixingYearFraction, yfValuationToFixing, fixingYearFraction, discountFactors[0], discountFactors[1], useHullApproximation);

			floatRate.convexity = floatRate.resetRate - unadjustedForwardRate;

			break;
		}
		default:
			MLIB_THROW("Invalid fixing type. Must be 'ADVANCE', 'ARREARS', or 'ARBITRARY_FIXING'");
			break;
		}

		return floatRate;
	}

	/* @brief			Get the curve multiple forward rates with consideration of convexity adjustment
	*  @param [in]		fromDates	    A vector of from dates
	*  @param [in]		toDates			A vector of to dates
	*  @param [in]		valuationDate	Valuation date
	*  @param [in]		dayCount		Day count of the curve
	*  @param [in]		fixingType		Fixing in Advance or in Arrears
	*  @param [in]		volObject		Volatility object
	*  @param [in]		convexityMethod			Convexity method
	*  @param [in]		curveCollection	Curve collection, e.g. EURYC
	*  @param [in]		curveIndex		OIS Curve index
	*  @return			The forward rates based on fromDates and toDates
	*/
	std::vector<FloatRateData> getCurveForwardRatesWithConvexityAdjustment(const DoubleVector& unadjustedForwardRates,
																const DateVector& fromDates,
																const DateVector& toDates,
																const LADate& valuationDate,
																const DayCountEnum& dayCount,
																const FixingTypeEnum& fixingType,
																const VolatilityPtr& volObject,
																const ConvexityMethodEnum& convexityMethod,
																const std::string& curveCollection,
																const std::string& curveIndex)
	{

		const size_t expectedSize = unadjustedForwardRates.size();

		std::vector<FloatRateData> floatRates(expectedSize);

		for (size_t i = 0; i < expectedSize; ++i)
		{
			floatRates[i] = getCurveForwardRateWithConvexityAdjustment(unadjustedForwardRates[i], fromDates[i], toDates[i], valuationDate, dayCount, fixingType, volObject, convexityMethod, curveCollection, curveIndex);
		}

		return floatRates;
	}

	/* @brief			Get CurveFixingEndDate, which uses the curve frequency, useful when implying forward rates from discount factors
	*  @param [in]		fixingDates	            A vector of fixing Dates
	*  @param [in]		curveCollection			ID of the yield curve
	*  @param [in]		curveIndex		        Index of the curve set. Default to STD
	*  @param [in]		businessDayAdj	        Rule that decides if end date should roll forward or backward etc, ie, MF. Default to 'NO_CHANGE'.
	*  @param [in]		calendar		        Calendar
	*  @param [in]		fwdInter				User input fwdInter flag
	*  @return			The forward rates based on the fixing dates
	*/
	DateVector getCurveFixingEndDates( const DateVector& fixingDates,
									   const LAString& curveCollection,
									   const LAString& curveIndex,
									   const BusinessDayAdjustmentEnum& businessDayAdj,
									   const LAString& calendar,
									   const BooleanEnum& fwdInter,
									   const CurveTenorEnum& curveTenorOverride )
	{
		// Get and check the Curve Frequency Tenor e.g. 1M, 3M, 6M, 12M
		CurveTenorEnum crvFreqTenor = (curveTenorOverride == NONE_CURVE_TENOR) ? toCurveTenorEnum(validateCurveAndGetCurveFrequency(curveCollection, curveIndex).getCString()) : curveTenorOverride;

		// Imply the toDate(s) using the fromDate(s) and curve frequency i.e. todate = fromDate + 3M 
		LAString bdAdj = toString(businessDayAdj).c_str();
		LAString cal = getDefaultCalendarForEmptyString(calendar, curveCollection);

		const DateVector toDates = LADateScheduleHelpers::getMultiDate(fixingDates, toString(crvFreqTenor).c_str(), bdAdj, cal, nullptr); // rollconvention* = nullptr
		return toDates;
	}

	/* @brief			Get CurveFixingEndDateAsTerms, which uses the curve frequency, useful when implying forward rates from discount factors
	*  @param [in]		fixingDates	            A vector of fixing Dates
	*  @param [in]		curveCollection			ID of the yield curve
	*  @param [in]		curveIndex		        Index of the curve set. Default to STD
	*  @param [in]		businessDayAdj	        Rule that decides if end date should roll forward or backward etc, ie, MF. Default to 'NO_CHANGE'.
	*  @param [in]		calendar		        Calendar
	*  @param [in]		fwdInter				User input fwdInter flag
	*  @return			The forward rates based on the fixing dates
	*/
	DoubleVector getCurveFixingEndDatesAsTerms( const DateVector& fixingDates,
									            const LAString& curveCollection,
									            const LAString& curveIndex,
									            const BusinessDayAdjustmentEnum& businessDayAdj,
									            const LAString& calendar,
									            const BooleanEnum& fwdInter,
									            const CurveTenorEnum& curveTenorOverride )
	{
		const DateVector toDates = getCurveFixingEndDates(fixingDates, curveCollection, curveIndex, businessDayAdj, calendar, fwdInter, curveTenorOverride );
		const DoubleVector toDatesAsTerm = convertCurveDatesToTerms( curveCollection, toDates );
		return toDatesAsTerm;
	}

	/* @brief			Get forward rates from the curve
	*  @param [in]		fixingDates	            A vector of fixing Dates
	*  @param [in]		curveCollection			ID of the yield curve
	*  @param [in]		curveIndex		        Index of the curve set. Default to STD
	*  @param [in]		businessDayAdj	        Rule that decides if end date should roll forward or backward etc, ie, MF. Default to 'NO_CHANGE'.
	*  @param [in]		calendar		        Calendar
	*  @param [in]		fwdInter				User input fwdInter flag
	*  @return			The forward rates based on the fixing dates
	*/
	DoubleVector getCurveForwardRates(const DateVector& fixingDates,
									  const LAString& curveCollection,
									  const LAString& curveIndex,
									  const BusinessDayAdjustmentEnum& businessDayAdj,
									  const LAString& calendar,
									  const BooleanEnum& fwdInter,
									  const CurveTenorEnum& curveTenorOverride)
	{
		const DateVector toDates = getCurveFixingEndDates(fixingDates, curveCollection, curveIndex, businessDayAdj, calendar, fwdInter, curveTenorOverride );
		return getCurveForwardRatesFromForwardDates(fixingDates, toDates, curveCollection, curveIndex, fwdInter);
	}

 
	/* @brief			Generates a date schedule tailored for use with charting / plotting a CurveCollection. If a startDate is not provided, uses the curveCollection as-of date.
	*  @param [in]		curveCollection		The curveCollection to use when accessing the curveIndices
	*  @param [in]		startDate			An optional string specifying the first date in the schedule. If empty, the curveCollection asOf date is used.
	*  @param [in]		maturity			A tenor describing the length of the date schedule, for example 10Y
	*  @param [in]		businessDayAdjust	The businessday adjustment, for example MODFOLLOWING
	*  @param [in]		calendar			The holiday calendar to use when rolling out the date schedule
	*  @param [in]		rollConvention		The roll convention to use when generating the date schedule, for example IMM
	*  @param [in]		frequency			The frquency of points in the date schedule, for example MONTHLY		
	*/
	DateVector generateDateScheduleForCurveCharting( const LAString& curveCollection,
													 const LAString& startDate,
													 const LAString& maturity,
													 const BusinessDayAdjustmentEnum& businessDayAdjust,
													 const LAString& calendar,
													 const LAString& rollConvention,
													 const LAString& frequency )
	{
		DateVector dateSchedule;

		LADataInstance* dataInstance = etrading::getDataInstance();
		checkIfCurveExists( dataInstance, curveCollection );

		LADate curveAsOfDate = getCurveAsOfDate( curveCollection );

		// Determine the first date in the schedule.
		// If a startDate string has been provided then use this, otherwise use the curve asOf date.
		// The startDate string can be a tenor (eg 1Y) relative to the curveAsOfDate,
		// or it can be an explicit date.
		LADate firstDate = ( startDate.size() > 0 ) ? validateMaturityDate( curveAsOfDate, startDate )
													: curveAsOfDate;
		dateSchedule.push_back( firstDate );

		// Calculate the last date in the schedule
		// The maturity string can be a tenor (eg 10Y) relative to the firstDate in the schedule,
		// or it can be an explicit date.
		LADate maturityDate	= validateMaturityDate( firstDate, maturity );

		//If not provided, Curve calendar will be used so that the fixingDates won't include weekends.
		LAString cal = getDefaultCalendarForEmptyString(calendar, curveCollection);

		LAString freq( frequency );
		LAString busDayAdj( toString(businessDayAdjust).c_str() );
		dateSchedule = LADateScheduleHelpers::generateSchedule(  firstDate,
														maturityDate,
														freq,
														busDayAdj,	// aka Sliding Rule
														cal,
														NULL,							// aka First Odd Date
														NULL,							// aka Last Odd Date
														NULL,							// roll day
														false,							// isStartRoll
														&rollConvention );
		return dateSchedule;
	}


	/* @brief			Generates a date schedule and uses the date schedule to calculate a vector of forward rates for a set of curveIndices.
	*  @param [out]		fixingDates			The fixing dates on which the forwardRates are calculated. Generated from schedule parameters
	*  @param [out]		forwardRates		A matrix containing the calculated forward rates for each curve, for each fixing date in the generated schedule
	*  @param [in]		curveCollection		The curveCollection to use when accessing the curveIndices
	*  @param [in]		curveIndices		The curveIndices to use when calculating forward rates
	*  @param [in]		startDate			An optional string specifying the first date in the schedule. If empty, the curveCollection asOf date is used.
	*  @param [in]		maturity			A tenor describing the length of the date schedule, for example 10Y
	*  @param [in]		businessDayAdjust	The businessday adjustment, for example MODFOLLOWING
	*  @param [in]		calendar			The holiday calendar to use when rolling out the date schedule
	*  @param [in]		rollConvention		The roll convention to use when generating the date schedule, for example IMM
	*  @param [in]		frequency			The frquency of points in the date schedule, for example MONTHLY		
    *  @param [in]		fwdInters			User input's fwdInter flags
	*/
	void getForwardRatesForCurveIndices( DateVector& fixingDates,
										 DoubleMatrix& forwardRates,
										 const LAString& curveCollection,
										 const LAStringVector& curveIndices,
										 const LAString& startDate,
										 const LAString& maturity,
										 const BusinessDayAdjustmentEnum& businessDayAdjust,
										 const LAString& calendar,
										 const LAString& rollConvention,
										 const LAString& frequency,
										 const LAStringVector& fwdInters)
	{
		// Generate the date schedule
		fixingDates = generateDateScheduleForCurveCharting( curveCollection,
															startDate,
															maturity,
															businessDayAdjust,
															calendar,
															rollConvention,
															frequency );
		
		// Calculate the forward rates for each curveIndex
		forwardRates.clear();

		if (fwdInters.size() != 0 && fwdInters.size() != curveIndices.size() )
		{
			throw LACoreInvalidData("#Error: isFwdInter flags must have the same size as curve indices.",__FILE__,__LINE__);
		}

		bool fwdIterpEmpty = (fwdInters.size() == 0 );

		for (size_t i=0; i<curveIndices.size(); i++)
		{
			const LAString curveIndex = curveIndices[i];
			const BooleanEnum fwdInter = fwdIterpEmpty ? NONE_BOOL : toBooleanEnum(fwdInters[i].getCString());

			DoubleVector forwardRatesForCurve = getCurveForwardRates( fixingDates,
																	  curveCollection,
																	  curveIndex,
	 																  businessDayAdjust,
																	  calendar,
																	  fwdInter);
			forwardRates.push_back( forwardRatesForCurve );
		}
	}

	/* @brief			Generates a date schedule and uses the date schedule to calculate a vector of discount factors for a set of curveIndices.
	*  @param [out]		paymentDates		The payment dates on which the discount factors are calculated. Generated from schedule parameters.
	*  @param [out]		discountFactors		A matrix containing the calculated discount factors for each curve, for each fixing date in the generated schedule
	*  @param [in]		curveCollection		The curveCollection to use when accessing the curveIndices
	*  @param [in]		curveIndices		The curveIndices to use when calculating discount factors
	*  @param [in]		startDate			An optional string specifying the first date in the schedule. If empty, the curveCollection asOf date is used.
	*  @param [in]		maturity			A tenor describing the length of the date schedule, for example 10Y
	*  @param [in]		businessDayAdjust	The businessday adjustment, for example MODFOLLOWING
	*  @param [in]		calendar			The holiday calendar to use when rolling out the date schedule
	*  @param [in]		rollConvention		The roll convention to use when generating the date schedule, for example IMM
	*  @param [in]		frequency			The frquency of points in the date schedule, for example MONTHLY		
	*/
	void getDiscountFactorsForCurveIndices( DateVector& paymentDates,
											DoubleMatrix& discountFactors,
											const LAString& curveCollection,
											const LAStringVector& curveIndices,
											const LAString& startDate,
											const LAString& maturity,
											const BusinessDayAdjustmentEnum& businessDayAdjust,
											const LAString& calendar,
											const LAString& rollConvention,
											const LAString& frequency )
	{
		// Generate the date schedule
		paymentDates = generateDateScheduleForCurveCharting( curveCollection,
															 startDate,
															 maturity,
															 businessDayAdjust,
															 calendar,
															 rollConvention,
															 frequency );
		
		// Calculate the discount factors for each curveIndex
		discountFactors.clear();
		for (size_t i=0; i<curveIndices.size(); i++)
		{
			const LAString curveIndex = curveIndices[i];

			// Check the curve is built
			getCurveStaticDataTableName( curveCollection, curveIndex );

			DoubleVector discountFactorsForCurve = getCurveDiscountFactors( paymentDates,
																			curveCollection,
																			curveIndex );
			discountFactors.push_back( discountFactorsForCurve );
		}
	}

    /* @brief			Calculates discount factors for a curve.
	*					If a spread has been provided, performs the zero-discount margin calculation
	*					described by O'Kane in "Credit Spreads Explained".
    *  @param [in]		paymentDates	An array of payment dates
    *  @param [in]		curveCollection	Curve set ID
    *  @param [in]		curveIndex		The curve index for which to calculate Discount Factors.
	*  @param [in]		spread			Spread to be added to the curve forward-rate when calculating the discount factors,
	*									following the zero discount margin approach described by O'Kane in "Credit Spreads Explained".
	*  @param [in]		fixingTableName An optional fixing table name. This field may be blank if no fixings are required.    *  @return			A array of discount factors
    */
	DoubleVector getDiscountFactorsForCurveIndexWithSpread( const DateVector& paymentDates,
															const std::string& curveCollection,
															const std::string& curveIndex,
															const double& spread,
															const std::string& fixingTableName )
	{
		getCurveStaticDataTableName( curveCollection, curveIndex );

		const DoubleVector discountFactors = getCurveDiscountFactors( paymentDates,
																	  curveCollection,
																	  curveIndex,
																	  spread,
																	  fixingTableName );
		return discountFactors;
	}

    /* @brief			Get the curve indics and curve tenors for stub rate function where the returned are order by curve tenors
    *  @param [in]		curveCollection	ID of the yield curve
    *  @return			curve indics and curve tenors
    */
    std::pair<LAStringVector, LAStringVector> getStubRateCurveIndicesTenors(const LAString& curveCollection)
    {
        //Get all curves under the curveCollection
        std::vector<LAStringVector> allCurves = getCurveNamesInCurveCollection(curveCollection);
        std::vector<LAStringVector> curves;
        curves.reserve(allCurves.size());

        // Initialize Yield Curve Pro
        CurveCalibrationData* curveCalibrationData = etrading::InitializeMLibETrading::instance().ycStaticDataObject( curveCollection, true ); // true = throw if curve does not exist

		const LAObject& yieldData = curveCalibrationData->getYieldData().get().get();

        //Exclude the FWDFXCONST, Cheapest-To-Deliver and XCCYBASIS curves
        for( size_t i=0; i < allCurves.size(); ++i )
        {
            auto curveInfo = allCurves[i];
            auto staticDataTable = curveInfo[0];

			// Get curve type
			LAString suffix = (staticDataTable == SWAP || staticDataTable == STD) ? "" : LAString( "_" ) + staticDataTable;

			LAString curveType("");
			const LADataHolder* dh = &yieldData.getData( CALIBRATION_DATA_CURVETYPE + suffix);
			if ( dh->isDefined() && !dh->isNull() )
			{
				curveType = dynamic_cast<const LADataString&>( dh->get() ).get();
			}						
			
			// Do not add FWDFXCONST, Cheapest-To-Deliver or XCCYBASIS curves
			if (curveType != CURVETYPE_FWDFXCONST
				&& curveType != CURVETYPE_CHEAPESTTODELIVER
				&& curveType != XCCYBASIS
				&& curveType.size() != 0)
			{
                curves.push_back(curveInfo);
            }
        }

        //Sort the curves by curve frequency, e.g. 1D, 1M, 3M, 6M, 12M
        std::sort(curves.begin(), curves.end(), []( const LAStringVector& curve1, const LAStringVector& curve2 )
        {
             return (getFrequencyOrTenorMonth(curve1[2]) < getFrequencyOrTenorMonth(curve2[2]));
        });

        //Populate curveIndcies and curveTenors for stubRate function
        auto curvesSize = curves.size();
        LAStringVector curveIndices;
        curveIndices.reserve(curvesSize);

        LAStringVector curveTenors;
        curveTenors.reserve(curvesSize);

        for( size_t i=0; i < curvesSize; ++i ) 
        {
            auto curveInfo = curves[i];
            auto curveIndex = curveInfo[1];
            auto curveTenor = curveInfo[2];
            curveIndices.push_back(curveIndex);
            curveTenors.push_back(curveTenor);
        }

        return std::make_pair(curveIndices, curveTenors);
    }

    /* @brief			Get the matching curveIndex (by staticDataTable) from the curveIndices 
    *  @param [in]		stubCurveIndex Stub Curve Index specified by the user  
    *  @param [in]		curveIndices     Indcies for the stub curves
    *  @return			the matching curveIndex from curveIndices
    */
    LAString getMatchingCurveIndex(const LAString& curveCollection, const LAString& stubCurveIndex, const LAStringVector& curveIndices)
    {
        auto staticTableToMatch = getCurveStaticDataTableName(curveCollection, stubCurveIndex);
        LAString matchedIndex = "";
        for( size_t i=0; i < curveIndices.size(); ++i )
        {
            auto curveIndex = curveIndices[i];
            auto currentStaticTable = getCurveStaticDataTableName(curveCollection, curveIndex);
            if (same(staticTableToMatch, currentStaticTable))
            {
                matchedIndex = curveIndex;
                break;
            }
        }

        if (matchedIndex.size() == 0 )
        {
            throw LACoreInvalidData( "#Error: User specified stubCurveIndex does not match any curve index in the curve collection", __FILE__, __LINE__ );
        }

        return matchedIndex;
    }

    /* @brief			Calculate toDate based on startDates and tenor
    *  @param [in]		startDate			The from date
    *  @param [in]		tenor				Tenor added to the from date
    *  @param [in]		businessDayAdj		Rule that decides if end date should roll forward or backward etc, ie, MF
    *  @param [in]		calendar			Calendar
    *  @param [in]		rollConvention		Roll conventions, ie, Normal, IMM, EOM, Lunar, etc
    *  @return			A vector of end dates
    */
    LADate getSingleDateFromTenor( const LADate& startDate,           
                                   const LAString & tenor,                    // TODO: Convert to StandardString
                                   const LAString & businessDayAdj,           // TODO: Convert to StandardString
                                   const LAString & calendar,                 // TODO: Convert to StandardString
                                   const LAString & rollConvention )          // TODO: Convert to StandardString
    {
        const LAString* rollConvPointer = ( rollConvention.size() == 0 ) ? nullptr : &rollConvention;
        LADate result = LADateScheduleHelpers::getDateWithRollConv( startDate, tenor, businessDayAdj, calendar, rollConvPointer );
        return result;
    }

    /* @brief			Calculate toDate based on startDates and tenor
    *  @param [in]		startDates			A vector of from dates
    *  @param [in]		tenor				Tenor added to the from date
    *  @param [in]		businessDayAdj		Rule that decides if end date should roll forward or backward etc, ie, MF
    *  @param [in]		calendar			Calendar
    *  @param [in]		rollConvention		Roll conventions, ie, Normal, IMM, EOM, Lunar, etc
    *  @return			A vector of end dates
    */
    DateVector getDateFromTenor( const DateVector& startDates,
                                 const LAString& tenor,                 // TODO: Convert to StandardString
                                 const LAString& businessDayAdj,        // TODO: Convert to StandardString
                                 const LAString& calendar,              // TODO: Convert to StandardString
                                 const LAString& rollConvention )       // TODO: Convert to StandardString
    {
        
        DateVector result;

        if ( rollConvention.size() != 0 )
        {
            MLIB_REQUIRE( boost::iequals( rollConvention.c_str(), "IMM" ) || boost::iequals( rollConvention.c_str(), "EOM" ), "Invalid RollConvention: Must be IMM, EOM or blank." )
        }

		//Handle tenor input as number, e.g. tenor 1, 2 same as 1D, 2D.
		auto tenorToUse = tenor;
		if (isNumber(tenor))
		{
			tenorToUse = tenor + "D";
		}

        MLIB_TRY( result = LADateScheduleHelpers::getMultiDate( startDates, tenorToUse, businessDayAdj, calendar, rollConvention.size() == 0 ? nullptr : &rollConvention ) , "Invalid Date/Tenor Input: Invalid Date, TenorString (no spaces), Calendar or BusinessDayAdj" );
        return result;
    }

    /* @brief			Method to get the curve spot date by applying multiple date shifts
    *  @param [in]		asOfDate		        The curve asOfDate
    *  @param [in]		fixingLag			    Tenor added to the from date
    *  @param [in]		fixingCalendar		    Fixing Calendar
    *  @param [in]		fixingBusDayAdj		    Rule that decides if end date should roll forward or backward etc, ie, MF
    *  @param [in]		paymentLag			    Tenor added to the from date
    *  @param [in]		paymentCalendar		    Fixing Calendar
    *  @param [in]		paymentBusDayAdj        Rule that decides if end date should roll forward or backward etc, ie, MF
    *  @return			The curve spot date
    */
	LADate getShiftedSpotDate(const LADate& asOfDate,
						      const LAString& fixingLag,			// TODO: Convert to StandardString
						      const LAString& fixingCalendar,		// TODO: Convert to StandardString
                              const LAString& fixingBusDayAdj,		// TODO: Convert to StandardString
                              const LAString& paymentLag,			// TODO: Convert to StandardString
						      const LAString& paymentCalendar,		// TODO: Convert to StandardString
                              const LAString& paymentBusDayAdj) 	// TODO: Convert to StandardString
    {
        LADate spotDate;

		// *** The Rule ***
        // (1) First adjust the asOfDate using the fixing lag and conventions
        // (2) Second adjust the result from (1) using the payment lag and conventions
		// Note: The multi-date method takes curveAsOfDate as a vector and returns a vector
        
		// 1.	Adjust the AsOfDate using the fixing lag and conventions
		MLIB_TRY( spotDate = LADateScheduleHelpers::getMultiDate( DateVector(1,asOfDate), fixingLag, fixingBusDayAdj, fixingCalendar, nullptr /* rollConvenction */ )[0] , "Invalid Shifted Spot Date: Invalid Input - AsOfDate, FixingLag, FixingCalendar or FixingBusDayAdj" );
        
		// 2.	Adjust the Result from (1) using the payment lag and conventions
		MLIB_TRY( spotDate = LADateScheduleHelpers::getMultiDate( DateVector(1,spotDate), paymentLag, paymentBusDayAdj, paymentCalendar, nullptr /* rollConvenction */ )[0] , "Invalid Shifted Spot Date: Invalid Input - PaymentLag, PaymentCalendar or PaymentBusDayAdj" );
		return spotDate;
    }

	/* @brief			Calculate the curve spot date based on the curve asOfDate, spot lag, fixing calendar and payment calendar
    *  @param [in]		curveAsOfDate		The curve asOfDate
    *  @param [in]		spotLag				Tenor added to the from date
    *  @param [in]		fixingCalendar		Fixing Calendar
	*  @param [in]		paymentCalendar		Payment Calendar
	*  @param [in]		businessDayAdj		Rule that decides if end date should roll forward or backward etc, ie, MF
    *  @param [in]		rollConvention		Roll conventions, ie, Normal, IMM, EOM, Lunar, etc
    *  @return			The curve spot date
    */
    LADate getCurveUSDSpotDate( const LADate& curveAsOfDate,
                                const LAString& spotLag,               // TODO: Convert to StandardString
                                const LAString& fixingCalendar,        // TODO: Convert to StandardString
							    const LAString& paymentCalendar,       // TODO: Convert to StandardString
							    const LAString& businessDayAdj,        // TODO: Convert to StandardString
                                const LAString& rollConvention )       // TODO: Convert to StandardString
    {
        LADate spotDate;
		if ( rollConvention.size() != 0 )
        {
            MLIB_REQUIRE( boost::iequals( rollConvention.c_str(), "IMM" ) || boost::iequals( rollConvention.c_str(), "EOM" ), "Invalid RollConvention: Must be IMM, EOM or blank." )
        }

		// *** The Rule *** Adjust the curve asOfDate by the spot lag on fixing calendar then at roll to next payment calendar business day 
		// Note: The multi-date method takes curveAsOfDate as a vector and returns a vector
        
		// 1.	Adjust curveAsOfDate by spot Lag with fixing calendar
		MLIB_TRY( spotDate = LADateScheduleHelpers::getMultiDate( DateVector(1,curveAsOfDate), spotLag, businessDayAdj, fixingCalendar, rollConvention.size() == 0 ? nullptr : &rollConvention )[0] , "Invalid Curve Spot Date: Invalid Input - CurveAsOfDate, SpotLag, FixingCalendar or BusinessDayAdj" );
        
		// 2.	Move adjust the spot date for payment calendar holidays
		MLIB_TRY( spotDate = LADateScheduleHelpers::getMultiDate( DateVector(1,spotDate), "0D", businessDayAdj, paymentCalendar, rollConvention.size() == 0 ? nullptr : &rollConvention )[0] , "Invalid Curve Spot Date: Invalid Input: PaymentCalendar" );
		return spotDate;
    }

    /* @brief			Calculate toDate based on startDate and yearFraction
    *  @param [in]		startDate			Start date
    *  @param [in]		yearFraction		Year fraction
    *  @param [in]		dayCount			Day count convention
    *  @return			The end date derived from the FromDate and given year fraction
    */
    LADate getDateFromYearFraction( const LADate& startDate,
                                   const double& yearFraction,
                                   const DayCountEnum& dayCount)
    {
		LADate startDateCopy( startDate );
        LAString dayCountString( toString(dayCount).c_str() );
        LADate ret = LADateScheduleHelpers::getDateFromTerm( startDateCopy, yearFraction, dayCountString );

        return ret;
    }



    //This function is the same as etrading::LACurveForwardRateHelpers::compound() function, but making use of the asOfDate and fixing table for past fixing dates
    double compound(LADataInstance* dataInstance,
                    const LAString& curveID,
                    const LAString& forecastCurveName,
                    const LADate& start_date,
                    const LADate& end_date,
                    const double spread,
                    const LAString& frequency_,
                    const bool is_start_roll,
                    const LAString& roll_convention_,
                    const LAString& calendar_,
                    const LAString& sliding_rule_,
                    const LAString& day_count_,
                    const LAString& interpolation_,
                    const LAString& compound_type_,
					const bool isFwdInter,
					const std::shared_ptr<FixingTable>& fixingTable,  //fixing table
					bool annualized,                                  //true to return annualized rate, default to true
					const LADate* first_odd,
                    const LADate* last_odd,
					const bool usePDay)
    {
        LAString frequency = frequency_; frequency.toUpper();
        LAString roll_convention = roll_convention_; roll_convention.toUpper();
        LAString calendar = calendar_; calendar.toUpper();
        LAString sliding_rule = sliding_rule_; sliding_rule.toUpper();
        LAString day_count = day_count_; day_count.toUpper();
        LAString interpolation = interpolation_; interpolation.toUpper();
        LAString compound_type_str = compound_type_; compound_type_str.toUpper();
    
        etrading::LACurveForwardRateHelpers::COMPOUND_TYPE compound_type;
        if(compound_type_str=="NORMAL")         compound_type = etrading::LACurveForwardRateHelpers::COMPOUND_NORMAL;
        else if(compound_type_str=="FLAT")      compound_type = etrading::LACurveForwardRateHelpers::COMPOUND_FLAT;
        else if(compound_type_str=="SIMPLE")    compound_type = etrading::LACurveForwardRateHelpers::COMPOUND_SIMPLE;
        else if(compound_type_str=="AVERAGE")   compound_type = etrading::LACurveForwardRateHelpers::AVERAGE;
        else
        {
            LAString msg;
            msg += "Unknown compound type:";
            msg += compound_type_;
            throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
        }

	
        LAPriceDataCalendar data_cal; data_cal.convertFromString(calendar);
        LAPriceDataSlidingRule data_sr; data_sr.convertFromString(sliding_rule);

        DateVector payment_dates;

	    // The rollDay(roll_convention param here) can be either string, NORMAL,STARTDATE,ENDDATE,LUNAR,EOM,IMM; or numeric type
	    // If the rollDay is numeric type, pass it to Pday; otherwise, pass it to rollConv
	    LAString* rollConv = &roll_convention;	
	    int* pDay = NULL;
	    int tempDay=0;
	    if (usePDay) 
	    {
		    if (isNumber(roll_convention)) 
		    {
			    tempDay =  roll_convention.getIntValue();
			    pDay = &tempDay;
			    rollConv = NULL;
		    }
	    }

        // payment_dates: daily fixing dates from start_date(exclusive) till end_date(inclusive)
        etrading::LADateHelpers::generateSchedule(start_date,
                                                 end_date,
                                                 frequency,
                                                 true, 
                                                 first_odd,
                                                 last_odd,
                                                 pDay, 
                                                 payment_dates,
                                                 &data_sr,
                                                 &data_cal,
                                                 is_start_roll,
                                                 rollConv);

        // allFixingDates: daily fixing dates from start_date(inclusive) till end_date(inclusive)
        DateVector allFixingDates;
        allFixingDates.reserve(payment_dates.size() + 1);
        allFixingDates.push_back(start_date);
        allFixingDates.insert(allFixingDates.end(), payment_dates.begin(), payment_dates.end());

        // Seperate the past & non-past fixing dates, 
        DateVector pastFixingDates;
        pastFixingDates.reserve(allFixingDates.size());
        DateVector nonPastFixingDates;
        nonPastFixingDates.reserve(allFixingDates.size());
        

    	const LADataDate& atr = dynamic_cast<const LADataDate& >(dataInstance->getObjectPool().getObject(curveID,ENCHKTYPE_ISDEFINED).get().getData(CALIBRATION_DATA_ASOFDATE,ISNOTNULL).get());
	    const LADate asOfDate = atr.get();

        if (end_date < asOfDate)
        {
            pastFixingDates = allFixingDates; 
        } 
        else if (start_date < asOfDate)
        {
            //Check whether the fixing date is past one by one
            for(size_t i = 0; i < allFixingDates.size(); i++)
            {
                auto fixingDt = allFixingDates[i];
                if (fixingDt < asOfDate)
                {
                    pastFixingDates.push_back(fixingDt);
                }
                else
                {
                    nonPastFixingDates.push_back(fixingDt);
                }
            }
        }
        //start_date >= asOfDate
        else 
        {
            nonPastFixingDates = allFixingDates;
        }

        // Getting the past rates from the fixing table, excluding the final fixing date 
        DoubleArray pastRates;

        if (pastFixingDates.size() > 1) 
        {

            size_t pastFixingDuration = pastFixingDates.size() - 1;
            pastRates.reserve(pastFixingDuration);
    
            if (pastFixingDuration > 0 && fixingTable == nullptr)
            {
    	        throw LACoreInvalidData("#Error: FixingTable required", __FILE__, __LINE__ );
            }
    
            for(size_t i = 0; i < pastFixingDuration; i++)
            {
                auto fixingDate = pastFixingDates[i];
                double rate = fixingTable->getFixingValue( toGregorianDateFromLADate(fixingDate) );
                pastRates.push_back(rate);
            }
        }

        // Getting the non past rates from the curve 
        DoubleArray nonPastRates;
        if (nonPastFixingDates.size() > 1) 
        {
            // m is the size of duration
            const size_t m = nonPastFixingDates.size() - 1;

            DateVector start_dates(m), end_dates(m);

            for(size_t i = 0; i < m; i++){
                start_dates[i] = nonPastFixingDates[i];
                end_dates[i] = nonPastFixingDates[i+1];
            }

            const LAString freq = SIMPLE;

            // Getting the non-past rates from the curve 
            nonPastRates = etrading::LACurveForwardRateHelpers::getMultiForwardRate(start_dates,
                                                                               end_dates,
                                                                               dataInstance,
                                                                               curveID,
                                                                               freq,
                                                                               day_count,
                                                                               sliding_rule,
                                                                               calendar,
                                                                               interpolation,
                                                                               forecastCurveName,
																			   isFwdInter,
																			   isFwdInter /* useFwdData */); 
        }

        // All the rates
        DoubleArray rates;
        rates.reserve(pastRates.size() + nonPastRates.size());
        rates = pastRates;
        rates.insert(rates.end(), nonPastRates.begin(), nonPastRates.end());

        // n is the size of duration
        size_t n = allFixingDates.size() - 1;

        //Average all the rates
        LAPriceDataDayCount dc; dc.convertFromString(day_count);
        DoubleVector x(3*n);
        copy(rates.begin(), rates.end(), x.begin());

        double yearFractionSum = 0.0;

        for(size_t i = 0; i < n; i++)
        {
            auto yearFraction = dc.getTerm(allFixingDates[i], allFixingDates[i+1]);
            yearFractionSum +=  yearFraction;
            x[n+i]     = yearFraction;
            x[n*2 + i] = spread;
        }
        std::shared_ptr<LACompoundMethod> method;
        switch(compound_type)
        {
                case etrading::LACurveForwardRateHelpers::COMPOUND_NORMAL:
                {
                    method.reset(new LACompoundMethod7());
                    break;
                }
                case etrading::LACurveForwardRateHelpers::COMPOUND_FLAT:
                {
                    method.reset(new LACompoundMethod8());
                    break;
                }
                case etrading::LACurveForwardRateHelpers::COMPOUND_SIMPLE:
                {
                    method.reset(new LACompoundMethod9());
                    x.push_back(dc.getTerm(start_date, end_date));
                    break;
                }
                case etrading::LACurveForwardRateHelpers::AVERAGE:
                {
                    method.reset(new LACompoundMethod11());
                    break;
                }
        }

        double rate = (*method)(x);

        if (annualized)
        {
          rate = rate/yearFractionSum;
        }
        return rate;
    }

    /* @brief			Calculate compound rate with fixing table
    *  @param [in]		startDates			A vector of start dates
    *  @param [in]		endDates			A vector of end dates
    *  @param [in]		curveCollection		ID of the curve set
    *  @param [in]		forecastCurveIndex	Forecasting curve index
    *  @param [in]		frequency			Frequency
    *  @param [in]		spread				Spread in basis points.
    *  @param [in]		stubType			StubType: None, ShortStart (SS), LongStart (LS), ShortEnd (SE) or LongEnd (LE).
    *  @param [in]		rollDayInput		Roll Day conventions, e.g. ENDDATE
    *  @param [in]		calendar			Calendar
    *  @param [in]		businessDayAdj		Rule that decides if end date should roll forward or backward etc, ie, MF
    *  @param [in]		dayCount			Day count convention
    *  @param [in]		interpolation		Interpolation method
    *  @param [in]		compoundType		Compound Type: NORMAL, FLAT, SIMPLE, SIMPLE, AVERAGE
    *  @param [in]		firstStubDate		End date of the front stub period
    *  @param [in]		lastStubDate		Start date of the end stub period
	*  @param [in]		fwdInter			User input fwdInter flag
	*  @param [in]		fixingTable		    Fixing table object
	*  @param [in]		annualized  		True to return annualized rate. Default to true
    *  @return			interest rates calculated based on the startDates, endDates, and compound type
    */
     DoubleVector calculateCompoundRateWithFixingTable( const DateVector& startDates,
                                         const DateVector& endDates,
                                         const LAString& curveCollection,
                                         const LAString& forecastCurveIndex,
                                         const LAString& frequency,
                                         double spread,
                                         const StubTypeEnum& stubType,
                                         const LAString& rollDayInput,
                                         const LAString& calendar,
                                         const BusinessDayAdjustmentEnum& businessDayAdj,
                                         const DayCountEnum& dayCount,
                                         const LAString& interpolation,
                                         const LAString& compoundType,
                                         const LAString& firstStubDate,
                                         const LAString& lastStubDate,
										 const BooleanEnum& fwdInter,
										 const std::shared_ptr<FixingTable>& fixingTable,
										 bool annualized)
    {

        if( startDates.size() == 0 || endDates.size() == 0 )
        {
            throw LACoreInvalidData( "#Error: a size of vector is zero.", __FILE__, __LINE__ );
        }

        if( startDates.size() != endDates.size() )
        {
            throw LACoreInvalidData( "#Error: The input startDates and endDates must be of the same size.", __FILE__, __LINE__ );
        }

        LADate* firstOddDt  = NULL;
        LADate tmpfirstOddDt;
        if ( firstStubDate.size() != 0 )
        {
            tmpfirstOddDt = stringToDate( firstStubDate, "#Error: Invalid 'firstStubDate'." );
            firstOddDt = &tmpfirstOddDt;
        }
        LADate* lastOddDt  = NULL;
        LADate tmplastOddDt;
        if ( lastStubDate.size() != 0 )
        {
            tmplastOddDt = stringToDate( lastStubDate, "#Error: Invalid 'lastStubDate'." );
            lastOddDt = &tmplastOddDt;
        }

 
        LAString rollD(getDefaultValueForEmptyString( rollDayInput, "ENDDATE" ) );
        LAString cType( getDefaultValueForEmptyString( compoundType, "FLAT" ) );

		LAString curIndex(forecastCurveIndex);
		LAString interp;
		LAString bdAdj = toString(businessDayAdj).c_str();
		LAString cal = calendar;
		LAString dayC("");

		bool isFwdInter;
		bool useFwdData;

		populateForwardRateConventions(curveCollection, curIndex, fwdInter, interp, bdAdj, cal, dayC, isFwdInter, useFwdData);

        DoubleVector ret;
        LADataInstance* dataInstance = etrading::getDataInstance();

        for( size_t i = 0; i < startDates.size(); i++ )
        {

            LADate startDt = startDates[i];
            LADate endDt = endDates[i];

            bool isStartRoll = etrading::isStartRollAndPopulateStubDatesFromStubType( stubType, firstOddDt, lastOddDt, startDt, endDt, frequency );

            double result = compound( dataInstance,
                            curveCollection,
                            forecastCurveIndex,
                            startDt,
                            endDt,
                            spread,
                            frequency,
                            isStartRoll,
                            rollD,
                            cal,
                            bdAdj,
                            dayC,
                            interp,
                            cType,
							isFwdInter,
							fixingTable,
							annualized,
							firstOddDt,
                            lastOddDt,
                            true );
            ret.push_back( result );
        }
       
        return ret;

        }

     void fixingTableExistenceValidation(const std::shared_ptr<FixingTable>& fixingTable, const LADate& asOfDate, const DateVector& fixingDates, const DateVector& paymentDates)
     {

       // If no fixing table is specified
		if ( fixingTable == nullptr)
		{
            for( size_t i = 0; i < fixingDates.size(); ++i )
            {
                LADate fixingDate = fixingDates[i];
                LADate paymentDate = paymentDates[i];
    
                // If fixingDate in the past and paymentDate is in the future, a fixing table is required
                if (fixingDate < asOfDate && paymentDate >= asOfDate)
                {
                    LAString erroMsg = "#Error: FixingTable required for fixingDate: " + fixingDate.stringWithFormat("DD-MM-YYYY");
        		    throw LACoreInvalidData( erroMsg.getCString(), __FILE__, __LINE__ );
                }
            }
		}
     }

    /* @brief			Method to interpolate an interpolation function
    *  @param [in]		xValues			X-axis values of the data
    *  @param [in]		yValues			Y-axis values of the data
    *  @param [in]		xPoint			X-axis point
    *  @param [in]		interpolation	Interpolation method
	*  @param [in]		joinDate		A join date as a double value used in hybrid interpolation
    *  @return			the Y-axis interpolated value
    */
    double interpolate( const DoubleVector& xValues,
                        const DoubleVector& yValues,
                        const double xPoint,
                        const InterpolationEnum& interpolationMethod,
						const double joinXValue ) 
    {
     
		MLIB_REQUIRE( xValues.size() == yValues.size(), "Interpolation x and y vectors must be the same size" )
		MLIB_REQUIRE( xValues.size() > 0, "Interpolation x and y data is empty" )
		return LAMathInterpolationUtilities::interpolate( xValues, yValues, xPoint, interpolationMethod, joinXValue);
    }

	// Interpolate a vector of xPoints
	DoubleVector interpolate( const DoubleVector& xValues,
							  const DoubleVector& yValues,
							  const DoubleVector xPoints,
							  const InterpolationEnum& interpolationMethod,
							  const double joinXValue )
	{
		MLIB_REQUIRE( xValues.size() == yValues.size(), "Interpolation x and y vectors must be the same size" )
		MLIB_REQUIRE( xValues.size() > 0, "Interpolation x and y data is empty" )
		return LAMathInterpolationUtilities::interpolate( xValues, yValues, xPoints, interpolationMethod, joinXValue);
	}

	/* @brief			Method to differentiate an interpolation function at a single point
	*  @param [in]		xValues			X-axis values of the data
	*  @param [in]		yValues			Y-axis values of the data
	*  @param [in]		xPoint			X-axis point
	*  @param [in]		interpolation	Interpolation method
	*  @param [in]		joinXValue		A join date as a double value used in hybrid interpolation
	*  @return			the Y-axis interpolated value
	*/
    double differentiate( const DoubleVector& xValues,
                          const DoubleVector& yValues,
                          const double xPoint,
                          const InterpolationEnum& interpolationMethod,
						  const double joinXValue ) 
    {
		MLIB_REQUIRE( xValues.size() == yValues.size(), "Interpolation x and y vectors must be the same size" )
		MLIB_REQUIRE( xValues.size() > 0, "Interpolation x and y data is empty" )
		return LAMathInterpolationUtilities::differentiate( xValues, yValues, xPoint, interpolationMethod, joinXValue );
    }

	/* @brief			Method to differentiate an interpolation function on a vector of single points
	*  @param [in]		xValues			X-axis values of the data
	*  @param [in]		yValues			Y-axis values of the data
	*  @param [in]		xPoint			X-axis point
	*  @param [in]		interpolation	Interpolation method
	*  @param [in]		joinXValue		A join date as a double value used in hybrid interpolation
	*  @return			the Y-axis interpolated value
	*/
	DoubleVector differentiate( const DoubleVector& xValues,
							    const DoubleVector& yValues,
							    const DoubleVector xPoints,
							    const InterpolationEnum& interpolationMethod,
							    const double joinXValue )
	{
		MLIB_REQUIRE( xValues.size() == yValues.size(), "Interpolation x and y vectors must be the same size" )
		MLIB_REQUIRE( xValues.size() > 0, "Interpolation x and y data is empty" )
		return LAMathInterpolationUtilities::differentiate( xValues, yValues, xPoints, interpolationMethod, joinXValue );
	}

	/* @brief			Method to differentiate an interpolation function discreteley given a start and end point
	*  @param [in]		xValues			X-axis values of the data
	*  @param [in]		yValues			Y-axis values of the data
	*  @param [in]		xPoint			X-axis point
	*  @param [in]		interpolation	Interpolation method
	*  @param [in]		joinXValue		A join date as a double value used in hybrid interpolation
	*  @return			the Y-axis interpolated value
	*/
	double differentiate( const DoubleVector& xValues,
						  const DoubleVector& yValues,
						  const double fromXPoint,
						  const double toXPoint,
						  const InterpolationEnum& interpolationMethod,
						  const LADate & asOfDate,
						  const DayCountEnum & accrualDaycount,
						  const CompoundingFrequencyEnum & compoundFrequency,
						  const double joinXValue )
	{
		MLIB_REQUIRE( xValues.size() == yValues.size(), "Interpolation x and y vectors must be the same size" )
		MLIB_REQUIRE( xValues.size() > 0, "Interpolation x and y data is empty" )
		
		// Native derivatives will be in the curve Act365 daycount basis
		const double derivativeAct365 = LAMathInterpolationUtilities::differentiate( xValues, yValues, fromXPoint, toXPoint, interpolationMethod, joinXValue );
		
		// *** IMPORTANT *** Calculate derivative in the accrual daycount basis not the internal curve Act/365 daycount basis
		const double derivativeAccrualBasis = derivativeAct365 * daycountConversionFactor( fromXPoint, toXPoint, asOfDate, accrualDaycount, compoundFrequency );
		return derivativeAccrualBasis;
	}

	/* @brief			Method to differentiate an interpolation function discreteley given a vector of ranges
	*  @param [in]		xValues			X-axis values of the data
	*  @param [in]		yValues			Y-axis values of the data
	*  @param [in]		xPoint			X-axis point
	*  @param [in]		interpolation	Interpolation method
	*  @param [in]		joinXValue		A join date as a double value used in hybrid interpolation
	*  @return			the Y-axis interpolated value
	*/
	DoubleVector differentiate( const DoubleVector& xValues,
								const DoubleVector& yValues,
								const DoubleVector fromXPoints,
								const DoubleVector toXPoints,
								const InterpolationEnum& interpolationMethod,
								const LADate & asOfDate,
								const DayCountEnum & accrualDaycount,
								const CompoundingFrequencyEnum & compoundFrequency,
								const double joinXValue )
	{
		MLIB_REQUIRE( xValues.size() == yValues.size(), "Interpolation x and y vectors must be the same size" )
		MLIB_REQUIRE( xValues.size() > 0, "Interpolation x and y data is empty" )
		
		// Native derivatives will be in the curve Act365 daycount basis
		const DoubleVector derivativesAct365 = LAMathInterpolationUtilities::differentiate( xValues, yValues, fromXPoints, toXPoints, interpolationMethod, joinXValue );
		
		// *** IMPORTANT *** Calculate derivatives in the accrual daycount basis not the internal curve Act/365 daycount basis
		DoubleVector derivativesAccrualBasis = derivativesAct365;
		for( size_t i = 0; i < derivativesAccrualBasis.size(); ++i )
		{
			derivativesAccrualBasis[i] *= daycountConversionFactor( fromXPoints[i], toXPoints[i], asOfDate, accrualDaycount, compoundFrequency );
		}
		return derivativesAccrualBasis;
	}

	/* @brief			Method to differentiate an integrate function
    *  @param [in]		xValues			X-axis values of the data
    *  @param [in]		yValues			Y-axis values of the data
    *  @param [in]		xPoint			X-axis point
    *  @param [in]		interpolation	Interpolation method
	*  @param [in]		joinDate		A join date as a double value used in hybrid interpolation
    *  @return			the Y-axis interpolated value
    */
    double integrate( const DoubleVector& xValues,
                      const DoubleVector& yValues,
                      const double lowerBound,
					  const double upperBound,
                      const InterpolationEnum& interpolationMethod,
					  const LADate & asOfDate,
					  const DayCountEnum & accrualDaycount,
					  const CompoundingFrequencyEnum & compoundFrequency,
					  const double joinXValue ) 
    {
		MLIB_REQUIRE( xValues.size() == yValues.size(), "Interpolation x and y vectors must be the same size" )
		MLIB_REQUIRE( xValues.size() > 0, "Interpolation x and y data is empty" )

		// Boundary Condition
		if ( MLIB_IS_EQUAL( lowerBound, upperBound ) )
		{
			return 0.0;
		}

		// Set Daycount Adjustment
		// The different choices were for performing tests, generally transformation of outputs is the required choice.
		const IntegrationDaycountAdjustment parameterAdjustment = IntegrationDaycountAdjustment::ADJUST_OUTPUTS;

		switch ( parameterAdjustment )
		{
			case IntegrationDaycountAdjustment::ADJUST_INPUTS:
			{
				// Transform the Inputs from Act/365 to Accrual Daycount Basis
				// ===========================================================
				// *** IMPORTANT *** Calculate xValues in the accrual daycount basis not the internal curve Act/365 daycount basis
				DoubleVector xValuesAccrualBasis = xValues;
				for( size_t i = 0; i < xValuesAccrualBasis.size(); ++i )
				{
					xValuesAccrualBasis[i] *= daycountConversionFactor( 0.0, xValues[i], asOfDate, accrualDaycount, compoundFrequency );
				}
				
				// Transform: Bounds into Accrual Daycount Basis
				const double lowerBoundAccrualBasis = lowerBound * daycountConversionFactor( 0.0, lowerBound, asOfDate, accrualDaycount, compoundFrequency );
				const double upperBoundAccrualBasis = upperBound * daycountConversionFactor( 0.0, upperBound, asOfDate, accrualDaycount, compoundFrequency );
				const double joinXValueAccrualBasis = joinXValue * daycountConversionFactor( 0.0, joinXValue, asOfDate, accrualDaycount, compoundFrequency );
				
				// Get the integrals in the accrual daycount basis
				const double integralAccrualBasis = LAMathInterpolationUtilities::integrate( xValuesAccrualBasis, yValues, lowerBoundAccrualBasis, upperBoundAccrualBasis, interpolationMethod, joinXValueAccrualBasis);
				return integralAccrualBasis;
			}
			case IntegrationDaycountAdjustment::ADJUST_OUTPUTS:
			{
				// Transform the Outputs from Act/365 to Accrual Daycount Basis
				// ===========================================================
				// Get the integrals in the accrual daycount basis
				const double integralAct365 = LAMathInterpolationUtilities::integrate( xValues, yValues, lowerBound, upperBound, interpolationMethod, joinXValue);
				
				// *** IMPORTANT *** Calculate xValues in the accrual daycount basis not the internal curve Act/365 daycount basis
				const double integralAccrualBasis = integralAct365 * daycountConversionFactor( lowerBound, upperBound, asOfDate, accrualDaycount, compoundFrequency );
				return integralAccrualBasis;
			}
			case IntegrationDaycountAdjustment::UNCHANGED:
			{
				// Keep the integral in the curve Act/365 daycount basis
				// ===========================================================
				const double integralAct365 = LAMathInterpolationUtilities::integrate( xValues, yValues, lowerBound, upperBound, interpolationMethod, joinXValue);
				return integralAct365;
			}
			default:
			{
				MLIB_THROW( "Integration Error: Invalid Interpolation Daycount Transformation" )
			}
		}
    }

	// Integrate a vector of xPoints
	DoubleVector integrate( const DoubleVector& xValues,
							const DoubleVector& yValues,
							const DoubleVector lowerBounds,
							const DoubleVector upperBounds,
							const InterpolationEnum& interpolationMethod,
							const LADate & asOfDate,
							const DayCountEnum & accrualDaycount,
							const CompoundingFrequencyEnum & compoundFrequency,
							const double joinXValue )
	{
		MLIB_REQUIRE( xValues.size() == yValues.size(), "Interpolation x and y vectors must be the same size" )
		MLIB_REQUIRE( xValues.size() > 0, "Interpolation x and y data is empty" )

		// Set Daycount Adjustment
		// The different choices were for performing tests, generally transformation of outputs is the required choice.
		const IntegrationDaycountAdjustment parameterAdjustment = IntegrationDaycountAdjustment::ADJUST_OUTPUTS;

		switch ( parameterAdjustment )
		{ 
			case IntegrationDaycountAdjustment::ADJUST_INPUTS:
			{
				// Transform the Inputs from Act/365 to Accrual Daycount Basis
				// ===========================================================
				// *** IMPORTANT *** Calculate xValues in the accrual daycount basis not the internal curve Act/365 daycount basis
				DoubleVector xValuesAccrualBasis = xValues;
				for( size_t i = 0; i < xValuesAccrualBasis.size(); ++i )
				{
					xValuesAccrualBasis[i] *= daycountConversionFactor( 0.0, xValues[i], asOfDate, accrualDaycount, compoundFrequency );
				}
				
				// Transform: Bounds into Accrual Daycount Basis
				DoubleVector lowerBoundsAccrualBasis = lowerBounds;
				DoubleVector upperBoundsAccrualBasis = upperBounds;
				for( size_t i = 0; i < xValuesAccrualBasis.size(); ++i )
				{
					lowerBoundsAccrualBasis[i] = lowerBounds[i] * daycountConversionFactor( 0.0, lowerBounds[i], asOfDate, accrualDaycount, compoundFrequency );
					upperBoundsAccrualBasis[i] = upperBounds[i] * daycountConversionFactor( 0.0, upperBounds[i], asOfDate, accrualDaycount, compoundFrequency );
				}
				const double joinXValueAccrualBasis = joinXValue * daycountConversionFactor( 0.0, joinXValue, asOfDate, accrualDaycount, compoundFrequency );		
				
				// Get the integrals in the accrual daycount basis, *** IMPORTANT *** use xValuesAccrualBasis
				const DoubleVector integralsAccrualBasis = LAMathInterpolationUtilities::integrate( xValuesAccrualBasis, yValues, lowerBoundsAccrualBasis, upperBoundsAccrualBasis, interpolationMethod, joinXValueAccrualBasis);
				return integralsAccrualBasis;
			}
			case IntegrationDaycountAdjustment::ADJUST_OUTPUTS:
			{
				// Transform the Outputs from Act/365 to Accrual Daycount Basis
				// ===========================================================
				// Get the integrals in the accrual daycount basis
				const DoubleVector integralAct365 = LAMathInterpolationUtilities::integrate( xValues, yValues, lowerBounds, upperBounds, interpolationMethod, joinXValue);
				
				// *** IMPORTANT *** Calculate xValues in the accrual daycount basis not the internal curve Act/365 daycount basis
				DoubleVector integralAccrualBasis = integralAct365;
				for( size_t i = 0; i < integralAccrualBasis.size(); ++i )
				{
					integralAccrualBasis[i] *= daycountConversionFactor( lowerBounds[i], upperBounds[i], asOfDate, accrualDaycount, compoundFrequency );
				}
				return integralAccrualBasis;
			}
			case IntegrationDaycountAdjustment::UNCHANGED:
			{
				// Keep the integral in the curve Act/365 daycount basis
				// ===========================================================
				const DoubleVector integralAct365 = LAMathInterpolationUtilities::integrate( xValues, yValues, lowerBounds, upperBounds, interpolationMethod, joinXValue);
				return integralAct365;
			}
			default:
			{
				MLIB_THROW( "Integration Error: Invalid Interpolation Daycount Transformation" )
			}
		}
	}

    /* @brief			Calculate the discount factors from baseDate to targetDates
    *  @param [in]		baseDate	    The date the discount factor is based on
    *  @param [in]		targetDates		A single or an array of to-date 
    *  @param [in]		curveCollection	Curve set ID
    *  @param [in]		curveIndex		The curve index for which to calculate Discount Factors. Default to OIS
    *  @return			A array of discount factors
    */
    DoubleVector getCurveDiscountFactors( const LADate& baseDate,
										  const DateVector& targetDates,
                                          const LAString& curveCollection,
                                          const LAString& curveIndex )
    {

		const DoubleVector dfAsOfToTargetDates = getCurveDiscountFactors(targetDates, curveCollection, curveIndex);

		const LADate curveAsOfDate = etrading::getCurveAsOfDate(curveCollection);

		if (baseDate == curveAsOfDate)
		{
			return dfAsOfToTargetDates;
		}
		else
		{
			const double dfAsOfToBaseDate = getCurveDiscountFactors(boost::assign::list_of(baseDate), curveCollection, curveIndex)[0];

			size_t expectedSize = dfAsOfToTargetDates.size();
			DoubleVector dfBaseToTargetDates(expectedSize);

			// DF(baseDate, targetDate) = DF(asOf, targetDate)/DF(asOf, baseDate)
			for (size_t i = 0; i < expectedSize; ++i)
			{
				double dfAsOfToTargetDate = dfAsOfToTargetDates[i];

				double df = dfAsOfToTargetDate / dfAsOfToBaseDate;

				dfBaseToTargetDates[i] = df;
			}
			return dfBaseToTargetDates;
		}
    }

	/* @brief			Calculate the discount factors from FromDates to ToDates
	*  @param [in]		fromDates	    A single or an array of From-date
	*  @param [in]		toDates			A single or an array of To-date
	*  @param [in]		curveCollection	Curve set ID
	*  @param [in]		curveIndex		The curve index for which to calculate Discount Factors. Default to OIS
	*  @return			A array of discount factors
	*/
	DoubleVector getCurveDiscountFactors(const DateVector& fromDates, const DateVector& toDates, const LAString& curveCollection, const LAString& curveIndex)
	{
		DoubleVector dfAsOfToDates = getCurveDiscountFactors(toDates, curveCollection, curveIndex);
		DoubleVector dfAsOfFromDates = getCurveDiscountFactors(fromDates, curveCollection, curveIndex);

		size_t expectedSize = dfAsOfToDates.size();
		DoubleVector dfFromToDates(expectedSize);

		// DF(fromDate, toDate) = DF(asOf, toDate)/DF(asOf, fromDate)
		for (size_t i = 0; i < expectedSize; ++i)
		{
			double dfAsOfToDate = dfAsOfToDates[i];
			double dfAsOfFromDate = dfAsOfFromDates[i];

			double df = dfAsOfToDate / dfAsOfFromDate;

			dfFromToDates[i] = df;
		}

		return dfFromToDates;
	}

	/* @brief			Converts a given set of yearFractions to ACT/365 yearFractions for Discount factor functions
    *  @param [in]		startDate	    start date
    *  @param [in]		yearFractions	A list of year fractions
    *  @param [in]		dayCount		Day count
    *  @param[in]		includelast		True(default):include the last day and not include start day; False:include start day and not include last day
    *  @return			Year fraction based on ACT365
    */
	void convertToYearFractionsACT365( const LADate& startDate, DoubleVector& yearFractions, const LAString& dayCount, const bool& includeLast )
    {
        LAString ACT365= "ACT/365";

        LAString dc = dayCount;
        if ( dc.toUpper() == ACT365 )
        {
            return;    // Do nothing if the daycount is already ACT365
        }

        for ( unsigned int i = 0; i < yearFractions.size(); ++i )
        {
			LADate startDateCopy( startDate );
			LAString dayCountCopy( dayCount );
            const LADate endDate        = LADateScheduleHelpers::getDateFromTerm( startDateCopy, yearFractions[i], dayCountCopy );
            yearFractions[i]            = LADateScheduleHelpers::getTerm( startDate, endDate, ACT365, includeLast );
        }
    }


	/* @brief			Get curve discount factor based on fromDate and year fractions
    *  @param [in]		fromDates			An array of from-dates in YYYYMMDD formate
    *  @param [in]		yearFractions		An array of year fraction yearFractions
    *  @param [in]		dayCount	        The daycount fraction used to generate the yearFraction
    *  @param [in]		curveCollection		Curve set ID
    *  @param [in]		curveIndex			Index of the curve set. Default to OIS
    *  @return			A array of discount factors
    */
    DoubleVector getCurveDiscountFactorsForwardStartingFromYearFractions( const DateVector& fromDates,
                                                                          const DoubleVector& yearFractions,
                                                                          const LAString& dayCount,
                                                                          const LAString& curveCollection,
                                                                          const LAString& curveIndex )
    {
        MLIB_REQUIRE( fromDates.size() > 0, "Discount Factor 'fromDates' cannot be empty" )
        MLIB_REQUIRE( fromDates.size() == yearFractions.size(), "Inconsistent Discount Factor Parmaeters - The number of discount factor 'fromDates' must match the number of 'yearFractions'" )

        LAString curIndex( curveIndex );
        LAString interp;
        LAString bdAdj( "NO_CHANGE" );
        LAString cal( "" );
        populateDiscountFactorConventions( curveCollection, curIndex, interp, bdAdj, cal );
		
		// Convert Year Fractions to ACT/365 ones
        DoubleVector yearFractionsAct365 = yearFractions;
        LADate curveAsOfDate = etrading::getCurveAsOfDate( curveCollection );
        convertToYearFractionsACT365( curveAsOfDate, yearFractionsAct365, dayCount );

        // We get discount factors from the new curve results object, if this is not possible we get them from the legacy object pool object
        DoubleVector discountFactors;
        
        const bool isEnabledCurveResults = etrading::isEnabledCurveResults();
        const bool doesExistDiscountFactors = etrading::doesExistCurveResultsDiscountFactors( curveCollection.getCString(), curveIndex.getCString() );

        if ( isEnabledCurveResults && doesExistDiscountFactors )
        {
            // New: Get Discount Factors from the Thread-Safe Curve Results Object
            discountFactors = getDiscountFactorsUsingYearFractionsFromCurveResultsObject( curveCollection.getCString(), curveIndex.getCString(), yearFractionsAct365 );
        }
        else
        {
            // LEGACY: Get Discount Factors from the Object Pool
            discountFactors = etrading::LACurveForwardRateHelpers::getMultiDF( fromDates, yearFractionsAct365, etrading::getDataInstance(), curveCollection, getDiscountFactorDayCount(), bdAdj, cal, interp, isBasisFlagForDiscountFactor(), curIndex );
        }

        MLIB_REQUIRE( discountFactors.size() > 0, "Discount Factor Results are Empty" )
        return discountFactors;
    }


	/* @brief			Get discount factors from asOfDate and yearFractions
    *  @param [in]		yearFractions	An array of yearFractions by year count
    *  @param [in]		dayCount		Day count
    *  @param [in]		curveCollection	Curve set ID
    *  @param [in]		curveIndex		Index of the curve set. Default to OIS
    *  @return			An array of discount factor
    */
	DoubleVector getCurveDiscountFactorsFromYearFractions( const DoubleVector& yearFractions,
                                                           const LAString& dayCount,
                                                           const LAString& curveCollection,
                                                           const LAString& curveIndex )
    {
        MLIB_REQUIRE( yearFractions.size() > 0, "Discount Factor 'yearFractions' cannot be empty" )

        LAString curIndex( curveIndex );
        LAString interp;
        LAString bdAdj( "NO_CHANGE" );
        LAString cal( "" );
        populateDiscountFactorConventions( curveCollection, curIndex, interp, bdAdj, cal );

        // Convert Year Fractions to ACT/365 ones
        DoubleVector yearFractionsAct365 = yearFractions;
        LADate curveAsOfDate = etrading::getCurveAsOfDate( curveCollection );
        convertToYearFractionsACT365( curveAsOfDate, yearFractionsAct365, dayCount );

        // We get discount factors from the new curve results object, if this is not possible we get them from the legacy object pool object
        DoubleVector discountFactors;
        
        const bool isEnabledCurveResults = etrading::isEnabledCurveResults();
        const bool doesExistDiscountFactors = etrading::doesExistCurveResultsDiscountFactors( curveCollection.getCString(), curveIndex.getCString() );

        if ( isEnabledCurveResults && doesExistDiscountFactors )
        {
            // New: Get Discount Factors from the Thread-Safe Curve Results Object
            discountFactors = getDiscountFactorsUsingYearFractionsFromCurveResultsObject( curveCollection.getCString(), curveIndex.getCString(), yearFractionsAct365 );
        }
        else
        {
            // LEGACY: Get Discount Factors from the Object Pool
            discountFactors = etrading::LACurveForwardRateHelpers::getMultiDF( yearFractionsAct365, etrading::getDataInstance(), curveCollection, getDiscountFactorDayCount(), interp, isBasisFlagForDiscountFactor(), curIndex );
        }

        MLIB_REQUIRE( discountFactors.size() > 0, "Discount Factor Results are Empty" )
        return discountFactors;
    }

	 /* @brief			populate and setting default values
    *  @param [in]		curveCollection	curve collection
    *  @param [inout]	curveIndex		curve index
    *  @param [in]		fwdInter		User specified fwdInter flag
    *  @param [inout]	interpolation	interpolation
    *  @param [inout]	businessDayAdj	business day adjustment
    *  @param [inout]	dayCount		day count
    */
    void populateZeroRateConventions( const LAString& curveCollection,
                                      LAString& curveIndex,
									  const etrading::BooleanEnum& fwdInter,
                                      LAString& interpolation,
                                      LAString& dayCount,
                                      LAString& frequency,
                                      bool& isFwdInter,
                                      LAString& businessDayAdj )
    {
        //curveIndex = etrading::getDefaultValueForEmptyString( curveIndex, "STD" );
        //Throw exception if the curve has not been built.
        LAString staticDataTable = etrading::getCurveStaticDataTableName( curveCollection, curveIndex );
        interpolation = etrading::getCurveInterpolation( curveCollection, staticDataTable );

		// If user has not specified the fwdInter flag, then use the default one
		isFwdInter = getfwdInterInfo( curveCollection, staticDataTable, fwdInter ).isFwdInter;

        etrading::validateFrequency( isFwdInter, frequency );

        businessDayAdj = etrading::getDefaultValueForEmptyString( businessDayAdj, "NO_CHANGE" );
        dayCount = etrading::getDefaultValueForEmptyString( dayCount, "ACT/365" );
    }


	/* @brief			Zero rate over the yearFractions from the curve's asof date
    *  @param [in]		yearFractions	A  list of given year fractions
    *  @param [in]		curveCollection	Curve collection id
    *  @param [in]		curveIndex		Curve index name. Default to STD
    *  @param [in]		frequency		Frequency. Default to SIMPLE for STD curve, ANNUAL for other curves
    *  @param [in]		dayCount		Day count convention. Default to ACT/365
    *  @return			Zero rate 
    */
    DoubleVector getCurveZeroRatesFromYearFractions( const DoubleVector& yearFractions,
                                                     const LAString& curveCollection,
                                                     const LAString& curveIndex,
                                                     const LAString& frequency,
                                                     const LAString& dayCount,
			                                         const BooleanEnum& fwdInter)
    {

        // Validate parameters
        if( yearFractions.size() == 0 )
        {
            throw LACoreInvalidData( "a size of vector is zero.", __FILE__, __LINE__ );
        }

        LAString curIndex( curveIndex );
        LAString interp;
        LAString dayC( dayCount );
        LAString freq( frequency );
        bool isFwdInter;
		LAString businessDayAdj;
        populateZeroRateConventions( curveCollection, curIndex, fwdInter, interp, dayC, freq, isFwdInter, businessDayAdj );

        DoubleArray arr = etrading::LACurveForwardRateHelpers::getMultiZeroRate( yearFractions, etrading::getDataInstance(), curveCollection,
                          freq, dayC, interp, curIndex, isFwdInter );

        return arr;
    }

	 /* @brief			Get the curve forward rate given curveAsOfDate and a set of toDates
    *  @param [in]		toDates			A vector of to dates
    *  @param [in]		curveCollection	ID of the yield curve
    *  @param [in]		curveIndex		Index of the curve set. Default to STD
    *  @param [in]		fwdInter		User input fwdInter flag
    *  @return			The forward rates based on fromDates and toDates
    */
    DoubleVector getCurveForwardRatesFromAsOfDate(     const DateVector& toDates,
                                                       const LAString& curveCollection,
                                                       const LAString& curveIndex,
													   const BooleanEnum& fwdInter)
    {
		const LADate curveAsOfDate = getCurveAsOfDate( curveCollection );

		DateVector fromDates(toDates.size());

		for(size_t i = 0; i < toDates.size(); ++i)
		{
			fromDates[i] = curveAsOfDate;
		}

		return getCurveForwardRatesFromForwardDates(fromDates, toDates, curveCollection, curveIndex, fwdInter);
	}


	/* @brief			Get the short rate(s) at fromDate(s), we use overnight forward rate as an approximation
    *  @param [in]		fromDates	    A vector of from dates
    *  @param [in]		curveCollection	ID of the yield curve
    *  @param [in]		curveIndex		Index of the curve set 
    *  @param [in]		fwdInter		User input fwdInter flag
    *  @return			The short rates based on fromDates
    */
	DoubleVector getCurveShortRates(const DateVector& fromDates, const std::string& curveCollection, const std::string& curveIndex, const BooleanEnum& fwdInter) 
	{
		// Model short rate at r(0), since r(t) = f(t, t), we use overnight forward rate as an approximation
	
		DateVector toDates(fromDates.size());

		for(size_t i = 0; i < fromDates.size(); ++i)
		{
			LADate nextDate = fromDates[i];
			nextDate.addDays(1);

			toDates[i] = nextDate;
		}

		// r(t)
		const DoubleVector shortRateAtFromDates = getCurveForwardRatesFromForwardDates( fromDates, toDates, curveCollection.c_str(), curveIndex.c_str(), fwdInter);

		return shortRateAtFromDates;
	}

	/* @brief			Retrieve the hybrid interpolation join date from a swap curve
	*  @param [in]		curveCollection	ID of the yield curve
	*  @param [in]		curveIndex		Index of the curve set
	*  @return			The join date used in hybrid interpolation of a swap curve
	*/
	LADate getHybridInterpolationJoinDate( const LAString& curveCollection, const LAString& curveIndex )
	{
		return getLinearSplineJoinDate( curveCollection, curveIndex );
	}

	/* @brief			Retrieve the hybrid interpolation join date as a double from a swap curve
	*  IMPORTANT NOTE:	Join Dates are Relative to the Curve's Swap Calibration Spot Dates NOT the curve asOfDate
	*  @param [in]		curveCollection	ID of the yield curve
	*  @param [in]		curveIndex		Index of the curve set
	*  @return			The join date used in hybrid interpolation of a swap curve
	*/
	double getHybridInterpolationJoinDateAsDouble( const LAString& curveCollection, const LAString& curveIndex )
	{
		return getLinearSplineJoinDateAsDouble( curveCollection, curveIndex );
	}

	/* @brief			Retrieve the linear spline join date from a swap curve
    *  @param [in]		curveCollection	ID of the yield curve
    *  @param [in]		curveIndex		Index of the curve set 
	*  @return			The join date used in linear spline interpolation of a swap curve
    */
	LADate getLinearSplineJoinDate(const LAString& curveCollection, const LAString& curveIndex)
	{
		LAObjectPool& objPool	= getDataInstance()->getObjectPool();
	
		LAString suffix = curveIndex;
		suffix.toUpper();
		if (suffix == STD)
		{
			suffix = "";
		}
		else
		{
			suffix = "_" + suffix;
		}

		const LAObject& ycp	= objPool.getObject(curveCollection, ENCHKTYPE_ISDEFINED).get();
		LADate joinDate;
		const LADataHolder* handle = &( ycp.getData(CALIBRATION_DATA_INTERPOLATION_JOINDATE + suffix, NOCHECK) );
		if ( handle->isDefined() && !handle->isNull() )
		{
			joinDate = dynamic_cast< const LADataDate& >( handle->get() ).get();
		}

		return joinDate;
	}


	
	/* @brief			Retrieve the linear spline join date as a double from a swap curve
	*  IMPORTANT NOTE:	Join Dates are Relative to the Curve's Swap Calibration Spot Dates NOT the curve asOfDate
	*  @param [in]		curveCollection	ID of the yield curve
	*  @param [in]		curveIndex		Index of the curve set
	*  @return			The join date used in linear spline interpolation of a swap curve
	*/
	double getLinearSplineJoinDateAsDouble( const LAString& curveCollection, const LAString& curveIndex )
	{
		LAObjectPool& objPool = getDataInstance()->getObjectPool();

		LAString suffix = curveIndex;
		suffix.toUpper();
		if ( suffix == STD )
		{
			suffix = "";
		}
		else
		{
			suffix = "_" + suffix;
		}

		const LAObject& ycp = objPool.getObject( curveCollection, ENCHKTYPE_ISDEFINED ).get();
		double joinDateAsDouble = 0.0;
		const LADataHolder* handle = &( ycp.getData( CALIBRATION_DATA_INTERPOLATION_JOINDATE_ASDOUBLE + suffix, NOCHECK ) );
		if ( handle->isDefined() && !handle->isNull() )
		{
			joinDateAsDouble = dynamic_cast< const LADataDouble& >( handle->get() ).get();
		}

		return joinDateAsDouble;
	}


	/* @brief			Calculate the EuroDollar Futures Convexity Adjustment
	* @param [in]		curveAsOfDate       The yield curve as of or valuation date
	* @param [in]		futuresStartDate    The futures start date
	* @param [in]		futuresEndDate      The futures end date
	* @param [in]		meanReversion       The Hull-White 1F Mean Reversion Parameter
	* @param [in]		volatility          The Hull-White 1F Volatility Parameter
	* @param [out]		Returns the EuroDollar Futures Convexity Adjustment
	*/
	double getCurveEuroDollarConvexityAdjustment(const LADate& curveAsOfDate, const LADate& futuresStartDate, const LADate& futuresEndDate, const double& meanReversion, const double& volatility)
	{

		if (meanReversion < 0)
		{
			throw LACoreInvalidData("#Error: Hull-White 1F Mean-Reversion parameter cannot be negative.", __FILE__, __LINE__);
		}

		if (volatility < 0)
		{
			throw LACoreInvalidData("#Error: Hull-White 1F Volatility parameter cannot be negative.", __FILE__, __LINE__);
		}

		if (futuresEndDate <  futuresStartDate)
		{
			throw LACoreInvalidData("#Error: The futures end date cannot be before it's start date.", __FILE__, __LINE__);
		}

		if (futuresEndDate <  curveAsOfDate)
		{
			throw LACoreInvalidData("#Error: The future has expired. The futures end date cannot be before the curveAsOfDate or valuation date.", __FILE__, __LINE__);
		}

		LAString dayCount( "ACT/365" );
		double T1 = LADateScheduleHelpers::getTerm(curveAsOfDate, futuresStartDate, dayCount, true); // includeLast = true
		double T2 = LADateScheduleHelpers::getTerm(curveAsOfDate, futuresEndDate, dayCount, true); // includeLast = true

		if (T1 > T2)
		{
			throw LACoreInvalidData("#Error: The future's start date cannot be greater than it's end date.", __FILE__, __LINE__);
		}

		double convexityAdjustment = 0.0;

		if (meanReversion > 0)
		{

		    // The Hull-White mean reversion parameter must be a value between 0 and 1. Throw an error if this is not the case.
			if ( meanReversion > 1.0 )
			{
	            throw LACoreInvalidData("#Error: CurveCalibration::calcDiscountFactor failed. Invalid futures market data, the convexity mean reversion parameter must be a value between 0 and 1", __FILE__, __LINE__);
		 	}

			// Hull-White 1 Factor Convexity Adjustment
			double B_T1_T2 = (1.0 - LAMath::exp(-meanReversion * (T2 - T1))) / meanReversion;
			double B_0_T1 = (1.0 - LAMath::exp(-meanReversion * T1)) / meanReversion;

			// This is the Hull-White 1 Factor Futures Convexity Adjustment
			// See Options, Futures and Other Derivatives 9ed by John Hull, in particular Technical Note 1 - Convexity Adjustments to Eurodollar Futures
			convexityAdjustment = (B_T1_T2 / (T2 - T1))
				* (B_T1_T2 * (1 - LAMath::exp(-2 * meanReversion * T1)) + (2 * meanReversion * B_0_T1 * B_0_T1))
				* (volatility * volatility / (4 * meanReversion));
		}
		//When meanReversion is zero
		else
		{
			// Use Ho-Lee Model Adjustment, note that the HW 1 Factor model reduces to the Ho-Lee model when mean reversion is zero
			convexityAdjustment = 0.5 * T1 * T2 * volatility * volatility;
		}

		return convexityAdjustment;
	}

	/* @brief			Future Price from FRA rate based on the given convexity adjustment
	* @param [in]		fraRate				The given fra rate
	* @param [in]		convexityAdjustment The given convexity adjustment between Future rate and Fra rate
	* @param [out]		Returns Future Price
	*/
	double fromFraRateToFuturePrice(const double& fraRate, const double& convexityAdjustment)
	{

		double futureRate = fraRate + convexityAdjustment;

		double futurePrice = (1.0 - futureRate) * 100.0;

		return futurePrice;

	}

	/* @brief			FRA rate from Future Price based on the given convexity adjustment
	* @param [in]		futurePrice			The given future price
	* @param [in]		convexityAdjustment The given convexity adjustment between Future rate and Fra rate
	* @param [out]		Returns Fra Rate
	*/
	double fromFuturePriceToFraRate(const double& futurePrice, const double& convexityAdjustment)
	{

		double futureRate = (100.0 - futurePrice) * 0.01;

		double fraRate = futureRate - convexityAdjustment;

		return fraRate;

	}

	/* @brief			Check if a curve was calibrated using global curve engine
	* @param [in]		curveCollection			Name of the curve collection
	* @param [in]		curveName				Curve name
	* @return			Boolean - was curve built from curve engine
	*/
	bool isCurveBuiltFromCurveEngine(const LAString& curveCollection, const LAString& curveName)
	{
		bool isFromCurveEngine = false;
		CurveCalibrationData* curveCalibrationData = etrading::InitializeMLibETrading::instance().ycStaticDataObject(curveCollection);
		LAString suffix = curveName == "STD" ? "" : "_" + LAString(curveName).toUpper();
		LADataHolder* dh = &(curveCalibrationData->getYieldData().get().getData(IR_CALIBRATION_DATA_BUILTBYCURVEENGINE + suffix, NOCHECK) );
		if (dh->isDefined() && !dh->isNull())
		{
			isFromCurveEngine = dynamic_cast<LADataBool&> (dh->get());
		}

		return isFromCurveEngine;
	}

	/* @brief Display the yield curve engine jacobian matrix
	* @param [out] matrix							Matrix being returned and displayed
	* @param [out] labelMatrix						Matrix that has label for each jacobian matrix element
	* @param [in] curveEngineObject					The name of the dual-bootstrapped object
	* @param [in] curveCollection					Curve collection name
	* @param [in] displayLabels						Display labels of the matrix
	* @param [in] displayInverseMatrix				Display inverse Jacobian matrix or not
	*/
	void displayCurveEngineJacobian(DoubleMatrix& matrix,
									LAStringMatrix& labelMatrix,
									const LAString& curveEngineObject,
									const LAString& curveCollection,
									bool displayLabels,
									bool displayInverseMatrix)
	{
		CurveCalibrationData* curveCalibrationData = etrading::InitializeMLibETrading::instance().ycStaticDataObject(curveCollection);
		LAString suffix = curveEngineObject == "STD" ? "" : "_" + LAString(curveEngineObject).toUpper();

		if (displayInverseMatrix)
		{
			LADataHolder* dh = &(curveCalibrationData->getYieldData().get().getData(IR_CALIBRATION_DATA_INVERSE_ENGINE_JACOBIAN + suffix, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				matrix = dynamic_cast<LADataDoubleMatrix&> (dh->get()).get();
			}
		}
		else
		{
			LADataHolder* dh = &(curveCalibrationData->getYieldData().get().getData(IR_CALIBRATION_DATA_ENGINE_JACOBIAN + suffix, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				matrix = dynamic_cast<LADataDoubleMatrix&> (dh->get()).get();
			}
		}

		if (displayLabels)
		{
			LADataHolder* dh = &(curveCalibrationData->getYieldData().get().getData(IR_CALIBRATION_DATA_ENGINE_JACOBIAN_LABEL + suffix, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				labelMatrix = dynamic_cast<LADataStringMatrix&> (dh->get()).get();
			}
		}
	}

	/* @brief Display the jacobian matrix of one yield curve
	* @param [out] matrix							Matrix being returned and displayed
	* @param [in] curveCollection					Curve collection name
	* @param [in] curveName							Name of yield curve
	* @param [in] displayInverseMatrix				Display inverse Jacobian matrix or not
	*/
	void displayCurveJacobian(DoubleMatrix& matrix,
								const LAString& curveCollection,
								const LAString& curveName,
								bool displayInverseMatrix)
	{
		bool curveFromEngine = etrading::isCurveBuiltFromCurveEngine(curveCollection, curveName);
		if (curveFromEngine)
		{
			LAString error = curveName + " was created in a yield curve engine. Please use the engine Jacobian display method.";
			throw LACoreInvalidData(error.getCString(), __FILE__, __LINE__);
		}

		CurveCalibrationData* curveCalibrationData = etrading::InitializeMLibETrading::instance().ycStaticDataObject(curveCollection);
		LAString suffix = curveName == "STD" ? "" : "_" + LAString(curveName).toUpper();

		if (displayInverseMatrix)
		{
			LADataHolder* dh = &(curveCalibrationData->getYieldData().get().getData(IR_CALIBRATION_DATA_INVERSE_CURVE_JACOBIAN + suffix, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				matrix = dynamic_cast<LADataDoubleMatrix&> (dh->get()).get();
			}
		}
		else
		{
			LADataHolder* dh = &(curveCalibrationData->getYieldData().get().getData(IR_CALIBRATION_DATA_CURVE_JACOBIAN + suffix, NOCHECK));
			if (dh->isDefined() && !dh->isNull())
			{
				matrix = dynamic_cast<LADataDoubleMatrix&> (dh->get()).get();
			}
		}
	}

	/* @brief			Get ValuationDate from the valuationSettingsLVB map, default by using curve's asOfDate
	* @param [in]		valuationSettingsLVB	ValuationSettings map
	* @param [in]		modelName			The model or curve collection name
	* @output			The matched valuationDate
	*/
	LADate getValuationDate(const LabelValueBlock& valuationSettingsLVB, const LAString& modelName )
	{
		// 1. Check to see if a valuationDate has been explicitly provided in the valuationSettingsLVB
		LADate valuationDate = valuationSettingsLVB.getOptionalValueAsDate(VALUATION_SETTING_KEYS::VALUATION_DATE, LADate());

		if (valuationDate == LADate())
		{
			// 2. Check to see if we have been given a valid credit model
			auto creditModelPtr  = getCreditModel( modelName.getCString(), false /* do not throw if credit model missing */ );
			if ( creditModelPtr != nullptr )
			{
				// Set the valuation date from the credit model asOf date
				valuationDate = creditModelPtr->getAsOfDate();
			}
			else
			{
				// 3. Set the valuation date from the curveAsOfDate.
				valuationDate = getCurveAsOfDate( modelName );
			}
		}

		return valuationDate;
	}


	/* @brief			Calculate the convexity due to libor fixed in arrears
	* @param [in]		forwardRate					Forward rate fixed in advance
	* @param [in]		theta						Input to the formula: When using forwardRate formula, theta = f*tao / (1 + f*tao) 2) When using discountFactor formula, theta= 1-P(0,T)/P(0,T')
	* @param [in]		fixingDate					Fixing date for forward rate fixed in advance
	* @param [in]		fixingEndDate  				Fixing end date, i.e. fixing date for forward rate fixed in arrears
	* @param [in]		valuationDate  				Valuation date
	* @param [in]		dayCount  					Fixing daycount
	* @param [in]		volatility  				Volatility input to the convexity formula
	* @param [in]		volatilityType  			Volatility type
	* @output			Convexity for libor in arrears
	*/
	double liborInArrearConvexityFormula(const double& forwardRate, const double& theta, const LADate& fixingDate, const LADate& valuationDate, const DayCountEnum& dayCount, const double& volatility, const VolatilityTypeEnum& volatilityType, const bool& approximation)
	{
		// Convexity:
		// 1) General formula: convexity = theta * F^2 * (exp(volatility^2 * Ts) - 1.0)
		// 2) John Hull approximation formula: convexity = theta * F^2 * (volatility^2 * Ts)

		// Ts is the year fraction from valuationDate to fixingDate
		const double ts = getYearFraction(valuationDate, fixingDate, dayCount);

		double convexity = 0.0;
		double lognormalVol = 0.0;

		switch (volatilityType)
		{
		case LOGNORMAL_VOLATILITY:

			lognormalVol = volatility;
			break;
		case NORMAL_VOLATILITY:

			//formula: vol_normal = forwardRate * vo_lognormal
			lognormalVol = volatility / forwardRate;
			break;
		default:
			// Should never reach here
			MLIB_THROW("Invalid volatility type. Must be 'LOGNORMAL' or 'NORMAL'");
			break;
		}

		if (approximation)
		{
			// covAdj Hull Lognormal(f* theta * vol ^ 2 * t), which is the first order taylor formula in HULL book , same as BB's lognormal & Hull
			convexity = forwardRate * theta * lognormalVol * lognormalVol * ts;
		}
		else
		{
			// covAdj Lognormal(f*theta*(exp(vol^2*T)-1)
			convexity = forwardRate * theta * (exp(lognormalVol * lognormalVol * ts) - 1.0);
		}

		return convexity;

	}

	/* @brief			Calculate the convexity due to libor fixed in arrears
	* @param [in]		forwardRate					Forward rate fixed in advance
	* @param [in]		fixingDate					Fixing date for forward rate fixed in advance
	* @param [in]		fixingEndDate  				Fixing date for forward rate fixed in arrears
	* @param [in]		valuationDate  				Valuation date
	* @param [in]		dayCount  					Fixing daycount
	* @param [in]		volatility  				Volatility input to the convexity formula
	* @param [in]		volatilityType  			Volatility type
	* @output			Convexity for libor in arrears
	*/
	double liborInArrearConvexity(const double& forwardRate, const LADate& fixingDate, const LADate& fixingEndDate, const LADate& valuationDate, const DayCountEnum& dayCount, const double& volatility, const VolatilityTypeEnum& volatilityType)
	{
		//When using in arrear formula, theta: f*tao / (1 + f*tao) 

		const double tao = getYearFraction(fixingDate, fixingEndDate, dayCount);

		const double theta = forwardRate * tao / (1.0 + forwardRate * tao);

		double convexity = liborInArrearConvexityFormula(forwardRate, theta, fixingDate, valuationDate, dayCount, volatility, volatilityType, true /** Hull book's approximation */ );

		return convexity;
	}
	
	/* @brief			Calculate the convexity due to libor fixed in arrears
	* @param [in]		forwardRate					Forward rate fixed in advance
	* @param [in]		fixingDate					Fixing date for forward rate fixed in advance
	* @param [in]		fixingEndDate  				Fixing end date, i.e. fixing date for forward rate fixed in arrears
	* @param [in]		valuationDate  				Valuation date
	* @param [in]		dayCount  					Fixing daycount
	* @param [in]		volatility  				Volatility input to the convexity formula
	* @param [in]		volatilityType  			Volatility type
	* @output			Convexity for libor in arrears
	*/
	double liborInArrearConvexityUsingDF(const double& forwardRate, const double& discountFactorAtFixingDate, const double& discountFactorAtFixingEndDate, const LADate& fixingDate, const LADate& valuationDate, const DayCountEnum& dayCount, const double& volatility, const VolatilityTypeEnum& volatilityType)
	{
		// When using discount factor formula, theta: 1-P(0,T)/P(0,T')

		//theta: 1-P(0,T)/P(0,T'), where T is the paymentDate of the inAdvanceFixingDate (i.e. fixingEndDate), T' is the paymentDate of the inArrearFixingDate (i.e. fixingDate)
		const double theta = 1.0 - discountFactorAtFixingEndDate / discountFactorAtFixingDate;

		double convexity = liborInArrearConvexityFormula(forwardRate, theta, fixingDate, valuationDate, dayCount, volatility, volatilityType, true /** Hull book's approximation */);

		return convexity;
	}

	/* @brief			Helper function to populate fixing start and end dates from fixingDates
	* @param [out]		fixingStartDates		Fixing start date
	* @param [out]		fixingEndDates			Fixing end date based on the curve tenor; when if there is front/end stub, it will be overrided by end date
	* @param [in]		allFixingDates  		All fixing dates calculated from accrued date and fixLag
	* @param [in]		curveFrequency  		Curve Frequency, e.g. Daily, Quartely, Semi-Annual, Annual
	* @param [in]		fixingBusinessDayAdj  	Fixing business day adjustment
	* @param [in]		fixingCalendar  		Fixing calendar
	* @param [in]		rollConvention  		rollConvention
	* @param [in]		isRegularSwap  		    True for swap with no stub, false for swap with front/end stub
	* @param [in]		isFrontStubType  		True for front stub, False for end stub
	*/
	void populateFixingStartEndDates(DateVector& fixingStartDates,
									 DateVector& fixingEndDates,
									 const DateVector& allFixingDates,
									 const FrequencyEnum& curveFrequency,
									 const LAString& fixingBusinessDayAdj,
									 const LAString& fixingCalendar,
									 const LAString& rollConvention,
									 const bool isRegularSwap,
									 const bool isFrontStubType)
	{
		// remove the last fixing so that the list only contains fixing start dates
		fixingStartDates = { allFixingDates.begin(), allFixingDates.end() - 1 };

		fixingEndDates = LADateScheduleHelpers::getMultiDate(fixingStartDates, getFrequencyTenor(curveFrequency), fixingBusinessDayAdj, fixingCalendar, rollConvention.size() == 0 ? nullptr : &rollConvention);

		if (!isRegularSwap)
		{
			const LADate overridedFixingEndDateForStub = isFrontStubType ? allFixingDates[1] : allFixingDates[allFixingDates.size() - 1];

			if (isFrontStubType)
			{
				// For front stub, use the calclulated front stub date
				fixingEndDates[0] = overridedFixingEndDateForStub;
			}
			else
			{
				// For end stub, use the calclulated end stub date
				auto expectedSize = fixingStartDates.size();
				fixingEndDates[expectedSize - 1] = overridedFixingEndDateForStub;
			}
		}

	}

	/* Function to calculate the accrual period or term given the forward rate start- and end-dates in year fraction terms
	* @param [in]		fixingStartTerm				fixing start date as a curve term (i.e. using hard-coded ACT/365)
	* @param [in]		fixingEndTerm  				fixing end date as a curve term (i.e. using hard-coded ACT/365)
	* @param [in]		asOfDate  					curve as of date
	* @param [in]		accrualDaycountEnum			instrument accrual daycount basis
	* @param [in]		compoundFrequencyEnum		compounding frequency enum - defaults to SIMPLE COMPOUNDING
	* @output			The forward rate accrual period
	*/
	double accrualPeriod( const double & fixingStartTerm,
						  const double & fixingEndTerm,
						  const LADate & asOfDate,
						  const DayCountEnum & accrualDaycount,
						  const CompoundingFrequencyEnum & compoundFrequency )
	{
		// We can take a short-cut and subtract year fractions only when the instrument daycount matches ...
		// ... the internal curve daycount of ACT/365, otherwise we have to convert the year fractions as below.
		if( accrualDaycount == DayCountEnum::ACT_365_DAYCOUNT )
		{
			const double accrualPeriod = fixingEndTerm - fixingStartTerm;
			return accrualPeriod;
		}

		// Important!!
		// The term here must be the converted from the internal ACT/365 daycount used for date transformation to a term in the curve daycount measure
		// --------------------------------------------------------------------------------------------------------------------------
		const LADate fixingStartDate	= convertCurveTermToDate( asOfDate, fixingStartTerm );
		const LADate fixingEndDate		= convertCurveTermToDate( asOfDate, fixingEndTerm );
		
		const double result				= accrualPeriod( fixingStartDate, fixingEndDate, asOfDate, accrualDaycount, compoundFrequency );
		return result;
	}

	/* Function to calculate the accrual period or term given the forward rate start- and end-dates in year fraction terms
	* @param [in]		fixingStartDate				fixing start date
	* @param [in]		fixingEndDate  				fixing end date
	* @param [in]		asOfDate  					curve as of date
	* @param [in]		accrualDaycount				instrument accrual daycount basis
	* @param [in]		compoundFrequency			compounding frequency enum - defaults to SIMPLE COMPOUNDING
	* @output			The forward rate accrual period
	*/
	double accrualPeriod( const LADate & fixingStartDate,
						  const LADate & fixingEndDate,
						  const LADate & asOfDate,
						  const DayCountEnum & accrualDaycount,
						  const CompoundingFrequencyEnum & compoundFrequency )
	{
		// Important!!
		// The term here must be the converted from the internal ACT/365 daycount used for date transformation to a term in the curve daycount measure
		// --------------------------------------------------------------------------------------------------------------------------

		// LA Rate Convention Method for Year Fraction Calculation
		const LAPriceDataConvention legacyConventionObject( toLADaycount(accrualDaycount), toLARateConvention(compoundFrequency) );
		
		const double curveInstrumentFromDateInTermFormat	= legacyConventionObject.getTerm( asOfDate, fixingStartDate );
		const double curveInstrumentToDateInTermFormat		= legacyConventionObject.getTerm( asOfDate, fixingEndDate );
		
	    const double result = curveInstrumentToDateInTermFormat - curveInstrumentFromDateInTermFormat;
        MLIB_REQUIRE( MLIB_IS_GREATER_THAN_ZERO( result ), "Invalid Accrual Period - The fixing start date must be before the fixing end date" )
		return result;
	}

	/* Function to calculate the accrual daycount adjustment from a curve daycount of ACT/365 to the daycount basis specified
	* @param [in]		fixingStartTerm				fixing start date as a curve term (i.e. using hard-coded ACT/365)
	* @param [in]		fixingEndTerm  				fixing end date as a curve term (i.e. using hard-coded ACT/365)
	* @param [in]		asOfDate  					curve as of date
	* @param [in]		accrualDaycount				instrument accrual daycount basis
	* @param [in]		compoundFrequency			compounding frequency enum - defaults to SIMPLE COMPOUNDING
	* @output			The accrual daycount adjustment to convert a year fraction to the correct daycount basis
	*/
	double daycountConversionFactor( const double & fixingStartTerm,
									 const double & fixingEndTerm,
									 const LADate & asOfDate,
									 const DayCountEnum & accrualDaycount,
									 const CompoundingFrequencyEnum & compoundFrequency )
	{
		MLIB_REQUIRE( MLIB_IS_LESS_THAN_OR_EQUAL( fixingStartTerm, fixingEndTerm ), "Invalid Daycount Conversion: Start date cannot be after the end date" )
		if ( MLIB_IS_EQUAL( fixingStartTerm, fixingEndTerm ) ) return 1.0; // No conversion to make, return 1.0

		// Short-cuts : known daycount adjustment factors
		switch( accrualDaycount )
		{
			case DayCountEnum::NONE_DAYCOUNT:
			{
				MLIB_THROW( "Invalid Daycount: Unable to calculate the accrual daycount adjustment" )
			}
			case DayCountEnum::ACT_365_DAYCOUNT:
			{
				return 1.0;
			}
			case DayCountEnum::ACT_360_DAYCOUNT:
			{
				return 360.0 / 365.0;
			}
		}
		
		const LADate fixingStartDate		= convertCurveTermToDate( asOfDate, fixingStartTerm );
		const LADate fixingEndDate			= convertCurveTermToDate( asOfDate, fixingEndTerm );
		const double accrualPeriodAct365	= fixingEndTerm - fixingStartTerm;
		
		const double result					= daycountConversionFactor( fixingStartDate, fixingEndDate, accrualPeriodAct365, asOfDate, accrualDaycount, compoundFrequency );
		return result;
	}

	/* Function to calculate the accrual daycount adjustment from a curve daycount of ACT/365 to the daycount basis specified given the forward rate start- and end-dates in year fraction terms
	* @param [in]		fixingStartDate				fixing start date
	* @param [in]		fixingEndDate  				fixing end date
	* @param [in]		accrualPeriodAct365			The date year fraction between the fixing Start- and End Dates in the curve year faction of ACT/365
	* @param [in]		asOfDate  					curve as of date
	* @param [in]		accrualDaycount				instrument accrual daycount basis
	* @param [in]		compoundFrequency			compounding frequency enum - defaults to SIMPLE COMPOUNDING
	* @output			The accrual daycount adjustment to convert a year fraction to the correct daycount basis
	*/
	double daycountConversionFactor( const LADate & fixingStartDate,
									 const LADate & fixingEndDate,
									 const double & accrualPeriodAct365,
									 const LADate & asOfDate,
									 const DayCountEnum & accrualDaycount,
									 const CompoundingFrequencyEnum & compoundFrequency )
	{
		// Important!!
		// The term here must be the converted from the internal ACT/365 daycount used for date transformation to a term in the curve daycount measure
		// --------------------------------------------------------------------------------------------------------------------------

		MLIB_REQUIRE( fixingStartDate <= fixingEndDate, "Invalid Daycount Conversion: Start date cannot be after the end date" )
		if ( fixingStartDate == fixingEndDate ) return 1.0; // No conversion to make, return 1.0

		// LA Rate Convention Method for Year Fraction Calculation
		const LAPriceDataConvention legacyConventionObject( toLADaycount(accrualDaycount), toLARateConvention(compoundFrequency) );
		
		const double curveInstrumentFromDateInTermFormat	= legacyConventionObject.getTerm( asOfDate, fixingStartDate );
		const double curveInstrumentToDateInTermFormat		= legacyConventionObject.getTerm( asOfDate, fixingEndDate );
		
		const double accrualPeriodForDaycount = curveInstrumentToDateInTermFormat - curveInstrumentFromDateInTermFormat;
		const double daycountConversionFactor =  accrualPeriodAct365 / accrualPeriodForDaycount;

		return daycountConversionFactor;
	}

	// oisHistoricalRates can be either: 1) fixingTable object handle name, or 2) a column of fixingDates and a column of fixintRates
	LAStringMatrix retrieveFixingTableMatrix(const LAStringMatrix& oisHistoricalRates)
	{
		// fixingTable object handle name
		if (oisHistoricalRates.size() == 1)
		{
			auto fixintTableName = etrading::trimObjectCounter(oisHistoricalRates[0][0].getCString());
			auto fixingTable = etrading::getFixingTable(fixintTableName);

			return fixingTable->getFixingTableDateValueMatrix();
		}

		//a column of fixingDates and a column of fixintRates
		return oisHistoricalRates;
	}

}



