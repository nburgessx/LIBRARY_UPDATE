// aqBondsCreate.cpp

/*
 * @brief			Swig Interface file for Bond Curve Fitting
 *					This is used to fit Bond Prices to a curve
 */

#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros
#include "TypeUtilities.h"          // Swig Marshalling Helper Methods
#include "aqBondCurve.h"
#include "BondUtilities.h"
#include "tryAqBondObject.h"
#include "ParameterValidation.h"
#include "JSONInfoBlock.h"          // JSON InfoBlock Container - tryAqBondCurveCreate


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
swig::NelsonSiegelSvenssonCalibrationResults aqBondCurveNelsonSiegelCalibrate( const std::vector<double>& bondMaturities,
																	  const std::vector<double>& bondYields,
																	  const swig::NelsonSiegelSvenssonParameters& initialGuess,
																	  const unsigned int& maxIterations,
																	  const unsigned int& maxStationaryStateIterations,
																	  const std::vector<double>& lowerBounds,
																	  const std::vector<double>& upperBounds )
{
	AQ_API_START

    // Call validation method and marshall swig inputs
    etrading::NelsonSiegelSvenssonCalibrationResults nsResult = validation::tryAqBondCurveNelsonSiegelCalibrate( bondMaturities,
                                                                                                                        bondYields,
                                                                                                                        swig::fromSwigNelsonSiegelSvenssonParameters( initialGuess ),
                                                                                                                        maxIterations,
                                                                                                                        maxStationaryStateIterations,
                                                                                                                        lowerBounds,
                                                                                                                        upperBounds );
    // Marshall Outputs
    return swig::toSwigNelsonSiegelSvenssonCalibrationResults( nsResult );
    
    AQ_API_END
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
swig::NelsonSiegelSvenssonCalibrationResults aqBondCurveSvenssonCalibrate( const std::vector<double>& bondMaturities,
															      const std::vector<double>& bondYields,
															      const swig::NelsonSiegelSvenssonParameters& initialGuess,
															      const unsigned int& maxIterations,
															      const unsigned int& maxStationaryStateIterations,
															      const std::vector<double>& lowerBounds,
															      const std::vector<double>& upperBounds )
{
	AQ_API_START

    // Call validation method and marshall swig inputs
    etrading::NelsonSiegelSvenssonCalibrationResults sResult = validation::tryAqBondCurveSvenssonCalibrate( bondMaturities,
                                                                                                                bondYields,
                                                                                                                swig::fromSwigNelsonSiegelSvenssonParameters( initialGuess ), 
                                                                                                                maxIterations,
                                                                                                                maxStationaryStateIterations,
                                                                                                                lowerBounds,
                                                                                                                upperBounds );
    // Marshall Outputs
    return swig::toSwigNelsonSiegelSvenssonCalibrationResults( sResult );

    AQ_API_END 
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
swig::PolynomialCalibrationResults aqBondCurvePolynomialCalibrate( const unsigned int polynomialOrder,
																  const std::vector<double>& bondMaturities,
																  const std::vector<double>& bondYields,
																  const unsigned int& maxIterations,
																  const unsigned int maxStationaryStateIterations,
																  const double lowerBound,
																  const double upperBound )
{
    AQ_API_START

    /// Call validation method
    etrading::PolynomialCalibrationResults nsResult = validation::tryAqBondCurvePolynomialCalibrate( polynomialOrder,
                                                                                                         bondMaturities,
                                                                                                         bondYields,
                                                                                                         maxIterations,
                                                                                                         maxStationaryStateIterations,
                                                                                                         lowerBound,
                                                                                                         upperBound );
    // Marshall Outputs
    return swig::toSwigPolynomialCalibrationResults( nsResult );
	    

    // Call validation method
    swig::PolynomialCalibrationResults result;
    return result;
    
    AQ_API_END
}

/* @brief	Nelson-Siegel interpolation. Given a set of maturities, calculates the corresponding bond yields
*  @param[in]	beta0						Long term yield
*  @param[in]	beta1						Slope
*  @param[in]	beta2						Curvature
*  @param[in]	lambda						Time decay
*  @param[in]	bondMaturities				A vector containing the bond maturities to interpolate
*  @returns		A vector of interpolated bond yields, one yield per supplied maturity.
*/
std::vector<double> aqBondCurveNelsonSiegelYield( const double& beta0,
										          const double& beta1,
										          const double& beta2,
										          const double& lambda,
										          const std::vector<double>& bondMaturities )
{
    AQ_API_START

    // Call validation method
    std::vector<double> result = validation::tryAqBondCurveNelsonSiegelYield( beta0, beta1, beta2, lambda, bondMaturities );
    return result;
    
    AQ_API_END	
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
std::vector<double> aqBondCurveSvenssonYield( const double& beta0,
									          const double& beta1,
									          const double& beta2,
									          const double& beta3,
									          const double& lambda1,
									          const double& lambda2,
									          const std::vector<double>& bondMaturities )
{
    AQ_API_START

    // Call validation method
    std::vector<double> result = validation::tryAqBondCurveSvenssonYield( beta0, beta1, beta2, beta3, lambda1, lambda2, bondMaturities );
    return result;
    
    AQ_API_END
}

/* @brief Polynomial interpolation. Given a vector of polynomial coefficients and a bond maturity, interpolates the corresponding bond yield on the curve
*  @param[in]	coefficients	Polynomial function coefficients: a_0 x^n + a_1 x^(n-1) + ... +a_n i.e. highest order coefficient is first in the vector.
*  @param[in]	bondMaturities	A vector containing the bond maturities to interpolate
*  @returns		The bond yield interpolated on the curve, corresponding to the supplied maturity
*/
std::vector<double> aqBondCurvePolynomialYield( const std::vector<double>& coefficients, const std::vector<double>& bondMaturities )
{
    AQ_API_START

    // Call validation method
    std::vector<double> result = validation::tryAqBondCurvePolynomialYield( coefficients, bondMaturities );
    return result;
    
    AQ_API_END	
}

/* @brief Function to display bond cashflows
*  @param[in]   bondObjectName
*  @param[in]   settlementDate
*  @param[in]   yield
*  @param[in]   yieldCalculationType
*  @param[in]   showColumnHeaders
*  @returns		Cashflow Display Information
*/
SWIG_STRINGMATRIX aqBondObjectDisplayCashflows( const std::string& bondObjectName, const std::string& settlementDate, const double& yield, const std::string& yieldCalculationType, const bool& showColumnHeaders )
{
    AQ_API_START

	SWIG_STRINGMATRIX result =  aqBondObjectDisplayCashflows( bondObjectName, settlementDate, yield, yieldCalculationType, showColumnHeaders, std::vector<std::string>() ); // columnList = Empty
	return result;

    AQ_API_END
}

/* @brief Function to display bond cashflows
*  @param[in]   bondObjectName
*  @param[in]   settlementDate
*  @param[in]   yield
*  @param[in]   yieldCalculationType
*  @param[in]   showColumnHeaders
*  @param[in]   columnList
*  @returns		Cashflow Display Information
*/
SWIG_STRINGMATRIX aqBondObjectDisplayCashflows( const std::string& bondObjectName, const std::string& settlementDate, const double& yield, const std::string& yieldCalculationType, const bool& showColumnHeaders, const std::vector<std::string>& columnList )
{
    AQ_API_START

    // Marshall Inputs
    AQLDate settlementDate_( etrading::stringToDate( settlementDate ) );
    
    // Call the Function
    AnyTypeMatrix cashflows = validation::tryAqBondObjectDisplayCashflows( bondObjectName, settlementDate_, yield, yieldCalculationType, showColumnHeaders, columnList );
            
    // Marshall Output(s)
	SWIG_STRINGMATRIX result = swig::fromAnyTypeMatrixToMatrixOfString( cashflows );
	return result;

    AQ_API_END
}

/* @brief Function to display bond cashflows
*  @param[in]   bondObjectName
*  @param[in]   showColumnHeaders
*  @param[in]   columnList
*  @returns		Cashflow Display Information
*/
SWIG_STRINGMATRIX aqBondObjectDisplaySchedule( const std::string& bondObjectName, const bool& showColumnHeaders )
{
    AQ_API_START

	SWIG_STRINGMATRIX result = aqBondObjectDisplaySchedule( bondObjectName, showColumnHeaders, std::vector<std::string>() ); // columnList = Empty
	return result;

    AQ_API_END
}


/* @brief Function to display bond cashflows
*  @param[in]   bondObjectName
*  @param[in]   showColumnHeaders
*  @param[in]   columnList
*  @returns		Cashflow Display Information
*/
SWIG_STRINGMATRIX aqBondObjectDisplaySchedule( const std::string& bondObjectName, const bool& showColumnHeaders, const std::vector<std::string>& columnList )
{
    AQ_API_START

    // Marshall Inputs

    // Call the Function
    AnyTypeMatrix schedule = validation::tryAqBondObjectDisplaySchedule( bondObjectName, showColumnHeaders, columnList );
            
    // Marshall Output(s)
	SWIG_STRINGMATRIX result = swig::fromAnyTypeMatrixToMatrixOfString( schedule );
	return result;

    AQ_API_END
}

/* @brief			swig interface for aqBondCurveCreate function. Creates a fitted bond curve from one or two named data blocks.
*  @param[in]		bondCurveName	Bond Curve object name
*  @param[in]		key1			Name of the first data block
*  @param[in]		dataBlock1		First data block
*  @param[in]		key2			Optional. Name of the second data block; pass empty string / empty matrix to omit
*  @param[in]		dataBlock2		Optional. Second data block
*  @returns		Returns the bond curve object handle name
*/
std::string aqBondCurveCreate( const std::string& bondCurveName,
                                const std::string& key1,
                                const std::vector<std::vector<std::string> >& dataBlock1,
                                const std::string& key2,
                                const std::vector<std::vector<std::string> >& dataBlock2 )
{
    AQ_API_START

    // Marshall Inputs - mirrors aqCreditModelCreate's pattern
    etrading::VariantMatrix variantDataBlock1;
    swig::buildVariantMatrix( variantDataBlock1, dataBlock1 );

    etrading::VariantMatrix variantDataBlock2;
    swig::buildVariantMatrix( variantDataBlock2, dataBlock2 );

    const etrading::JSONInfoBlockTuple infoBlock1  = etrading::JSONInfoBlock::createInfoBlock( variantDataBlock1 );
    const etrading::JSONInfoBlockTuple infoBlock2  = etrading::JSONInfoBlock::createInfoBlock( variantDataBlock2 );
    const etrading::JSONInfoBlockTuples infoBlocks = { infoBlock1, infoBlock2 };

    const std::vector<std::string> dataBlockNames = { key1, key2 };

    // Call validation method
    std::string result = validation::tryAqBondCurveCreate( bondCurveName, dataBlockNames, infoBlocks );
    return result;

    AQ_API_END
}

/* @brief			swig interface for aqBondCurveDisplay function. Displays the bond curve calibration as a matrix of pillar dates and yield points.
*  @param[in]		bondCurveName	The bond curve object name
*  @returns		A matrix of pillar dates and yield points
*/
SWIG_STRINGMATRIX aqBondCurveDisplay( const std::string& bondCurveName )
{
    AQ_API_START

    // Call the Function
    AnyTypeMatrix display = validation::tryAqBondCurveDisplay( bondCurveName );

    // Marshall Output(s)
    SWIG_STRINGMATRIX result = swig::fromAnyTypeMatrixToMatrixOfString( display );
    return result;

    AQ_API_END
}

/* @brief			swig interface for aqBondCurveYield function. Interpolated yield off a bond curve at a reference date.
*  @param[in]		bondCurveName	The bond curve object name
*  @param[in]		referenceDate	The forward reference date
*  @returns		The interpolated yield
*/
double aqBondCurveYield( const std::string& bondCurveName, const std::string& referenceDate )
{
    AQ_API_START

    // Marshall Inputs
    AQLDate referenceDate_( etrading::stringToDate( referenceDate ) );

    // Call validation method
    double result = validation::tryAqBondCurveYield( bondCurveName, referenceDate_ );
    return result;

    AQ_API_END
}

/* @brief			swig interface for aqBondObjectPriceFromBondCurve function. Prices a bond using a bond curve to discount the coupons.
*  @param[in]		bondObjectName	Bond object name
*  @param[in]		settlementDate	The settlement date to use for bond pricing
*  @param[in]		bondCurveName	The bond curve object name
*  @returns		The bond price
*/
double aqBondObjectPriceFromBondCurve( const std::string& bondObjectName, const std::string& settlementDate, const std::string& bondCurveName )
{
    AQ_API_START

    // Marshall Inputs
    AQLDate settlementDate_( etrading::stringToDate( settlementDate ) );

    // Call validation method
    double result = validation::tryAqBondObjectPriceFromBondCurve( bondObjectName, settlementDate_, bondCurveName );
    return result;

    AQ_API_END
}

/* @brief			swig interface for aqBondObjectYieldFromBondCurve function. Calculates the yield-to-maturity of a bond using a bond curve to discount the coupons.
*  @param[in]		bondObjectName	Bond object name
*  @param[in]		settlementDate	The settlement date to use for bond pricing
*  @param[in]		bondCurveName	The bond curve object name
*  @returns		The bond yield-to-maturity
*/
double aqBondObjectYieldFromBondCurve( const std::string& bondObjectName, const std::string& settlementDate, const std::string& bondCurveName )
{
    AQ_API_START

    // Marshall Inputs
    AQLDate settlementDate_( etrading::stringToDate( settlementDate ) );

    // Call validation method
    double result = validation::tryAqBondObjectYieldFromBondCurve( bondObjectName, settlementDate_, bondCurveName );
    return result;

    AQ_API_END
}
