// tryMeCurveResults.cpp

/*
 * @brief			Validation of Curve Results
 * @Created:		3rd April 2019
 * @Author:			Nicholas Burgess
 * @Department:		Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */
#include "tryMeCurveResults.h"
#include "CurveResultsContainer.h"          // incl. CurveResults and CurveResultsContainer
#include "RecordMacros.h"                   // Record Macros
#include "CreateDataFile.h"                 // Record File Creation
#include "StructuredExceptionHandler.h"     // Validation Start and End Macros for Structured Exception Management
#include "DateUtilities.h"                  // Date helper methods and AQLStringMatrix to Date and Value vector helper
#include "tryMeUtilityClean.h"              // trimming utility methods
#include "LADateScheduleHelpers.h"          // Convert Strings to Dates and vice versa
#include "LabelValueBlock.h"                // String Matrix Conversion Helpers to/from AQLStringMatrix
#include "CoreEnumerations.h"
#include "CurveGroup.h"

// Namespaces for Test File Recording
using etrading::CreateDataFile;
using etrading::decorateFilename;

namespace validation
{
    // Function to enable / disable the curve results object
	std::string tryMeCurveResultsEnable( const bool & enable )
	{
		VALID_EXCEPTION_START
	
		// Record Inputs
        RECORD_INPUTS( enable )

		// *** No Enable Guard Required Here ***

        // Enable Curve Results and Confirm Enabled / Disabled Status
        etrading::CurveResultsContainer::getInstance().enableCurveResults( enable );
		const bool isEnabled = etrading::CurveResultsContainer::getInstance().isEnabled();

		std::string result = "Curve Results: DISABLED";
		if( isEnabled )
		{
			result = "Curve Results: ENABLED";
		}

        RECORD_OUTPUTS_AND_RETURN_RESULT( result )
		
		VALID_EXCEPTION_END
	}

	// Function to check if the curve results object is enabled / disabled
	std::string tryMeCurveResultsIsEnabled()
	{
		VALID_EXCEPTION_START
	
		// Record Inputs
        RECORD_INPUTS_NO_ARGUMENTS()
		
		// *** No Enable Guard Required Here ***

        // Get Curve Results Enabled / Disabled Status
        const bool isEnabled = etrading::CurveResultsContainer::getInstance().isEnabled();

		std::string result = "Curve Results: DISABLED";
		if( isEnabled )
		{
			result = "Curve Results: ENABLED";
		}

        RECORD_OUTPUTS_AND_RETURN_RESULT( result )
		
		VALID_EXCEPTION_END
	}

    // Function to update discount factors for a chosen curve results object
    std::string tryMeCurveResultsDiscountFactorsUpdate( const AQLStringMatrix & curveLVB,
														const AQLStringMatrix & parameterLVB,
														const AQLStringMatrix & discountFactorLVB,
														const StandardStringMatrix & forwardAdjustments )
    {
        VALID_EXCEPTION_START
	
        // Trim Discount Factors
        const AQLStringMatrix trimmedDiscountFactors = validation::trimLAStringMatrix( discountFactorLVB );

		// Record Inputs
        RECORD_INPUTS( curveLVB, parameterLVB, trimmedDiscountFactors ) // forwardAdjustments // TODO: Fix me! - Record Inputs does not support StandardStringMatrix
		
		// Check if Curve Results Enabled
		AQ_REQUIRE( etrading::CurveResultsContainer::getInstance().isEnabled(), "Curve Results have been Disabled" )

        // Create the Discount Factors and Curve Results Object
        // -------------------------------------------------------------------------------
        etrading::CurveDescriptionAndDiscountFactorResults curveDescriptionAndDiscFactors
            = etrading::curveDescriptionAndDiscountFactorResults( curveLVB, parameterLVB, discountFactorLVB, forwardAdjustments );


		std::shared_ptr<etrading::CurveResults> curveResults( new etrading::CurveResults( curveDescriptionAndDiscFactors.curveDescription_,
																						  nullptr, // nullptr = curveConventionsAndMarketData 
																						  curveDescriptionAndDiscFactors.discountFactorsResults_ ) ); 

        // Update Curve Results Container Singleton
        // -------------------------------------------------------------------------------
        const StandardString & curveCollection  = curveResults->curveDescription()->curveCollection();
        const StandardString & curveIndex       = curveResults->curveDescription()->curveIndexList();
        etrading::CurveResultsContainer::getInstance().addCurveResults( curveCollection, curveIndex, curveResults );
        
        // Record Outputs and Return the Result
        std::string result = curveCollection + ":" + curveIndex;
        RECORD_OUTPUTS_AND_RETURN_RESULT( result )
		
		VALID_EXCEPTION_END

    }

