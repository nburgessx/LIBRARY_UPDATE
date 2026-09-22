// aqBondOptionObject.cpp

/*
 * @brief			Swig interface for aqBondOptionObject... functions
 */

#include "aqBondOptionObject.h"
#include "AQLCoreTemplateType.h"
#include "TypeUtilities.h"
#include "tryAqBondOptionObject.h"
#include "APISetUp.h"					// AQ_API_START and AQ_API_END Macros

/* @brief			Function to create and store a bond option from a label/value block
*  @param [in]		objectName			BondOption object name
*  @param [in]		optionLVB			Option definition as a key/value matrix
*  @param [in]		validateKeys		Optional. Default TRUE. Check the LVB keys
*  @return			BondOption object handle
*/
std::string aqBondOptionObjectCreate( const std::string& objectName,
                                       const SWIG_STRINGMATRIX& optionLVB,
                                       const bool validateKeys )
{
    AQ_API_START

    // Marshall Inputs
    AQLStringMatrix optionLVB_;
    swig::buildStringMatrix( optionLVB_, optionLVB );

    // Call Function and Return Result
    std::string result = validation::tryAqBondOptionObjectCreate( objectName, optionLVB_, validateKeys );
    return result;

    AQ_API_END
}

/* @brief			Function to display a cached bond option's input parameters
*  @param [in]		objectName			BondOption object name
*  @return			Display of the bond-option input parameters
*/
SWIG_STRINGMATRIX aqBondOptionObjectDisplay( const std::string& objectName )
{
    AQ_API_START

    // Call the Function
    AnyTypeMatrix display = validation::tryAqBondOptionObjectDisplay( objectName );

    // Marshall Output(s)
    SWIG_STRINGMATRIX result = swig::fromAnyTypeMatrixToMatrixOfString( display );
    return result;

    AQ_API_END
}

/* @brief			Function to calculate the PV of a cached bond option
*  @param [in]		objectName			BondOption object name
*  @param [in]		valuationSettings	Valuation settings as a key/value matrix
*  @param [in]		bondPrice			Bond spot price at settlement date
*  @param [in]		repoRate			Bond's repo rate
*  @param [in]		repoDaycount		Bond's repo daycount
*  @param [in]		discountRate		Continuously compounded risk free rate
*  @param [in]		discountDayCount	Day count for the discounting
*  @return			Present value of the bond option
*/
double aqBondOptionObjectPV( const std::string& objectName,
                              const SWIG_STRINGMATRIX& valuationSettings,
                              const double bondPrice,
                              const double repoRate,
                              const std::string& repoDaycount,
                              const double discountRate,
                              const std::string& discountDayCount )
{
    AQ_API_START

    // Marshall Inputs
    AQLStringMatrix valuationSettings_;
    swig::buildStringMatrix( valuationSettings_, valuationSettings );

    // Call Function and Return Result
    double result = validation::tryAqBondOptionObjectPV( objectName,
                                                           valuationSettings_,
                                                           bondPrice,
                                                           repoRate,
                                                           repoDaycount,
                                                           discountRate,
                                                           discountDayCount );
    return result;

    AQ_API_END
}

/* @brief			Function to calculate the Greeks of a cached bond option, by bump-and-revalue
*  @param [in]		greekType			Greek type: Analytical or Numerical
*  @param [in]		objectName			BondOption object name
*  @param [in]		valuationSettings	Valuation settings as a key/value matrix
*  @param [in]		bondSpotPrice		Bond spot price at settlement date
*  @param [in]		repoRate			Bond's repo rate
*  @param [in]		repoDaycount		Bond's repo daycount
*  @param [in]		discountRate		Continuously compounded risk free rate
*  @param [in]		discountDayCount	Day count for the discounting
*  @param [in]		deltaBump			Delta bump size
*  @param [in]		gammaBump			Gamma bump size
*  @param [in]		vegaBump			Vega bump size
*  @param [in]		thetaBump			Theta bump size
*  @param [in]		rhoBump				Rho bump size
*  @param [in]		showColumnHeaders	Optional. Default TRUE. Include a header row
*  @return			Bond option Greeks' display
*/
SWIG_STRINGMATRIX aqBondOptionObjectGreeks( const std::string& greekType,
                                             const std::string& objectName,
                                             const SWIG_STRINGMATRIX& valuationSettings,
                                             const double bondSpotPrice,
                                             const double repoRate,
                                             const std::string& repoDaycount,
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
    AnyTypeMatrix greeks = validation::tryAqBondOptionObjectGreeks( greekType,
                                                                     objectName,
                                                                     valuationSettings_,
                                                                     bondSpotPrice,
                                                                     repoRate,
                                                                     repoDaycount,
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
