// tryAqSwapResults.h

/*
 * @brief			Validation of Swap Results
 */
#pragma once
#include <string>
#include <vector>
#include "AQLDate.h"
#include "Variant.h"            // Includes DateVector, DoubleVector, VariantVector, VariantMatrix et al.
#include "CoreEnumerations.h"

namespace validation
{
	// Function to enable / disable the swap results object
	std::string tryAqSwapResultsEnable( const bool & enable );

	// Function to check if the swap results object is enabled / disabled
	std::string tryAqSwapResultsIsEnabled();

	// Function to update swap discount and forward rate risk
    std::string tryAqSwapResultsRiskUpdate( const std::string & swapHandle,
											const AQLDate & asOfDate,
											const AQLStringMatrix & discountRiskLVB,		// discountRiskLVB = Table of Payment Dates and Coupons
											const AQLStringMatrix & forwardRiskLVB );	// forwardRiskLVB = Table of Fixing Dates and Annuities

    // Function to update swap discount risk
    std::string tryAqSwapResultsDiscountRiskUpdate( const std::string & swapHandle,
													const AQLDate & asOfDate,
													const AQLStringMatrix & discountRiskLVB ); // discountRiskLVB = Table of Payment Dates and Coupons

	// Function to update swap forward rate risk
    std::string tryAqSwapResultsForwardRiskUpdate( const std::string & swapHandle,
												   const AQLDate & asOfDate,
												   const AQLStringMatrix & forwardRiskLVB ); // forwardRiskLVB = Table of Fixing Dates and Annuities

	// Function to delete a swap results object
    std::string tryAqSwapResultsDelete( const std::string & swapHandle  );
    
    // Function to delete all Curve results objects
    std::string tryAqSwapResultsDeleteAll();

	// Function to display swap risk totals
    DoubleVector tryAqSwapResultsRiskTotals( const std::string & swapHandle, const etrading::RiskTypeEnum & riskType );

	// Function to display a swap results object
    etrading::VariantMatrix tryAqSwapResultsDisplay( const std::string & swapHandle );

}
