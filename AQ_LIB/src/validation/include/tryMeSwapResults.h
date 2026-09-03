// tryMeSwapResults.h

/*
 * @brief			Validation of Swap Results
 * @Created:		2nd January 2020
 * @Author:			Nicholas Burgess
 * @Department:		Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */
#pragma once
#include <string>
#include <vector>
#include "LADate.h"
#include "Variant.h"            // Includes DateVector, DoubleVector, VariantVector, VariantMatrix et al.
#include "CoreEnumerations.h"

namespace validation_api
{
	// Function to enable / disable the swap results object
	std::string tryMeSwapResultsEnable( const bool & enable );

	// Function to check if the swap results object is enabled / disabled
	std::string tryMeSwapResultsIsEnabled();

	// Function to update swap discount and forward rate risk
    std::string tryMeSwapResultsRiskUpdate( const std::string & swapHandle,
											const LADate & asOfDate,
											const LAStringMatrix & discountRiskLVB,		// discountRiskLVB = Table of Payment Dates and Coupons
											const LAStringMatrix & forwardRiskLVB );	// forwardRiskLVB = Table of Fixing Dates and Annuities

    // Function to update swap discount risk
    std::string tryMeSwapResultsDiscountRiskUpdate( const std::string & swapHandle,
													const LADate & asOfDate,
													const LAStringMatrix & discountRiskLVB ); // discountRiskLVB = Table of Payment Dates and Coupons

	// Function to update swap forward rate risk
    std::string tryMeSwapResultsForwardRiskUpdate( const std::string & swapHandle,
												   const LADate & asOfDate,
												   const LAStringMatrix & forwardRiskLVB ); // forwardRiskLVB = Table of Fixing Dates and Annuities

	// Function to delete a swap results object
    std::string tryMeSwapResultsDelete( const std::string & swapHandle  );
    
    // Function to delete all Curve results objects
    std::string tryMeSwapResultsDeleteAll();

	// Function to display swap risk totals
    DoubleVector tryMeSwapResultsRiskTotals( const std::string & swapHandle, const etrading::RiskTypeEnum & riskType );

	// Function to display a swap results object
    etrading::VariantMatrix tryMeSwapResultsDisplay( const std::string & swapHandle );

}
