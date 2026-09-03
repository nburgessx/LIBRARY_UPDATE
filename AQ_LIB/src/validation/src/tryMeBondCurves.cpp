#include "tryMeBondCurves.h"
#include "NelsonSiegelFitting.h"
#include "PolynomialFitting.h"
#include "BondCurves.h"
#include "ObjectUtilities.h"


// Includes for Structured Exception Handling and Input/Output Logging
#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "CommonConstants.h"
#include "RecordMacros.h"
#include "DataUtilities.h"	// For MLIB_TO_STRING macros

// Namespaces for Input/Output Logging
using etrading::CreateDataFile;
using etrading::decorateFilename;

namespace validation_api
{

	/* @brief Fits a Nelson-Siegel parameterized curve to the supplied bond maturities and yields.
	*  @param[in]		bondMaturities					A vector containing the bond maturities to fit
	*  @param[in]		bondYields						A vector containing the bond yields to fit
	*  @param[in]		initialGuess					A structure containing the initial guess for beta0, beta1, beta2 and lambda
	*  @param[in]		maxIterations					The maximum number of iterations allowed.
	*  @param[in]		maxStationaryStateIterations	The maximum number of iterations around a stationary point
	*  @param[in]		lowerBounds						A vector containing the lower bounds for beta0, beta1, beta2, lambda. If empty vector, default is -50 for betas, 0 for lambda
	*  @param[in]		upperBounds						A vector containing the upper bounds for beta0, beta1, beta2, lambda. If empty vector, default is +50 for betas and lambda
	*  @returns		The calibrated parameters, store in a NelsonSiegelSvenssonCalibrationResults structure
	*/
	etrading::NelsonSiegelSvenssonCalibrationResults tryMeBondCurveNelsonSiegelCalibrate( const DoubleVector& bondMaturities,
																				          const DoubleVector& bondYields,
																				          const etrading::NelsonSiegelSvenssonParameters& initialGuess,
																				          const unsigned int& maxIterations,
																				          const unsigned int& maxStationaryStateIterations,
																				          const DoubleVector& lowerBounds,
																				          const DoubleVector& upperBounds )
	{
		VALID_EXCEPTION_START
	
		// Record Inputs for logs, tests and playback - Note the first argument is the decorated file prefix and the second the suffix
        RECORD_INPUTS( initialGuess.beta0_, initialGuess.beta1_, initialGuess.beta2_, initialGuess.lambda1_, maxIterations, maxStationaryStateIterations, lowerBounds, upperBounds, bondMaturities, bondYields );

		// Use the default lower bound, unless user supplied an appropriate lower bound vector
		DoubleVector lowBounds = { -50.0, -50.0, -50.0, 0.0 };  // beta0, beta1, beta2, lambda
		if ( lowerBounds.size() > 0 )
		{
            MLIB_REQUIRE( lowerBounds.size() == 4, "Invalid LowerBounds: Require 4 LowerBounds Values" )
			lowBounds = lowerBounds;
		}

		// Use the default upper bound, unless user supplied an appropriate upper bound vector
		DoubleVector upBounds = { 50.0, 50.0, 50.0, 50.0 };  // beta0, beta1, beta2, lambda
		if ( upperBounds.size() > 0 )
		{
            MLIB_REQUIRE( upperBounds.size() == 4, "Invalid UpperBounds: Require 4 UpperBounds Values" )
			upBounds = upperBounds;
		}

		MLIB_REQUIRE( lowBounds.size() == upBounds.size(), "Number of Nelson Siegel lower bounds should match number of upper bounds.");
		MLIB_REQUIRE( bondYields.size() == bondMaturities.size(), "Number of bond yields should match the number of bond maturities. ");

		MLIB_REQUIRE( bondMaturities.size() >= 4, "Require a minimum of 4 bond maturities." );
		MLIB_REQUIRE( bondYields.size() >= 4, "Require a minimum of 4 bond yields." );

		etrading::NelsonSiegelSvenssonCalibrationResults calibrationResult = etrading::calibrateNelsonSiegelSvenssonToBondYields( etrading::NELSON_SIEGEL_INTERPOLATION,
																														          bondMaturities,
																														          bondYields,
																														          initialGuess,
																														          lowBounds,
																														          upBounds,
																														          maxIterations,
																														          maxStationaryStateIterations );
		// Record outputs for logs, tests and playback
		if ( CreateDataFile::recordEnabled() )
        {
			FUNCTION_NAME
			std::string fileName = functionName;
			fileName += "_outputs";
			CreateDataFile outputFile( LAString( fileName.c_str() ) );

			DoubleVector result(4);
			result[0] = calibrationResult.parameters_.beta0_;
			result[1] = calibrationResult.parameters_.beta1_;
			result[2] = calibrationResult.parameters_.beta2_;
			result[3] = calibrationResult.parameters_.lambda1_;
			outputFile.write( "output", result );
        }

		return calibrationResult;

		VALID_EXCEPTION_END
	}