    // Function to display all discount factors to curve results object
    etrading::VariantMatrix tryMeCurveResultsDiscountFactorsDisplayAll( const std::string & curveCollection, const std::string & curveIndex )
    {
        VALID_EXCEPTION_START
	
		// Record Inputs
        RECORD_INPUTS( curveCollection, curveIndex )

		// Check if Curve Results Enabled
		AQ_REQUIRE( etrading::CurveResultsContainer::getInstance().isEnabled(), "Curve Results have been Disabled" )

        // Check if Curve Results Exist
        if ( !etrading::doesExistCurveResultsDiscountFactors( curveCollection, curveIndex ) )
        {
            AQ_THROW( "Discount Factor Results for Curve Collection '" + curveCollection + "' and/or Curve Index '" + curveIndex + "' do not exist" );
        }

        // Get Curve Results
        const etrading::VariantMatrix results = etrading::CurveResultsContainer::getInstance().getCurveResults( curveCollection, curveIndex )->discountFactorResults()->paymentDatesAndDiscountFactors();

        RECORD_OUTPUTS_AND_RETURN_RESULT( results )
		
		VALID_EXCEPTION_END
    }

    // Function to display discount factors to curve results object for the payment dates provided
    DoubleVector tryMeCurveResultsDiscountFactorsDisplay( const std::string & curveCollection, const std::string & curveIndex, const DateVector & paymentDates )
    {
        VALID_EXCEPTION_START
	
		// Record Inputs
        RECORD_INPUTS( curveCollection, curveIndex, paymentDates )

		// Check if Curve Results Enabled
		AQ_REQUIRE( etrading::CurveResultsContainer::getInstance().isEnabled(), "Curve Results have been Disabled" )

        // Check if Curve Results Exist
        if ( !etrading::doesExistCurveResultsDiscountFactors( curveCollection, curveIndex ) )
        {
            AQ_THROW( "Discount Factors Results for Curve Collection '" + curveCollection + "' and/or Curve Index '" + curveIndex + "' do not exist" );
        }

        // Get Curve Results
        const DoubleVector results = etrading::CurveResultsContainer::getInstance().getCurveResults( curveCollection, curveIndex )->discountFactorResults()->getDiscountFactors( paymentDates );

        RECORD_OUTPUTS_AND_RETURN_RESULT( results )
		
		VALID_EXCEPTION_END
    }

    // Function to delete a curve results object
    std::string tryMeCurveResultsDelete( const std::string & curveCollection, const std::string & curveIndex  )
    {
        VALID_EXCEPTION_START
	
		// Record Inputs
        RECORD_INPUTS( curveCollection, curveIndex )

		// *** No Enable Guard Required Here ***

        // Update Curve Results Container Singleton
        bool success = etrading::CurveResultsContainer::getInstance().deleteCurveResults( curveCollection, curveIndex );

        // Record Outputs and Return the Result
        std::string result;
        if ( success == true )
        {
            result = "Deleted Curve Result Object: " + curveCollection + ":" + curveIndex;
        }
        else
        {
            result = "Curve Results for Curve Collection '" + curveCollection + "' and/or Curve Index '" + curveIndex + "' do not exist";
        }
        
        RECORD_OUTPUTS_AND_RETURN_RESULT( result )
		
		VALID_EXCEPTION_END
    }
    
