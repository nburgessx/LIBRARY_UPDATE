/*
 * @brief			Class which defines the Inflation Curve class.
 * @Created:		26 May 2020
 * @Author:			Ian Castleton
 * @Department:		Quant Research & Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */


#include "InflationCurve.h"
#include "ZeroCouponInflationSwap.h"
#include "CoreEnumerations.h"
#include "ObjectUtilities.h"
#include "DataUtilities.h"
#include "LADateScheduleHelpers.h"

namespace etrading
{

	InflationCurve::InflationCurve( const std::string& objectName,
									const std::vector<std::string>& propertyKeys,
									const std::vector<TableInfo>& infoBlocks )
								: IsLWOObject( objectName, INFLATION_CURVE ),
								  freeObject_( objectName )
	{
		// Create a FreeObject from each property label-value block, and concatenate to our FreeObject data member
		const bool allowJaggedData = false;
		for (unsigned int gridCounter = 0; gridCounter < infoBlocks.size(); gridCounter++)
		{
			const std::vector<std::string>& columnNames = std::get<0>(infoBlocks[gridCounter]);
			const std::vector<etrading::ContainedTypeEnum>& colTypes = std::get<1>(infoBlocks[gridCounter]);
			const VariantMatrix& rangeData = std::get<2>(infoBlocks[gridCounter]);
			freeObject_ += createFreeObjectFromGrid( objectName, columnNames, colTypes, rangeData, propertyKeys[gridCounter], allowJaggedData);
		}

		validateKeys();
		calibrate();
	}

	InflationCurve::InflationCurve( const std::string& objectName,
									const FreeObject& freeObject)
									: IsLWOObject( objectName, INFLATION_CURVE ),
									freeObject_( freeObject )
	{
		calibrate();
	}

	InflationCurve::~InflationCurve()
	{
	}

	const SchemaObject InflationCurve::toSchemaObject() const
	{
		SchemaObject schemaObject (INFLATION_CURVE, getRefToName() );
		toSchemaObject( schemaObject );

		return schemaObject;
	}

	// This is a helper method to populate the supplied SchemaObject
	void const InflationCurve::toSchemaObject( SchemaObject& schemaObject ) const
	{
		const std::vector<std::string> keyNames = freeObject_.keyNames();
		const std::vector<std::vector<Variant> >& allDataView = freeObject_.viewAllData();

		for ( size_t i = 0; i < freeObject_.numberOfSchemas(); i++ )
		{
			schemaObject.addDataSchema( freeObject_.viewSchema(i) );

			const std::string& propertyName = keyNames[i];
			const auto columnIndices = freeObject_.columnsOfSchema( propertyName );
			VariantMatrix variantMatrix;
			std::for_each(columnIndices.cbegin(), columnIndices.cend(), [&variantMatrix, &allDataView]( const int idx )
			{
				variantMatrix.push_back( allDataView[idx] );
			});

			schemaObject.setDataForSchema( propertyName, variantMatrix );
		}
	}

	/*
	*  @brief  Utility method which validates the property keys of this InflationCurve, to verify that all are recognized key names.
	*          Will throw if one of the keys is not expected.
	*/
	void InflationCurve::validateKeys() const
	{
		const std::vector<std::string>& propertyNames = freeObject_.keyNames();

		const bool validateKeys = true;
		for (size_t i = 0; i < propertyNames.size(); i++)
		{
			const std::string& propertyName = propertyNames[i];
			LabelValueBlock inputLVB = propertyToLabelValueBlock( propertyName );

			InflationCurveEnum inflationCurveEnum = toInflationCurveEnum(propertyName);
			switch ( inflationCurveEnum )
			{
			case MODEL_PROPERTIES:
				validateKeysForLVB( inflation_curve_properties_lvbKeys(), inputLVB.getKeys(), validateKeys, propertyName);
				break;

			default:
				break;
			}
		}
	}

	/* Brief Returns the curveCollection from which a discount curve is obtained.
	*/
	std::string InflationCurve::getCurveCollection() const
	{
		return curveCollection_;
	}

	LADate InflationCurve::getAsOfDate() const
	{
		return asOfDate_;
	}

	CCY InflationCurve::getCurrency() const
	{
		return currency_;
	}

	/* @brief	Returns the name of this curve.
	*			This name used internally to label each dummy calibration instrument.
	*/
	std::string InflationCurve::getInflationIndexName() const
	{
		return inflationIndexName_;
	}