	/* @brief Fits a Svensson parameterized curve to the supplied bond maturities and yields.
	*  @param[in]		bondMaturities					A vector containing the bond maturities to fit
	*  @param[in]		bondYields						A vector containing the bond yields to fit
	*  @param[in]		initialGuess					A structure containing the initial guess for beta0, beta1, beta2, beta3, lambda1 and lambda2
	*  @param[in]		maxIterations					The maximum number of iterations allowed.
	*  @param[in]		maxStationaryStateIterations	The maximum number of iterations around a stationary point
	*  @param[in]		lowerBounds						A vector containing the lower bounds for beta0, beta1, beta2, lambda. If empty vector, default is -50 for betas, 0 for lambdas
	*  @param[in]		upperBounds						A vector containing the upper bounds for beta0, beta1, beta2, lambda. If empty vector, default is +50 for betas and lambdas
	*/
	etrading::NelsonSiegelSvenssonCalibrationResults tryMeBondCurveSvenssonCalibrate( const DoubleVector& bondMaturities,
																			          const DoubleVector& bondYields,
																			          const etrading::NelsonSiegelSvenssonParameters& initialGuess,
																			          const unsigned int& maxIterations,
																			          const unsigned int& maxStationaryStateIterations,
																			          const DoubleVector& lowerBounds,
																			          const DoubleVector& upperBounds )
	{
		VALID_EXCEPTION_START
	
		// Record Inputs for logs, tests and playback - Note the first argument is the decorated file prefix and the second the suffix
        RECORD_INPUTS( initialGuess.beta0_, initialGuess.beta1_, initialGuess.beta2_, initialGuess.beta3_, initialGuess.lambda1_, initialGuess.lambda2_, maxIterations, maxStationaryStateIterations, lowerBounds, upperBounds, bondMaturities, bondYields )

		// Use the default lower bound, unless user supplied an appropriate lower bound vector
		DoubleVector lowBounds = { -50.0, -50.0, -50.0, -50.0, 0.0, 0.0 };  // beta0, beta1, beta2, beta3, lambda1, lambda2
		if ( lowerBounds.size() > 0 )
		{
            MLIB_REQUIRE( lowerBounds.size() == 6, "Invalid LowerBounds: Require 6 LowerBounds Values" )
			lowBounds = lowerBounds;
		}

		// Use the default upper bound, unless user supplied an appropriate upper bound vector
		DoubleVector upBounds = { 50.0, 50.0, 50.0, 50.0, 50.0, 50.0 };  // beta0, beta1, beta2, beta3, lambda1, lambda2
		if ( upperBounds.size() > 0 )
		{
		    MLIB_REQUIRE( upperBounds.size() == 6, "Invalid UpperBounds: Require 6 UpperBound Values" )
            upBounds = upperBounds;
		}

		MLIB_REQUIRE( lowBounds.size() == upBounds.size(), "Number of Nelson Siegel lower bounds should match number of upper bounds.")
		MLIB_REQUIRE( bondYields.size() == bondMaturities.size(), "Number of bond yields should match the number of bond maturities. ")

		MLIB_REQUIRE( bondMaturities.size() >= 6, "Require a minimum of 6 bond maturities." )
		MLIB_REQUIRE( bondYields.size() >= 6, "Require a minimum of 6 bond yields." )

		etrading::NelsonSiegelSvenssonCalibrationResults calibrationResult = etrading::calibrateNelsonSiegelSvenssonToBondYields( etrading::SVENSSON_INTERPOLATION,
																														          bondMaturities,
																														          bondYields,
																														          initialGuess,
																														          lowBounds,
																														          upBounds,
																														          maxIterations,
																														          maxStationaryStateIterations );

		// Record outputs for logs, tests and playback
		if ( CreateDataFile::recordEnabled() )
        {
			FUNCTION_NAME
			std::string fileName = functionName;
			fileName += "_outputs";
			CreateDataFile outputFile( LAString( fileName.c_str() ) );

			DoubleVector result(6);
			result[0] = calibrationResult.parameters_.beta0_;
			result[1] = calibrationResult.parameters_.beta1_;
			result[2] = calibrationResult.parameters_.beta2_;
			result[3] = calibrationResult.parameters_.beta3_;
			result[4] = calibrationResult.parameters_.lambda1_;
			result[5] = calibrationResult.parameters_.lambda2_;
			outputFile.write( "output", result );
        }

		return calibrationResult;

		VALID_EXCEPTION_END
	}

