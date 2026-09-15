#include "tryAqSwapObjectPricing.h"

#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "CommonConstants.h"
#include "RecordMacros.h"

#include <memory>

#include "SwapUtilities.h"
#include "ObjectUtilities.h"
#include "AQObjUtilities.h"
#include "CoreEnumerations.h"
#include "CrossCurrencySwap.h"
#include "FeeLeg.h"
#include "SwapGenerator.h"
#include "SwapValidation.h"
#include "CreditDefaultSwap.h"
#include "ConstantMaturitySwap.h"
#include "TotalReturnSwap.h"
#include "CreditModel.h"
#include "CreditBasketModel.h"
#include "AQObjUtilities.h"
#include "ExceptionMacros.h"
#include "tryAqCurveDiscountFactor.h"
#include "AQLCurveForwardRateHelpers.h"
#include "AQLDateScheduleHelpers.h"
#include "SettingsValidation.h"
#include "ValuationSettings.h"

// OMP Threading Header
#include <omp.h>

using etrading::CreateDataFile;
using etrading::decorateFilename;
using etrading::Swap;
using etrading::CrossCurrencySwap;
using etrading::Schedule;

namespace validation
{
    
    
    
    // -------------------------------------------------------------------------------------------------------
    // HELPER UTILITIES
    // Note: No thread guard or start macros required for utility functions




    /* @brief			Helper utility method for the aqSwapObjectPV method
	*  @param [in]		swapName			Swap object name
	*  @param [in]		valuationSettingsLVB	A LVB containing ModelName, CurveCollection, ValuationDate, etc.
	*  @param [in]		legName				Leg name
    *  @param [in]		fixingTableNames	Fixing table object names
	*  @return			Swap PV
	*/
	double validationUtilitySwapPV( const std::string& swapName, const LabelValueBlock& valuationSettingsLVB, const AQLString& legName, const LabelValueBlock& fixingTableNames )
	{
        // Record Inputs for logs, tests and playback
        AQ_RECORD_INPUTS( swapName, valuationSettingsLVB, legName, fixingTableNames);

		auto swap           = etrading::getSwap(swapName);
        double result       = swap->pv(valuationSettingsLVB, fixingTableNames, legName);

		// Record Outputs AND Return the Result for logs, tests and playback
        AQ_RECORD_OUTPUTS_AND_RETURN_RESULT( result );
	}

    // END OF HELPER UTILITIES
    // Note: No thread guard or start macros required for utility functions
    // -------------------------------------------------------------------------------------------------------





    /* @brief			validation interface for the aqSwapObjectPVs method
	*  @param [in]		swapNames		    Swap object names
	*  @param [in]		valuationSettingsLVB    A LVB containing ModelName, CurveCollection, ValuationDate, etc.
	*  @param [in]		legNames	        Leg names
    *  @param [in]		fixingTableNames	Fixing table object names
    *  @param [in]		optimize            Optimize performance using OMP threading
	*  @return			Swap PV
	*/
	std::vector<double> tryAqSwapObjectPVs(const std::vector<std::string>& swapNames, const std::vector<LabelValueBlock>& valuationSettingsLVB, std::vector<std::string>& legNames, std::vector<LabelValueBlock>& fixingTableNames, const bool optimize)
    {
        VALID_EXCEPTION_START

        // We don't record inputs since this is done within the below nested calling function

        // Validate Input Vector Sizes
        // --------------------------------------------------
        const size_t nSwaps = swapNames.size();
        AQ_REQUIRE( valuationSettingsLVB.size() == nSwaps, "The number of curve collections must match the number of swaps" );

        // Default leg names to blank if missing or if only 1 leg name is provided use that for all swaps
        if ( legNames.empty() )     legNames.resize( nSwaps, "" );
        if ( legNames.size() == 1 ) legNames.resize( nSwaps, legNames[0] );
        AQ_REQUIRE( legNames.size() == nSwaps, "The number of leg names must match the number of swaps" );

        // Default fixing table names to blank if missing or if only 1 fixing table is provided use that for all swaps
        if ( fixingTableNames.empty() ) fixingTableNames.resize( nSwaps, LabelValueBlock() );
        if ( fixingTableNames.size() == 1 ) fixingTableNames.resize( nSwaps, fixingTableNames[0] );
        AQ_REQUIRE( fixingTableNames.size() == nSwaps, "The number of fixing tables must match the number of swaps" );
        // --------------------------------------------------
        
        std::vector< double > results(swapNames.size(), 0.0);

        // The optimize parameter controls if we are to thread using OMP
        if ( optimize )
        {
            // Thread Swap Calculations using OMP
            const int nThreads = omp_get_max_threads();
            #pragma omp parallel for num_threads( nThreads )
            for( int i = 0; i < int( swapNames.size() ); ++i )
            {
                results[i] = validationUtilitySwapPV( swapNames[i], valuationSettingsLVB[i], AQLString( legNames[i].c_str() ), fixingTableNames[i] );
            }
        }
        else
        {
            // Sequential Swap Calculations
            for( size_t i = 0; i < swapNames.size(); ++i )
            {
                // Use an empty label value block if no fixing table provided
		        results[i] = validationUtilitySwapPV( swapNames[i], valuationSettingsLVB[i], AQLString( legNames[i].c_str() ), fixingTableNames[i] );
            }
        }

        return results;

		VALID_EXCEPTION_END
    }

 	/* @brief			validation interface for the aqSwapObjectPV method
	*  @param [in]		swapName			Swap object name
	*  @param [in]		curveCollection		Valuation settings
	*  @param [in]		legName				Leg name
    *  @param [in]		fixingTableNames	Fixing table object names
	*  @return			Swap PV
	*/
	double tryAqSwapObjectPV(const std::string& swapName, const LabelValueBlock& valuationSettingsLVB, const AQLString& legName, const LabelValueBlock& fixingTableNames)
	{
		VALID_EXCEPTION_START
		
		// Marshall Valuation Settings to use Curve Collection Names
		const etrading::LabelValueBlock valuationLVBusingCurveCollection = etrading::ValuationSettings( valuationSettingsLVB.toStandardStringMatrix() ).getValuationSettingsLVB();

        // Call the swapPV utility function above - This is to allow the swapPV to be called from other functions without the thread guard macro
        double result = validationUtilitySwapPV( swapName, valuationLVBusingCurveCollection, legName, fixingTableNames );
        return result;

		VALID_EXCEPTION_END
	}

	/* @brief			validation interface for the aqSwapObjectPV01 method
	*  @param [in]		swapName			Swap object name
	*  @param [in]		curveCollection		Valuation settings
    *  @param [in]		fixingTableNames	Fixing table object names
	*  @return			Swap PV01
	*/
	double tryAqSwapObjectPV01(const std::string& swapName, const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames)
	{
		VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        AQ_RECORD_INPUTS( swapName, valuationSettingsLVB, fixingTableNames);

		auto swap = etrading::getSwap(swapName);

		// Marshall Valuation Settings to use Curve Collection Names
		const etrading::LabelValueBlock valuationLVBusingCurveCollection = etrading::ValuationSettings( valuationSettingsLVB.toStandardStringMatrix() ).getValuationSettingsLVB();

		double ret = swap->pv01(valuationLVBusingCurveCollection, fixingTableNames);
		
		// Record Outputs AND Return the Result for logs, tests and playback
        AQ_RECORD_OUTPUTS_AND_RETURN_RESULT( ret );

		VALID_EXCEPTION_END
	}

	/* @brief			validation interface for the aqSwapObjectParRate method
	*  @param [in]		swapName		        Swap object name
	*  @param [in]		valuationSettingsLVB    Valuation settings map
    *  @param [in]		fixingTableNames        Fixing table object names
    *  @param [in]		legName             	(Optional) Leg Name
	*  @return			Swap par rate
	*/
	double tryAqSwapObjectParRate(const std::string& swapName, const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames, const AQLString& legName)
	{
		VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        AQ_RECORD_INPUTS( swapName, valuationSettingsLVB, fixingTableNames, legName );

		auto swap = etrading::getSwap(swapName);
		
		// Marshall Valuation Settings to use Curve Collection Names
		const etrading::LabelValueBlock valuationLVBusingCurveCollection = etrading::ValuationSettings( valuationSettingsLVB.toStandardStringMatrix() ).getValuationSettingsLVB();

        double result = 0.0;
        
        if ( legName == "" )
        {
            result = swap->parRate(valuationLVBusingCurveCollection, fixingTableNames);
        }
        else
        {
            const double pvSwap                     = swap->pv( valuationLVBusingCurveCollection, fixingTableNames );
            const double pvTargetLeg                = swap->pv( valuationLVBusingCurveCollection, fixingTableNames, legName );
            const double annuityTargetLeg           = swap->annuity( valuationLVBusingCurveCollection, legName, true ); // true = include sign
            const double pvSwapWithoutTargetLeg     = pvSwap - pvTargetLeg;

            AQ_REQUIRE( ! AQ_IS_EQUAL_ZERO( annuityTargetLeg ), "Invalid Par Rate, Target Leg cannot have a zero annuity" )
            
            // Negative sign required so that target leg offsets swap value to bring to par.                
            result = -pvSwapWithoutTargetLeg / annuityTargetLeg;
        }

		// Record Outputs AND Return the Result for logs, tests and playback
        AQ_RECORD_OUTPUTS_AND_RETURN_RESULT( result );

		VALID_EXCEPTION_END
	}

