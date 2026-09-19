#include "BondCurves.h"
#include "AQObjUtilities.h"
#include "ObjectUtilities.h"
#include "DataUtilities.h"
#include "ExceptionMacros.h"

#include "AQLDateScheduleHelpers.h"
#include "ScheduleValidation.h"

#include <boost/algorithm/string.hpp>
#include <cmath>


namespace etrading
{
	namespace
	{
		/* @brief Linearly interpolates between two (date, value) nodes at the targetDate, using ACT/365 year fractions as the time axis.
		*/
		double linearInterpolate( const AQLDate& date1, double value1, const AQLDate& date2, double value2, const AQLDate& targetDate )
		{
			const double totalYears = getYearFraction( date1, date2, ACT_365_DAYCOUNT );
			AQ_REQUIRE( totalYears != 0.0, "Cannot linearly interpolate: the two node dates are identical." );

			const double targetYears = getYearFraction( date1, targetDate, ACT_365_DAYCOUNT );
			const double fraction = targetYears / totalYears;

			return value1 + fraction * ( value2 - value1 );
		}

		/* @brief	Interpolates/extrapolates a value at targetDate from a sorted map of (date, value) nodes,
		*			per the supplied interpolation (between nodes) and extrapolation (outside the node range) methods.
		*			With FLAT/FLAT this reproduces plain piecewise-constant-forward / flat-extrapolation behaviour.
		*/
		double interpolateOnCurveMap( const std::map<AQLDate, double>& nodes, const AQLDate& targetDate, BondCurveInterpolationEnum interpolationMethod, BondCurveInterpolationEnum extrapolationMethod )
		{
			AQ_REQUIRE( ! nodes.empty(), "Cannot interpolate: no calibration points are available." );

			if ( nodes.size() == 1 )
			{
				return nodes.begin()->second;
			}

			auto itUpper = nodes.lower_bound( targetDate );

			if ( itUpper == nodes.end() )
			{
				// targetDate is after the last node: extrapolate on the right
				auto itLast = std::prev( nodes.end() );
				if ( extrapolationMethod == BONDCURVE_LINEAR )
				{
					auto itSecondLast = std::prev( itLast );
					return linearInterpolate( itSecondLast->first, itSecondLast->second, itLast->first, itLast->second, targetDate );
				}
				return itLast->second; // FLAT: carry the last node's value forward
			}

			if ( itUpper->first == targetDate || itUpper == nodes.begin() )
			{
				// Exact match at a node, OR targetDate is before the first node: extrapolate on the left
				if ( itUpper->first != targetDate && extrapolationMethod == BONDCURVE_LINEAR )
				{
					auto itSecond = std::next( itUpper );
					return linearInterpolate( itUpper->first, itUpper->second, itSecond->first, itSecond->second, targetDate );
				}
				return itUpper->second;
			}

			// targetDate falls strictly between two distinct nodes
			auto itLower = std::prev( itUpper );
			if ( interpolationMethod == BONDCURVE_LINEAR )
			{
				return linearInterpolate( itLower->first, itLower->second, itUpper->first, itUpper->second, targetDate );
			}
			return itUpper->second; // FLAT (piecewise-constant): step to the node on/after targetDate
		}
	}
	/* @brief Nelson-Siegel interpolation. Given a set of calibration parameters and a bond maturity, interpolates the corresponding bond yield on the curve
	*  @param[in]	parameters	NelsonSiegel parameters structure: beta0, beta1, beta2, lambda1 are used.
	*  @param[in]	tau			Bond maturity in years
	*  @returns		The bond yield interpolated on the curve, corresponding to the supplied maturity
	*/
	double NelsonSiegelInterpolator( const NelsonSiegelSvenssonParameters& parameters, double tau )
	{
		const double normalizedT = tau / parameters.lambda1_;
		const double yield = parameters.beta0_
							+ parameters.beta1_ *  ( 1.0 - exp( -normalizedT )) / normalizedT
							+ parameters.beta2_ * (( 1.0 - exp( -normalizedT )) / normalizedT - exp( -normalizedT ));
		return yield;
	}