	/* @brief Main API: Calibrates Polynomial-interpolation coefficients to the specified bond yields and maturities
	 *
	 * @param[in]		polynomialOrder		The order of the polynomial: 0 = constant line, 1 = linear, 2 = quadratic etc
	 * @param[in]		bondMaturities		A vector of bond maturities, to fit
	 * @param[in]		bondYields			A vector of input bond yields to fit
	 * @param[in]		maxIterations		The maximum number of iterations allowed.
	 * @param[in]		maxStationaryStateIterations	The maximum number of iterations around a stationary point
	 * @param[in]		lowerBound			Lower bound to be applied to each each polynomial coefficient
	 * @param[in]		upperBound			Upper bound to be applied to each each polynomial coefficient
	 * returns		The calibrated parameters, store in a PolynomialCalibrationResults structure
	 */
	etrading::PolynomialCalibrationResults tryMeBondCurvePolynomialCalibrate( const unsigned int polynomialOrder,
																			  const DoubleVector& bondMaturities,
																			  const DoubleVector& bondYields,
																		 	  const unsigned int& maxIterations,
																			  const unsigned int maxStationaryStateIterations,
																			  const double lowerBound,
																			  const double upperBound )
	{
		VALID_EXCEPTION_START
	
		// Record Inputs for logs, tests and playback - Note the first argument is the decorated file prefix and the second the suffix
        RECORD_INPUTS( polynomialOrder, bondMaturities, bondYields, maxIterations, maxStationaryStateIterations, lowerBound, upperBound );

		MLIB_REQUIRE( bondYields.size() == bondMaturities.size(), "Number of bond yields should match the number of bond maturities. ");

		MLIB_REQUIRE( bondMaturities.size() >= polynomialOrder, "Require a minimum of " + MLIB_TO_STRING_FROM_SIZE_T(polynomialOrder) + " of Bond Maturities, NumberOfDataPoints >= PolynomialOrder" );
		MLIB_REQUIRE( bondYields.size() >= polynomialOrder, "Require a minimum of " + MLIB_TO_STRING_FROM_SIZE_T(polynomialOrder) + " of Bond Yields, NumberOfDataPoints >= PolynomialOrder" );

		etrading::PolynomialCalibrationResults calibrationResult = etrading::calibratePolynomialBondYields( polynomialOrder,
																										    bondMaturities,
																										    bondYields,
																										    maxIterations,
																										    maxStationaryStateIterations,
																										    lowerBound,
																										    upperBound );

		// Record outputs for logs, tests and playback
		DoubleVector coefficients = calibrationResult.coefficients_;
		RECORD_OUTPUTS ( coefficients );

		return calibrationResult;

		VALID_EXCEPTION_END
	}