    // Function to delete all Curve results objects
    std::string tryMeCurveResultsDeleteAll()
    {
        VALID_EXCEPTION_START
	
		// *** No Enable Guard Required Here ***

		// *** No Inputs/Outputs to Record ***

        // Update Curve Results Container Singleton
        etrading::CurveResultsContainer::getInstance().deleteAllCurveResults();
        
        // Record Outputs and Return the Result
        const std::string result = "All Curve Result Objects have been Deleted";
        return result;
		
		VALID_EXCEPTION_END
    }

	// Function to display forward rates from a curve results object for the fixing dates provided
    DoubleVector tryMeCurveResultsForwardRatesDisplay( const std::string & curveCollection,
                                                       const std::string & curveIndex,
                                                       const DateVector & fixingDates,
                                                       const bool & isFwdInter,                                            // Defaults to isFwdInter False i.e. interpolate on Forwards = false
                                                       const etrading::BusinessDayAdjustmentEnum& fixingBusinessDayAdj,    // Defaults to NONE_BUSINESS_DAY_ADJ
                                                       const std::string & fixingCalendar )                                // Defaults to Blank Calendar
    {
		VALID_EXCEPTION_START
	
		// Record Inputs
        RECORD_INPUTS( curveCollection, curveIndex, fixingDates, isFwdInter )

		// Check if Curve Results Enabled
		AQ_REQUIRE( etrading::CurveResultsContainer::getInstance().isEnabled(), "Curve Results have been Disabled" )

        // Check if Curve Results Exist
        if ( !etrading::doesExistCurveResultsDiscountFactors( curveCollection, curveIndex ) )
        {
            AQ_THROW( "Forward Rate Results for Curve Collection '" + curveCollection + "' and/or Curve Index '" + curveIndex + "' do not exist" )
        }

        // Get Curve Results
        DoubleVector results;
		
		results = etrading::CurveResultsContainer::getInstance().getCurveResults( curveCollection, curveIndex )->discountFactorResults()->implyForwardRates( fixingDates, fixingBusinessDayAdj, fixingCalendar, isFwdInter, etrading::SIMPLE_COMPOUNDING );

		RECORD_OUTPUTS_AND_RETURN_RESULT( results )
		
		VALID_EXCEPTION_END
    }

