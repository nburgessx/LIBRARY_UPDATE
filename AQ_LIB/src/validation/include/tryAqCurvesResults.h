// tryAqCurvesResults.h

/*
 * @brief			Validation of Curve Results
 * @Created:		3rd April 2019
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
	// Function to enable / disable the curve results object
	std::string tryAqCurvesResultsEnable( const bool & enable );

	// Function to check if the curve results object is enabled / disabled
	std::string tryAqCurvesResultsIsEnabled();

    // Function to update discount factors for a chosen curve results object
    std::string tryAqCurvesResultsDiscountFactorsUpdate( const AQLStringMatrix & curveLVB,
														const AQLStringMatrix & parameterLVB,
														const AQLStringMatrix & discountFactorLVB,
														const StandardStringMatrix & forwardAdjustments = StandardStringMatrix() );

    // Function to display all discount factors stored in curve results object
    etrading::VariantMatrix tryAqCurvesResultsDiscountFactorsDisplayAll( const std::string & curveCollection, const std::string & curveIndex );

    // Function to display discount factors from the curve results object for the payment dates provided
    DoubleVector tryAqCurvesResultsDiscountFactorsDisplay( const std::string & curveCollection, const std::string & curveIndex, const DateVector & paymentDates );

    // Function to delete a curve results object
    std::string tryAqCurvesResultsDelete( const std::string & curveCollection, const std::string & curveIndex  );
    
    // Function to delete all Curve results objects
    std::string tryAqCurvesResultsDeleteAll();

	// Function to display forward rates from a curve results object for the fixing dates provided
    DoubleVector tryAqCurvesResultsForwardRatesDisplay( const std::string & curveCollection,
                                                       const std::string & curveIndex,
                                                       const DateVector & fixingDates,
                                                       const bool & isFwdInter = false,
                                                       const etrading::BusinessDayAdjustmentEnum& fixingBusinessDayAdj = etrading::NONE_BUSINESS_DAY_ADJ,
                                                       const std::string & fixingCalendar = ""  );

    // Function to update the jacobian for the chosen curve results object
    std::string tryAqCurvesResultsJacobianUpdate( const AQLStringMatrix & curveLVB,
                                                 const AQLStringMatrix & discountFactorParameterLVB,
                                                 const AQLStringMatrix & discountFactors,
                                                 const AQLStringMatrix & jacobianParameterLVB,
                                                 const std::vector<bool> & outrightInstruments,
												 const DoubleVector & marketDataShiftSizeInPercent,
                                                 const AQLStringMatrix jacobianMatrix );

    // Function to display Jacobian stored in curve results object
    etrading::VariantMatrix tryAqCurvesResultsJacobianDisplay( const std::string & curveCollection, const std::string & curveIndex, const etrading::RiskTypeEnum & riskType );
    
	// Function to display Jacobian Discount Factor Delta
    etrading::VariantMatrix tryAqCurvesResultsJacobianDiscountFactorDelta( const std::string & curveCollection, const std::string & curveIndex );

    // Function to display Jacobian Risk Totals
    etrading::VariantMatrix tryAqCurvesResultsJacobianRiskTotals( const std::string & curveCollection, const std::string & curveIndex, const etrading::RiskTypeEnum & riskType, const bool & useOutrightInstrumentsOnly = true );

    // Function to display the Jacobian new implied Discount Factors
    etrading::VariantMatrix tryAqCurvesResultsJacobianImplyNewDiscountFactors( const std::string & curveCollection, const std::string & curveIndex );

	// Function to create a curve group object
    std::string tryAqCurvesGroupCreate( const StandardString & groupName, const StandardStringVector & curveHandles );

	// Function to get the curve collection for a given curve group object
    std::string tryAqCurvesGroupCollectionName( const StandardString & groupName );

}