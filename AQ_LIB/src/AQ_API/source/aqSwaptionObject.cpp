// aqSwaptionObject.cpp

/*
 * @brief			Swig interface for aqSwaptionObject... functions
 */

#include "aqSwaptionObject.h"
#include "AQLCoreTemplateType.h"
#include "TypeUtilities.h"
#include "tryAqSwaptionObject.h"
#include "APISetUp.h"					// AQ_API_START and AQ_API_END Macros

/* @brief			Function to create and store a swaption from a label/value block
*  @param [in]		objectName			Swaption object name
*  @param [in]		tradeLVB			Swaption definition as a key/value matrix
*  @param [in]		validateKeys		Optional. Default TRUE. Check the LVB keys
*  @return			Swaption object handle
*/
std::string aqSwaptionObjectCreate( const std::string& objectName,
                                     const SWIG_STRINGMATRIX& tradeLVB,
                                     const bool validateKeys )
{
    AQ_API_START

    // Marshall Inputs
    AQLStringMatrix tradeLVB_;
    swig::buildStringMatrix( tradeLVB_, tradeLVB );
    LabelValueBlock tradeLVBAsLabelValueBlock( tradeLVB_ );

    // Call Function and Return Result
    std::string result = validation::tryAqSwaptionObjectCreate( objectName, tradeLVBAsLabelValueBlock, validateKeys );
    return result;

    AQ_API_END
}

/* @brief			Function to display a cached swaption's input parameters
*  @param [in]		objectName			Swaption object name
*  @return			Display of the swaption input parameters
*/
SWIG_STRINGMATRIX aqSwaptionObjectDisplay( const std::string& objectName )
{
    AQ_API_START

    // Call the Function
    AnyTypeMatrix display = validation::tryAqSwaptionObjectDisplay( objectName );

    // Marshall Output(s)
    SWIG_STRINGMATRIX result = swig::fromAnyTypeMatrixToMatrixOfString( display );
    return result;

    AQ_API_END
}

/* @brief			Function to calculate the PV of a cached swaption
*  @param [in]		objectName			Swaption object name
*  @param [in]		valuationSettingsLVB	Valuation settings as a key/value matrix
*  @return			The swaption PV
*/
double aqSwaptionObjectPV( const std::string& objectName,
                           const SWIG_STRINGMATRIX& valuationSettingsLVB )
{
    AQ_API_START

    // Marshall Inputs
    AQLStringMatrix valuationSettingsLVB_;
    swig::buildStringMatrix( valuationSettingsLVB_, valuationSettingsLVB );

    // Call Function and Return Result
    double result = validation::tryAqSwaptionObjectPV( objectName, valuationSettingsLVB_ );
    return result;

    AQ_API_END
}

/* @brief			Function to calculate the implied volatility from a cached swaption's price
*  @param [in]		objectName			Swaption object name
*  @param [in]		price				Swaption price
*  @param [in]		valuationSettingsLVB	Valuation settings as a key/value matrix
*  @return			The swaption implied vol
*/
double aqSwaptionObjectImpliedVol( const std::string& objectName,
                                    const double price,
                                    const SWIG_STRINGMATRIX& valuationSettingsLVB )
{
    AQ_API_START

    // Marshall Inputs
    AQLStringMatrix valuationSettingsLVB_;
    swig::buildStringMatrix( valuationSettingsLVB_, valuationSettingsLVB );

    // Call Function and Return Result
    double result = validation::tryAqSwaptionObjectImpliedVol( objectName, price, valuationSettingsLVB_ );
    return result;

    AQ_API_END
}

/* @brief			Function to calculate the Delta of a cached swaption
*  @param [in]		objectName			Swaption object name
*  @param [in]		valuationSettingsLVB	Valuation settings as a key/value matrix
*  @return			The swaption Delta
*/
double aqSwaptionObjectDelta( const std::string& objectName,
                               const SWIG_STRINGMATRIX& valuationSettingsLVB )
{
    AQ_API_START

    // Marshall Inputs
    AQLStringMatrix valuationSettingsLVB_;
    swig::buildStringMatrix( valuationSettingsLVB_, valuationSettingsLVB );

    // Call Function and Return Result
    double result = validation::tryAqSwaptionObjectDelta( objectName, valuationSettingsLVB_ );
    return result;

    AQ_API_END
}

/* @brief			Function to calculate the Gamma of a cached swaption
*  @param [in]		objectName			Swaption object name
*  @param [in]		valuationSettingsLVB	Valuation settings as a key/value matrix
*  @return			The swaption Gamma
*/
double aqSwaptionObjectGamma( const std::string& objectName,
                               const SWIG_STRINGMATRIX& valuationSettingsLVB )
{
    AQ_API_START

    // Marshall Inputs
    AQLStringMatrix valuationSettingsLVB_;
    swig::buildStringMatrix( valuationSettingsLVB_, valuationSettingsLVB );

    // Call Function and Return Result
    double result = validation::tryAqSwaptionObjectGamma( objectName, valuationSettingsLVB_ );
    return result;

    AQ_API_END
}

/* @brief			Function to calculate the Vega of a cached swaption
*  @param [in]		objectName			Swaption object name
*  @param [in]		valuationSettingsLVB	Valuation settings as a key/value matrix
*  @return			The swaption Vega
*/
double aqSwaptionObjectVega( const std::string& objectName,
                              const SWIG_STRINGMATRIX& valuationSettingsLVB )
{
    AQ_API_START

    // Marshall Inputs
    AQLStringMatrix valuationSettingsLVB_;
    swig::buildStringMatrix( valuationSettingsLVB_, valuationSettingsLVB );

    // Call Function and Return Result
    double result = validation::tryAqSwaptionObjectVega( objectName, valuationSettingsLVB_ );
    return result;

    AQ_API_END
}

/* @brief			Function to calculate the Theta of a cached swaption
*  @param [in]		objectName			Swaption object name
*  @param [in]		valuationSettingsLVB	Valuation settings as a key/value matrix
*  @return			The swaption Theta
*/
double aqSwaptionObjectTheta( const std::string& objectName,
                               const SWIG_STRINGMATRIX& valuationSettingsLVB )
{
    AQ_API_START

    // Marshall Inputs
    AQLStringMatrix valuationSettingsLVB_;
    swig::buildStringMatrix( valuationSettingsLVB_, valuationSettingsLVB );

    // Call Function and Return Result
    double result = validation::tryAqSwaptionObjectTheta( objectName, valuationSettingsLVB_ );
    return result;

    AQ_API_END
}
