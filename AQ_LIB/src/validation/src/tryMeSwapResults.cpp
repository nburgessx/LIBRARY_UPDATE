// tryMeSwapResults.cpp

/*
 * @brief			Validation of Swap Results
 * @Created:		2nd January 2020
 * @Author:			Nicholas Burgess
 * @Department:		Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */
#include "tryMeSwapResults.h"
#include "SwapResultsContainer.h"           // incl. SwapResults and SwapResultsContainer
#include "RecordMacros.h"                   // Record Macros
#include "CreateDataFile.h"                 // Record File Creation
#include "StructuredExceptionHandler.h"     // Validation Start and End Macros for Structured Exception Management
#include "DateUtilities.h"                  // Date helper methods and LAStringMatrix to Date and Value vector helper
#include "tryMeUtilityClean.h"              // trimming utility methods
#include "LADateScheduleHelpers.h"          // Convert Strings to Dates and vice versa
#include "LabelValueBlock.h"                // String Matrix Conversion Helpers to/from LAStringMatrix
#include "CoreEnumerations.h"

#include <boost/algorithm/string.hpp>		// boost::to_upper

// Namespaces for Test File Recording
using etrading::CreateDataFile;
using etrading::decorateFilename;

namespace validation
{
	// Function to enable / disable the swap results object
	std::string tryMeSwapResultsEnable( const bool & enable )
	{
		VALID_EXCEPTION_START
	
		// Record Inputs
        RECORD_INPUTS( enable )

		// *** No Enable Guard Required Here ***

        // Enable Swap Results and Confirm Enabled / Disabled Status
        etrading::SwapResultsContainer::getInstance().enableSwapResults( enable );
		const bool isEnabled = etrading::SwapResultsContainer::getInstance().isEnabled();

		std::string result = "Swap Results: DISABLED";
		if( isEnabled )
		{
			result = "Swap Results: ENABLED";
		}

        RECORD_OUTPUTS_AND_RETURN_RESULT( result )
		
		VALID_EXCEPTION_END
	}

	// Function to check if the swap results object is enabled / disabled
	std::string tryMeSwapResultsIsEnabled()
	{
		VALID_EXCEPTION_START
	
		// Record Inputs
        RECORD_INPUTS_NO_ARGUMENTS()
		
		// *** No Enable Guard Required Here ***

        // Get Swap Results Enabled / Disabled Status
        const bool isEnabled = etrading::SwapResultsContainer::getInstance().isEnabled();

		std::string result = "Swap Results: DISABLED";
		if( isEnabled )
		{
			result = "Swap Results: ENABLED";
		}

        RECORD_OUTPUTS_AND_RETURN_RESULT( result )
		
		VALID_EXCEPTION_END
	}

	// Function to update swap discount and forward rate risk
	std::string tryMeSwapResultsRiskUpdate( const std::string & swapHandle,
											const LADate & asOfDate,
								 			const LAStringMatrix & discountRiskLVB,		// discountRiskLVB = Table of Payment Dates and Coupons
								 			const LAStringMatrix & forwardRiskLVB )		// forwardRiskLVB = Table of Fixing Dates and Annuities
	{
		VALID_EXCEPTION_START

		// Uppercase Swap Handle
		std::string swapHandleUpperCase = swapHandle;
		boost::to_upper( swapHandleUpperCase );

        // Trim Discount Factor Risk LVB
        const LAStringMatrix trimmedDiscountRisk = validation::trimLAStringMatrix( discountRiskLVB );
		const LAStringMatrix trimmedForwardRisk = validation::trimLAStringMatrix( forwardRiskLVB );

		// Record Inputs
        RECORD_INPUTS( swapHandleUpperCase, asOfDate, trimmedDiscountRisk, trimmedForwardRisk )
		
		// Check if Swap Results Enabled
		AQ_REQUIRE( etrading::SwapResultsContainer::getInstance().isEnabled(), "Swap Results have been Disabled" )

        // Marshall Data
		DateVector paymentDates;
		DoubleVector coupons;
		etrading::populateDateValueVectorsFromStringMatrix( paymentDates, coupons, trimmedDiscountRisk );

		DateVector fixingDates;
		DoubleVector annuities;
		etrading::populateDateValueVectorsFromStringMatrix( fixingDates, annuities, trimmedForwardRisk );

		// Create the Swap Risk Object
		const etrading::SwapDiscountRiskData discountRiskData( paymentDates, coupons );
		const etrading::SwapForwardRiskData forwardRiskData( fixingDates, annuities );
		std::shared_ptr<etrading::SwapRiskResults> swapDiscountFactorRisk( new etrading::SwapRiskResults( asOfDate, discountRiskData, forwardRiskData ) );
        
		// Create the Swap Results Object
		std::shared_ptr<etrading::SwapResults> swapResults( new etrading::SwapResults( swapDiscountFactorRisk ) ); 

        // Update Swap Results Container Singleton
        // -------------------------------------------------------------------------------
        etrading::SwapResultsContainer::getInstance().addSwapResults( swapHandleUpperCase, swapResults );
        
        // Record Outputs and Return the Result
        const std::string result = swapHandleUpperCase;
        RECORD_OUTPUTS_AND_RETURN_RESULT( result )
		
		VALID_EXCEPTION_END
	}

