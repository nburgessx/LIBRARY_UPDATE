// tryAqSwapsResults.h

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
#include "AQLDate.h"
#include "Variant.h"            // Includes DateVector, DoubleVector, VariantVector, VariantMatrix et al.
#include "CoreEnumerations.h"

namespace validation
{
	// Function to enable / disable the swap results object
	std::string tryAqSwapsResultsEnable( const bool & enable );

	// Function to check if the swap results object is enabled / disabled
	std::string tryAqSwapsResultsIsEnabled();

	// Function to update swap discount and forward rate risk
    std::string tryAqSwapsResultsRiskUpdate( const std::string & swapHandle,
											const AQLDate & asOfDate,
											const AQLStringMatrix & discountRiskLVB,		// discountRiskLVB = Table of Payment Dates and Coupons
											const AQLStringMatrix & forwardRiskLVB );	// forwardRiskLVB = Table of Fixing Dates and Annuities

    // Function to update swap discount risk
    std::string tryAqSwapsResultsDiscountRiskUpdate( const std::string & swapHandle,
													const AQLDate & asOfDate,
													const AQLStringMatrix & discountRiskLVB ); // discountRiskLVB = Table of Payment Dates and Coupons

	// Function to update swap forward rate risk
    std::string tryAqSwapsResultsForwardRiskUpdate( const std::string & swapHandle,
												   const AQLDate & asOfDate,
												   const AQLStringMatrix & forwardRiskLVB ); // forwardRiskLVB = Table of Fixing Dates and Annuities

	// Function to delete a swap results object
    std::string tryAqSwapsResultsDelete( const std::string & swapHandle  );
    
    // Function to delete all Curve results objects
    std::string tryAqSwapsResultsDeleteAll();

	// Function to display swap risk totals
    DoubleVector tryAqSwapsResultsRiskTotals( const std::string & swapHandle, const etrading::RiskTypeEnum & riskType );

	// Function to display a swap results object
    etrading::VariantMatrix tryAqSwapsResultsDisplay( const std::string & swapHandle );

}