    // Function to update the jacobian for the chosen curve results object
    std::string tryMeCurveResultsJacobianUpdate( const AQLStringMatrix & curveLVB,
                                                 const AQLStringMatrix & discountFactorParameterLVB,
                                                 const AQLStringMatrix & discountFactors,
                                                 const AQLStringMatrix & jacobianParameterLVB,
												 const std::vector<bool> & outrightInstruments,
                                                 const DoubleVector & marketDataShiftSizeInPercent,
                                                 const AQLStringMatrix jacobianMatrix )
    {
        VALID_EXCEPTION_START
	
        // Trim Jacobian
        const AQLStringMatrix trimmedJacobian = validation::trimLAStringMatrix( jacobianMatrix );

        RECORD_INPUTS( curveLVB, discountFactorParameterLVB, discountFactors, jacobianParameterLVB, marketDataShiftSizeInPercent, trimmedJacobian );
        
		// Check if Curve Results Enabled
		AQ_REQUIRE( etrading::CurveResultsContainer::getInstance().isEnabled(), "Curve Results have been Disabled" )

        // Input Validation
        if ( !curveLVB.empty() && curveLVB[0].size() != 2 )
        {
            AQ_THROW("Invalid Data: CurveLVB data cannot be empty and must have column size 2")
        }

        if ( !jacobianParameterLVB.empty() && jacobianParameterLVB[0].size() != 2 )
        {
            AQ_THROW("Invalid Data: JacobianParameterLVB data cannot be empty and must have column size 2")
        }

        AQ_REQUIRE( !marketDataShiftSizeInPercent.empty(),    "Invalid Data: MarketDataShiftSize parameter is empty" )
        AQ_REQUIRE( !trimmedJacobian.empty(),                 "Invalid Data: Jacobian cannot be empty" )
        
        // Shift Size should match trimmed Jacobian column size less the first column of dates    
        AQ_REQUIRE( marketDataShiftSizeInPercent.size() == trimmedJacobian[0].size() - 1, "Invalid Data: The number of marketDataShiftSize parameters must match the number of Jacobian columns" )
        
        // Create the Discount Factors and Curve Results Object
        // -------------------------------------------------------------------------------
        etrading::CurveDescriptionAndDiscountFactorResults curveDescriptionAndDiscFactors
            = etrading::curveDescriptionAndDiscountFactorResults( curveLVB, discountFactorParameterLVB, discountFactors );
        // -------------------------------------------------------------------------------

        // Extract Curve LVB Values
        LabelValueBlock curveLVB_( curveLVB );
        const etrading::CurveTypeEnum curveTypeEnum             = etrading::toCurveTypeEnum( curveLVB_.getCompulsoryValue( "CURVETYPE" ) );
        const etrading::CurveTenorEnum curveTenorEnum           = etrading::toCurveTenorEnum( curveLVB_.getCompulsoryValue( "CURVETENOR" ) );
        const std::string curveCollection                       = curveLVB_.getCompulsoryValue( "CURVECOLLECTION" );
        const std::string curveIndex                            = curveLVB_.getCompulsoryValue( "CURVEINDEX" );
        
        // Extract Parameter LVB Values
        LabelValueBlock jacobianParameterLVB_( jacobianParameterLVB );
        
        // As of Date is part of the discountFactorParameterLVB which is stored in the discount factor results object pointer
        const AQLDate asOfDate                                   = curveDescriptionAndDiscFactors.discountFactorsResults_->asOfDate();
        const etrading::RiskTypeEnum riskType                   = etrading::toRiskTypeEnum( jacobianParameterLVB_.getCompulsoryValue("RISKTYPE") );
        const etrading::ShiftTypeEnum shiftType                 = etrading::toShiftTypeEnum( jacobianParameterLVB_.getCompulsoryValue("SHIFTTYPE") );
        const double gradientShiftSize                          = jacobianParameterLVB_.getCompulsoryValueAsDouble("GRADIENTSHIFTSIZE");
        
        AQ_REQUIRE( !AQ_IS_EQUAL_ZERO( gradientShiftSize ), "Invalid Data: GradientShiftSize cannot be zero or less than 1.0e-14 in absolute terms" )

        // Create Curve Result Object
        // -------------------------------------------------------------------------------
        
        // Create the Jacobian
        std::shared_ptr<etrading::JacobianResults> jacobianResults;
        if ( shiftType == etrading::FLAT_SHIFT_TYPE )
        {
            AQ_REQUIRE( marketDataShiftSizeInPercent.size() == 1, "Invalid Data: When using Flat-Shift Jacobians only a single MarketDataShiftSize parameter should be provided." )
            const double flatShiftMarketDataShiftSize = marketDataShiftSizeInPercent[0];
            
            // Jacobian Data
            DateVector riskDates;
            DoubleVector flatShiftJacobian;
            etrading::JacobianResults::populateFlatShiftJacobianFromStringMatrix( riskDates, flatShiftJacobian, etrading::convertToStandardStringMatrix( trimmedJacobian ) );
            
            // FLAT_SHIFT JACOBIAN
            jacobianResults = std::shared_ptr<etrading::JacobianResults>( new etrading::JacobianResults( asOfDate,
                                                                                                         curveDescriptionAndDiscFactors.curveDescription_,
                                                                                                         riskType,
                                                                                                         gradientShiftSize,
                                                                                                         riskDates,
                                                                                                         flatShiftMarketDataShiftSize,
                                                                                                         flatShiftJacobian,
                                                                                                         curveDescriptionAndDiscFactors.discountFactorsResults_ ) );
        }
        else if ( shiftType == etrading::PERTURBED_SHIFT_TYPE )
        {
            // Jacobian Data
            DateVector riskDates;
            StandardStringVector perturbedInstruments;
			DoubleMatrix perturbedJacobian;
            etrading::JacobianResults::populatePerturbedJacobianFromStringMatrix( riskDates, perturbedInstruments, perturbedJacobian, etrading::convertToStandardStringMatrix( trimmedJacobian ) );

            // PERTURBED JACOBIAN
            jacobianResults = std::shared_ptr<etrading::JacobianResults>( new etrading::JacobianResults( asOfDate,
                                                                                                         curveDescriptionAndDiscFactors.curveDescription_,
                                                                                                         riskType,
                                                                                                         gradientShiftSize,
                                                                                                         riskDates,
                                                                                                         marketDataShiftSizeInPercent,
                                                                                                         perturbedInstruments,
																										 outrightInstruments,
                                                                                                         perturbedJacobian,
                                                                                                         curveDescriptionAndDiscFactors.discountFactorsResults_ ) );
        }
        else
        {
            AQ_THROW( "Invalid Data: ShiftType must be FLAT_SHIFT or PERTURBED" )
        }

		// Update the Curve Results
		std::shared_ptr<etrading::JacobianResults> jacobianResultsByDiscountFactor	= nullptr;
		std::shared_ptr<etrading::JacobianResults> jacobianResultsByForwardRates	= nullptr;
		std::shared_ptr<etrading::JacobianResults> jacobianResultsByCompoundRates	= nullptr;
		
		switch( riskType )
		{
			case etrading::DISCOUNT_FACTOR_RISK_TYPE:
			{
				jacobianResultsByDiscountFactor = jacobianResults;
				break;
			}
			case etrading::FORWARD_RATE_RISK_TYPE:
			{
				jacobianResultsByForwardRates = jacobianResults;
				break;
			}
			case etrading::COMPOUND_RATE_RISK_TYPE:
			{
				jacobianResultsByCompoundRates = jacobianResults;
				break;
			}
			default:
			{
				AQ_THROW("Invalid Risk Type: The risk type must be 'DISCOUNT_FACTORS', 'FORWARD_RATES' or 'COMPOUND_RATES'.")
			}
		}
		
		std::shared_ptr<etrading::CurveResults> curveResults( new etrading::CurveResults( curveDescriptionAndDiscFactors.curveDescription_,
																						  nullptr, // curveConventionsAndMarketData
																						  curveDescriptionAndDiscFactors.discountFactorsResults_,
																						  jacobianResultsByDiscountFactor,
																						  jacobianResultsByForwardRates,
																						  jacobianResultsByCompoundRates ) );  

        // Update Curve Results Container Singleton
        // -------------------------------------------------------------------------------
        etrading::CurveResultsContainer::getInstance().addCurveResults( curveCollection, curveIndex, curveResults );
        
        // Record Outputs and Return the Result
        std::string result = curveCollection + ":" + curveIndex;
        
        RECORD_OUTPUTS_AND_RETURN_RESULT( result )
		
		VALID_EXCEPTION_END
    }