	/* @brief Nelson-Siegel-Svensson interpolation. Given a set of calibration parameters and a bond maturity, interpolates the corresponding bond yield on the curve
	*  @param[in]	parameters	NelsonSiegel parameters structure: beta0, beta1, beta2, beta3, lambda1, lambda2 are used.
	*  @param[in]	tau			Bond maturity in years
	*  @returns		The bond yield interpolated on the curve, corresponding to the supplied maturity
	*/
	double SvenssonInterpolator( const NelsonSiegelSvenssonParameters& parameters, double tau )
	{
		const double normalizedT1 = tau / parameters.lambda1_;
		const double normalizedT2 = tau / parameters.lambda2_;

		const double yield = parameters.beta0_
							+ parameters.beta1_ *  ( 1.0 - exp( -normalizedT1 )) / normalizedT1
							+ parameters.beta2_ * (( 1.0 - exp( -normalizedT1 )) / normalizedT1 - exp( -normalizedT1 ))
							+ parameters.beta3_ * (( 1.0 - exp( -normalizedT2 )) / normalizedT2 - exp( -normalizedT2 ));
		return yield;
	}

	/* @brief Polynomial interpolation. Given a vector of polynomial coefficients and a bond maturity, interpolates the corresponding bond yield on the curve
	*  @param[in]	coefficients	Polynomial function coefficients: a_0 x^n + a_1 x^(n-1) + ... +a_n i.e. highest order coefficient is first in the vector.
	*  @param[in]	tau				Bond maturity in years
	*  @returns		The bond yield interpolated on the curve, corresponding to the supplied maturity
	*/
	double PolynomialInterpolator( const DoubleVector& coefficients, double tau )
	{
		const size_t nCoefficients = coefficients.size();
		
		AQ_REQUIRE( nCoefficients > 0, "Missing polynomial coefficients vector supplied to PolynomialInterpolator. ");

		/*
		* We implement HORNER'S METHOD below:
		* https://en.wikipedia.org/wiki/Horner%27s_method
		*
		* The following loop calculates a polynomial function by operating on the factorized expression as follows:
		* Assuming a cubic function:  
		*   a0 x^3 + a1 x^2 + a2 x + a3
		* = x ( a0 x^2 + a1 x + a2 ) + a3
		* = x ( x ( a0 x + a1 ) + a2 ) + a3
		* = x ( x ( x ( a0 ) + a1 ) + a2 ) + a3  <-- the code below operates on this expression
		* 
		* Therefore the code below begins with:
		* yield = coefficient[0], then
		* at each iteration, multiplies the current yield quantity by tau and adds on coefficient[i],
		* ultimately building up the full polynomial expression.
		*/
		double yield = coefficients[0];
		for ( size_t i=1; i<nCoefficients; i++ )
		{
			yield = yield * tau + coefficients[i];
		}

		return yield;

	}

	// --- Bond Curve Stripping


	/* @brief Main Constructor
	 * @param[in] objectName    The name of this BondCurve instance
	 * @param[in] propertyKeys  A vector containing the names of each configuration block
	 * @param[in] infoBlocks    A vector containing the configuation blocks
	 */
	BondCurve::BondCurve( const std::string& objectName,
                          const std::vector<std::string>& propertyKeys,
                          const std::vector<TableInfo>& infoBlocks ) 
						: IsAQObject(objectName, BOND_CURVE), 
						  freeObject_(objectName)
	{

		// Create a FreeObject from each property label-value block, and concatenate to our FreeObject data member
		const bool allowJaggedData = false;
        for( unsigned int gridCounter = 0; gridCounter < infoBlocks.size(); gridCounter++ )
        {
            const std::vector<std::string>& columnNames = std::get<0>( infoBlocks[ gridCounter ] );
            const std::vector<etrading::ContainedTypeEnum>& colTypes = std::get<1>( infoBlocks[ gridCounter ]) ;
            const VariantMatrix& rangeData = std::get<2>( infoBlocks[ gridCounter ] );
            freeObject_ += createFreeObjectFromGrid( objectName, columnNames, colTypes, rangeData, propertyKeys[ gridCounter ], allowJaggedData );
        }

		// Verify that all BondCurve properties are valid and expected
		validateKeys();

		calibrate();
	}

