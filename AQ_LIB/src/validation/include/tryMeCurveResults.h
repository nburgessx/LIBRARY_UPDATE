// tryMeCurveResults.h

/*
 * @brief			Validation of Curve Results
 * @Created:		3rd April 2019
 * @Author:			Nicholas Burgess
 * @Department:		Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */
#pragma once
#include <string>
#include <vector>
#include "LADate.h"
#include "Variant.h"            // Includes DateVector, DoubleVector, VariantVector, VariantMatrix et al.
#include "CoreEnumerations.h"

namespace validation_api
{
	// Function to enable / disable the curve results object
	std::string tryMeCurveResultsEnable( const bool & enable );

	// Function to check if the curve results object is enabled / disabled
	std::string tryMeCurveResultsIsEnabled();

    // Function to update discount factors for a chosen curve results object
    std::string tryMeCurveResultsDiscountFactorsUpdate( const LAStringMatrix & curveLVB,
														const LAStringMatrix & parameterLVB,
														const LAStringMatrix & discountFactorLVB,
														const StandardStringMatrix & forwardAdjustments = StandardStringMatrix() );

    // Function to display all discount factors stored in curve results object
    etrading::VariantMatrix tryMeCurveResultsDiscountFactorsDisplayAll( const std::string & curveCollection, const std::string & curveIndex );

    // Function to display discount factors from the curve results object for the payment dates provided
    DoubleVector tryMeCurveResultsDiscountFactorsDisplay( const std::string & curveCollection, const std::string & curveIndex, const DateVector & paymentDates );

    // Function to delete a curve results object
    std::string tryMeCurveResultsDelete( const std::string & curveCollection, const std::string & curveIndex  );
    
    // Function to delete all Curve results objects
    std::string tryMeCurveResultsDeleteAll();

	// Function to display forward rates from a curve results object for the fixing dates provided
    DoubleVector tryMeCurveResultsForwardRatesDisplay( const std::string & curveCollection,
                                                       const std::string & curveIndex,
                                                       const DateVector & fixingDates,
                                                       const bool & isFwdInter = false,
                                                       const etrading::BusinessDayAdjustmentEnum& fixingBusinessDayAdj = etrading::NONE_BUSINESS_DAY_ADJ,
                                                       const std::string & fixingCalendar = ""  );

    // Function to update the jacobian for the chosen curve results object
    std::string tryMeCurveResultsJacobianUpdate( const LAStringMatrix & curveLVB,
                                                 const LAStringMatrix & discountFactorParameterLVB,
                                                 const LAStringMatrix & discountFactors,
                                                 const LAStringMatrix & jacobianParameterLVB,
                                                 const std::vector<bool> & outrightInstruments,
												 const DoubleVector & marketDataShiftSizeInPercent,
                                                 const LAStringMatrix jacobianMatrix );

    // Function to display Jacobian stored in curve results object
    etrading::VariantMatrix tryMeCurveResultsJacobianDisplay( const std::string & curveCollection, const std::string & curveIndex, const etrading::RiskTypeEnum & riskType );
    
	// Function to display Jacobian Discount Factor Delta
    etrading::VariantMatrix tryMeCurveResultsJacobianDiscountFactorDelta( const std::string & curveCollection, const std::string & curveIndex );

    // Function to display Jacobian Risk Totals
    etrading::VariantMatrix tryMeCurveResultsJacobianRiskTotals( const std::string & curveCollection, const std::string & curveIndex, const etrading::RiskTypeEnum & riskType, const bool & useOutrightInstrumentsOnly = true );

    // Function to display the Jacobian new implied Discount Factors
    etrading::VariantMatrix tryMeCurveResultsJacobianImplyNewDiscountFactors( const std::string & curveCollection, const std::string & curveIndex );

	// Function to create a curve group object
    std::string tryMeCurveGroupCreate( const StandardString & groupName, const StandardStringVector & curveHandles );

	// Function to get the curve collection for a given curve group object
    std::string tryMeCurveGroupCollectionName( const StandardString & groupName );

}