	/* @brief Returns a LabelValue block containing the configuration information for the specified propertyKey
	* @param [in]   propertyKey   The property to be displayed
	* @param [out]  A LabelValue block containing the properties
	*/
	LabelValueBlock InflationCurve::propertyToLabelValueBlock( const std::string& propertyKey ) const
	{
		LAStringMatrix stringMatrix = getLAStringMatrixFromFreeObject( freeObject_, propertyKey );
		LabelValueBlock lvb( stringMatrix );

		return lvb;
	}

	/* @brief Populates a default swap expression label value block for use with calibration instruments
	 */
	LabelValueBlock InflationCurve::setupSwapExpressionLVBforCalibration() const
	{
		// Set up the Swap Expression LVB used for repricing swap calibration instruments
		LAStringVector keys;
		LAStringVector values;

		keys.reserve(5);
		values.reserve(5);

		// We standardize these trade keys because we are only interested in par rate calculations 
		keys.push_back(IRS_KEY::PAY_RECEIVE.c_str());					values.push_back("PAY");
		keys.push_back(IRS_KEY::NOTIONAL.c_str());					    values.push_back("1.0");
		keys.push_back(IRS_KEY::EFFECTIVE_DATE.c_str());				values.push_back("");
		keys.push_back(IRS_KEY::MATURITY_DATE.c_str());				    values.push_back("");
		keys.push_back(SWAP_EXPRESSION_KEY::RATE_OR_SPREAD2.c_str());	values.push_back("0.0");

		LabelValueBlock swapExpressionLVB = etrading::populateLabelValueBlock(keys, values);

		return swapExpressionLVB;
	}

	/* @brief Parses the market data and constructs a map from instrument maturity date to input market data point.
	 *        The purpose is to ensure that all calibration points are accessed in the order of increasing maturity.
	 *
	 * @param [in]   referenceDate		A reference start date used to calculate maturity dates from tenors.
	 * @returns The InflationMarketDataMap
	 */
	InflationMarketDataMap InflationCurve::loadMarketDataMap( const LADate& referenceDate ) const
	{
		// Read the market data and perform sanity checks
		const bool throwIfKeyMissing = false;
		VariantMatrix marketData = getVariantMatrixFromFreeObject( freeObject_, toString(INFLATIONCURVE_ZC_INFLATIONSWAPS), throwIfKeyMissing );
		const size_t numMarketDataColumns = marketData.size();

		InflationMarketDataMap marketDataMap;
		if (numMarketDataColumns == 0)
		{
			// Return an empty map if there are no columns to read.
			return marketDataMap;
		}

		MLIB_REQUIRE(numMarketDataColumns == 2, toString(INFLATIONCURVE_ZC_INFLATIONSWAPS) + ": should contain 2 columns. Found " + std::to_string(static_cast<long long> (numMarketDataColumns)) + " columns.");

		// Iterate through the calibration marketData rows and for each row determine the instrumentMaturityDate and instrument quote.
		const size_t numMarketDataRows = marketData[0].size();
		for (size_t i = 0; i < numMarketDataRows; i++)
		{
			std::string instrumentID;
			LADate instrumentMaturityDate;

			// Only process the row if the data in column 0 is non-blank. i.e. trim blank rows
			std::string dataInColumnZero = marketData[0][i];
			if (dataInColumnZero != "")
			{
				instrumentID = "";
				std::string  instrumentTenorOrMaturityDate = dataInColumnZero;
				instrumentMaturityDate = validateMaturityDate( referenceDate, instrumentTenorOrMaturityDate.c_str() );

				Variant value = marketData[1][i];
				double instrumentQuote = std::numeric_limits<double>::quiet_NaN();

				switch (value.getType())
				{
				case STRING_VALUE:
				{
					std::string stringValue = value.getValue<std::string>();
					boost::trim( stringValue );
					MLIB_REQUIRE( stringValue != "", "Missing instrument quote." );
					char * pFirstNonNumber;
                    instrumentQuote = strtod( stringValue.c_str(), &pFirstNonNumber );
					break;
				}
				case DOUBLE_VALUE:
					instrumentQuote = value.getValue<double>();
					break;
				case INTEGER_VALUE:
					instrumentQuote = static_cast<double> (value.getValue<int>());
					break;
				default:
					MLIB_THROW("Unexpected data type in Inflation Curve Market Data ");
				}

				ZCInflationSwapMarketData marketData;
				marketData.tenorOrDate = instrumentTenorOrMaturityDate;
				marketData.instrumentQuote = instrumentQuote;

				if (marketDataMap.find(instrumentMaturityDate) != marketDataMap.end())
				{
					MLIB_THROW("Found two or more calibration instruments with the same maturity date '"
						+ instrumentMaturityDate.stringWithFormat("DD-MM-YYYY")
						+ "'.");
				}
				marketDataMap[ instrumentMaturityDate ] = marketData;
			}
		}
		return marketDataMap;
	}