	/* @brief Copy Constructor
	 */
	BondCurve::BondCurve(const BondCurve& rhs)
		: IsAQObject( rhs.getRefToName(), BOND_CURVE ),
		  freeObject_( rhs.freeObject_ ),
		  settlementDate_( rhs.settlementDate_ ),
		  yieldCalculationTypeEnum_( rhs.yieldCalculationTypeEnum_ ),
		  yieldQuoteInPercent_( rhs.yieldQuoteInPercent_ ),
		  curveTypeEnum_( rhs.curveTypeEnum_ ),
		  benchmarkBondCurveName_( rhs.benchmarkBondCurveName_ ),
		  interpolationMethod_( rhs.interpolationMethod_ ),
		  extrapolationMethod_( rhs.extrapolationMethod_ ),
		  spread_( rhs.spread_ ),
		  calibratedYields_( rhs.calibratedYields_ )
	{
	}

	/* @brief Constructor used by deserialization
	 * @param[in] objectName    The name of this BondCurve instance
	 * @param[in] freeObject    A freeObject constructed from the serialized data
	 */
	BondCurve::BondCurve( const std::string& objectName, const FreeObject& freeObject ) 
					: IsAQObject(objectName, BOND_CURVE ), 
					  freeObject_(freeObject)
	{
		calibrate();
    }

	std::shared_ptr<BondCurve> BondCurve::clone() const
    {
       auto copy = std::make_shared<BondCurve>(*this);
       return copy;
    }

	/* @brief Used to serialize an instance of this class
	 * @param[out] the populated SchemaObject
	 */
	const SchemaObject BondCurve::toSchemaObject() const
	{
		SchemaObject schemaObject( BOND_CURVE, getRefToName());
        toSchemaObject(schemaObject);

        return schemaObject;
	}

    // This is a helper method to populate the supplied SchemaObject
    void const BondCurve::toSchemaObject(SchemaObject& schemaObject) const
   	{
        
		const std::vector<std::string> keyNames = freeObject_.keyNames();
		const std::vector<std::vector<Variant> >& allDataView = freeObject_.viewAllData();

		for (size_t i=0; i < freeObject_.numberOfSchemas(); i++)
		{
			schemaObject.addDataSchema(freeObject_.viewSchema(i));

			const std::string& propertyName = keyNames[i];
			const auto columnIndices = freeObject_.columnsOfSchema( propertyName );
			VariantMatrix variantMatrix;
			std::for_each( columnIndices.cbegin(), columnIndices.cend(), [&variantMatrix, &allDataView]( const int idx )
            {
				variantMatrix.push_back( allDataView[ idx ] );
            } );

			schemaObject.setDataForSchema(propertyName, variantMatrix);
		}
	}