    // Function to display Jacobian stored in curve results object
    etrading::VariantMatrix tryMeCurveResultsJacobianDisplay( const std::string & curveCollection, const std::string & curveIndex, const etrading::RiskTypeEnum & riskType )
    {
        VALID_EXCEPTION_START
	
		// Record Inputs
        RECORD_INPUTS( curveCollection, curveIndex );

		// Check if Curve Results Enabled
		AQ_REQUIRE( etrading::CurveResultsContainer::getInstance().isEnabled(), "Curve Results have been Disabled" )

		// Get the Jacobian Results
		std::shared_ptr<etrading::JacobianResults> jacobianResults = etrading::getJacobianResults( curveCollection, curveIndex, riskType );
		
        etrading::VariantMatrix displayResults;
        
        // Get Curve Jacobian Display Results
        if( jacobianResults->shiftType() == etrading::FLAT_SHIFT_TYPE )
        {
            displayResults = jacobianResults->displayFlatShiftJacobian();
        }
        else if ( jacobianResults->shiftType() == etrading::PERTURBED_SHIFT_TYPE )
        {
            displayResults = jacobianResults->displayPerturbedJacobian();
        }
        else
        {
            AQ_THROW("Invalid Jacobian: ShiftType must be 'FLAT-SHIFT' or 'PERTURBED'.")
        }
        
        RECORD_OUTPUTS_AND_RETURN_RESULT( displayResults )

        VALID_EXCEPTION_END
    }