	/* @brief	Load seasonality data into curve. The seasonality data consists of two coluns:
	*			Month	seasonaityAdjustment
	*			The seasonalityAdjustment is in terms of a forward rate, as defined in Kerkhof's inflation book p30
	*/
	void InflationCurve::loadSeasonality()
	{
		monthlySeasonalData_.clear();

		// Read the market data and perform sanity checks
		const bool throwIfKeyMissing = false;
		VariantMatrix seasonalityMatrix = getVariantMatrixFromFreeObject( freeObject_, toString( INFLATIONCURVE_SEASONALITY ), throwIfKeyMissing );
		const size_t numSeasonalityColumns = seasonalityMatrix.size();	
		if ( numSeasonalityColumns == 0 )
		{
			// There are no columns to read
			return;
		}

		MLIB_REQUIRE( numSeasonalityColumns == 2, toString( INFLATIONCURVE_SEASONALITY ) + ": should contain 2 columns. Found " + std::to_string(static_cast<long long> (numSeasonalityColumns)) + " columns." );

		// Iterate through the seasonality rows and for each row obtain the seasonality quote.
		const size_t numSeasonalityRows = seasonalityMatrix[0].size();
		MLIB_REQUIRE( numSeasonalityRows == 12, toString( INFLATIONCURVE_SEASONALITY ) + ": should contain 12 rows. Found " + std::to_string(static_cast<long long> (numSeasonalityRows)) + " rows." );
		for ( size_t i = 0; i < numSeasonalityRows; i++ )
		{
			// Only process the row if the data in column 0 is non-blank. i.e. trim blank rows
			std::string dataInColumnZero = seasonalityMatrix[0][i];
			if ( dataInColumnZero != "" )
			{
				std::string  monthString = dataInColumnZero;

				Variant value = seasonalityMatrix[1][i];
				double seasonalityQuote = std::numeric_limits<double>::quiet_NaN();

				switch ( value.getType() )
				{
				case STRING_VALUE:
				{
					std::string stringValue = value.getValue<std::string>();
					boost::trim( stringValue );
					MLIB_REQUIRE( stringValue != "", "Missing seasonality quote." );
					char * pFirstNonNumber;
                    seasonalityQuote = strtod( stringValue.c_str(), &pFirstNonNumber );
					break;
				}
				case DOUBLE_VALUE:
					seasonalityQuote = value.getValue<double>();
					break;
				case INTEGER_VALUE:
					seasonalityQuote = static_cast<double> ( value.getValue<int>() );
					break;
				default:
					MLIB_THROW("Unexpected data type in Inflation Curve Seasonality Data ");
				}

				monthlySeasonalData_.push_back( seasonalityQuote );
			}
		}

		/* Check the actual number of monthly seasonal points we loaded.
		*  To prevent user error, we take the approach that you *must* provide all seasonal points;
		*  The curve will not build unless there are exactly 12 points.
		*  These points can be all zero if no seasonality adjustment is required.
		*/
		const int nSeasonalPoints = monthlySeasonalData_.size();
		
		MLIB_REQUIRE(nSeasonalPoints == 12, "Require exactly 12 monthly seasonal data points.");
	}