	/* @brief		Called by constructor to calibrate yields to maturity from the provided bond quotes
	*/
	void BondCurve::calibrate()
	{
		LabelValueBlock curveProperties = toLabelValueBlock( toString( BONDCURVE_PROPERTIES ) );

		curveTypeEnum_ = toBondCurveTypeEnum( curveProperties.getOptionalValueAsString( BONDCURVE_PROPERTIES_KEY::CURVE_TYPE, toString( BONDCURVE_TYPE_OUTRIGHT ) ) );

		settlementDate_				= curveProperties.getCompulsoryValueAsDate( BONDCURVE_PROPERTIES_KEY::SETTLEMENT_DATE );
		yieldCalculationTypeEnum_	= toYieldCalculationTypeEnum( curveProperties.getOptionalValueAsString( BONDCURVE_PROPERTIES_KEY::YIElD_CALCULATION_TYPE ) );
		yieldQuoteInPercent_		= curveProperties.getCompulsoryValueAsBool( BONDCURVE_PROPERTIES_KEY::YIELD_QUOTE_IN_PERCENT );

		// A spread curve defaults to Linear/Flat (sensible for a handful of spread nodes); a plain bond curve keeps the historical PiecewiseConstant/Flat default.
		const std::string defaultInterpolation = ( curveTypeEnum_ == BONDCURVE_TYPE_SPREAD ) ? INTERPOLATION_KEYS::LINEAR_INTERPOLATION : INTERPOLATION_KEYS::PIECEWISE_CONSTANT;
		interpolationMethod_		= toBondCurveInterpolationEnum( curveProperties.getOptionalValue( BONDCURVE_PROPERTIES_KEY::INTERPOLATION, defaultInterpolation ) );
		extrapolationMethod_		= toBondCurveInterpolationEnum( curveProperties.getOptionalValue( BONDCURVE_PROPERTIES_KEY::EXTRAPOLATION, INTERPOLATION_KEYS::FLAT ) );

		// A spread overlay interpolated Flat (step) between nodes is a genuine discontinuity, not a smoothing
		// artifact - no fix can make a step function look smooth. Spread curves therefore only support Linear
		// interpolation between spread nodes; Extrapolation beyond the first/last node may still be Flat or Linear.
		AQ_REQUIRE( ( curveTypeEnum_ != BONDCURVE_TYPE_SPREAD ) || ( interpolationMethod_ == BONDCURVE_LINEAR ),
			"A BondSpreadCurve's Interpolation must be Linear - Flat/PiecewiseConstant interpolation of the spread overlay produces genuine discontinuities between spread nodes." );

		// Optional flat additive yield shock (e.g. for risk shocks), in the same percent/decimal convention as the yield quotes. Defaults to zero.
		const double rawSpread = curveProperties.getOptionalValueAsDouble( BONDCURVE_PROPERTIES_KEY::SPREAD, 0.0 );
		spread_ = yieldQuoteInPercent_ ? rawSpread / 100.0 : rawSpread;

		LabelValueBlock bondQuoteLVB = toLabelValueBlock( toString( BONDCURVE_MARKETDATA ) );
		const std::vector<std::string> bondInstrumentIds = bondQuoteLVB.getKeys();
		AQ_REQUIRE( ! bondInstrumentIds.empty(), "BondCurve_MarketData is required and must contain at least one bond quote." );

		if ( curveTypeEnum_ == BONDCURVE_TYPE_SPREAD )
		{
			benchmarkBondCurveName_ = curveProperties.getCompulsoryValueAsString( BONDCURVE_PROPERTIES_KEY::BENCHMARK_BOND_CURVE, toString( BONDCURVE_PROPERTIES ) );
			auto benchmarkBondCurve = getBondCurve( benchmarkBondCurveName_ );

			// The benchmark's own raw quotes, keyed by maturity date - used below so a spread node measured against
			// a benchmark bond's own maturity compares like-for-like (raw quote vs raw quote), rather than against
			// that bond's bootstrapped curve pillar (a subtly different number - see the comment further below).
			LabelValueBlock benchmarkQuoteLVB = benchmarkBondCurve->toLabelValueBlock( toString( BONDCURVE_MARKETDATA ) );
			const bool benchmarkYieldQuoteInPercent = benchmarkBondCurve->getYieldQuoteInPercent();

			std::map<AQLDate, double> benchmarkRawYieldsByDate;
			for ( const std::string& benchmarkBondId : benchmarkQuoteLVB.getKeys() )
			{
				const double rawBenchmarkYield = benchmarkQuoteLVB.getCompulsoryValueAsDouble( benchmarkBondId );
				const double benchmarkBondYield = benchmarkYieldQuoteInPercent ? rawBenchmarkYield / 100.0 : rawBenchmarkYield;
				const AQLDate benchmarkMaturityDate = getBond( benchmarkBondId )->getSchedule()->getMaturityDate();
				benchmarkRawYieldsByDate[ benchmarkMaturityDate ] = benchmarkBondYield;
			}

			/* Derive one spread node per bond supplied in BondCurve_MarketData: the bond's own yield, less the
			   benchmark's yield at that same maturity, where "the benchmark's yield" is ALWAYS a smooth, Linear
			   interpolation between the benchmark's own raw quotes (exact match at a coincident date, degrading
			   to flat beyond the benchmark's own range) - regardless of the benchmark curve's own configured
			   Interpolation/Extrapolation. Deliberately NOT benchmarkBondCurve->getYield(): that reads the
			   benchmark through whatever interpolation IT is configured with, which - if Flat - is a step function.
			   Two spread bonds maturing only a few days apart, straddling one of the benchmark's own pillars, would
			   then be measured against different sides of that step: a jump baked into the spread node itself,
			   before any spread-side interpolation ever runs. Using a smooth reference here, independent of the
			   benchmark's own display/pricing convention, is what a "spread over a benchmark" means in practice,
			   and it is also what makes a coincident spread bond reproduce its own yield exactly (the exact-match
			   branch of interpolateOnCurveMap below), with no separate override needed. */
			std::map<AQLDate, double> spreadNodes;
			std::map<AQLDate, double> spreadBondOwnYields;
			for ( const std::string& bondId : bondInstrumentIds )
			{
				const double rawBondYield = bondQuoteLVB.getCompulsoryValueAsDouble( bondId );
				const double bondYield = yieldQuoteInPercent_ ? rawBondYield / 100.0 : rawBondYield;

				auto bondInstrument = getBond( bondId );
				const AQLDate bondMaturityDate = bondInstrument->getSchedule()->getMaturityDate();
				AQ_REQUIRE( spreadNodes.count( bondMaturityDate ) == 0, "Duplicate maturity date: " + bondMaturityDate.convertDateToString() + " for bond ID " + bondId.c_str() );

				const double benchmarkYield = interpolateOnCurveMap( benchmarkRawYieldsByDate, bondMaturityDate, BONDCURVE_LINEAR, BONDCURVE_FLAT );
				spreadNodes[ bondMaturityDate ] = bondYield - benchmarkYield;
				spreadBondOwnYields[ bondMaturityDate ] = bondYield;
			}

			/* Re-run the SAME bootstrap mechanism a plain BondCurve uses (price from yield, then solve via
			   yieldFromPriceAndBondCurve), but targeting the benchmark's own bonds with (benchmark's own raw yield +
			   interpolated spread) as the calibration yield, instead of overlaying spread arithmetically on top of
			   the benchmark's already-calibrated pillars. This guarantees a genuinely zero spread reproduces the
			   benchmark curve exactly, since it is then the identical bootstrap over the identical bond quotes.
			   The spread bonds' own maturities are added as further pillars (their own yield + shock) where they
			   do not already coincide with a benchmark bond's maturity - where they do coincide, the benchmark-bond
			   target computed here already equals that spread bond's own yield exactly, by construction above. */
			std::map<AQLDate, double> targetYields;         // map from maturityDate to yieldPlusSpread
			std::map<AQLDate, std::string> targetBondIds;   // map from maturityDate to bondId

			for ( const std::string& bondId : benchmarkQuoteLVB.getKeys() )
			{
				const double benchmarkBondYield = benchmarkRawYieldsByDate.at( getBond( bondId )->getSchedule()->getMaturityDate() );

				auto bondInstrument = getBond( bondId );
				const AQLDate bondMaturityDate = bondInstrument->getSchedule()->getMaturityDate();
				AQ_REQUIRE( targetYields.count( bondMaturityDate ) == 0, "Duplicate maturity date: " + bondMaturityDate.convertDateToString() + " for benchmark bond ID " + bondId.c_str() );

				const double interpolatedSpread = interpolateOnCurveMap( spreadNodes, bondMaturityDate, interpolationMethod_, extrapolationMethod_ );
				targetYields[ bondMaturityDate ] = benchmarkBondYield + interpolatedSpread + spread_;
				targetBondIds[ bondMaturityDate ] = bondId;
			}

			// Add the spread bonds' own maturities as further pillars, where they don't already coincide with a
			// benchmark bond's maturity (a coincident date is already set correctly above).
			for ( const std::string& bondId : bondInstrumentIds )
			{
				auto bondInstrument = getBond( bondId );
				const AQLDate bondMaturityDate = bondInstrument->getSchedule()->getMaturityDate();

				if ( targetYields.count( bondMaturityDate ) == 0 )
				{
					targetYields[ bondMaturityDate ] = spreadBondOwnYields[ bondMaturityDate ] + spread_;
					targetBondIds[ bondMaturityDate ] = bondId;
				}
			}

			// Iterate through all target yields sorted by increasing maturity date, and calibrate a bond curve yield point for each,
			// via the same bootstrap mechanism a plain BondCurve uses.
			for ( const auto& target : targetYields )
			{
				const AQLDate& sortedMaturityDate = target.first;
				const double yieldPlusSpread = target.second;

				const std::string& bondId = targetBondIds[ sortedMaturityDate ];
				auto bondInstrument = getBond( bondId );

				const double bondPrice = bondInstrument->price( settlementDate_, yieldPlusSpread, yieldCalculationTypeEnum_ );
				bondInstrument->yieldFromPriceAndBondCurve( settlementDate_, bondPrice, *this );
			}
		}
		else if ( curveTypeEnum_ == BONDCURVE_TYPE_OUTRIGHT )
		{
			/* Construct a map from bond maturity date to bond quote. This is used to detect duplicates in the input market data.
			   i.e. detect two or more bond quotes for the same maturity date.
			   The map also sorts the bond quotes in order of increasing maturity date.
			 */
			std::map<AQLDate, double> marketDataBondQuotes;		// map from maturityDate to bondYieldQuote
			std::map<AQLDate, std::string> marketDataBondIds;    // map from maturityDate to bondId

			for ( const std::string& bondId : bondInstrumentIds )
			{
				const double bondYieldQuote = bondQuoteLVB.getCompulsoryValueAsDouble( bondId );

				// Check for duplicate bond maturity dates
				auto bondInstrument = getBond( bondId );
				const AQLDate bondMaturityDate = bondInstrument->getSchedule()->getMaturityDate();
				AQ_REQUIRE ( marketDataBondQuotes.count( bondMaturityDate ) == 0, "Duplicate maturity date: " + bondMaturityDate.convertDateToString() + " for bond ID " + bondId.c_str() );

				marketDataBondQuotes[ bondMaturityDate ] = yieldQuoteInPercent_ ? bondYieldQuote / 100.0 : bondYieldQuote;
				marketDataBondIds[ bondMaturityDate ] = bondId;
			}

			// Iterate through all the bond quotes sorted by increasing maturity date,
			// and calibrate a bond curve yield point for each quote
			for ( const auto& bondQuote : marketDataBondQuotes )
			{
				const AQLDate& sortedMaturityDate = bondQuote.first;
				const double bondYieldQuote = bondQuote.second;
				const double yieldPlusSpread = bondYieldQuote + spread_;

				const std::string bondId = marketDataBondIds[ sortedMaturityDate ];
				auto bondInstrument = getBond( bondId );

				//  Calculate implied bond price from yield
				const double bondPrice = bondInstrument->price( settlementDate_, yieldPlusSpread, yieldCalculationTypeEnum_ );

				// Calibrate the bond curve to this bond instrument.
				// NOTE: This bond curve is updated by this method in order to populate the new calibration point.
				bondInstrument->yieldFromPriceAndBondCurve( settlementDate_, bondPrice, *this );
			}
		}
		else
		{
			AQ_THROW( "Invalid CurveType '" << toString( curveTypeEnum_ ) << "'." );
		}
	}


