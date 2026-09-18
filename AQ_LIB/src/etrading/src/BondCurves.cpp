#include "BondCurves.h"
#include "AQObjUtilities.h"
#include "ObjectUtilities.h"
#include "DataUtilities.h"
#include "ExceptionMacros.h"

#include "AQLDateScheduleHelpers.h"

#include <boost/algorithm/string.hpp>
#include <cmath>


namespace etrading
{
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
		  interpolationMethod_( rhs.interpolationMethod_ ),
		  extrapolationMethod_( rhs.extrapolationMethod_ ),
		  calibratedYields_( rhs.calibratedYields_ ),
		  calibratedDiscountFactors_( rhs.calibratedDiscountFactors_ )
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
		LabelValueBlock bondQuoteLVB;

		const std::string spreadPropertiesName = toString( BONDSPREADCURVE_PROPERTIES );
		const bool isSpreadCurve =  freeObject_.doesKeyExist( spreadPropertiesName );

		if ( isSpreadCurve )
		{
			// Read the properties for a spread curve
		
			LabelValueBlock spreadCurveProperties = toLabelValueBlock( spreadPropertiesName );
			
			spread_						= spreadCurveProperties.getCompulsoryValueAsDouble( BONDSPREADCURVE_PROPERTIES_KEY::SPREAD );
			auto benchmarkBondCurveName	= spreadCurveProperties.getCompulsoryValueAsString( BONDSPREADCURVE_PROPERTIES_KEY::BENCHMARK_BOND_CURVE, spreadPropertiesName );
			auto benchmarkBondCurve		= getBondCurve( benchmarkBondCurveName );

			// Read parameters from the benchmark curve

			settlementDate_				= benchmarkBondCurve->getSettlementDate();
			yieldCalculationTypeEnum_	= benchmarkBondCurve->getYieldCalculationTypeEnum();
			yieldQuoteInPercent_		= benchmarkBondCurve->getYieldQuoteInPercent();
			interpolationMethod_		= benchmarkBondCurve->getInterpolationMethod();
			extrapolationMethod_		= benchmarkBondCurve->getExtrapolationMethod();
			bondQuoteLVB				= benchmarkBondCurve->toLabelValueBlock( toString( BONDCURVE_MARKETDATA ) );
		}
		else
		{			
			spread_ = 0.0;

			// Read the properties for a standard bond curve, constructed from bond instruments

			LabelValueBlock curveProperties = toLabelValueBlock( toString( BONDCURVE_PROPERTIES ) );

			settlementDate_				= curveProperties.getCompulsoryValueAsDate( BONDCURVE_PROPERTIES_KEY::SETTLEMENT_DATE );
			yieldCalculationTypeEnum_	= toYieldCalculationTypeEnum( curveProperties.getOptionalValueAsString( BONDCURVE_PROPERTIES_KEY::YIElD_CALCULATION_TYPE ) );
			yieldQuoteInPercent_		= curveProperties.getCompulsoryValueAsBool( BONDCURVE_PROPERTIES_KEY::YIELD_QUOTE_IN_PERCENT );
			interpolationMethod_        = curveProperties.getOptionalValue( BONDCURVE_PROPERTIES_KEY::INTERPOLATION, INTERPOLATION_KEYS::PIECEWISE_CONSTANT );
			extrapolationMethod_        = curveProperties.getOptionalValue( BONDCURVE_PROPERTIES_KEY::EXTRAPOLATION, INTERPOLATION_KEYS::FLAT );

			AQ_REQUIRE( boost::iequals( interpolationMethod_, INTERPOLATION_KEYS::PIECEWISE_CONSTANT ), "Only PiecewiseConstant interpolation is supported." );
			AQ_REQUIRE( boost::iequals( extrapolationMethod_, INTERPOLATION_KEYS::FLAT ), "Only Flat extrapolation is supported." );

			bondQuoteLVB                = toLabelValueBlock( toString( BONDCURVE_MARKETDATA ) );
		}