	// @brief	Called by constructors to calibrate inflation curve from the provided market data
	void InflationCurve::calibrate()
	{
		// Read the curve properties block
		LabelValueBlock curveProperties = propertyToLabelValueBlock( toString( INFLATIONCURVE_PROPERTIES ));

		asOfDate_					= curveProperties.getCompulsoryValueAsDate( INFLATION_CURVE_PROPERTIES_KEY::ASOF_DATE );
		spotLag_					= curveProperties.getCompulsoryValue( INFLATION_CURVE_PROPERTIES_KEY::SPOT_LAG );
		spotBusinessDayAdjustment_	= curveProperties.getCompulsoryValue( INFLATION_CURVE_PROPERTIES_KEY::SPOT_BUSINESSDAY_ADJUSTMENT );
		spotCalendar_				= curveProperties.getCompulsoryValue( INFLATION_CURVE_PROPERTIES_KEY::SPOT_CALENDAR );

		curveCollection_			= curveProperties.getCompulsoryValue( INFLATION_CURVE_PROPERTIES_KEY::CURVE_COLLECTION );
		currency_					= toCCYEnum(curveProperties.getCompulsoryValue( INFLATION_CURVE_PROPERTIES_KEY::CURRENCY ));

		inflationIndexName_			= curveProperties.getCompulsoryValue( INFLATION_CURVE_PROPERTIES_KEY::INFLATION_INDEX );
		fixingTableName_			= curveProperties.getCompulsoryValue( INFLATION_CURVE_PROPERTIES_KEY::FIXING_TABLE );

		annualInterpolationType_	= toInterpolationEnum( curveProperties.getCompulsoryValue( INFLATION_CURVE_PROPERTIES_KEY::ANNUAL_INTERPOLATION ) );

		auto swapGeneratorName		= curveProperties.getCompulsoryValue( INFLATION_CURVE_PROPERTIES_KEY::SWAP_GENERATOR );
		auto swapGenerator			= etrading::getSwapGenerator( swapGeneratorName );

		MLIB_REQUIRE( annualInterpolationType_ == LOG_LINEAR_INTERPOLATION, "Only LogLinear AnnualInterpolation is supported" );

		// Perform consistency checks between the Inflation Curve and the curveCollection
		MLIB_REQUIRE( curveCollection_.size() > 0, "Please provide a valid CurveCollection" );

		const std::string curveCurrency = getCurveCurrency(curveCollection_.c_str()).getCString();
		MLIB_REQUIRE( boost::iequals(curveCurrency, toString(currency_)), "CurveCollection currency does not match Inflation Curve currency: '" + curveCurrency + "' vs '" + toString(currency_) + ".");

		const LADate curveAsOfDate = getCurveAsOfDate(curveCollection_.c_str());
		MLIB_REQUIRE( curveAsOfDate == asOfDate_, "CurveCollection as-of date does not match Inflation Curve as-of date: '" + curveAsOfDate.stringWithFormat() + "' vs '" + asOfDate_.stringWithFormat() + ".");

		// Read the market data, sort maturities in chronological order and perform sanity checks
		const InflationMarketDataMap marketDataMap = loadMarketDataMap( asOfDate_ );

		loadSeasonality();

		calibrateToZCInflationSwaps( asOfDate_, swapGeneratorName, marketDataMap );

		loadCPIFirstYear();

		initializeAnnualInterpolator();
	}

	/* @brief	Allow the user to specify CPI fixing overrides in the first year, before the first swap instrument,
	*			and where seasonality does not apply
	*/
	void InflationCurve::loadCPIFirstYear()
	{
		// Read the market data and perform sanity checks
		const bool throwIfKeyMissing = false;
		VariantMatrix cpiFirstYearMatrix = getVariantMatrixFromFreeObject( freeObject_, toString( INFLATIONCURVE_CPI_FIRST_YEAR ), throwIfKeyMissing);
		const size_t numColumns = cpiFirstYearMatrix.size();
		if (numColumns == 0)
		{
			// There are no columns to read
			return;
		}

		MLIB_REQUIRE( numColumns == 2, toString(INFLATIONCURVE_CPI_FIRST_YEAR) + ": should contain 2 columns. Found " + std::to_string(static_cast<long long> (numColumns)) + " columns.");

		// Iterate through the CPI rows and for each row obtain the CPI quote.
		const size_t numCpiFirstYearRows = cpiFirstYearMatrix[0].size();

		for (size_t i = 0; i < numCpiFirstYearRows; i++)
		{
			// Only process the row if the data in column 0 is non-blank. i.e. trim blank rows
			std::string dataInColumnZero = cpiFirstYearMatrix[0][i];
			if (dataInColumnZero != "")
			{
				std::string  cpiTenorOrMaturityDate = dataInColumnZero;
				LADate cpiQuoteDate = validateMaturityDate( asOfDateWithFixlag_, cpiTenorOrMaturityDate.c_str());

				if ( inflationResetType_ == INFLATION_RESET_TYPE_MONTHLY_INTERPOLATION )
				{
					cpiQuoteDate.setDay( 1 );
				}

				Variant value = cpiFirstYearMatrix[1][i];
				double cpiQuote = std::numeric_limits<double>::quiet_NaN();

				switch (value.getType())
				{
				case STRING_VALUE:
				{
					std::string stringValue = value.getValue<std::string>();
					boost::trim(stringValue);
					MLIB_REQUIRE(stringValue != "", "Missing CPI First Year quote.");
					char * pFirstNonNumber;
                    cpiQuote = strtod( stringValue.c_str(), &pFirstNonNumber );
					break;
				}
				case DOUBLE_VALUE:
					cpiQuote = value.getValue<double>();
					break;
				case INTEGER_VALUE:
					cpiQuote = static_cast<double> (value.getValue<int>());
					break;
				default:
					MLIB_THROW("Unexpected data type in Inflation Curve CPI First Year data" );
				}

				// Store the CPI overrides in the curve as node points
				calibratedInflationPoints_[ cpiQuoteDate ] = cpiQuote;
			}
		}
	}