    /* @brief			validation interface for the aqSwapObjectAnnuity method
	*  @param [in]		swapName		Swap object name
	*  @param [in]		curveCollection Valuation settings
	*  @param [in]		legName		    Leg name
	*  @return			Swap PV
	*/
	double tryAqSwapObjectAnnuity(const std::string& swapName, const LabelValueBlock& valuationSettingsLVB, const AQLString& legName)
	{
		VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        AQ_RECORD_INPUTS( swapName, valuationSettingsLVB, legName);

		auto swap = etrading::getSwap(swapName);
		double ret = swap->annuity(valuationSettingsLVB, legName);

		// Record Outputs AND Return the Result for logs, tests and playback
        AQ_RECORD_OUTPUTS_AND_RETURN_RESULT( ret );

		VALID_EXCEPTION_END
	}

    /* @brief			validation interface for the aqSwapObjectSpread method
	*  @param [in]		swapName		Swap name
	*  @param [in]		curveCollection Curve collections
    *  @param [in]		fixingTableName	Fixing table object names
	*  @param [in]		spreadLegName   Leg name the spread will be applied to
	*  @return			The spread that make the swap PV zero
	*/
	double tryAqSwapObjectSpread(const std::string& swapName, const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames, const AQLString& spreadLegName)
	{
		VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        AQ_RECORD_INPUTS( swapName, valuationSettingsLVB, fixingTableNames, spreadLegName);

		auto swap = etrading::getSwap(swapName);

		// Marshall Valuation Settings to use Curve Collection Names
		const etrading::LabelValueBlock valuationLVBusingCurveCollection = etrading::ValuationSettings( valuationSettingsLVB.toStandardStringMatrix() ).getValuationSettingsLVB();

        double ret = swap->spread(valuationLVBusingCurveCollection, fixingTableNames, false, spreadLegName);
      
		// Record Outputs AND Return the Result for logs, tests and playback
        AQ_RECORD_OUTPUTS_AND_RETURN_RESULT( ret );

		VALID_EXCEPTION_END
	}

    /* @brief			validation interface for the aqSwapObjectParSpread method 
    *  @param [in]		swapName			Swap name
	*  @param [in]		valuationSettingsLVB	Curve collections
    *  @param [in]		fixingTableNames	Fixing table object names
	*  @param [in]		spreadLegName       Leg name the spread will be applied to
	*  @return			The spread that make the swap PV zero, without taking the legs' existing spreads into account
	*/
	double tryAqSwapObjectParSpread(const std::string& swapName, const LabelValueBlock& valuationSettingsLVB, const LabelValueBlock& fixingTableNames, const AQLString& spreadLegName)
	{
		VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        AQ_RECORD_INPUTS( swapName, valuationSettingsLVB, fixingTableNames, spreadLegName);

		auto swap = etrading::getSwap(swapName);

		// Marshall Valuation Settings to use Curve Collection Names
		const etrading::LabelValueBlock valuationLVBusingCurveCollection = etrading::ValuationSettings( valuationSettingsLVB.toStandardStringMatrix() ).getValuationSettingsLVB();

		double ret = swap->spread(valuationLVBusingCurveCollection, fixingTableNames, true, spreadLegName);
      
		// Record Outputs AND Return the Result for logs, tests and playback
        AQ_RECORD_OUTPUTS_AND_RETURN_RESULT( ret );

		VALID_EXCEPTION_END
	}


    /* @brief			validation interface for the aqSwapObjectDisplayCashflows method
	*  @param [in]		swapName		    Swap name
	*  @param [in]		modelNames          A LVB containing ModelName or CurveCollection per leg
	*  @param [in]		legName		        Leg name
    *  @param [in]		fixingTableNames	Fixing table object names
	*  @param [in]		showColumnHeaders   showColumnHeaders
	*  @param [in]	    columnList          Column header names to show specified columns. Default to empty list showing all columns.
	*  @return			Swap display
	*/
	std::vector<AnyTypeMatrix> tryAqSwapObjectDisplayCashflows(const std::string& swapName, 
                                                            const LabelValueBlock& modelNames, 
                                                            const AQLString& legName, 
                                                            const LabelValueBlock& fixingTableNames, 
                                                            bool showColumnHeaders,
                                                            const std::vector<std::string>& columnList)
	{
		VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        AQ_RECORD_INPUTS( swapName, modelNames, legName, fixingTableNames, showColumnHeaders, columnList );

		auto swap = etrading::getSwap(swapName); 
        
        auto ret = swap->view(modelNames, fixingTableNames, legName, showColumnHeaders, etrading::toCashflowHeaderEnumSet(columnList));
        
		if (CreateDataFile::recordEnabled()) 
		{
			CreateDataFile file(decorateFilename("tryAqSwapObjectDisplayCashflows_outputs", swapName.c_str()));
			for (size_t i = 0; i < ret.size(); ++i)
			{
				file.write("output", ret[i]);
			}
		}

		return ret;

		VALID_EXCEPTION_END
	}

	/* @brief			validation interface for the aqSwapObjectAccruedInterest method
	*  @param [in]		swapName		Swap object name
	*  @param [in]		valuationSettingsLVB Valuation settings
	*  @param [in]		legName		    Leg name
    *  @param [in]		fixingTableNames	Fixing table object names
	*  @return			Swap Leg's AccruedInterest
	*/
	double tryAqSwapObjectAccruedInterest(const std::string& swapName, const LabelValueBlock& valuationSettingsLVB, const AQLString& legName, const LabelValueBlock& fixingTableNames)
	{
	    VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        AQ_RECORD_INPUTS( swapName, valuationSettingsLVB, legName, fixingTableNames);

		auto swap = etrading::getSwap(swapName);

		double result = swap->accruedInterest(valuationSettingsLVB, legName, fixingTableNames);

        // Record Outputs AND Return the Result for logs, tests and playback
        AQ_RECORD_OUTPUTS_AND_RETURN_RESULT( result );

		VALID_EXCEPTION_END
	}

	/* @brief validation interface for the aqCDSObjectPVFromHazardRate method.
	*  	      Calculates the PV of the specified Credit Default Swap, or of one of the legs.
	*
	* @param[in]	curveCollection		The collection containing the OIS curve for discounting
	* @param[in]	hazardRate			The CDS hazard rate, used to calculate survival probabilities
	* @param[in]	recoveryRate		The estimated amount of capital recovered after default
	* @param [in]	legName			    If specified, calculate the PV of the single swap leg
	* @param[in]	includeAccruedInterest	Specifies whether the accruedInterest should be included in the PV
	* @param[out]	The calculated PV value
	*/
    double tryAqCDSObjectPVFromHazardRate( const std::string& swapName, const LabelValueBlock& valuationSettingsLVB, const double hazardRate, const double recoveryRate, const AQLString& legName, const bool includeAccruedInterest )
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
        AQ_RECORD_INPUTS( swapName, valuationSettingsLVB, hazardRate, recoveryRate, legName, includeAccruedInterest );

		auto swap = etrading::getSwap(swapName);

		if (swap->getSwapType() != etrading::CREDIT_DEFAULT_SWAP )
		{
			AQ_THROW( "Swap '" + swapName + "' has incorrect Swap Type of '" + toString( swap->getSwapType() ) + "'. It is not a credit default swap." );
		}
		auto cds = std::dynamic_pointer_cast<etrading::CreditDefaultSwap>( swap );
		if ( cds == nullptr )
		{
			AQ_THROW( "Swap '" + swapName + "' is not a credit default swap." );
		}
		
		double result = cds->pvFromHazardRate( valuationSettingsLVB, hazardRate, recoveryRate, legName, includeAccruedInterest );
		        
		// Record Outputs AND Return the Result for logs, tests and playback
        AQ_RECORD_OUTPUTS_AND_RETURN_RESULT( result );