	/* @brief		Returns the yield interpolated from the BondCurve for the specified couponDate,
	*				per this curve's interpolationMethod_/extrapolationMethod_.
	* @param[in]	couponDate	The date for which the yield is required
	* @returns		The interpolated yield
	*/
	double BondCurve::getYield( const AQLDate& couponDate ) const
	{
		AQ_REQUIRE( ! calibratedYields_.empty(), "No bond curve calibration points have been found. Check bond quotes input." );

		AQ_REQUIRE( couponDate >= settlementDate_, "Invalid couponDate '" + couponDate.stringWithFormat() + "' is earlier than bond curve settlementDate '" + settlementDate_.stringWithFormat() + "'." );

		return interpolateOnCurveMap( calibratedYields_, couponDate, interpolationMethod_, extrapolationMethod_ );
	}

	/* @brief Updates the yield calibration stored in the curve by adding a yield point for the specified pillarDate.
	*        This method intended to be used by the calibration process when fitting the curve to input bond quotes.
	*
	* @param [in]   bondMaturityDate	The date corresponding to this coupon yield
	* @param [in]   yield				The estimate of the yield for this curve pillar date
	*/
	void BondCurve::setCalibrationPoint( const AQLDate& bondMaturityDate, const double& yield)
	{
		calibratedYields_[ bondMaturityDate ] = yield;
	}