	/* @brief	As a post calibration step, initialize an interpolator with the discovered node points.
	*			This interpolator allows annual CPI values from the long end of the curve to be estimated:
	*			In this part of the curve node points may be spaced apart 10 or more years, the interpolator
	*			is used to obtain further points with 1Y spacing.
	*/
	void InflationCurve::initializeAnnualInterpolator()
	{
		// Should put in the base CPI value.

		switch (annualInterpolationType_ )
		{
		case LOG_LINEAR_INTERPOLATION:
		{
			// TODO: Check this: What internal daycount to use in the curve?
			const size_t nCalibrationPoints = calibratedInflationPoints_.size();

			// Reserve vectors of terms and logCPI values, used to initialize the interpolator
			std::vector<double> terms;
			terms.reserve( nCalibrationPoints );

			std::vector<double> logCPIs;
			logCPIs.reserve( nCalibrationPoints );

			// For each node point, convert dates to terms and calculate logCPI
			for ( auto point : calibratedInflationPoints_ )
			{
				const LADate pillarDate = point.first;
				const double pillarTerm = getYearFraction( asOfDateWithFixlag_, pillarDate, accrualDayCount_ );
				terms.push_back( pillarTerm );

				const double logCPI = std::log( point.second );
				logCPIs.push_back( logCPI );
			}
			annualInterpolator_ = InterpolationFactory::generateInterpolation( terms, logCPIs, LINEAR_INTERPOLATION );
			break;
		}
		default:
			MLIB_THROW( "Unsupported interpolation type: " + toString( annualInterpolationType_ ) );
		}
	}

	/* @brief Internal helper: calibrate the inflation curve using Zero Coupon Inflation Swap quotes
	 * @param [in]  asOfDate			A reference start date used to calculate maturity dates from tenors.
	 * @param [in]  swapGeneratorName	The swap generator used to obtain conventions and to create calibration instruments
	 * @param [in]  marketDataMap		Input market data, a map from maturity date to break-even par rate,
	 */
	void InflationCurve::calibrateToZCInflationSwaps( const LADate& asOfDate, const std::string& swapGeneratorName, const InflationMarketDataMap& marketDataMap )
	{

		// Additional parameters for creating ZC inflation swap instruments
		const bool isXccySwap = false;
		LabelValueBlock swapPropertiesLVB;
		LabelValueBlock valuationSettingsLVB( curveCollection_, "" );

		// Calculate the effective date for calibration instruments
		const LADate effectiveDate = LADateScheduleHelpers::getDate( asOfDate, spotLag_.c_str(), spotBusinessDayAdjustment_.c_str(), spotCalendar_.c_str() );
		LabelValueBlock swapExpressionLVB(  setupSwapExpressionLVBforCalibration(),
											IRS_KEY::EFFECTIVE_DATE,
											std::to_string(static_cast<long long>(LADateScheduleHelpers::getExcelDate( effectiveDate ))));

		// Calibrate to ZerouCoupon Inflation Swaps
		bool firstInstrument = true;
		for (auto it = marketDataMap.begin(); it != marketDataMap.end(); ++it)
		{
			const LADate& maturityDate = it->first;
			const ZCInflationSwapMarketData& marketData = it->second;

			// Construct the calibration instrument
			swapExpressionLVB = LabelValueBlock( swapExpressionLVB, IRS_KEY::MATURITY_DATE, MLIB_TO_STRING_FROM_INT( LADateScheduleHelpers::getExcelDate( maturityDate ) ) );
			swapExpressionLVB = LabelValueBlock( swapExpressionLVB, SWAP_EXPRESSION_KEY::RATE_OR_SPREAD1, MLIB_TO_STRING_FROM_DOUBLE( marketData.instrumentQuote ) );

			auto swapInstrument = createSwapFromGenerator( inflationIndexName_, swapGeneratorName, swapExpressionLVB, swapPropertiesLVB, isXccySwap );
			std::shared_ptr<ZeroCouponInflationSwap> inflationSwapInstrument = std::dynamic_pointer_cast<ZeroCouponInflationSwap>( swapInstrument );

			if ( firstInstrument )
			{
				firstInstrument = false;
				storeConventionsFromCalibrationIntrument( inflationSwapInstrument );
			}

			// NOTE: The InflationCurve is updated by this method in order to populate the new calibration point
			const double inflationIndexLevel = inflationSwapInstrument->impliedInflationIndexAtPar( valuationSettingsLVB, *this );
		}
	}

