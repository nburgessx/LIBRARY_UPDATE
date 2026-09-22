// aqVolatilitySABR.cpp

/*
 * @brief			Swig interface for aqVolatilitySABR... functions
 */

#include "aqVolatilitySABR.h"
#include "AQLCoreTemplateType.h"
#include "TypeUtilities.h"          // Swig Marshalling Helper Methods
#include "ExceptionMacros.h"
#include "tryAqVolatilitySABRCalibrate.h"
#include "tryAqVolatilitySABRGetPrem.h"
#include "tryAqVolatilitySABRGetVol.h"
#include "tryAqVolatilitySABROutputParameter.h"
#include "tryAqVolatilitySABRSetupConvention.h"
#include "tryAqVolatilitySABRSetupParameter.h"
#include "tryAqVolatilitySABRSetupSwaptionVol.h"
#include "APISetUp.h"					// AQ_API_START and AQ_API_END Macros

/* @brief			Function to calibrate a SABR volatility matrix (legacy procedural, ID-string driven)
*  @return			Confirmation message
*/
std::string aqVolatilitySABRCalibrate( const std::string& approxMethod,
                                        const std::vector<std::string>& calibFlg,
                                        const std::string& calibMethod,
                                        const std::string& curveSetID,
                                        const std::string& alphaID,
                                        const std::string& betaID,
                                        const std::string& nuID,
                                        const std::string& rhoID,
                                        const std::string& convID,
                                        const std::string& capConvID,
                                        const std::vector<std::string>& swapVolID,
                                        const std::string& target,
                                        const std::vector<double>& weight,
                                        const std::vector<int>& sgn,
                                        const std::string& forwardID,
                                        const double forwardShiftValue,
                                        const std::string& numeraireID,
                                        const SWIG_STRINGMATRIX& curveMat,
                                        const std::string& volType )
{
    AQ_API_START

    // Marshall Inputs
    AQLStringVector calibFlg_;
    swig::buildStringVector( calibFlg_, calibFlg );

    AQLStringVector swapVolID_;
    swig::buildStringVector( swapVolID_, swapVolID );

    AQLStringMatrix curveMat_;
    swig::buildStringMatrix( curveMat_, curveMat );

    // Call Function and Return Result
    std::string result = validation::tryAqVolatilitySABRCalibrate(
        approxMethod, calibFlg_, calibMethod, curveSetID, alphaID, betaID, nuID, rhoID,
        convID, capConvID, swapVolID_, target, weight, sgn, forwardID, forwardShiftValue,
        numeraireID, curveMat_, volType ).getCString();
    return result;

    AQ_API_END
}

/* @brief			Function to calculate a SABR-implied premium (legacy procedural, ID-string driven)
*  @return			SABR-implied premium at the expiry/tenor/strike point
*/
double aqVolatilitySABRGetPrem( const std::string& expPoint,
                                 const std::string& tenorPoint,
                                 const double strike,
                                 const int sign,
                                 const std::string& forwardID,
                                 const std::string& numeraireID,
                                 const std::string& alphaID,
                                 const std::string& betaID,
                                 const std::string& nuID,
                                 const std::string& rhoID,
                                 const std::string& approxMethod,
                                 const double shift,
                                 const std::string& volType )
{
    AQ_API_START

    // Call Function and Return Result
    double result = validation::tryAqVolatilitySABRGetPrem(
        expPoint, tenorPoint, strike, sign, forwardID, numeraireID, alphaID, betaID, nuID,
        rhoID, approxMethod, shift, volType );
    return result;

    AQ_API_END
}

/* @brief			Function to calculate a SABR-implied volatility (legacy procedural, ID-string driven)
*  @return			SABR-implied volatility at the expiry/tenor/strike point
*/
double aqVolatilitySABRGetVol( const std::string& expPoint,
                                const std::string& tenorPoint,
                                const double strike,
                                const std::string& forwardID,
                                const std::string& alphaID,
                                const std::string& betaID,
                                const std::string& nuID,
                                const std::string& rhoID,
                                const std::string& approxMethod,
                                const double shift,
                                const std::string& volType )
{
    AQ_API_START

    // Call Function and Return Result
    double result = validation::tryAqVolatilitySABRGetVol(
        expPoint, tenorPoint, strike, forwardID, alphaID, betaID, nuID, rhoID,
        approxMethod, shift, volType );
    return result;

    AQ_API_END
}