	// Function to update swap discount risk
	std::string tryMeSwapResultsDiscountRiskUpdate( const std::string & swapHandle,
													const LADate & asOfDate,
													const LAStringMatrix & discountRiskLVB ) // discountRiskLVB = Table of Payment Dates and Coupons
	{
		VALID_EXCEPTION_START
	
		// Uppercase Swap Handle
		std::string swapHandleUpperCase = swapHandle;
		boost::to_upper( swapHandleUpperCase );

        // Trim Discount Factor Risk LVB
        const LAStringMatrix trimmedDiscountRisk = validation::trimLAStringMatrix( discountRiskLVB );

		// Record Inputs
        RECORD_INPUTS( swapHandleUpperCase, asOfDate, trimmedDiscountRisk )
		
		// Check if Swap Results Enabled
		AQ_REQUIRE( etrading::SwapResultsContainer::getInstance().isEnabled(), "Swap Results have been Disabled" )

        // Marshall Data
		DateVector paymentDates;
		DoubleVector coupons;
		etrading::populateDateValueVectorsFromStringMatrix( paymentDates, coupons, trimmedDiscountRisk );

		// Create the Swap Risk Object
		const etrading::SwapDiscountRiskData discountRiskData( paymentDates, coupons );
		std::shared_ptr<etrading::SwapRiskResults> swapDiscountFactorRisk( new etrading::SwapRiskResults( asOfDate, discountRiskData ) );
        
		// Create the Swap Results Object
		std::shared_ptr<etrading::SwapResults> swapResults( new etrading::SwapResults( swapDiscountFactorRisk ) ); 

        // Update Swap Results Container Singleton
        // -------------------------------------------------------------------------------
        etrading::SwapResultsContainer::getInstance().addSwapResults( swapHandleUpperCase, swapResults );
        
        // Record Outputs and Return the Result
        const std::string result = swapHandleUpperCase;
        RECORD_OUTPUTS_AND_RETURN_RESULT( result )
		
		VALID_EXCEPTION_END
	}

	// Function to update swap forward rate risk
    std::string tryMeSwapResultsForwardRiskUpdate( const std::string & swapHandle,
												   const LADate & asOfDate,
												   const LAStringMatrix & forwardRiskLVB ) // forwardRiskLVB = Table of Fixing Dates and Annuities
	{
		VALID_EXCEPTION_START
	
		// Uppercase Swap Handle
		std::string swapHandleUpperCase = swapHandle;
		boost::to_upper( swapHandleUpperCase );

        // Trim Forward Rate Risk LVB
        const LAStringMatrix trimmedForwardRisk = validation::trimLAStringMatrix( forwardRiskLVB );

		// Record Inputs
        RECORD_INPUTS( swapHandleUpperCase, asOfDate, trimmedForwardRisk )
		
		// Check if Swap Results Enabled
		AQ_REQUIRE( etrading::SwapResultsContainer::getInstance().isEnabled(), "Swap Results have been Disabled" )

		// Marshall Data
        DateVector fixingDates;
		DoubleVector annuities;
		etrading::populateDateValueVectorsFromStringMatrix( fixingDates, annuities, trimmedForwardRisk );

		// Create the Swap Risk Object
		const etrading::SwapForwardRiskData forwardRiskData( fixingDates, annuities );
		std::shared_ptr<etrading::SwapRiskResults> swapForwardRateRisk( new etrading::SwapRiskResults( asOfDate, forwardRiskData ) );
        
		// Create the Swap Results Object
		std::shared_ptr<etrading::SwapResults> swapResults( new etrading::SwapResults( swapForwardRateRisk ) ); 

        // Update Swap Results Container Singleton
        // -------------------------------------------------------------------------------
        etrading::SwapResultsContainer::getInstance().addSwapResults( swapHandleUpperCase, swapResults );
        
        // Record Outputs and Return the Result
        const std::string result = swapHandleUpperCase;
        RECORD_OUTPUTS_AND_RETURN_RESULT( result )
		
		VALID_EXCEPTION_END
	}