	// Function to display Jacobian Discount Factor Delta
	etrading::VariantMatrix tryMeCurveResultsJacobianDiscountFactorDelta( const std::string & curveCollection, const std::string & curveIndex )
	{
		VALID_EXCEPTION_START
		
		// Record Inputs
        RECORD_INPUTS( curveCollection, curveIndex )

		// Check if Curve Results Enabled
		AQ_REQUIRE( etrading::CurveResultsContainer::getInstance().isEnabled(), "Curve Results have been Disabled" )

        // Get the Jacobian Results by DISCOUNT FACTOR
		std::shared_ptr<etrading::JacobianResults> jacobianResults = etrading::getJacobianResults( curveCollection, curveIndex, etrading::DISCOUNT_FACTOR_RISK_TYPE );
        
		// Get Curve Jacobian Dates and Discount Factor Jacobian
        if( jacobianResults->shiftType() == etrading::FLAT_SHIFT_TYPE )
        {
			const DateVector riskDates = jacobianResults->riskDateVector();
			const DoubleVector flatShiftJacobian = jacobianResults->flatShiftJacobian();
			const double gradientShiftSize = jacobianResults->gradientShiftSize();

			AQ_REQUIRE( riskDates.size() == flatShiftJacobian.size(), "Invalid Risk Jacobian: Number of Risk Dates must match the number of Discount Factor Deltas" )
			AQ_REQUIRE( !AQ_IS_EQUAL_ZERO( gradientShiftSize ), "Invalid Risk Jacobian: Gradient Shift Size must not be zero" )

			// Populate Results Matrix
			etrading::VariantMatrix ResultsMatrix( flatShiftJacobian.size() ); // Row Size
			for( size_t i = 0; i < flatShiftJacobian.size(); ++i )
			{
				etrading::VariantVector thisRow(2);									// Column Size: Two Columns
				thisRow[0]          = riskDates[i];									// Column 1
				thisRow[1]          = flatShiftJacobian[i] / gradientShiftSize;		// Column 2 - Divide by the gradient shift size to get the unitless delta, which is more useful
				ResultsMatrix[i]    = thisRow;										// Results Row
			}
			RECORD_OUTPUTS_AND_RETURN_RESULT( ResultsMatrix )
        }
        else if ( jacobianResults->shiftType() == etrading::PERTURBED_SHIFT_TYPE )
        {
			const DateVector riskDates = jacobianResults->riskDateVector();
			const DoubleMatrix perturbedJacobian = jacobianResults->perturbedJacobian();
			const double gradientShiftSize = jacobianResults->gradientShiftSize();

			AQ_REQUIRE( riskDates.size() == perturbedJacobian.size(), "Invalid Risk Jacobian: Number of Risk ates must match the number of Discount Factor Deltas" )
			AQ_REQUIRE( !AQ_IS_EQUAL_ZERO( gradientShiftSize ), "Invalid Risk Jacobian: Gradient Shift Size must not be zero" )

			// Populate Results Matrix
			etrading::VariantMatrix ResultsMatrix( perturbedJacobian.size() ); // Row Size
			for( size_t i = 0; i < perturbedJacobian.size(); ++i )
			{
				etrading::VariantVector thisRow( perturbedJacobian[i].size() + 1 );		// Coumn Size: 1 Risk Date + Number of Jacobian Columns
				thisRow[0] = riskDates[i];				
				
				// Interate Over Perturbed Jacobian Columns
				for( size_t j = 0; j < perturbedJacobian[i].size(); ++j )
				{
					thisRow[j+1] = perturbedJacobian[i][j] / gradientShiftSize;		// Divide by the gradient shift size to get the unitless delta, which is more useful
				}
				
				ResultsMatrix[i] = thisRow;	// Results Row
			}
			RECORD_OUTPUTS_AND_RETURN_RESULT( ResultsMatrix )
        }

		AQ_THROW("Invalid Jacobian: ShiftType must be 'FLAT-SHIFT' or 'PERTURBED'.")
        
        VALID_EXCEPTION_END
	}