	/* @brief	Returns the bond curve calibration as a matrix.
	*			Column 0 contains curve pillar dates
	*			Column 1 contains the calibrated yield points
	*/
	AnyTypeMatrix BondCurve::displayBondCurve() const
	{
		AnyTypeMatrix yieldMatrix;

		for ( auto yieldPillar : calibratedYields_ )
		{
			AnyTypeVector row;
			const AQLDate pillarDate = yieldPillar.first;
			const double yield = yieldPillar.second;

			const int dateAsInt = static_cast<long long> (AQLDateScheduleHelpers::getExcelDate(pillarDate));
			row.push_back(dateAsInt);
			row.push_back(yield);

			yieldMatrix.push_back( row );
		}

		return yieldMatrix;
	}

	/* @brief	Returns the curve's calibrated pillar dates, in increasing order.
	*			Used when building a BOND_SPREAD_CURVE over this curve as a benchmark.
	*/
	std::vector<AQLDate> BondCurve::getCalibratedPillarDates() const
	{
		std::vector<AQLDate> pillarDates;
		pillarDates.reserve( calibratedYields_.size() );
		for ( const auto& yieldPillar : calibratedYields_ )
		{
			pillarDates.push_back( yieldPillar.first );
		}
		return pillarDates;
	}

	/*
	*  @brief  validates the property keys of this BondCurve, to verify that all are recognized key names.
	*          Will throw if one of the keys is not expected.
	*/
	void BondCurve::validateKeys() const
	{
		const std::vector<std::string>& propertyNames = freeObject_.keyNames();

		const bool validateKeys = true;
		for ( size_t i = 0; i < propertyNames.size(); i++)
		{
			const std::string& propertyName = propertyNames[i];
			LabelValueBlock inputLVB = toLabelValueBlock( propertyName );

			const BondCurveEnum bondCurveEnum = etrading::toBondCurveEnum( propertyName );
			switch (bondCurveEnum)
			{
				case BONDCURVE_PROPERTIES:
					validateKeysForLVB( bond_curve_properties_lvbKeys(), inputLVB.getKeys(), validateKeys, propertyName );
					break;

				default:
					break;
			}
		}

	}

	/* @brief Returns a LabelValue block containing the configuration information for the specified propertyKey
	* @param [in]   propertyKey   The property to be displayed
	* @param [out]  A LabelValue block containing the properties
	*/
	LabelValueBlock BondCurve::toLabelValueBlock( const std::string& propertyKey ) const
	{
		AQLStringMatrix stringMatrix = getAQLStringMatrixFromFreeObject( freeObject_, propertyKey );
		LabelValueBlock lvb( stringMatrix );

		return lvb;
	}

	// Simple data getters

	AQLDate BondCurve::getSettlementDate() const
	{
		return settlementDate_;
	}

	YieldCalculationTypeEnum BondCurve::getYieldCalculationTypeEnum() const
	{
		return yieldCalculationTypeEnum_;
	}

	bool BondCurve::getYieldQuoteInPercent() const
	{
		return yieldQuoteInPercent_;
	}

	BondCurveInterpolationEnum BondCurve::getInterpolationMethod() const
	{
		return interpolationMethod_;
	}

	BondCurveInterpolationEnum BondCurve::getExtrapolationMethod() const
	{
		return extrapolationMethod_;
	}
}