	// Function to delete a swap results object
	std::string tryMeSwapResultsDelete( const std::string & swapHandle )
	{
		VALID_EXCEPTION_START
	
		// Uppercase Swap Handle
		std::string swapHandleUpperCase = swapHandle;
		boost::to_upper( swapHandleUpperCase );

		// Record Inputs
        RECORD_INPUTS( swapHandleUpperCase )

		// *** No Enable Guard Required Here ***

        // Update Curve Results Container Singleton
        bool success = etrading::SwapResultsContainer::getInstance().deleteSwapResults( swapHandleUpperCase );

        // Record Outputs and Return the Result
        std::string result;
        if ( success == true )
        {
            result = "Deleted Swap Result Object: " + swapHandleUpperCase;
        }
        else
        {
            result = "Swap Results for Swap '" + swapHandleUpperCase + "' do not exist";
        }
        
        RECORD_OUTPUTS_AND_RETURN_RESULT( result )
		
		VALID_EXCEPTION_END
	}
    
    // Function to delete all Curve results objects
	std::string tryMeSwapResultsDeleteAll()
	{
		VALID_EXCEPTION_START
	
		// *** No Enable Guard Required Here ***

		// *** No Inputs/Outputs to Record ***

        // Update Swap Results Container Singleton
        etrading::SwapResultsContainer::getInstance().deleteAllSwapResults();
        
        // Record Outputs and Return the Result
        const std::string result = "All Swap Result Objects have been Deleted";
        return result;
		
		VALID_EXCEPTION_END
	}
	
	// Function to display swap risk totals
	DoubleVector tryMeSwapResultsRiskTotals( const std::string & swapHandle, const etrading::RiskTypeEnum & riskType )
	{
		VALID_EXCEPTION_START
	
		// Uppercase Swap Handle
		std::string swapHandleUpperCase = swapHandle;
		boost::to_upper( swapHandleUpperCase );

		// Record Inputs
        RECORD_INPUTS( swapHandleUpperCase, riskType )

		// Check if Swap Results Enabled
		AQ_REQUIRE( etrading::SwapResultsContainer::getInstance().isEnabled(), "Swap Results have been Disabled" )

        // Check if Swap Risk Results Exist
        if ( !etrading::doesExistSwapRiskResults( swapHandleUpperCase ) )
        {
            AQ_THROW( "Swap Risk Results for Swap '" + swapHandleUpperCase + "' do not exist" );
        }

        // Get Swap Results
		DoubleVector results;
		switch( riskType )
		{
			case etrading::DISCOUNT_FACTOR_RISK_TYPE:
			{
				// DISCOUNT_FACTOR RISK: Return Coupon Parameters
				results = etrading::SwapResultsContainer::getInstance().getSwapResults( swapHandleUpperCase )->swapRiskResults()->coupons();
				AQ_REQUIRE( results.size() > 0, "Discount Factor Risks Do Not Exist for Swap Handle: " + swapHandleUpperCase )
				break;
			}
			case etrading::FORWARD_RATE_RISK_TYPE:
			{
				// FORWARD_RATE RISK: Return Annuity Parameters
				results = etrading::SwapResultsContainer::getInstance().getSwapResults( swapHandleUpperCase )->swapRiskResults()->annuities();
				AQ_REQUIRE( results.size() > 0, "Forward Rate Risks Do Not Exist for Swap Handle: " + swapHandleUpperCase )
				break;
			}
			default:
			{
				AQ_THROW("Invalid RiskType: Only DISCOUNT_FACTOR and FORWARD_RATE risk types supported")
			}
		}
        
        RECORD_OUTPUTS_AND_RETURN_RESULT( results )
		
		VALID_EXCEPTION_END
	}

	// Function to display a swap results object
	etrading::VariantMatrix tryMeSwapResultsDisplay( const std::string & swapHandle )
	{
		VALID_EXCEPTION_START
	
		// Uppercase Swap Handle
		std::string swapHandleUpperCase = swapHandle;
		boost::to_upper( swapHandleUpperCase );

		// Record Inputs
        RECORD_INPUTS( swapHandleUpperCase )

		AQ_THROW("Not Implemented")
		etrading::VariantMatrix results;

		RECORD_OUTPUTS_AND_RETURN_RESULT( results )

		VALID_EXCEPTION_END

	}

}