	/* @brief	During calibration, retrieve convention parameters from calibration instrument
	*/
	void InflationCurve::storeConventionsFromCalibrationIntrument( const std::shared_ptr<ZeroCouponInflationSwap>& calibrationInstrument )
	{
		std::shared_ptr<FixedLeg> fixedLeg;
		std::shared_ptr<InflationLeg> inflationLeg;
		calibrationInstrument->identifyLegs( fixedLeg, inflationLeg );

		auto schedule			= inflationLeg->getSchedule();
		auto inflationSchedule	= std::dynamic_pointer_cast<InflationSchedule>( schedule );

		inflationResetType_		= inflationSchedule->getInflationResetType();

		fixingAdvanceOrArrears_	= inflationSchedule->getFixingAdvanceOrArrears();
		fixingbusinessDayAdj_	= inflationSchedule->getFixingBusinessDayAdj();
		fixingCalendar_			= inflationSchedule->getFixingCalendar().getCString();
		fixLag_					= inflationSchedule->getFixLag().getCString();
		accrualDayCount_		= inflationSchedule->getAccrualDaycount();

		asOfDateWithFixlag_		= applyFixingLagToDate( asOfDate_, fixLag_ );
		
		/* When using an InflationResetType of "MonthlyInterpolation", the CPI value does not change within a month.
		*  So follow a convention where we set the day-of-month to 1 after applying the lag.
		*/
		if ( inflationResetType_ == INFLATION_RESET_TYPE_MONTHLY_INTERPOLATION )
		{
			asOfDateWithFixlag_.setDay( 1 );
		}

		/* Add an anchor node point at the beginning of the curve at asOfDateWithFixLag_.
		* This prevents the interpolator from entering "extrapolation mode" for terms
		* shorter than the first swap instrument maturity.
		*/
		auto fixingTable = getFixingTable( fixingTableName_ );
		const double inflationFixing = fixingTable->getFixingValue( toGregorianDateFromLADate( asOfDateWithFixlag_ ) );
		calibratedInflationPoints_[ asOfDateWithFixlag_ ] = inflationFixing;
	}

	/* @brief	Returns the inflation level for the specified date, no further lag adjustment.
	*			The inflation level is NOT adjusted for seasonality
	*/
	double InflationCurve::getUnadjustedInflationIndexForDate( const LADate& laggedDate ) const
	{
		double inflationIndex = 0.;
		auto it = calibratedInflationPoints_.find( laggedDate );
		if ( it != calibratedInflationPoints_.end() )
		{
			// Requested date is a node point. Return the corresponding value
			inflationIndex = it->second;
		}
		else
		{
			// Requested date is not a node point; use interpolator
			const double term = getYearFraction( asOfDateWithFixlag_, laggedDate, accrualDayCount_ );
			const double logCPI = annualInterpolator_->interpolate( term );
			inflationIndex = std::exp( logCPI );
		}

		return inflationIndex;
	}

	/* @brief	Returns the inflation level for the specified date, no further lag adjustment.
	*			The inflation level IS adjusted for seasonality
	*/
	double InflationCurve::getMonthlyInflationIndexForLaggedDate( const LADate& laggedDate ) const
	{
		// First check the fixingTable to see if there is a fixing for this date
		auto fixingTable = getFixingTable( fixingTableName_ );
		FixingResult fixingResult = fixingTable->getOptionalFixingValue( toGregorianDateFromLADate(laggedDate ) );
		
		if ( fixingResult.fixingFound_ )
		{
			// We found an inflation fixing! Return it
			const double inflationFixing = fixingResult.fixingValue_;
			return inflationFixing;
		}
		else
		{
			// No fixing available for this date; interpolate from curve
			const double unadjustedInflationIndex = getUnadjustedInflationIndexForDate( laggedDate );

			const double seasonalityFactor	= getSeasonalityFactor( laggedDate );
			const double inflationIndex		= unadjustedInflationIndex * seasonalityFactor;
			return inflationIndex;
		}
	}