    // Function to display Jacobian Risk Totals
    etrading::VariantMatrix tryMeCurveResultsJacobianRiskTotals( const std::string & curveCollection, const std::string & curveIndex, const etrading::RiskTypeEnum & riskType, const bool & useOutrightInstrumentsOnly )
    {
        VALID_EXCEPTION_START
		
		// Record Inputs
        RECORD_INPUTS( curveCollection, curveIndex )

		// Check if Curve Results Enabled
		AQ_REQUIRE( etrading::CurveResultsContainer::getInstance().isEnabled(), "Curve Results have been Disabled" )

        // Get the Jacobian Results
		std::shared_ptr<etrading::JacobianResults> jacobianResults = etrading::getJacobianResults( curveCollection, curveIndex, riskType );
        
		// Get Curve Jacobian Risk Dates and Risk Totals
        etrading::DateVector riskDates = jacobianResults->riskDateVector();
        etrading::DoubleVector riskTotals;

        if( jacobianResults->shiftType() == etrading::FLAT_SHIFT_TYPE )
        {
            riskTotals = jacobianResults->calculateFlatShiftRiskTotals();
        }
        else if ( jacobianResults->shiftType() == etrading::PERTURBED_SHIFT_TYPE )
        {
            riskTotals = jacobianResults->calculatePerturbedRiskTotals( useOutrightInstrumentsOnly );
        }
        else
        {
            AQ_THROW("Invalid Jacobian: ShiftType must be 'FLAT-SHIFT' or 'PERTURBED'.")
        }
        
        AQ_REQUIRE( riskDates.size() == riskTotals.size(), "Invalid Risk Jacobian: Number of Risk Dates must match the number of Risk Totals" )

        // Populate Results Matrix
        etrading::VariantMatrix ResultsMatrix( riskTotals.size() );
        for( size_t i = 0; i < riskTotals.size(); ++i )
        {
            etrading::VariantVector thisRow(2);     // Two Columns
            thisRow[0]          = riskDates[i];     // Column 1
            thisRow[1]          = riskTotals[i];    // Column 2
            ResultsMatrix[i]    = thisRow;          // Results Row
        }

        RECORD_OUTPUTS_AND_RETURN_RESULT( ResultsMatrix )

        VALID_EXCEPTION_END
    }