		/* Construct a map from bond maturity date to bond quote. This is used to detect duplicates in the input market data.
		   i.e. detect two or more bond quotes for the same maturity date.
		   The map also sorts the bond quotes in order of increasing maturity date.
		 */
		std::map<AQLDate, double> marketDataBondQuotes;		// map from maturityDate to bondYieldQuote
		std::map<AQLDate, std::string> marketDataBondIds;    // map from maturityDate to bondId

		const std::vector<std::string> bondInstrumentIds = bondQuoteLVB.getKeys();

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


	/* @brief		Returns the yield interpolated from the BondCurve for the specified couponDate
	*				Note: the method uses piecewise-constant interpolation.
	* @param[in]	couponDate	The date for which the yield is required
	* @returns		The interpolated yield
	*/
	double BondCurve::getYield( const AQLDate& couponDate ) const
	{
		AQ_REQUIRE( ! calibratedYields_.empty(), "No bond curve calibration points have been found. Check bond quotes input." );

		AQ_REQUIRE( couponDate >= settlementDate_, "Invalid couponDate '" + couponDate.stringWithFormat() + "' is earlier than bond curve settlementDate '" + settlementDate_.stringWithFormat() + "'." );

		double yieldPoint = std::numeric_limits<double>::quiet_NaN();

		// Find the pillarDate in calibratedYields_ where the pillarDate is not considered earlier than couponDate
		// i.e. the pillarDate on or after couponDate
		auto it = calibratedYields_.lower_bound( couponDate );
		if ( it != calibratedYields_.end() )
		{
			yieldPoint = it->second;
		}
		else
		{
			// Extrapolate-flat in yield; return the final yield point
			yieldPoint = calibratedYields_.rbegin()->second;
		}

		return yieldPoint;
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

	/* @brief Updates the calibration stored in the curve by adding a discountFactor point for the specified pillar date.
	*         This method intended to be used by the calibration process when fitting the curve to input bond quotes.
	*
	* @param [in]   bondMaturityDate	The date corresponding to this coupon yield
	* @param [in]   discountFactor		The discountFactor at the bond curve pillar date
	*/
	void BondCurve::setDiscountFactorAtCalibrationPoint( const AQLDate& bondMaturityDate, const double& discountFactor )
	{
		calibratedDiscountFactors_[ bondMaturityDate ] = discountFactor;
	}	

	/* @brief	Returns the bond curve calibration as a matrix.
	*			Column 0 contains curve pillar dates
	*			Column 1 contains the calibrated yield points
	*			Column 2 contains the calibrated discount factors, when available
	*			(calibratedDiscountFactors_ holds one entry per pillar date - see
	*			setDiscountFactorAtCalibrationPoint); omitted otherwise
	*/
	AnyTypeMatrix BondCurve::displayBondCurve() const
	{
		AnyTypeMatrix yieldMatrix;

		// Check if we have discount factors to display
		const bool discountFactorsAvailable = ( calibratedYields_.size() == calibratedDiscountFactors_.size() );

		for ( auto yieldPillar : calibratedYields_ )
		{
			AnyTypeVector row;
			const AQLDate pillarDate = yieldPillar.first;
			const double yield = yieldPillar.second;

			const int dateAsInt = static_cast<long long> (AQLDateScheduleHelpers::getExcelDate(pillarDate));
			row.push_back(dateAsInt);
			row.push_back(yield);

			if ( discountFactorsAvailable )
			{
				auto dfIterator = calibratedDiscountFactors_.find( pillarDate );
				AQ_REQUIRE( dfIterator != calibratedDiscountFactors_.end(), "BondCurve does not contain a discountFactor for pillar date: " + pillarDate.convertDateToString() );
				const double discountFactor = dfIterator->second;
				row.push_back( discountFactor );
			}
						
			yieldMatrix.push_back( row );
		}

		return yieldMatrix;
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

				case BONDSPREADCURVE_PROPERTIES:
					validateKeysForLVB( bond_spread_curve_properties_lvbKeys(), inputLVB.getKeys(), validateKeys, propertyName );
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

	std::string BondCurve::getInterpolationMethod() const
	{
		return interpolationMethod_;
	}

	std::string BondCurve::getExtrapolationMethod() const
	{
		return extrapolationMethod_;
	}
}