	/* @brief	Returns the forward inflation rate for the specified date, no further lag adjustment.
	*			The forward rate is calculated from the CPI level at the curve pillars which bracket the
	*			specified date and is given as the log return of the CPI level. See Kerkhof p30
	*/
	ForwardRateResults InflationCurve::getForwardRateForDate( const LADate& laggedDate ) const
	{
		ForwardRateResults forwardRateResults;

		// Find the upper and lower curve pillar points which bracket the input date
		auto pillarGreaterThanOrEqual = calibratedInflationPoints_.lower_bound( laggedDate );
		if ( pillarGreaterThanOrEqual == calibratedInflationPoints_.begin() )
		{
			// the first pillar in calibratedInflationPoints_ is after than or equal to the input date.
			// Therefore calculate the foward rate between the lagged asOfDate and this first pillar.
			forwardRateResults.upperBracketDate	= pillarGreaterThanOrEqual->first;
			const double upperBracketCPI		= pillarGreaterThanOrEqual->second;
		
			forwardRateResults.lowerBracketDate = asOfDateWithFixlag_;
			forwardRateResults.lowerBracketDate.setDay( 1 );  // Fixings are always asOf the 1st of the month
			
			auto fixingTable = getFixingTable( fixingTableName_ );
			const double lowerBracketCPI = fixingTable->getFixingValue( toGregorianDateFromLADate( forwardRateResults.lowerBracketDate ) );

			// Calculate the log return of the growth rate
			forwardRateResults.forwardRate = std::log( upperBracketCPI / lowerBracketCPI );
		}
		else if ( pillarGreaterThanOrEqual == calibratedInflationPoints_.end() )
		{
			// all the calibratedInflationPoints_ are smaller than the laggedDate we are looking for.
			// We cannot find bracketing pillar dates.
			MLIB_THROW( "Forward rate calculation does not allow extrapolation." );
		}
		else
		{
			// The general case; the input date is between two pillar dates
			forwardRateResults.upperBracketDate = pillarGreaterThanOrEqual->first;
			const double upperBracketCPI		= pillarGreaterThanOrEqual->second;
			
			auto pillarLessThan = std::prev( pillarGreaterThanOrEqual );
			forwardRateResults.lowerBracketDate	= pillarLessThan->first;
			const double lowerBracketCPI		= pillarLessThan->second;

			// Calculate the log return of the growth rate
			forwardRateResults.forwardRate = std::log( upperBracketCPI / lowerBracketCPI );
		}

		return forwardRateResults;
	}


	/* @brief	Get the the seasonality adjustment for the specified date, no further lag adjustment.
	*			The calculation is performed using the exponential seasonal model from Kerkhof p30
	*/
	double InflationCurve::getSeasonalityFactor( const LADate& laggedDate ) const
	{	
		double totalSeasonalFactor = 1.0;

		const int inputMonth	= laggedDate.monthOfYear();
		const int baseMonth		= asOfDateWithFixlag_.monthOfYear();

		if ( inputMonth == baseMonth )
		{
			// No monthly seasonal adjustment required. Exit early.
			return totalSeasonalFactor;
		}

		const double term = getYearFraction( asOfDateWithFixlag_, laggedDate, accrualDayCount_ );
		if ( term < 1.0 )
		{
			// No seasonality adjustment below 1 year
			return totalSeasonalFactor;
		}

		/* From the seasonal adjustment factor point of view, we either allow exactly zero seasonal data points,
		   or there must be exactly 12 monthly points.
		*/
		const int nSeasonalPoints = monthlySeasonalData_.size();
		if ( nSeasonalPoints == 0 )
		{
			// No seasonality data; exit early
			return totalSeasonalFactor;
		}

		MLIB_REQUIRE( nSeasonalPoints == 12, "Require exactly 12 monthly seasonal data points." );

		for ( int month = baseMonth; month != inputMonth; month++ )
		{
			/* Some subtlety here: to calculate the seasonal factor between month m and m+1 we
			   use the seasonal rate for month m+1. However the monthlySeasonalData_ is a
			   zero-based vector, so we index directly using "month" mod 12.
			*/
			month = month % 12;

			const double seasonalRate = monthlySeasonalData_[ month ];

			// Calculate the exponential seasonal adjustment over the month.
			const double seasonalFactor = std::exp( seasonalRate / 12.0 );
			totalSeasonalFactor *= seasonalFactor;

		}

		return totalSeasonalFactor;
	}

