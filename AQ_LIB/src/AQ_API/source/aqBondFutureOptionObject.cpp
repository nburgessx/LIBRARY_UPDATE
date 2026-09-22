// aqBondFutureOptionObject.cpp

/*
 * @brief			Swig interface for aqBondFutureOptionObject... functions
 */

#include "aqBondFutureOptionObject.h"
#include "AQLCoreTemplateType.h"
#include "TypeUtilities.h"
#include "tryAqBondOptionObject.h"
#include "APISetUp.h"					// AQ_API_START and AQ_API_END Macros

/* @brief			Function to calculate the PV of a cached bond-future option
*  @param [in]		objectName			BondOption object name
*  @param [in]		valuationSettings	Valuation settings as a key/value matrix
*  @param [in]		bondFuturePrice		Bond forward price at option expiry date
*  @param [in]		discountRate		Continuously compounded risk free rate
*  @param [in]		discountDayCount	Day count for the discounting
*  @return			Present value of the bond-future option
*/
double aqBondFutureOptionObjectPV( const std::string& objectName,
                                    const SWIG_STRINGMATRIX& valuationSettings,
                                    const double bondFuturePrice,
                                    const double discountRate,
                                    const std::string& discountDayCount )
{
    AQ_API_START

    // Marshall Inputs
    AQLStringMatrix valuationSettings_;
    swig::buildStringMatrix( valuationSettings_, valuationSettings );

    // Call Function and Return Result
    double result = validation::tryAqBondFutureOptionObjectPV( objectName,
                                                                 valuationSettings_,
                                                                 bondFuturePrice,
                                                                 discountRate,
                                                                 discountDayCount );
    return result;

    AQ_API_END
}

/* @brief			Function to calculate the Greeks of a cached bond-future option, by bump-and-revalue
*  @param [in]		greekType			Greek type: Analytical or Numerical
*  @param [in]		objectName			BondOption object name
*  @param [in]		valuationSettings	Valuation settings as a key/value matrix
*  @param [in]		bondFuturePrice		Bond forward price at option expiry date
*  @param [in]		discountRate		Continuously compounded risk free rate
*  @param [in]		discountDayCount	Day count for the discounting
*  @param [in]		deltaBump			Delta bump size
*  @param [in]		gammaBump			Gamma bump size
*  @param [in]		vegaBump			Vega bump size
*  @param [in]		thetaBump			Theta bump size
*  @param [in]		rhoBump				Rho bump size
*  @param [in]		showColumnHeaders	Optional. Default TRUE. Include a header row
*  @return			Bond-future option Greeks' display
*/
SWIG_STRINGMATRIX aqBondFutureOptionObjectGreeks( const std::string& greekType,
                                                   const std::string& objectName,
                                                   const SWIG_STRINGMATRIX& valuationSettings,
                                                   const double bondFuturePrice,
                                                   const double discountRate,
                                                   const std::string& discountDayCount,
                                                   const double deltaBump,
                                                   const double gammaBump,
                                                   const double vegaBump,
                                                   const double thetaBump,
                                                   const double rhoBump,
                                                   const bool showColumnHeaders )
{
    AQ_API_START

    // Marshall Inputs
    AQLStringMatrix valuationSettings_;
    swig::buildStringMatrix( valuationSettings_, valuationSettings );

    // Call the Function
    AnyTypeMatrix greeks = validation::tryAqBondFutureOptionObjectGreeks( greekType,
                                                                           objectName,
                                                                           valuationSettings_,
                                                                           bondFuturePrice,
                                                                           discountRate,
                                                                           discountDayCount,
                                                                           deltaBump,
                                                                           gammaBump,
                                                                           vegaBump,
                                                                           thetaBump,
                                                                           rhoBump,
                                                                           showColumnHeaders );

    // Marshall Output(s)
    SWIG_STRINGMATRIX result = swig::fromAnyTypeMatrixToMatrixOfString( greeks );
    return result;

    AQ_API_END
}