    // Function to display the Jacobian new implied Discount Factors
    etrading::VariantMatrix tryMeCurveResultsJacobianImplyNewDiscountFactors( const std::string & curveCollection, const std::string & curveIndex )
    {
        VALID_EXCEPTION_START
	
		// Record Inputs
        RECORD_INPUTS( curveCollection, curveIndex )
		
		// Check if Curve Results Enabled
		AQ_REQUIRE( etrading::CurveResultsContainer::getInstance().isEnabled(), "Curve Results have been Disabled" )

        // Check if Curve Results Exist
        if( !etrading::doesExistCurveResultsDiscountFactors( curveCollection, curveIndex ) )
        {
            AQ_THROW( "Discount Factors for Curve Collection '" + curveCollection + "' and/or Curve Index '" + curveIndex + "' do not exist" )
        }

        // Get Jacobian Results - This method also checks if the results exist
        std::shared_ptr<etrading::JacobianResults> jacobianResults = etrading::getJacobianResults( curveCollection, curveIndex, etrading::DISCOUNT_FACTOR_RISK_TYPE );
        
        // Get Curve Jacobian Risk Dates and New Discount Factors
        etrading::DateVector riskDates              = jacobianResults->riskDateVector();
        etrading::DoubleVector newDiscountFactors   = jacobianResults->implyNewDiscountFactors();
        
        AQ_REQUIRE( riskDates.size() == newDiscountFactors.size(), "Invalid Risk Jacobian: Number of Risk Dates must match the number of New Discount Factors" )

        // Populate Results Matrix
        etrading::VariantMatrix ResultsMatrix( newDiscountFactors.size() );
        for( size_t i = 0; i < newDiscountFactors.size(); ++i )
        {
            etrading::VariantVector thisRow(2);             // Two Columns
            thisRow[0]          = riskDates[i];             // Column 1
            thisRow[1]          = newDiscountFactors[i];    // Column 2
            ResultsMatrix[i]    = thisRow;                  // Results Row
        }

        RECORD_OUTPUTS_AND_RETURN_RESULT( ResultsMatrix )

        VALID_EXCEPTION_END
    }

	// Function to create a curve group object
    std::string tryMeCurveGroupCreate( const StandardString & groupName, const StandardStringVector & curveHandles )
    {
        VALID_EXCEPTION_START
	
		// Uppercase the GroupName
		std::string groupNameUppercase = groupName;
		boost::to_upper( groupNameUppercase ); 

        // Record Inputs
        RECORD_INPUTS( groupNameUppercase, curveHandles )
		
		// Check if Curve Results Enabled
		AQ_REQUIRE( etrading::CurveResultsContainer::getInstance().isEnabled(), "Curve Results have been Disabled" )

        // Create the Curve Collection Object
        // -------------------------------------------------------------------------------
        std::shared_ptr<etrading::CurveGroup> curveGroupObj( new etrading::CurveGroup( curveHandles ) );
		std::shared_ptr<etrading::CurveResults> curveResults( new etrading::CurveResults( curveGroupObj ) ); 

        // Update Curve Results Container Singleton
        // -------------------------------------------------------------------------------
        etrading::CurveResultsContainer::getInstance().addCurveResults( groupNameUppercase, "", curveResults );
        
        // Record Outputs and Return the Result
        RECORD_OUTPUTS_AND_RETURN_RESULT( groupNameUppercase )
		
		VALID_EXCEPTION_END
    }

	// Function to get the curve collection for a given curve group object
	std::string tryMeCurveGroupCollectionName( const StandardString & groupName )
	{
		VALID_EXCEPTION_START

		// Uppercase the GroupName
		std::string groupNameUppercase = groupName;
		boost::to_upper( groupNameUppercase ); 

        // Record Inputs
        RECORD_INPUTS( groupNameUppercase )
		
		// Check if Curve Results Enabled
		AQ_REQUIRE( etrading::CurveResultsContainer::getInstance().isEnabled(), "Curve Results have been Disabled" )

        // Check if Curve Results Exist
        if ( !etrading::doesExistCurveGroup( groupNameUppercase ) )
        {
            AQ_THROW( "Curve Group '" + groupNameUppercase + "' does not exist" );
        }

        // Get the Curve Collection
        const std::string curveCollectionName = etrading::CurveResultsContainer::getInstance().getCurveResults( groupNameUppercase, "" )->curveGroup()->curveCollection();
		RECORD_OUTPUTS_AND_RETURN_RESULT( curveCollectionName )
		
		VALID_EXCEPTION_END
	}

}