	 /* @brief Creates a BondCurve, calibrated from bond quotes
	 * @param [in] objectName        The name of the Credit Model object to create
	 * @param [in] propertyNames     A vector of property names corresponding to each label-value block of properties.
	 *                               BONDCURVE_PROPERTIES, BONDCURVE_MARKETDATA
	 * @param [in] infoBlocks        A vector of containing the label-value blocks of properties
	 * @param [out]                  The objectName
	 */
    std::string tryMeBondCurveCreate( const std::string& objectName,
                                      const std::vector<std::string>& dataBlockNames,
                                      const etrading::JSONInfoBlockTuples& infoBlocks )
    {
		VALID_EXCEPTION_START
		
        // Perform initial basic sanity checks
        size_t nColumnHeaders   = dataBlockNames.size();
        size_t nDataColumns     = infoBlocks.size();
		
        MLIB_REQUIRE( nDataColumns > 0, "Invalid InfoBlock Data: Empty InfoBlock - No data provided" )
        MLIB_REQUIRE( nColumnHeaders == nDataColumns, "Invalid InfoBlock Data: Number of Data Column Headers " +  std::to_string(static_cast<long long>(nColumnHeaders)) + " does not match the actual number of Data Columns " +  std::to_string(static_cast<long long>(nDataColumns)) )

		// Recording of inputs for playback
		if (CreateDataFile::recordEnabled()) 
		{
			CreateDataFile file(decorateFilename("tryMeBondCurveCreate_inputs", objectName.c_str()));
			file.write("generatorFunction", "tryMeBondCurveCreate");
			file.write("objectName", objectName);

			// Write out each propertyName and corresponding block of property config data
			for( unsigned int i = 0; i < dataBlockNames.size(); i++ )
			{
				const LAString dataBlockName( dataBlockNames[i].c_str() );
				const etrading::VariantMatrix& configData = std::get<2>( infoBlocks[ i ] );
				file.write( dataBlockName, transpose(configData) );
			}
		}

		// Verify that the supplied propertyNames have been set
        const bool hasAnEmptyName = std::any_of( dataBlockNames.cbegin(),
												 dataBlockNames.cend(),
												 []( const std::string & dataBlockName ) -> bool
        {
            return ( dataBlockName.empty() || dataBlockName == "" );
        } );

        MLIB_REQUIRE( !hasAnEmptyName, "Invalid InfoBlock: Invalid Data Column Header - One of the Column Names is empty or invalid " + etrading::containerAsString( dataBlockNames ) )

		// Verify that the supplied propertyNames match the BondCurveEnum
		std::set<etrading::BondCurveEnum> enumSet;
		std::for_each(dataBlockNames.cbegin(),
					  dataBlockNames.cend(),
					  [&enumSet] (const std::string & dataBlockName )
		{	
			etrading::BondCurveEnum bondCurveEnum = etrading::toBondCurveEnum( dataBlockName );
			enumSet.insert( bondCurveEnum );
		});

		// Create the Bond Curve object
		etrading::BondCurve bondCurve( objectName, dataBlockNames, infoBlocks );

		// ..  and store in the cache
        etrading::copyToCache<etrading::BondCurve>( bondCurve );

		if (CreateDataFile::recordEnabled()) 
		{
			CreateDataFile file(decorateFilename("tryMeBondCurveCreate_outputs", objectName.c_str()));
			file.write("output", objectName);
		}

        return objectName;
		
		VALID_EXCEPTION_END
	}

	/* @brief Displays the bond curve calibration as a matrix of pillar dates and yield points.
	*  @param[in]	bondCurveName	The name of the bond curve object to display
	*/
	AnyTypeMatrix tryMeBondCurveDisplay( const std::string& bondCurveName )
	{
		VALID_EXCEPTION_START

		RECORD_INPUTS( bondCurveName )

		auto bondCurve = etrading::getBondCurve( bondCurveName );

		AnyTypeMatrix result = bondCurve->displayBondCurve();

		// Record Outputs AND Return the Result for logs, tests and playback
        RECORD_OUTPUTS_AND_RETURN_RESULT( result )

		VALID_EXCEPTION_END
	}