	/* @brief	Obtains the inflation level for the specified date, adjusting by the fixing lag
	*
	* @param[in]	date					Return the CPI for this date
	* @param[in]	inflationResetType		Specifies how the lag should be taken into account: MonthlyInterpolation or DailyInterpolation
	* @param[in]	fixLag					Adjust the specified date backwards by this lag tenor.
	* @returns	The calculated CPI level
	*/
	double InflationCurve::getInflationIndexForDate( const LADate& date, const InflationResetTypeEnum& inflationResetType, const std::string& fixLag ) const
	{
		switch (inflationResetType)
		{
		case INFLATION_RESET_TYPE_MONTHLY_INTERPOLATION:
		{
			/* When using an InflationResetType of "MonthlyInterpolation", the CPI value does not change within a month.
			*  So follow a convention where we set the day-of-month to 1 after applying the lag.
			*/
			LADate fixingDate = applyFixingLagToDate( date, fixLag );
			fixingDate.setDay( 1 );
			return getMonthlyInflationIndexForLaggedDate( fixingDate );
			break;
		}
		case INFLATION_RESET_TYPE_DAILY_INTERPOLATION:
		{
			/* 1. Calculate the lagged date, and calculate the monthly inflation index at the beginning of the month
			*/
			LADate beginningOfMonthFixingDate = applyFixingLagToDate( date, fixLag );
			beginningOfMonthFixingDate.setDay( 1 );
			const double thisMonthInflationIndex = getMonthlyInflationIndexForLaggedDate( beginningOfMonthFixingDate );

			/* 2. Calculate the inflation index for the beginning of the next month
			*/
			LADate nextMonthFixingDate = beginningOfMonthFixingDate;
			nextMonthFixingDate.addMonths( 1 );
			const double nextMonthInflationIndex = getMonthlyInflationIndexForLaggedDate( nextMonthFixingDate );

			/* Calculate the number of days in the month for the input date
			*/
			const int dayOfMonth = date.dayOfMonth();
			const int numDaysInMonth = date.intervalToEndOfMonth() + dayOfMonth;

			/* Linear interpolation between the two monthly CPI levels
			*/
			const double inflationIndex = thisMonthInflationIndex + (double)( dayOfMonth - 1 ) / (double)(numDaysInMonth) * ( nextMonthInflationIndex - thisMonthInflationIndex );
			return inflationIndex;

			break;
		}
		default:
			MLIB_THROW( "Only an InflationResetType of 'MonthlyInterpolation' and 'DailyInterpolation' is supported" );

		}

	}

	/* @brief Updates the inflation index calibration stored in the curve by adding a node point defined for the specified date.
	 *        This method intended to be used by the calibration process while boostrapping the inflation index points from input break-even par-rates.
	 *
	 * @param [in]   date			The date of this calibration point 
	 * @param [in]   inflationIndex	The inflation index to set for the specified date
	 */
	void InflationCurve::setCalibrationPoint( const LADate& date, const double inflationIndex )
	{
		calibratedInflationPoints_[ date ] = inflationIndex;
	}

	/* @brief	Returns a matrix of node points representing calibrated values
	*			The format is: Date (including lag), IndexLevel
	*/
	AnyTypeMatrix InflationCurve::getCalibrationParameters() const
	{
		AnyTypeMatrix calibrationParameters;

		for (auto it = calibratedInflationPoints_.begin(); it != calibratedInflationPoints_.end(); ++it)
		{
			AnyTypeVector row;
			const LADate maturityDate = it->first;
			const double cpiValue = it->second;

			const int dateAsInt = static_cast<long long> (LADateScheduleHelpers::getExcelDate(maturityDate));
			row.push_back( dateAsInt );
			row.push_back( cpiValue );

			calibrationParameters.push_back( row );
		}

		return calibrationParameters;

	}

	/* @brief	Calculate a fixing date which is lagged back by the amount specified by fixLagTenor.
	*  @param	inputDate		The date to be lagged
	*  @param	@fixLagTenor	The lag amount, expressed as a tenor. A positive tenor "2M" lags the date backwards
	*  @returns	A fixing date which is lagged back from the inputDate by the amount specified by fixLagTenor
	*/
	LADate InflationCurve::applyFixingLagToDate( const LADate& inputDate, const std::string& fixLagTenor ) const
	{
		DateVector unadjustedDates = { inputDate };
		DateVector fixingDates = validateAndGenerateFixingSchedule( unadjustedDates,
																	toString(fixingbusinessDayAdj_),
																	fixingCalendar_,
																	fixLagTenor,
																	toString( fixingAdvanceOrArrears_ ), true /* includeLastExtraFixingDate */);
		const LADate fixingDate = fixingDates[0];
		return fixingDate;
	}

	/* Brief	Utility method to return the Zero Coupon Inflation Swap market data used to calibrate the curve.
	*			This method mainly used for testing
	*/
	std::vector<ZCInflationSwapMarketData> InflationCurve::getZCInflationSwapInputMarketData() const
	{
		const InflationMarketDataMap marketDataMap = loadMarketDataMap( asOfDate_ );

		std::vector<ZCInflationSwapMarketData> zcInflationSwapQuotes;

		/* Iterate through the marketDataMap and construct a vector of instrument quotes.
		*  Each quote consists of a tenor (or date) along with the break-even par rate
		*/
		for (auto it = marketDataMap.begin(); it != marketDataMap.end(); ++it)
		{
			const ZCInflationSwapMarketData& marketData = it->second;
			zcInflationSwapQuotes.push_back( marketData );
		}

		return zcInflationSwapQuotes;
	}
}