/* @brief			Function to return a stored SABR grid as a matrix (legacy procedural, ID-string driven)
*  @return			The stored SABR grid, as a string matrix (row x column)
*/
SWIG_STRINGMATRIX aqVolatilitySABROutputParameter( const std::string& gridID )
{
    AQ_API_START

    // Call the Function
    DoubleVector ret;
    size_t row = 0;
    size_t column = 0;
    validation::tryAqVolatilitySABROutputParameter( gridID, ret, row, column );

    // ret is flattened row-major (see AQLMathSwaptionVolUtility::outPutSABRGrid) -
    // reshape it back into a row x column matrix, mirroring how AQ_XLL's
    // xllVolatility.cpp does the same reshape for the worksheet function.
    AQLStringMatrix matrix( row, AQLStringVector( column ) );
    for ( size_t r = 0; r < row; ++r )
    {
        for ( size_t c = 0; c < column; ++c )
        {
            matrix[r][c] = AQLString( std::to_string( ret[r * column + c] ).c_str() );
        }
    }

    // Marshall Output to Standard String Matrix
    SWIG_STRINGMATRIX resultsStringMatrix = swig::fromStringMatrixToMatrixOfString( matrix );
    return resultsStringMatrix;

    AQ_API_END
}

/* @brief			Function to store SABR calibration convention data under an ID (legacy procedural, ID-string driven)
*  @return			TRUE on success
*/
bool aqVolatilitySABRSetupConvention( const std::string& conventionID,
                                       const SWIG_STRINGMATRIX& convData )
{
    AQ_API_START

    // Marshall Inputs
    AQLStringMatrix convData_;
    swig::buildStringMatrix( convData_, convData );

    // Call Function. tryAqVolatilitySABRSetupConvention is void - convData is
    // read by the function, not written back for the caller's benefit (as
    // AQ_XLL's xllVolatility.cpp notes for the same call) - TRUE confirms the
    // setup succeeded.
    validation::tryAqVolatilitySABRSetupConvention( conventionID, convData_ );
    return true;

    AQ_API_END
}

/* @brief			Function to store a SABR parameter grid under an ID (legacy procedural, ID-string driven)
*  @return			TRUE on success
*/
bool aqVolatilitySABRSetupParameter( const std::string& gridID,
                                      const std::string& conventionID,
                                      const SWIG_STRINGMATRIX& gridData )
{
    AQ_API_START

    // Marshall Inputs
    AQLStringMatrix gridData_;
    swig::buildStringMatrix( gridData_, gridData );

    // Call Function. As SetupConvention above: gridData is read, not written
    // back; TRUE confirms the setup succeeded.
    validation::tryAqVolatilitySABRSetupParameter( gridID, conventionID, gridData_ );
    return true;

    AQ_API_END
}

/* @brief			Function to store a swaption vol/strike/sign matrix set under an ID (legacy procedural, ID-string driven)
*  @return			TRUE on success
*/
bool aqVolatilitySABRSetupSwaptionVol( const std::string& gridID,
                                        const SWIG_STRINGMATRIX& volMat,
                                        const SWIG_STRINGMATRIX& strikeMat,
                                        const SWIG_STRINGMATRIX& signMat )
{
    AQ_API_START

    // Marshall Inputs
    AQLStringMatrix volMat_;
    swig::buildStringMatrix( volMat_, volMat );

    AQLStringMatrix strikeMat_;
    swig::buildStringMatrix( strikeMat_, strikeMat );

    AQLStringMatrix signMat_;
    swig::buildStringMatrix( signMat_, signMat );

    // Call Function. The validation header marks volMat/strikeMat/signMat
    // [inout], but the implementation only reads them (see AQ_XLL's
    // xllVolatility.cpp note on the same call) - nothing is written back
    // that the caller needs, so TRUE confirms the setup succeeded.
    validation::tryAqVolatilitySABRSetupSwaptionVol( gridID, volMat_, strikeMat_, signMat_ );
    return true;

    AQ_API_END
}
