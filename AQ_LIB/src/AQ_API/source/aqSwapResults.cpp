// aqSwapResults.cpp

/*
 * @brief			Swig interface for aqSwapResults... functions (the Jacobian risk store)
 */

#include "aqSwapResults.h"
#include "AQLCoreTemplateType.h"
#include "TypeUtilities.h"
#include "tryAqSwapResults.h"
#include "ParameterValidation.h"    // etrading::stringToDate
#include "CoreEnumerations.h"       // etrading::toRiskTypeEnum
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros

/* @brief			swig interface for the aqSwapResultsEnable function
*  @param [in]		enable		TRUE to enable, FALSE to disable
*  @return			Status string
*/
std::string aqSwapResultsEnable( bool enable )
{
    AQ_API_START

    std::string result = validation::tryAqSwapResultsEnable( enable );
    return result;

    AQ_API_END
}

/* @brief			swig interface for the aqSwapResultsIsEnabled function
*  @return			Whether the swap results (Jacobian risk) store is currently enabled
*/
std::string aqSwapResultsIsEnabled()
{
    AQ_API_START

    std::string result = validation::tryAqSwapResultsIsEnabled();
    return result;

    AQ_API_END
}

/* @brief			swig interface for the aqSwapResultsRiskUpdate function
*  @param [in]		swapHandle			A cached swap handle
*  @param [in]		asOfDate			The as-of date for the risk being stored (YYYYMMDD)
*  @param [in]		discountRiskLVB		Table of payment dates and coupons
*  @param [in]		forwardRiskLVB		Table of fixing dates and annuities
*  @return			Status string
*/
std::string aqSwapResultsRiskUpdate( const std::string& swapHandle,
                                  const std::string& asOfDate,
                                  const SWIG_STRINGMATRIX & discountRiskLVB,
                                  const SWIG_STRINGMATRIX & forwardRiskLVB )
{
    AQ_API_START

    AQLDate asOfDateAsAQLDate( etrading::stringToDate( asOfDate ) );

    AQLStringMatrix discountRiskAsStringMatrix;
    swig::buildStringMatrix( discountRiskAsStringMatrix, discountRiskLVB );

    AQLStringMatrix forwardRiskAsStringMatrix;
    swig::buildStringMatrix( forwardRiskAsStringMatrix, forwardRiskLVB );

    std::string result = validation::tryAqSwapResultsRiskUpdate( swapHandle, asOfDateAsAQLDate, discountRiskAsStringMatrix, forwardRiskAsStringMatrix );
    return result;

    AQ_API_END
}

/* @brief			swig interface for the aqSwapResultsDiscountRiskUpdate function
*  @param [in]		swapHandle			A cached swap handle
*  @param [in]		asOfDate			The as-of date for the risk being stored (YYYYMMDD)
*  @param [in]		discountRiskLVB		Table of payment dates and coupons
*  @return			Status string
*/
std::string aqSwapResultsDiscountRiskUpdate( const std::string& swapHandle,
                                          const std::string& asOfDate,
                                          const SWIG_STRINGMATRIX & discountRiskLVB )
{
    AQ_API_START

    AQLDate asOfDateAsAQLDate( etrading::stringToDate( asOfDate ) );

    AQLStringMatrix discountRiskAsStringMatrix;
    swig::buildStringMatrix( discountRiskAsStringMatrix, discountRiskLVB );

    std::string result = validation::tryAqSwapResultsDiscountRiskUpdate( swapHandle, asOfDateAsAQLDate, discountRiskAsStringMatrix );
    return result;

    AQ_API_END
}

/* @brief			swig interface for the aqSwapResultsForwardRiskUpdate function
*  @param [in]		swapHandle			A cached swap handle
*  @param [in]		asOfDate			The as-of date for the risk being stored (YYYYMMDD)
*  @param [in]		forwardRiskLVB		Table of fixing dates and annuities
*  @return			Status string
*/
std::string aqSwapResultsForwardRiskUpdate( const std::string& swapHandle,
                                         const std::string& asOfDate,
                                         const SWIG_STRINGMATRIX & forwardRiskLVB )
{
    AQ_API_START

    AQLDate asOfDateAsAQLDate( etrading::stringToDate( asOfDate ) );

    AQLStringMatrix forwardRiskAsStringMatrix;
    swig::buildStringMatrix( forwardRiskAsStringMatrix, forwardRiskLVB );

    std::string result = validation::tryAqSwapResultsForwardRiskUpdate( swapHandle, asOfDateAsAQLDate, forwardRiskAsStringMatrix );
    return result;

    AQ_API_END
}

/* @brief			swig interface for the aqSwapResultsDelete function
*  @param [in]		swapHandle		A cached swap handle
*  @return			Status string
*/
std::string aqSwapResultsDelete( const std::string& swapHandle )
{
    AQ_API_START

    std::string result = validation::tryAqSwapResultsDelete( swapHandle );
    return result;

    AQ_API_END
}

/* @brief			swig interface for the aqSwapResultsDeleteAll function
*  @return			Status string
*/
std::string aqSwapResultsDeleteAll()
{
    AQ_API_START

    std::string result = validation::tryAqSwapResultsDeleteAll();
    return result;

    AQ_API_END
}

/* @brief			swig interface for the aqSwapResultsRiskTotals function
*  @param [in]		swapHandle		A cached swap handle
*  @param [in]		riskType		The risk type to total, e.g. DISCOUNT, FORWARD
*  @return			Risk totals stored for a swap results object
*/
std::vector<double> aqSwapResultsRiskTotals( const std::string& swapHandle, const std::string& riskType )
{
    AQ_API_START

    std::vector<double> result = validation::tryAqSwapResultsRiskTotals( swapHandle, etrading::toRiskTypeEnum( riskType ) );
    return result;

    AQ_API_END
}

/* @brief			swig interface for the aqSwapResultsDisplay function
*  @param [in]		swapHandle		A cached swap handle
*  @return			Display of a swap results object
*/
SWIG_STRINGMATRIX aqSwapResultsDisplay( const std::string& swapHandle )
{
    AQ_API_START

    etrading::VariantMatrix result = validation::tryAqSwapResultsDisplay( swapHandle );

    SWIG_STRINGMATRIX resultsStringMatrix = swig::fromVariantMatrixToMatrixOfString( result );
    return resultsStringMatrix;

    AQ_API_END
}