	/* @brief Inspects the Yield Term Structure of a Bond Curve and returns the yield used on a particular forward reference date
	*  @param[in] bondCurveName		The Bond Curve Name
	*  @param[in] referenceDate		The forward reference date
	*/
	double tryMeBondYield(const std::string& bondCurveName, const LADate& referenceDate )
	{
	 VALID_EXCEPTION_START

	  RECORD_INPUTS(bondCurveName);

	 auto bondCurve = etrading::getBondCurve(bondCurveName);
	 const double yield = bondCurve->getYield( referenceDate );
	 
	 // Record Outputs AND Return the Result for logs, tests and playback
	 RECORD_OUTPUTS_AND_RETURN_RESULT(yield)

	 VALID_EXCEPTION_END

	}

	/* @brief Calculates the price of a bond using a bond curve to discount the coupons
	*  @param[in] bondObjectName	Name of the bond object to price
	*  @param[in] settlementDate	The settlement date to use for bond pricing
	*  @param[in] bondCurveName		Name of the bond curve
	*/
	double tryMeLWOBondPriceFromBondCurve( const std::string& bondObjectName, const LADate& settlementDate, const std::string & bondCurveName )
	{
		VALID_EXCEPTION_START

		// Recording of inputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_INPUTS( bondObjectName, std::string(), bondObjectName, settlementDate, bondCurveName )

		const etrading::BondPtr bondObject      = etrading::getBond( bondObjectName );
		const etrading::BondCurvePtr bondCurve  = etrading::getBondCurve( bondCurveName );

		const double result = bondObject->priceFromBondCurve( settlementDate, *bondCurve );

		// Recording of outputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
        RECORD_DECORATED_OUTPUTS_AND_RETURN_RESULT(bondObjectName, std::string(), result )

		VALID_EXCEPTION_END
	}

	/* @brief Calculates the yield-to-maturity of a bond using a bond curve to discount the coupons
	*  @param[in] bondObjectName	Name of the bond object to price
	*  @param[in] settlementDate	The settlement date to use for bond pricing
	*  @param[in] bondCurveName		Name of the bond curve
	*/
	double tryMeLWOBondYieldFromBondCurve( const std::string& bondObjectName, const LADate& settlementDate, const std::string & bondCurveName )
	{
		VALID_EXCEPTION_START

		// Recording of inputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
		RECORD_DECORATED_INPUTS( bondObjectName, std::string(), bondObjectName, settlementDate, bondCurveName )

		const etrading::BondPtr bondObject      = etrading::getBond( bondObjectName );
		const etrading::BondCurvePtr bondCurve  = etrading::getBondCurve( bondCurveName );

		const double result = bondObject->yieldFromBondCurve( settlementDate, *bondCurve );

		// Recording of outputs for testing and playback - Note we decorate the file with the swap name prefix. The prefix is argument 1 and the suffix argument 2.
        RECORD_DECORATED_OUTPUTS_AND_RETURN_RESULT(bondObjectName, std::string(), result )

		VALID_EXCEPTION_END
	}

