// aqSwapResults.h

/*
 * @brief			Swig interface for aqSwapResults... functions (the Jacobian risk store)
 */

#pragma once

#include "SwigTypes.h"

#include <string>
#include <vector>

/* @brief			swig interface for the aqSwapResultsEnable function
*  @param [in]		enable		TRUE to enable, FALSE to disable
*  @return			Status string
*/
std::string aqSwapResultsEnable( bool enable );

/* @brief			swig interface for the aqSwapResultsIsEnabled function
*  @return			Whether the swap results (Jacobian risk) store is currently enabled
*/
std::string aqSwapResultsIsEnabled();

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
                                  const SWIG_STRINGMATRIX & forwardRiskLVB );

/* @brief			swig interface for the aqSwapResultsDiscountRiskUpdate function
*  @param [in]		swapHandle			A cached swap handle
*  @param [in]		asOfDate			The as-of date for the risk being stored (YYYYMMDD)
*  @param [in]		discountRiskLVB		Table of payment dates and coupons
*  @return			Status string
*/
std::string aqSwapResultsDiscountRiskUpdate( const std::string& swapHandle,
                                          const std::string& asOfDate,
                                          const SWIG_STRINGMATRIX & discountRiskLVB );

/* @brief			swig interface for the aqSwapResultsForwardRiskUpdate function
*  @param [in]		swapHandle			A cached swap handle
*  @param [in]		asOfDate			The as-of date for the risk being stored (YYYYMMDD)
*  @param [in]		forwardRiskLVB		Table of fixing dates and annuities
*  @return			Status string
*/
std::string aqSwapResultsForwardRiskUpdate( const std::string& swapHandle,
                                         const std::string& asOfDate,
                                         const SWIG_STRINGMATRIX & forwardRiskLVB );

/* @brief			swig interface for the aqSwapResultsDelete function
*  @param [in]		swapHandle		A cached swap handle
*  @return			Status string
*/
std::string aqSwapResultsDelete( const std::string& swapHandle );

/* @brief			swig interface for the aqSwapResultsDeleteAll function
*  @return			Status string
*/
std::string aqSwapResultsDeleteAll();

/* @brief			swig interface for the aqSwapResultsRiskTotals function
*  @param [in]		swapHandle		A cached swap handle
*  @param [in]		riskType		The risk type to total, e.g. DISCOUNT, FORWARD
*  @return			Risk totals stored for a swap results object
*/
std::vector<double> aqSwapResultsRiskTotals( const std::string& swapHandle, const std::string& riskType );

/* @brief			swig interface for the aqSwapResultsDisplay function
*  @param [in]		swapHandle		A cached swap handle
*  @return			Display of a swap results object
*/
SWIG_STRINGMATRIX aqSwapResultsDisplay( const std::string& swapHandle );