		VALID_EXCEPTION_END
	}

	/* @brief validation interface for the aqCDSObjectPV method.
	*   Calculates the PV of the specified Credit Default Swap, or of one of the legs.
	*
	* @param[in]	swapName			Swap object name
	* @param[in]	creditModelName		Credit Model object name
	* @param[in]	legName			    If specified, calculate the PV of the single swap leg
	* @returns	The calculated PV value
	*/
	double tryAqCDSObjectPV( const std::string& swapName, const std::string& creditModelName,  const AQLString& legName )
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
        AQ_RECORD_INPUTS( swapName, creditModelName, legName );

		auto swap = etrading::getSwap(swapName);

		if (swap->getSwapType() != etrading::CREDIT_DEFAULT_SWAP )
		{
			AQ_THROW( "Swap '" + swapName + "' has incorrect Swap Type of '" + toString( swap->getSwapType() ) + "'. It is not a credit default swap." );
		}
		auto cds = std::dynamic_pointer_cast<etrading::CreditDefaultSwap>( swap );
		if ( cds == nullptr )
		{
			AQ_THROW( "Swap '" + swapName + "' is not a credit default swap." );
		}
		
		auto creditModel = etrading::getCreditModel( creditModelName );

		double result = cds->pv( *creditModel, legName );
		        
		// Record Outputs AND Return the Result for logs, tests and playback
        AQ_RECORD_OUTPUTS_AND_RETURN_RESULT( result );

		VALID_EXCEPTION_END
	
	}

	/* @brief	validation interface for the aqCDSObjectPVByIntegration method.
	*			Calculates the total PV of all the Credit Default Swap Legs, by integrating the payoff over survivial probability.
	*			The integration over survival probability is equivalent to an integration over survival time, with an appropriate change of variable.
	*
	* @param[in]	swapName							Swap object name
	* @param[in]	creditModelName						Credit Model object name
	* @param[in]	legName								If specified, calculate the PV of the single swap leg
	* @param[in]	numberOfIntegrationPoints			Specifies the number of (x,y) points to use in the numerical integration
	* @param[in]	evaluateInParallel					When TRUE, evaluate loops in parallel, where possible.
	* @param[in]	payDefaultCashflowsOnNextCouponDate	When default occurs, whether to pay out the protection and accrued interest immediately, or wait to the next coupon date.
	*													TRUE means wait to the next coupon date. This flag is used to match the PV by integration to the analytic PV formula.
	* @returns	The calculated PV value
	*/
	double tryAqCDSObjectPVByIntegration( const std::string& swapName, const std::string& creditModelName, const AQLString& legName, const size_t numberOfIntegrationPoints, const bool evaluateInParallel, const bool payDefaultCashflowsOnNextCouponDate )
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
        AQ_RECORD_INPUTS( swapName, creditModelName, legName, numberOfIntegrationPoints, evaluateInParallel, payDefaultCashflowsOnNextCouponDate );

		auto swap = etrading::getSwap(swapName);

		if (swap->getSwapType() != etrading::CREDIT_DEFAULT_SWAP )
		{
			AQ_THROW( "Swap '" + swapName + "' has incorrect Swap Type of '" + toString( swap->getSwapType() ) + "'. It is not a credit default swap." );
		}
		auto cds = std::dynamic_pointer_cast<etrading::CreditDefaultSwap>( swap );
		if ( cds == nullptr )
		{
			AQ_THROW( "Swap '" + swapName + "' is not a credit default swap." );
		}
		
		auto creditModel = etrading::getCreditModel( creditModelName );

		double result = cds->pvByIntegration( *creditModel, legName, numberOfIntegrationPoints, evaluateInParallel, payDefaultCashflowsOnNextCouponDate );
		        
		// Record Outputs AND Return the Result for logs, tests and playback
        AQ_RECORD_OUTPUTS_AND_RETURN_RESULT( result );

		VALID_EXCEPTION_END
	}

	/* @brief	validation interface for the aqCDSObjectPVByMonteCarlo method.
	*			Calculates the total PV of all the Credit Default Swap Legs, by a monte-carlo simulation over survivial probability.
	*			The monte-carlo over survival probability is equivalent to a monte-carlo over survival time, with an appropriate change of variable.
	*
	* @param[in]	swapName							Swap object name
	* @param[in]	creditModelName						Credit Model object name
	* @param[in]	legName								If specified, calculate the PV of the single swap leg
	* @param[in]	mcParametersLVB						A label value block specifying Monte-Carlo / Random number generator parameters. 
	* @param[in]	payDefaultCashflowsOnNextCouponDate	When default occurs, whether to pay out the protection and accrued interest immediately, or wait to the next coupon date.
	*													TRUE means wait to the next coupon date. This flag is used to match the PV by integration to the analytic PV formula.
	* @param[out]	standardError						The Monte-Carlo standard error in the result.
	*
	* @returns	The calculated PV value
	*/
	double tryAqCDSObjectPVByMonteCarlo( const std::string& swapName, const std::string& creditModelName, const AQLString& legName, const LabelValueBlock& mcParametersLVB, const bool payDefaultCashflowsOnNextCouponDate, double& standardError )
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
        AQ_RECORD_INPUTS( swapName, creditModelName, legName, mcParametersLVB, payDefaultCashflowsOnNextCouponDate );

		auto swap = etrading::getSwap(swapName);

		if (swap->getSwapType() != etrading::CREDIT_DEFAULT_SWAP )
		{
			AQ_THROW( "Swap '" + swapName + "' has incorrect Swap Type of '" + toString( swap->getSwapType() ) + "'. It is not a credit default swap." );
		}
		auto cds = std::dynamic_pointer_cast<etrading::CreditDefaultSwap>( swap );
		if ( cds == nullptr )
		{
			AQ_THROW( "Swap '" + swapName + "' is not a credit default swap." );
		}
		
		auto creditModel = etrading::getCreditModel( creditModelName );

		double result = cds->pvByMonteCarlo( *creditModel, legName, mcParametersLVB, payDefaultCashflowsOnNextCouponDate, standardError );
		        
		// Record Outputs AND Return the Result for logs, tests and playback
        AQ_RECORD_OUTPUTS_AND_RETURN_RESULT( result );

		VALID_EXCEPTION_END
	}

	/* @brief Calculates the risky annuity of the specified Credit Default Swap Premium leg.
	*
	* @param [in]	swapName			The name of the credit default swap object
	* @param[in]	valuationSettingsLVB	Valuation settings
	* @param[in]	hazardRate			The CDS hazard rate, used to calculate survival probabilities
	* @param[in]	recoveryRate		The estimated amount of capital recovered after default
	* @param[in]	legName				The Premium leg to use when calculating the risky annuity. A mandatory parameter
	* @param[in]	includeAccruedInterest	Specifies whether the accruedInterest should be included in the risky annuity
	* @returns	The risky annuity
	*/
	double tryAqCDSObjectRiskyAnnuityFromHazardRate( const std::string& swapName, const LabelValueBlock& valuationSettingsLVB, const double hazardRate, const double recoveryRate, const AQLString& legName, const bool includeAccruedInterest )
	{
		VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        AQ_RECORD_INPUTS( swapName, valuationSettingsLVB, hazardRate, recoveryRate, legName, includeAccruedInterest );

		auto swap = etrading::getSwap(swapName);

		auto cds = std::dynamic_pointer_cast<etrading::CreditDefaultSwap>( swap );
		if ( cds == nullptr )
		{
			AQ_THROW( "Swap '" + swapName + "' is not a credit default swap." );
		}

		double result = cds->riskyAnnuityFromHazardRate(valuationSettingsLVB, hazardRate, recoveryRate, legName, includeAccruedInterest );

		// Record Outputs AND Return the Result for logs, tests and playback
        AQ_RECORD_OUTPUTS_AND_RETURN_RESULT( result );

		VALID_EXCEPTION_END
	}

	/* @brief Calculates the risky annuity of the specified Credit Default Swap Premium leg.
	*
	* @param [in]	swapName			The name of the credit default swap object
	* @param[in]	creditModelName		Credit Model object name
	* @param[in]	legName				The Premium leg to use when calculating the risky annuity. A mandatory parameter
	* @returns	The risky annuity
	*/
	double tryAqCDSObjectRiskyAnnuity( const std::string& swapName, const std::string& creditModelName, const AQLString& legName )
	{
		VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        AQ_RECORD_INPUTS( swapName, creditModelName, legName );

		auto swap = etrading::getSwap(swapName);

		auto cds = std::dynamic_pointer_cast<etrading::CreditDefaultSwap>( swap );
		if ( cds == nullptr )
		{
			AQ_THROW( "Swap '" + swapName + "' is not a credit default swap." );
		}
		
		auto creditModel = etrading::getCreditModel( creditModelName );

		double result = cds->riskyAnnuity( *creditModel, legName );

		// Record Outputs AND Return the Result for logs, tests and playback
        AQ_RECORD_OUTPUTS_AND_RETURN_RESULT( result );

		VALID_EXCEPTION_END
	}

	/* @brief[in]	Computes the accrued year fraction from the previous coupon date to the specified date
	*				Used in accrued interest calculations.
	*
	* @param [in]	swapName			The name of the credit default swap object
	* @param[in]	creditModelName		Credit Model object name
	* @param[in]	toDate				The date to which we wish to calculate the year fraction
	* @param[in]	legName				The Premium leg name
	* @returns: The year fraction
	*/
	double tryAqCDSObjectAccruedYearFraction( const std::string& swapName, const std::string& creditModelName, const AQLDate& toDate, const AQLString& legName )
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		AQ_RECORD_INPUTS( swapName, creditModelName, toDate, legName );

		auto swap = etrading::getSwap(swapName);

		auto cds = std::dynamic_pointer_cast<etrading::CreditDefaultSwap>(swap);
		if (cds == nullptr)
		{
			AQ_THROW("Swap '" + swapName + "' is not a credit default swap.");
		}

		auto creditModel = etrading::getCreditModel(creditModelName);

		double result = cds->accruedYearFraction( *creditModel, toDate, legName);

		// Record Outputs AND Return the Result for logs, tests and playback
		AQ_RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END
	}

    /* @brief Calculates the risky annuity of the specified Credit Default Swap Premium leg.
	*
	* @param [in]	swapName			The name of the credit default swap object
	* @param[in]	creditModelName		Credit Model object name
	* @param[in]	legName				The Premium leg to use when calculating the risky annuity. A mandatory parameter
	* @returns	The risky annuity
	*/
	double tryAqCDSObjectCS01( const std::string& swapName, const std::string& creditModelName, const AQLString& legName )
	{
		VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        AQ_RECORD_INPUTS( swapName, creditModelName, legName );

		auto swap = etrading::getSwap(swapName);

		auto cds = std::dynamic_pointer_cast<etrading::CreditDefaultSwap>( swap );
		if ( cds == nullptr )
		{
			AQ_THROW( "Swap '" + swapName + "' is not a credit default swap." );
		}
		
		auto creditModel = etrading::getCreditModel( creditModelName );

        double oneBasisPoint = 0.0001;
		double result = cds->riskyAnnuity( *creditModel, legName ) * oneBasisPoint;

		// Record Outputs AND Return the Result for logs, tests and playback
        AQ_RECORD_OUTPUTS_AND_RETURN_RESULT( result );

		VALID_EXCEPTION_END
	}

	/* @brief Calculates the par spread of the specified Credit Default Swap.
	*
	* @param[in]	swapName				The name of the credit default swap object
	* @param[in]	valuationSettingsLVB		A label value block containing a single collection name or a curveCollection per leg
	* @param[in]	hazardRate				The CDS hazard rate, used to calculate survival probabilities
	* @param[in]	recoveryRate			The estimated amount of capital recovered after default
	* @param[in]	premiumLegName			The Premium leg name of the CDS
	* @param[in]	protectionLegName		The Protection leg name of the CDS
	* @param[in]	includeAccruedInterest	Specifies whether the accruedInterest should be included in the risky annuity
	* @returns	The CDS par spread
	*/
	double tryAqCDSObjectParSpreadFromHazardRate( const std::string& swapName, const LabelValueBlock& valuationSettingsLVB, const double hazardRate, const double recoveryRate, const AQLString& premiumLegName, const AQLString& protectionLegName, const bool includeAccruedInterest )
	{
		VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        AQ_RECORD_INPUTS( swapName, valuationSettingsLVB, hazardRate, recoveryRate, premiumLegName, protectionLegName, includeAccruedInterest );

		auto swap = etrading::getSwap(swapName);

		auto cds = std::dynamic_pointer_cast<etrading::CreditDefaultSwap>( swap );
		if ( cds == nullptr )
		{
			AQ_THROW( "Swap '" + swapName + "' is not a credit default swap." );
		}
		
		double result = cds->parSpreadFromHazardRate( valuationSettingsLVB, hazardRate, recoveryRate, premiumLegName, protectionLegName, includeAccruedInterest );

		// Record Outputs AND Return the Result for logs, tests and playback
        AQ_RECORD_OUTPUTS_AND_RETURN_RESULT( result );

		VALID_EXCEPTION_END
	}

	/* @brief Calculates the par spread of the specified Credit Default Swap.
	*
	* @param[in]	swapName				The name of the credit default swap object
	* @param[in]	creditModelName			Credit Model object name
	* @param[in]	premiumLegName			The Premium leg name of the CDS
	* @param[in]	protectionLegName		The Protection leg name of the CDS
	* @returns	The CDS par spread
	*/
	double tryAqCDSObjectParSpread( const std::string& swapName, const std::string& creditModelName, const AQLString& premiumLegName, const AQLString& protectionLegName )
	{
		VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        AQ_RECORD_INPUTS( swapName, creditModelName, premiumLegName, protectionLegName );

		auto swap = etrading::getSwap(swapName);

		auto cds = std::dynamic_pointer_cast<etrading::CreditDefaultSwap>( swap );
		if ( cds == nullptr )
		{
			AQ_THROW( "Swap '" + swapName + "' is not a credit default swap." );
		}
		
		auto creditModel = etrading::getCreditModel( creditModelName );

		double result = cds->parSpread( *creditModel, premiumLegName, protectionLegName );

		// Record Outputs AND Return the Result for logs, tests and playback
        AQ_RECORD_OUTPUTS_AND_RETURN_RESULT( result );

		VALID_EXCEPTION_END
	}

	/* @brief Solves for the hazard rate, given the specified CDS par spread
	*
	* @param[in]	swapName				The name of the credit default swap object
	* @param[in]	valuationSettingsLVB		A label value block containing a single collection name or a curveCollection per leg
	* @param[in]	parSpread				The CDS par spread ( as a decimal )
	* @param[in]	recoveryRate			The estimated amount of capital recovered after default
	* @param[in]	premiumLegName			The Premium leg name of the CDS
	* @param[in]	protectionLegName		The Protection leg name of the CDS
	* @param[in]	includeAccruedInterest	Specifies whether the accruedInterest should be included in the risky annuity
	* @returns	The CDS hazard rate
	*/
	double tryAqCDSObjectHazardRateFromParSpread( const std::string& swapName, const LabelValueBlock& valuationSettingsLVB, const double parSpread, const double recoveryRate, const AQLString& premiumLegName, const AQLString& protectionLegName, const bool includeAccruedInterest )
	{
		VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        AQ_RECORD_INPUTS( swapName, valuationSettingsLVB, parSpread, recoveryRate, premiumLegName, protectionLegName, includeAccruedInterest );

		auto swap = etrading::getSwap(swapName);

		auto cds = std::dynamic_pointer_cast<etrading::CreditDefaultSwap>( swap );
		if ( cds == nullptr )
		{
			AQ_THROW( "Swap '" + swapName + "' is not a credit default swap." );
		}
		
		double result = cds->hazardRateFromParSpread( valuationSettingsLVB, parSpread, recoveryRate, premiumLegName, protectionLegName, includeAccruedInterest );

		// Record Outputs AND Return the Result for logs, tests and playback
        AQ_RECORD_OUTPUTS_AND_RETURN_RESULT( result );

		VALID_EXCEPTION_END
	}

	/* @brief Solves for the hazard rate, given the specified CDS par spread
	*
	* @param[in]	swapName				The name of the credit default swap object
	* @param[in]	creditModelName			Credit Model object name
	* @param[in]	premiumLegName			The Premium leg name of the CDS
	* @param[in]	protectionLegName		The Protection leg name of the CDS
	* @returns	The CDS hazard rate
	*/
	double tryAqCDSObjectHazardRateFromParSpread( const std::string& swapName, const std::string& creditModelName, const double parSpread, const AQLString& premiumLegName, const AQLString& protectionLegName )
	{
		VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        AQ_RECORD_INPUTS( swapName, creditModelName, parSpread, premiumLegName, protectionLegName );

		auto swap = etrading::getSwap(swapName);

		auto cds = std::dynamic_pointer_cast<etrading::CreditDefaultSwap>( swap );
		if ( cds == nullptr )
		{
			AQ_THROW( "Swap '" + swapName + "' is not a credit default swap." );
		}
		
		auto creditModel = etrading::getCreditModel( creditModelName );

		double result = cds->hazardRateFromParSpread( parSpread, *creditModel, premiumLegName, protectionLegName );

		// Record Outputs AND Return the Result for logs, tests and playback
        AQ_RECORD_OUTPUTS_AND_RETURN_RESULT( result );

		VALID_EXCEPTION_END
	}

    /* @brief Creates a CreditModel, calibrated from CDS or bond quotes
	 * @param [in] objectName        The name of the Credit Model object to create
	 * @param [in] propertyNames     A vector of property names corresponding to each label-value block of properties.
	 *                               MODEL_PROPERTIES, ( CDS_MARKETDATA or BOND_MARKETDATA )
	 * @param [in] infoBlocks        A vector of containing the label-value blocks of properties
	 * @param [out]                  The objectName
	 */
    std::string tryAqCreditModelCreate( const std::string& objectName,
                                           const std::vector<std::string>& dataBlockNames,
                                           const etrading::JSONInfoBlockTuples& infoBlocks )
    {
		VALID_EXCEPTION_START
		
        // Perform initial basic sanity checks
        size_t nColumnHeaders   = dataBlockNames.size();
        size_t nDataColumns     = infoBlocks.size();
		
        AQ_REQUIRE( nDataColumns > 0, "Invalid InfoBlock Data: Empty InfoBlock - No data provided" )
        AQ_REQUIRE( nColumnHeaders == nDataColumns, "Invalid InfoBlock Data: Number of Data Column Headers " +  std::to_string(static_cast<long long>(nColumnHeaders)) + " does not match the actual number of Data Columns " +  std::to_string(static_cast<long long>(nDataColumns)) )

		// Recording of inputs for playback
		if (CreateDataFile::recordEnabled()) 
		{
			CreateDataFile file(decorateFilename("tryAqCreditModelCreate_inputs", objectName.c_str()));
			file.write("generatorFunction", "tryAqCreditModelCreate");
			file.write("objectName", objectName);

			// Write out each propertyName and corresponding block of property config data
			for( unsigned int i = 0; i < dataBlockNames.size(); i++ )
			{
				const AQLString dataBlockName( dataBlockNames[i].c_str() );
				const etrading::VariantMatrix& configData = std::get<2>( infoBlocks[ i ] );
				file.write( dataBlockName, transpose(configData) );
			}
		}

		// Verify that the supplied propertyNames have been set
        const bool hasAnEmptyName = std::any_of( dataBlockNames.cbegin(),
												 dataBlockNames.cend(),
												 []( const std::string & dataBlockName ) -> bool
        {
            return ( dataBlockName.empty() || dataBlockName == "" );
        } );

        AQ_REQUIRE( !hasAnEmptyName, "Invalid InfoBlock: Invalid Data Column Header - One of the Column Names is empty or invalid " + etrading::containerAsString( dataBlockNames ) )

		// Verify that the supplied propertyNames match the CreditModelEnum
		std::set<etrading::CreditModelEnum> enumSet;
		std::for_each(dataBlockNames.cbegin(),
					  dataBlockNames.cend(),
					  [&enumSet] (const std::string & dataBlockName )
		{	
			etrading::CreditModelEnum creditModelEnum = etrading::toCreditModelEnum( dataBlockName );
			enumSet.insert( creditModelEnum );
		});

		// Create the Credit Model object
		etrading::CreditModel creditModel( objectName, dataBlockNames, infoBlocks );

		// ..  and store in the cache
        etrading::copyToCache<etrading::CreditModel>( creditModel );

		if (CreateDataFile::recordEnabled()) 
		{
			CreateDataFile file(decorateFilename("tryAqCreditModelCreate_outputs", objectName.c_str()));
			file.write("output", objectName);
		}

        return objectName;
		
		VALID_EXCEPTION_END
	}

	/*  @brief Obtains the credit model asOf / valuation Date
	 *  @param[in] creditModelName	Credit Model object name
	 *  @returns The asOfDate.
	 */
	AQLDate tryAqCreditModelAsOfDate( const std::string& creditModelName )
	{
		VALID_EXCEPTION_START
	
		AQ_RECORD_INPUTS( creditModelName );
	
		auto creditModel = etrading::getCreditModel( creditModelName );
		AQLDate result = creditModel->getAsOfDate();

	    AQ_RECORD_OUTPUTS_AND_RETURN_RESULT( result );

		VALID_EXCEPTION_END
	}

	/* @brief Obtains the calibration parameters from the specified credit model
	 * @param[in] creditModelName	Credit Model object name
	 * @param[out]                  A matrix containing payment dates and hazard rates
	 */
	AnyTypeMatrix tryAqCreditModelCalibrationParameters( const std::string& creditModelName )
	{
		VALID_EXCEPTION_START
	
		AQ_RECORD_INPUTS( creditModelName );

		auto creditModel = etrading::getCreditModel( creditModelName );

		AnyTypeMatrix result = creditModel->getCalibrationParameters();

		 // Record Outputs AND Return the Result for logs, tests and playback
        AQ_RECORD_OUTPUTS_AND_RETURN_RESULT( result );

		VALID_EXCEPTION_END
	}

	double tryAqCreditModelHazardRate( const std::string& creditModelName, const AQLDate& paymentDate )
	{
		VALID_EXCEPTION_START
	
		AQ_RECORD_INPUTS( creditModelName, paymentDate );

		auto creditModel = etrading::getCreditModel( creditModelName );

		double result = creditModel->getHazardRate( paymentDate );

		 // Record Outputs AND Return the Result for logs, tests and playback
        AQ_RECORD_OUTPUTS_AND_RETURN_RESULT( result );

		VALID_EXCEPTION_END
	}

	double tryAqCreditModelSurvivalProbability( const std::string& creditModelName, const AQLDate& toDate, const AQLDate& fromDate )
	{
		VALID_EXCEPTION_START
	
		AQ_RECORD_INPUTS( creditModelName, toDate, fromDate );

		auto creditModel = etrading::getCreditModel( creditModelName );

		// User must specify a valid 'toDate'.
		if ( toDate == AQLDate() )
		{
			AQ_THROW( "Please specify a valid toDate.");
		}

		double result = creditModel->getSurvivalProbability( toDate, fromDate );

		 // Record Outputs AND Return the Result for logs, tests and playback
        AQ_RECORD_OUTPUTS_AND_RETURN_RESULT( result );

		VALID_EXCEPTION_END
	}

	// Compute the Survival Probability using the Credit Basket Model
	double tryAqCreditBasketModelSurvivalProbability( const std::string& creditBasketModelName, const AQLDate& toDate, const AQLDate& fromDate )
	{
		VALID_EXCEPTION_START
	
		AQ_RECORD_INPUTS( creditBasketModelName, toDate, fromDate );

		auto creditBasketModel = etrading::getCreditBasketModel( creditBasketModelName );

		// User must specify a valid 'toDate'.
		if ( toDate == AQLDate() )
		{
			AQ_THROW( "Please specify a valid toDate.");
		}

		double result = creditBasketModel->getFirstToDefaultHomogeneousBasketSurvivalProbability( toDate, fromDate );

		 // Record Outputs AND Return the Result for logs, tests and playback
        AQ_RECORD_OUTPUTS_AND_RETURN_RESULT( result );

		VALID_EXCEPTION_END
	}


	double tryAqCreditModelDefaultProbability( const std::string& creditModelName, const AQLDate& toDate, const AQLDate& fromDate )
	{
		VALID_EXCEPTION_START
	
		AQ_RECORD_INPUTS( creditModelName, toDate, fromDate );

		auto creditModel = etrading::getCreditModel( creditModelName );

		// User must specify a valid 'toDate'.
		if ( toDate == AQLDate() )
		{
			AQ_THROW( "Please specify a valid toDate.");
		}

		double result = creditModel->getDefaultProbability( toDate, fromDate );

		 // Record Outputs AND Return the Result for logs, tests and playback
        AQ_RECORD_OUTPUTS_AND_RETURN_RESULT( result );

		VALID_EXCEPTION_END
	}

	/* @brief	Given a survival probability, calculate the implied survival date from the specified credit model.
	*			i.e. this function is the inverse of getSurvivalProbability().
	*
	*  @param[in]	creditModelName			Credit Model object name
	*  @param[in]	survivalProbability		The input survivalProbability
	*
	*  @returns		The survival date corresponding to the input survivalProbability.
	*/ 
	AQLDate tryAqCreditModelImpliedSurvivalDate( const std::string& creditModelName, const double survivalProbability )
	{
		VALID_EXCEPTION_START
	
		AQ_RECORD_INPUTS( creditModelName, survivalProbability );

		auto creditModel = etrading::getCreditModel( creditModelName );

		AQLDate survivalDate = creditModel->getImpliedSurvivalDate( survivalProbability );
	    
		if ( CreateDataFile::recordEnabled() )
        {
            CreateDataFile file( "tryAqCreditModelImpliedSurvivalDate_outputs" );
            file.write( "output", survivalDate );
        }
        return survivalDate;

		VALID_EXCEPTION_END			
	}

    DoubleVector tryAqCreditModelRiskyDiscountFactors( const std::string& creditModelName, const DateVector& paymentDates )
	{
        VALID_EXCEPTION_START

        // Recording of inputs for playback
        AQ_RECORD_INPUTS( creditModelName, paymentDates )

        // Validate PaymentDate vector Size
        AQ_REQUIRE( paymentDates.size() > 0, "No payment dates specified.")

        // Load the Credit Model and Discount Curve
        auto creditModel                    = etrading::getCreditModel( creditModelName );
        std::string curveCollection         = creditModel->getCDSCurveCollection();
        AQLString curveCollectionAsAQLString  = AQLString( curveCollection.c_str() );


        // Discount Factor Calculation
        // ----------------------------------------------------------------------------------------------------

        // Discount Curve Information
        AQLString curveIndex( "OIS" );
        AQLString interp;
        AQLString busdayAdj( "NO_CHANGE" );
        AQLString calendar;
        populateDiscountFactorConventions( curveCollectionAsAQLString, curveIndex, interp, busdayAdj, calendar );

        // Get the Spot Discount Factors
        DoubleVector discountFactors = etrading::AQLCurveForwardRateHelpers::getMultiSpotDiscountFactors( paymentDates, etrading::getDataInstance(), curveCollectionAsAQLString, getDayCount(), busdayAdj, calendar, interp, isBasisFlag(), curveIndex );
        AQ_REQUIRE( discountFactors.size() > 0, "No discount factors have been returned" );
        AQ_REQUIRE( discountFactors.size() == paymentDates.size(), "Number of discount factors and payment dates do not match." )


        // Risky Discount Factor Calculation
        //
        // Mutliply the Risk-Free Discount Factor Results by the Credit Survival Probability
        // ----------------------------------------------------------------------------------------------------

        DoubleVector riskyDiscountFactors( discountFactors.size(), 0.0 );

        AQ_REQUIRE( discountFactors.size() == riskyDiscountFactors.size(), "Number of DiscountFactors and RiskyDiscountFactors do not match." )
        for( size_t i = 0; i < riskyDiscountFactors.size(); ++i )
        {
            // We must cast from gregorian date to AQLDate here
            AQLDate paymentDate = paymentDates[i]; 
            
            // User must specify a valid 'paymentDates'.
		    AQ_REQUIRE( paymentDates[i] != AQLDate(), "Invalid Payment Date(s)" );
        
            // Calculate survival probability from credit model 'asOfDate' to 'paymentDate'
            // Note the credit model checks that the asOfDate in the yieldCurve and creditModel is the same
		    double survivalProbability = creditModel->getSurvivalProbability( paymentDates[i] ); 

            // Calculate the risky discount factor
            riskyDiscountFactors[i] = discountFactors[i] * survivalProbability;
        }

         // Record Outputs AND Return the Result for logs, tests and playback
        AQ_RECORD_OUTPUTS_AND_RETURN_RESULT( riskyDiscountFactors );

		VALID_EXCEPTION_END
	}

	/* @brief validation interface for the aqCreditObjectSpread method.
	*  Calculates the Forward spread between start date and end date implied by the credit model.
	*  If the startDate is the model AsOf Date, returns the spot spread.
	*  See article "Credit Derivatives Handbook" by JPM p20-21.
	*
	* @param[in]	creditModelName		Credit Model object name
	* @param[in]	StartDate			The date on which credit protection begins
	* @param[in]	EndDate				The date on which credit protection ends
	* @returns	The calculated forward spread
	*/
	double tryAqCreditObjectSpread( const std::string& creditModelName, const AQLDate& startDate, const AQLDate& endDate )
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		AQ_RECORD_INPUTS(creditModelName, startDate, endDate);

		auto creditModel    = etrading::getCreditModel( creditModelName );
		const double result = creditModel->getForwardSpread( startDate, endDate );

		// Record Outputs AND Return the Result for logs, tests and playback
		AQ_RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END
	}

	double tryAqCreditObjectIndexSpread( const std::string& creditModelName, const AQLDate& startDate, const AQLDate& endDate )
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		AQ_RECORD_INPUTS( creditModelName, startDate, endDate );

		auto creditModel = etrading::getCreditModel( creditModelName );
		const double result = creditModel->getIndexForwardSpread( startDate, endDate );

		// Record Outputs AND Return the Result for logs, tests and playback
		AQ_RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END
	}

	/* @brief validation interface for the aqCreditObjectOptionPV method.
	*  Calculates the value of a credit option i.e. option on a credit default swap instrument.
	*  See article "The Valuation of Credit Default Swap Options" by Hull and White
	*
	* @param[in]	creditModelName		Credit Model object name
	* @param[in]	payerReceiver		Whether the option is a payer (PUT, the right to sell risk, pay CDS spread for protection)
	* 									or receiver (CALL, buy risk, receive CDS spread)
	* @param[in]	strike				The strike spread
	* @param[in]	optionExpiryDate	The expiry date of the option; the effective date of the underlying CDS
	* @param[in]	cdsMaturityDate		The maturity date of the underlying CDS
	* @param[in]	volatility			The volatility of the underlying CDS spread
	* @returns	The calculated option price
	*/
	double tryAqCreditObjectOptionPV( const std::string& creditModelName, const std::string& payerReceiver, const double strike, const AQLDate& optionExpiryDate, const AQLDate& cdsMaturityDate, const double volatility )
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		AQ_RECORD_INPUTS(creditModelName, payerReceiver, strike, optionExpiryDate, cdsMaturityDate, volatility );

		auto creditModel = etrading::getCreditModel(creditModelName);

		etrading::PayerReceiverSwaptionEnum payerReceiverSwaptionEnum = etrading::toPayerReceiverSwaptionEnum( payerReceiver );

		const double result = creditModel->getSingleNameKnockoutOptionValue( payerReceiverSwaptionEnum, strike, optionExpiryDate, cdsMaturityDate, volatility );

		// Record Outputs AND Return the Result for logs, tests and playback
		AQ_RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END
	}

	/* @brief validation interface for the aqCreditObjectOptionPVFromForward method.
	*  Calculates the value of a credit option i.e. option on a CDS instrument
	*  See article "The Valuation of Credit Default Swap Options" by Hull and White
	*
	* @param[in]	creditModelName				Credit Model object name
	* @param[in]	payerReceiverSwaptionEnum	Whether the option is a payer (PUT, the right to sell risk, pay CDS spread for protection)
	* 											or receiver (CALL, buy risk, receive CDS spread)
	* @param[in]	strike						The strike spread
	* @param[in]	optionExpiryDate			The expiry date of the option; the effective date of the underlying CDS
	* @param[in]	cdsMaturityDate				The maturity date of the underlying CDS
	* @param[in]	volatility					The volatility of the underlying CDS spread
	* @param[in]	forwardSpread				The forward spread at the option expiry date
	* @returns	The calculated option price
	*/
	double tryAqCreditObjectOptionPVFromForward(const std::string& creditModelName, const std::string& payerReceiver, const double strike, const AQLDate& optionExpiryDate, const AQLDate& cdsStartDate, const AQLDate& cdsMaturityDate, const double volatility, const double forwardSpread)
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		AQ_RECORD_INPUTS(creditModelName, payerReceiver, strike, optionExpiryDate, cdsStartDate, cdsMaturityDate, volatility, forwardSpread );

		auto creditModel = etrading::getCreditModel(creditModelName);

		etrading::PayerReceiverSwaptionEnum payerReceiverSwaptionEnum = etrading::toPayerReceiverSwaptionEnum(payerReceiver);

		const double result = creditModel->getOptionValueFromForward( payerReceiverSwaptionEnum, strike, optionExpiryDate, cdsStartDate, cdsMaturityDate, volatility, forwardSpread );

		// Record Outputs AND Return the Result for logs, tests and playback
		AQ_RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END
	}

	/* @brief validation interface for the aqCreditObjectIndexOptionPV method.
	*  Calculates the value of a credit index option i.e. option on a CDS index
	*  See Bloomberg Whitepaper "Pricing Credit Index Options", March 1st, 2012
	*
	* @param[in]	creditModelName				Credit Model object name
	* @param[in]	optionLVB					Option parameters such as strike, expiryDate, cdsCoupon, volatility
	* @returns	The calculated option price
	*/
	double tryAqCreditObjectIndexOptionPV( const std::string& creditModelName, const etrading::LabelValueBlock& optionLVB )
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		AQ_RECORD_INPUTS(creditModelName, optionLVB );

		auto creditModel = etrading::getCreditModel(creditModelName);

		const double result = creditModel->getIndexOptionValue( optionLVB );

		// Record Outputs AND Return the Result for logs, tests and playback
		AQ_RECORD_OUTPUTS_AND_RETURN_RESULT( result );

		VALID_EXCEPTION_END
	}

	/* @brief  validation interface for the aqCreditObjectIndexOptionImpliedVol method.
	*  Calculates the implied vol of a credit index option, given a target CDS index option quote and CDS index forward at option expiry
	*  See Bloomberg Whitepaper "Pricing Credit Index Options", March 1st, 2012
	*
	* @param[in]	creditModelName				Credit Model object name
	* @param[in]	optionLVB					Option parameters such as strike, expiryDate, cdsCoupon, volatility
	* @returns	The implied volatility
	*/
	double tryAqCreditObjectIndexOptionImpliedVol( const std::string& creditModelName, const etrading::LabelValueBlock& optionLVB )
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		AQ_RECORD_INPUTS(creditModelName, optionLVB );

		auto creditModel = etrading::getCreditModel( creditModelName );

		const double result = creditModel->getIndexOptionImpliedVol( optionLVB );

		// Record Outputs AND Return the Result for logs, tests and playback
		AQ_RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END
	}

	/* @brief validation interface for the aqCreditObjectIndexOptionVega method.
	*  Calculates the value of a credit index option i.e. option on a CDS index
	*  See Bloomberg Whitepaper "Pricing Credit Index Options", March 1st, 2012
	*
	* @param[in]	creditModelName				Credit Model object name
	* @param[in]	optionLVB					Option parameters such as strike, expiryDate, cdsCoupon, volatility
	* @param[in]	volatilityBump				The volatility bump size
	* @returns	The calculated option vega
	*/
	double tryAqCreditObjectIndexOptionVega( const std::string& creditModelName, const etrading::LabelValueBlock& optionLVB, const double volatilityBump )
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		AQ_RECORD_INPUTS(creditModelName, optionLVB, volatilityBump );

		auto creditModel = etrading::getCreditModel(creditModelName);

		const double result = creditModel->getIndexOptionVega( optionLVB, volatilityBump );

		// Record Outputs AND Return the Result for logs, tests and playback
		AQ_RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END
	}

	/* @brief validation interface for the aqCreditObjectIndexOptionCS01 method.
	*  Calculates the value of a credit index option i.e. option on a CDS index
	*  See Bloomberg Whitepaper "Pricing Credit Index Options", March 1st, 2012
	*
	* @param[in]	creditModelName				Credit Model object name
	* @param[in]	optionLVB					Option parameters such as strike, expiryDate, cdsCoupon, volatility
	* @returns	The calculated option CS01
	*/
	double tryAqCreditObjectIndexOptionCS01( const std::string& creditModelName, const etrading::LabelValueBlock& optionLVB )
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		AQ_RECORD_INPUTS(creditModelName, optionLVB);

		auto creditModel = etrading::getCreditModel(creditModelName);

		const double result = creditModel->getIndexOptionCS01( optionLVB );

		// Record Outputs AND Return the Result for logs, tests and playback
		AQ_RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END

	}
	
	/* @brief validation interface for the aqCreditObjectIndexOptionTheta method.
	*  Calculates the value of a credit index option i.e. option on a CDS index
	*  See Bloomberg Whitepaper "Pricing Credit Index Options", March 1st, 2012
	*
	* @param[in]	creditModelName				Credit Model object name
	* @param[in]	optionLVB					Option parameters such as strike, expiryDate, cdsCoupon, volatility
	* @returns	The calculated option theta
	*/
	double tryAqCreditObjectIndexOptionTheta( const std::string& creditModelName, const etrading::LabelValueBlock& optionLVB )
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		AQ_RECORD_INPUTS(creditModelName, optionLVB);

		auto creditModel = etrading::getCreditModel(creditModelName);

		const double result = creditModel->getIndexOptionTheta( optionLVB );

		// Record Outputs AND Return the Result for logs, tests and playback
		AQ_RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END
	}

	/* @brief Calculates the implied vol of a credit option, given a target CDS option quote
	*  See article "The Valuation of Credit Default Swap Options" by Hull and White
	*
	* @param[in]	creditModelName		Credit Model object name
	* @param[in]	payerReceiver		Whether the option is a payer (PUT, the right to sell risk, pay CDS spread for protection)
	* 									or receiver (CALL, buy risk, receive CDS spread)
	* @param[in]	strike				The strike spread
	* @param[in]	optionExpiryDate	The expiry date of the option; the effective date of the underlying CDS
	* @param[in]	cdsMaturityDate		The maturity date of the underlying CDS
	* @param[in]	targetOptionValue	Calculate the implied vol for this target option value
	* @returns	The implied volatility
	*/
	double tryAqCreditObjectOptionImpliedVol( const std::string& creditModelName, const std::string& payerReceiver, const double strike, const AQLDate& optionExpiryDate, const AQLDate& cdsMaturityDate, const double targetOptionValue )
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		AQ_RECORD_INPUTS( creditModelName, payerReceiver, strike, optionExpiryDate, cdsMaturityDate, targetOptionValue );

		auto creditModel = etrading::getCreditModel(creditModelName);

		etrading::PayerReceiverSwaptionEnum payerReceiverSwaptionEnum = etrading::toPayerReceiverSwaptionEnum( payerReceiver );

		const double result = creditModel->getImpliedVol( payerReceiverSwaptionEnum, strike, optionExpiryDate, cdsMaturityDate, targetOptionValue );

		// Record Outputs AND Return the Result for logs, tests and playback
		AQ_RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END
	}

	/* @brief Calculates the implied vol of a credit option, given a target CDS option quote and a CDS forward spread.
	*  See article "The Valuation of Credit Default Swap Options" by Hull and White
	*
	* @param[in]	creditModelName		Credit Model object name
	* @param[in]	payerReceiver		Whether the option is a payer (PUT, the right to sell risk, pay CDS spread for protection)
	* 									or receiver (CALL, buy risk, receive CDS spread)
	* @param[in]	strike				The strike spread
	* @param[in]	optionExpiryDate	The expiry date of the option; the effective date of the underlying CDS
	* @param[in]	cdsStartDate		If the option is exercised, the date on which the underlying CDS is delivered
	* @param[in]	cdsMaturityDate		The maturity date of the underlying CDS
	* @param[in]	targetOptionValue	Calculate the implied vol for this target option value
	* @param[in]	forwardSpread		The CDS forward spread at the option expiry date
	* @returns	The implied volatility
	*/
	double tryAqCreditObjectOptionImpliedVolFromForward( const std::string& creditModelName, const std::string& payerReceiver, const double strike, const AQLDate& optionExpiryDate, const AQLDate& cdsStartDate, const AQLDate& cdsMaturityDate, const double targetOptionValue, const double forwardSpread )
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		AQ_RECORD_INPUTS(creditModelName, payerReceiver, strike, optionExpiryDate, cdsStartDate, cdsMaturityDate, targetOptionValue, forwardSpread);

		auto creditModel = etrading::getCreditModel(creditModelName);

		etrading::PayerReceiverSwaptionEnum payerReceiverSwaptionEnum = etrading::toPayerReceiverSwaptionEnum(payerReceiver);

		const double result = creditModel->getImpliedVolFromForward( payerReceiverSwaptionEnum, strike, optionExpiryDate, cdsStartDate, cdsMaturityDate, targetOptionValue, forwardSpread );

		// Record Outputs AND Return the Result for logs, tests and playback
		AQ_RECORD_OUTPUTS_AND_RETURN_RESULT(result);

		VALID_EXCEPTION_END
	}


    /* @brief Creates a Credit Basket-Model, constructed from underlying Credit Models
	 * @param [in] objectName        The name of the Credit Basket-Model object to create
	 * @param [in] propertyNames     A vector of property names corresponding to each label-value block of properties.
	 *                               MODEL_PROPERTIES, CREDIT_MODELS.
	 * @param [in] infoBlocks        A vector of containing the label-value blocks of properties.
	 *                               The CREDIT_MODELS block consists of CreditModelName and correlationBeta.
	 * @param [out]                  The objectName
	 */
    std::string tryAqCreditBasketModelCreate( const std::string& objectName,
												 const std::vector<std::string>& dataBlockNames,
												 const etrading::JSONInfoBlockTuples& infoBlocks )
    {
		VALID_EXCEPTION_START
		
        // Perform initial basic sanity checks
        size_t nColumnHeaders   = dataBlockNames.size();
        size_t nDataColumns     = infoBlocks.size();
		
        AQ_REQUIRE( nDataColumns > 0, "Invalid InfoBlock Data: Empty InfoBlock - No data provided" )
        AQ_REQUIRE( nColumnHeaders == nDataColumns, "Invalid InfoBlock Data: Number of Data Column Headers " +  std::to_string(static_cast<long long>(nColumnHeaders)) + " does not match the actual number of Data Columns " +  std::to_string(static_cast<long long>(nDataColumns)) )

		// Recording of inputs for playback
		if (CreateDataFile::recordEnabled()) 
		{
			CreateDataFile file(decorateFilename("tryAqCreditBasketModelCreate_inputs", objectName.c_str()));
			file.write("generatorFunction", "tryAqCreditBasketModelCreate");
			file.write("objectName", objectName);

			// Write out each propertyName and corresponding block of property config data
			for( unsigned int i = 0; i < dataBlockNames.size(); i++ )
			{
				const AQLString dataBlockName( dataBlockNames[i].c_str() );
				const etrading::VariantMatrix& configData = std::get<2>( infoBlocks[ i ] );
				file.write( dataBlockName, transpose(configData) );
			}
		}

		// Verify that the supplied propertyNames have been set
        const bool hasAnEmptyName = std::any_of( dataBlockNames.cbegin(),
												 dataBlockNames.cend(),
												 []( const std::string & dataBlockName ) -> bool
        {
            return ( dataBlockName.empty() || dataBlockName == "" );
        } );

        AQ_REQUIRE( !hasAnEmptyName, "Invalid InfoBlock: Invalid Data Column Header - One of the Column Names is empty or invalid " + etrading::containerAsString( dataBlockNames ) )

		// Verify that the supplied propertyNames match the CreditModelEnum
		std::set<etrading::CreditModelEnum> enumSet;
		std::for_each(dataBlockNames.cbegin(),
					  dataBlockNames.cend(),
					  [&enumSet] (const std::string & dataBlockName )
		{	
			etrading::CreditModelEnum creditModelEnum = etrading::toCreditModelEnum( dataBlockName );
			enumSet.insert( creditModelEnum );
		});

		// Create the Credit Basket Model object
		etrading::CreditBasketModel creditBasketModel( objectName, dataBlockNames, infoBlocks );

		// ..  and store in the cache
        etrading::copyToCache<etrading::CreditBasketModel>( creditBasketModel );

		if (CreateDataFile::recordEnabled()) 
		{
			CreateDataFile file(decorateFilename("tryAqCreditBasketModelCreate_outputs", objectName.c_str()));
			file.write("output", objectName);
		}

        return objectName;
		
		VALID_EXCEPTION_END
	}


	/* @brief validation interface for the aqObjConstantMaturitySwapPVFromConvexity() method.
	*   Calculates the PV of a Constant Maturity Swap.
	*
	* @param[in]	swapName			Swap object name
	* @param[in]	valuationSettingsLVB	A label value block containing a single collection name or a curveCollection per leg
	* @param[in]	convexityAdjustment	The convexity adjustment which is added to the PV of the CMS leg.
	* @param[in]	legName			    If specified, calculate the PV of the single swap leg
	* @param[in]	fixingTableNames	Fixing table object names
	* @returns	The calculated PV value
	*/
	double tryAqCMSObjectPVUsingConvexityAdjustment( const std::string& swapName, const LabelValueBlock& valuationSettingsLVB, const double convexityAdjustment, const AQLString& legName, const LabelValueBlock& fixingTableNames )
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
        AQ_RECORD_INPUTS( swapName, valuationSettingsLVB, convexityAdjustment, legName, fixingTableNames );

		auto swap = etrading::getSwap(swapName);

		if (swap->getSwapType() != etrading::CONSTANT_MATURITY_SWAP )
		{
			AQ_THROW( "Swap '" + swapName + "' has incorrect Swap Type of '" + toString( swap->getSwapType() ) + "'. It is not a constant maturity swap." );
		}
		auto cms = std::dynamic_pointer_cast<etrading::ConstantMaturitySwap>( swap );
		if ( cms == nullptr )
		{
			AQ_THROW( "Swap '" + swapName + "' is not a constant maturity swap." );
		}

		double result = cms->pvUsingConvexityAdjustment( valuationSettingsLVB, convexityAdjustment, fixingTableNames, legName);
		        
		// Record Outputs AND Return the Result for logs, tests and playback
        AQ_RECORD_OUTPUTS_AND_RETURN_RESULT( result );

		VALID_EXCEPTION_END	
	}

	/* @brief validation interface for the aqObjConstantMaturitySwapParRateFromConvexity() method.
	*   Calculates the ParRate of a Constant Maturity Swap.
	*
	* @param[in]	swapName			Swap object name
	* @param[in]	valuationSettingsLVB	A label value block containing a single collection name or a curveCollection per leg
	* @param[in]	convexityAdjustment	The convexity adjustment which is added to the PV of the CMS leg.
	* @param[in]	fixingTableNames	Fixing table object names
	* @returns	The calculated ParRate value
	*/
	double tryAqCMSObjectParRateUsingConvexityAdjustment( const std::string& swapName, const LabelValueBlock& valuationSettingsLVB, const double convexityAdjustment, const LabelValueBlock& fixingTableNames )
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
        AQ_RECORD_INPUTS( swapName, valuationSettingsLVB, convexityAdjustment, fixingTableNames );

		auto swap = etrading::getSwap(swapName);

		if (swap->getSwapType() != etrading::CONSTANT_MATURITY_SWAP )
		{
			AQ_THROW( "Swap '" + swapName + "' has incorrect Swap Type of '" + toString( swap->getSwapType() ) + "'. It is not a constant maturity swap." );
		}
		auto cms = std::dynamic_pointer_cast<etrading::ConstantMaturitySwap>( swap );
		if ( cms == nullptr )
		{
			AQ_THROW( "Swap '" + swapName + "' is not a constant maturity swap." );
		}

		double result = cms->parRateUsingConvexityAdjustment( valuationSettingsLVB, convexityAdjustment, fixingTableNames);

		// Record Outputs AND Return the Result for logs, tests and playback
        AQ_RECORD_OUTPUTS_AND_RETURN_RESULT( result );

		VALID_EXCEPTION_END	
	
	}

	/* @brief validation interface for the aqTRSObjectPV method.
	*   Calculates the PV of the specified Total Return Swap.
	*
	* @param[in]	swapName			The name of the total return swap object
	* @param[in]	creditModelName		Credit Model object name
	* @param[in]	legName			    If specified, calculate the PV of the single swap leg
	* @param[in]	fixingTableNames	Fixing table object names
	* @returns	The calculated PV value
	*/
	double tryAqTRSObjectPV( const std::string& swapName, const std::string& creditModelName,  const std::string& legName, const LabelValueBlock& fixingTableNames )
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
        AQ_RECORD_INPUTS( swapName, creditModelName, legName, fixingTableNames );

		auto swap = etrading::getSwap( swapName );

		if (swap->getSwapType() != etrading::TOTAL_RETURN_SWAP )
		{
			AQ_THROW( "Swap '" + swapName + "' has incorrect Swap Type of '" + toString( swap->getSwapType() ) + "'. It is not a total return swap." );
		}
		auto trs = std::dynamic_pointer_cast<etrading::TotalReturnSwap>( swap );
		if ( trs == nullptr )
		{
			AQ_THROW( "Swap '" + swapName + "' is not a total return swap." );
		}

		double result = trs->pv( creditModelName, fixingTableNames, legName);
		        
		// Record Outputs AND Return the Result for logs, tests and playback
        AQ_RECORD_OUTPUTS_AND_RETURN_RESULT( result );

		VALID_EXCEPTION_END
	}

	/* @brief Calculates the par spread of the specified Total Return Swap.
	*
	* @param[in]	swapName				The name of the total return swap object
	* @param[in]	creditModelName			Credit Model object name
	* @param[in]	fixingTableNames		Fixing table object names
	* @returns	The par spread of the TRS premium leg
	*/
	double tryAqTRSObjectParRate( const std::string& swapName, const std::string& creditModelName, const LabelValueBlock& fixingTableNames )
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
        AQ_RECORD_INPUTS( swapName, creditModelName, fixingTableNames );

		auto swap = etrading::getSwap( swapName );

		if (swap->getSwapType() != etrading::TOTAL_RETURN_SWAP )
		{
			AQ_THROW( "Swap '" + swapName + "' has incorrect Swap Type of '" + toString( swap->getSwapType() ) + "'. It is not a total return swap." );
		}
		auto trs = std::dynamic_pointer_cast<etrading::TotalReturnSwap>( swap );
		if ( trs == nullptr )
		{
			AQ_THROW( "Swap '" + swapName + "' is not a total return swap." );
		}

		double result = trs->parRate( creditModelName, fixingTableNames);
		        
		// Record Outputs AND Return the Result for logs, tests and playback
        AQ_RECORD_OUTPUTS_AND_RETURN_RESULT( result );

		VALID_EXCEPTION_END
	}

	/* @brief Calculates the par spread of the specified Total Return Swap's float leg.
	*
	* @param[in]	swapName				The name of the total return swap object
	* @param[in]	creditModelName			Credit Model object name
	* @param[in]	fixingTableNames		Fixing table object names
	* @returns	The par spread of the TRS float leg
	*/
	double tryAqTRSObjectParSpread( const std::string& swapName, const std::string& creditModelName, const LabelValueBlock& fixingTableNames )
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
        AQ_RECORD_INPUTS( swapName, creditModelName, fixingTableNames );

		auto swap = etrading::getSwap( swapName );

		if (swap->getSwapType() != etrading::TOTAL_RETURN_SWAP )
		{
			AQ_THROW( "Swap '" + swapName + "' has incorrect Swap Type of '" + toString( swap->getSwapType() ) + "'. It is not a total return swap." );
		}
		auto trs = std::dynamic_pointer_cast<etrading::TotalReturnSwap>( swap );
		if ( trs == nullptr )
		{
			AQ_THROW( "Swap '" + swapName + "' is not a total return swap." );
		}

		double result = trs->spread( creditModelName, fixingTableNames);
		        
		// Record Outputs AND Return the Result for logs, tests and playback
        AQ_RECORD_OUTPUTS_AND_RETURN_RESULT( result );

		VALID_EXCEPTION_END
	}

	/* @brief validation interface for the aqTRSObjectPV method.
	*	Calculates the annity of the specified Total Return Swap leg
	*
	* @param[in]	swapName			The name of the total return swap object
	* @param[in]	creditModelName		Credit Model object name
	* @param[in]	legName			    Mandatory, calculate the annuity of this leg.
	* @returns	The calculated annuity value
	*/
	double tryAqTRSObjectAnnuity( const std::string& swapName, const std::string& creditModelName,  const std::string& legName )
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
        AQ_RECORD_INPUTS( swapName, creditModelName, legName );

		auto swap = etrading::getSwap( swapName );

		if (swap->getSwapType() != etrading::TOTAL_RETURN_SWAP )
		{
			AQ_THROW( "Swap '" + swapName + "' has incorrect Swap Type of '" + toString( swap->getSwapType() ) + "'. It is not a total return swap." );
		}
		auto trs = std::dynamic_pointer_cast<etrading::TotalReturnSwap>( swap );
		if ( trs == nullptr )
		{
			AQ_THROW( "Swap '" + swapName + "' is not a total return swap." );
		}

		double result = trs->annuity( creditModelName, legName );
		        
		// Record Outputs AND Return the Result for logs, tests and playback
        AQ_RECORD_OUTPUTS_AND_RETURN_RESULT( result );

		VALID_EXCEPTION_END
	}

}