	/* @brief	Nelson-Siegel interpolation. Given a set of maturities, calculates the corresponding bond yields
	*  @param[in]	beta0						Long term yield
	*  @param[in]	beta1						Slope
	*  @param[in]	beta2						Curvature
	*  @param[in]	lambda						Time decay
	*  @param[in]	bondMaturities				A vector containing the bond maturities to interpolate
	*  @returns		A vector of interpolated bond yields, one yield per supplied maturity.
	*/
	DoubleVector tryMeBondCurveNelsonSiegelYield( const double& beta0,
												  const double& beta1,
												  const double& beta2,
												  const double& lambda,
												  const DoubleVector& bondMaturities )
	{
		VALID_EXCEPTION_START
	
		// Record Inputs for logs, tests and playback - Note the first argument is the decorated file prefix and the second the suffix
        RECORD_INPUTS( beta0, beta1, beta2, lambda, bondMaturities )

		etrading::NelsonSiegelSvenssonParameters parameters;
		parameters.beta0_ = beta0;
		parameters.beta1_ = beta1;
		parameters.beta2_ = beta2;
		parameters.lambda1_ = lambda;

		size_t nMaturities = bondMaturities.size();
		DoubleVector yields( nMaturities );

		for ( size_t i=0; i < nMaturities; i++ )
		{
			double bondMaturity = bondMaturities[i];
			yields[i] = etrading::NelsonSiegelInterpolator( parameters, bondMaturity );
		}

		// Record outputs for logs, tests and playback
		RECORD_OUTPUTS_AND_RETURN_RESULT( yields )

		VALID_EXCEPTION_END
	}

	/* @brief	Nelson-Siegel-Svensson interpolation. Given a set of maturities, calculates the corresponding bond yields
	*  @param[in]	beta0						Long term yield
	*  @param[in]	beta1						Slope
	*  @param[in]	beta2						Curvature
	*  @param[in]	beta3						Secondary curvature
	*  @param[in]	lambda1						Time decay
	*  @param[in]	lambda2						Time decay
	*  @param[in]	bondMaturities				A vector containing the bond maturities to interpolate
	*  @returns		A vector of interpolated bond yields, one yield per supplied maturity.
	*/
	DoubleVector tryMeBondCurveSvenssonYield( const double& beta0,
											  const double& beta1,
											  const double& beta2,
											  const double& beta3,
											  const double& lambda1,
											  const double& lambda2,
											  const DoubleVector& bondMaturities )
	{
		VALID_EXCEPTION_START
	
		// Record Inputs for logs, tests and playback - Note the first argument is the decorated file prefix and the second the suffix
        RECORD_INPUTS( beta0, beta1, beta2, beta3, lambda1, lambda2, bondMaturities )

		etrading::NelsonSiegelSvenssonParameters parameters;
		parameters.beta0_   = beta0;
		parameters.beta1_   = beta1;
		parameters.beta2_   = beta2;
		parameters.beta3_   = beta3;
		parameters.lambda1_ = lambda1;
		parameters.lambda2_ = lambda2;

		size_t nMaturities = bondMaturities.size();
		DoubleVector yields( nMaturities );

		for ( size_t i=0; i < nMaturities; i++ )
		{
			double bondMaturity = bondMaturities[i];
			yields[i] = etrading::SvenssonInterpolator( parameters, bondMaturity );
		}

		// Record outputs for logs, tests and playback
		RECORD_OUTPUTS_AND_RETURN_RESULT( yields )

		VALID_EXCEPTION_END
	}

	/* @brief Polynomial interpolation. Given a vector of polynomial coefficients and a bond maturity, interpolates the corresponding bond yield on the curve
	*  @param[in]	coefficients	Polynomial function coefficients: a_0 x^n + a_1 x^(n-1) + ... +a_n i.e. highest order coefficient is first in the vector.
	*  @param[in]	bondMaturities	A vector containing the bond maturities to interpolate
	*  @returns		The bond yield interpolated on the curve, corresponding to the supplied maturity
	*/
	DoubleVector tryMeBondCurvePolynomialYield( const DoubleVector& coefficients, const DoubleVector& bondMaturities )
	{
		VALID_EXCEPTION_START
	
		// Record Inputs for logs, tests and playback - Note the first argument is the decorated file prefix and the second the suffix
        RECORD_INPUTS( coefficients, bondMaturities )

		size_t nMaturities = bondMaturities.size();
		DoubleVector yields( nMaturities );

		for ( size_t i=0; i < nMaturities; i++ )
		{
			double bondMaturity = bondMaturities[i];
			yields[i] = etrading::PolynomialInterpolator( coefficients, bondMaturity );
		}

		// Record outputs for logs, tests and playback
		RECORD_OUTPUTS_AND_RETURN_RESULT( yields )

		VALID_EXCEPTION_END
	}


}