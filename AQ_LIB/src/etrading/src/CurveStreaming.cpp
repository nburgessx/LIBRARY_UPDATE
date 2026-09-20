// CurveStreaming.cpp

/*
 * @brief			Curve Streaming Methods
 */
#include "CurveStreaming.h"

#include "CurveValidation.h"
#include "CurveBuildDefaults.h"
#include "InitializeETrading.h"
#include "ParameterValidation.h"
#include "AQLDefinitions.h"
#include "AQLDateSchedule.h"
#include "AQLCurveForwardRateHelpers.h"
#include "AQLCurveCalibrationHelpers.h"
#include "AQLPriceDataCalendar.h"
#include "CurveCalibrationData.h"
#include "AQLMarketData.h"
#include "AQLPriceDataInterpolation.h"
#include "CommonConstants.h"
#include "AQLStaticData.h"
#include "ExceptionMacros.h"
#include "Solvers.h"
#include "CurveUtilities.h"
#include "SimpsonsRuleIntegration.h"
#include "CurveResultsContainer.h"
#include "DiscountFactorResults.h"

#include <cctype>
#include <utility>
#include <boost/assign.hpp>
#include <set>
#include <numeric>



namespace etrading
{
	/* @brief			setCurveForwardRates function, which overrides curve discount factors making them equivalent to the forwards provided
	*  @param [in]		curveCollection		                The curveCollection to use when accessing the curveIndices
	*  @param [in]		curveIndex			                The curveindex within the curve
	*  @param [in]		fixingDates                         The fixing dates - Note Fixing- and PaymentDates are just date labels and are equivalent
	*  @param [in]		forwardRates		                The new forward rates
    *  @param [in]		setCorrespondingDiscountFactors	    Set the corresponding discount factors (for STD curves only)
	*/
    AQLString setCurveForwardRates( const AQLString& curveCollection,
                                   const AQLString& curveIndex,
                                   const DateVector& fixingDates,
                                   const DoubleVector& forwardRates,
                                   const bool setCorrespondingDiscountFactors )
    {
        AQ_REQUIRE( curveCollection.size() != 0, "Missing Curve Collection" )
        AQ_REQUIRE( curveIndex.size() != 0, "Missing Curve Index" )

		AQLString result;
		
		// Get the CurveIndices alias list
		const AQLStringVector curveIndices = curveIndexAliasList( curveCollection, curveIndex );

		// Check if a STD curve
		const bool isSTDSwapCurve = etrading::isSTDCurve( curveCollection, curveIndex );

        if( isSTDSwapCurve )
        {
            // *** STD Curve ***
            // Set Forwards Rates & Imply and Set Equivalent Discount Factors
            // -------------------------------------------
            
            // Build the ForwardRatesTable Struct
            DoubleVector termStarts = convertCurveDatesToTerms( curveCollection, fixingDates ); 
            
            // Calculate TermEnds
            DoubleVector termEnds( termStarts.size() );
            const double curveFrequency = etrading::getCurveFrequencyAsYearFraction( curveCollection, curveIndex );
            for( size_t i = 0; i < termStarts.size(); ++i )
            {
                termEnds[i] = termStarts[i] + curveFrequency;
            } 
            
            ForwardRateTable forwardRateTable;
            forwardRateTable.forwardStartTerms_ = termStarts;
            forwardRateTable.forwardEndTerms_   = termEnds;
            forwardRateTable.forwardRates_      = forwardRates;
            
            // Set the Forward Rates Table
            etrading::AQLCurveCalibrationHelpers::setCurveForwardRateTable( etrading::getDataInstance(), curveCollection, curveIndices, forwardRateTable );

            if ( setCorrespondingDiscountFactors )
            {
                // *** Set Forward Equivalent Discount Factors ***
                // ------------------------------------------------
                
                // Solve for equivalent discount factors that imply the forward rates provided and set the curve discount factors
                const EquivalentDiscountFactors::SolverResults solverResults = setForwardRateEquivalentDiscountFactors( fixingDates, forwardRates, curveCollection, curveIndex );
                AQLString nIterations( (int) solverResults.numberOfIterations_ );

                // Return Extra Solver Result Information in the case where we solve for equivalent discount factors
                result = "Forward rates set for curve " + curveCollection + ", curve index " + curveIndex + ". Implied Discount Factors Solved in " + nIterations + " Iteration(s)";
                
            }
            else
            {
                // *** Set Dummy Discount Factors ***
                // ------------------------------------------------

                // Set Discount Factors to have the same dimensions as the forward rates with a placeholder value of 1.0
                DoubleVector dummyDiscountFactors( fixingDates.size(), 1.0 );
            
                DiscountFactorTable discountFactorTable;
                discountFactorTable.terms_ = termStarts;
                discountFactorTable.discountFactors_ = dummyDiscountFactors;
            
                DateVector paymentDates = convertCurveTermsToDates( curveCollection, termEnds );
                setCurveDiscountFactors( curveCollection, curveIndex, paymentDates, dummyDiscountFactors, false ); // False = Don't set corresponding forwards, since this would make the function circular

				// Set Discount Factors in the Curve Results Object
				if( etrading::isEnabledCurveResults() && etrading::doesExistCurveResultsDiscountFactors( curveCollection.c_str(), curveIndex.c_str() ) )
				{
					AQ_REQUIRE( termEnds.size() == dummyDiscountFactors.size(), "Invalid Discount Factors, The number of payment dates and discount factors do not match." )
					for( AQLString thisIndex : curveIndices )
					{
						etrading::CurveResultsContainer::getInstance().getCurveResults( curveCollection.c_str(), thisIndex.c_str() )->discountFactorResults()->setDiscountFactorsUsingTerms( termEnds, dummyDiscountFactors );
					}
				}

				result = "Forward rates set for curve " + curveCollection + ", curve index " + curveIndex;
            }
        }
        else
        {
            // Non-STD Curve
            // Set Equivalent Discount Factors - No Forwards Available to Set on Non-STD Curves
            // --------------------------------------------------------------------------------

            // Solve for equivalent discount factors that imply the forward rates provided and set the curve discount factors
            const EquivalentDiscountFactors::SolverResults solverResults = setForwardRateEquivalentDiscountFactors( fixingDates, forwardRates, curveCollection, curveIndex );
            AQLString nIterations( (int) solverResults.numberOfIterations_ );

			// Return Extra Solver Result Information in the case where we solve for equivalent discount factors
			result = "Forward rates set for curve " + curveCollection + ", curve index " + curveIndex + ". Implied Discount Factors Solved in " + nIterations + " Iteration(s)";
        }

		// Notify user of result
        return result;
    }

    /* @brief			setCurveDiscountFactors function, which overrides curve discount factors
	*  @param [in]		curveCollection		        The curveCollection to use when accessing the curveIndices
	*  @param [in]		curveIndex			        The curveindex within the curve
	*  @param [in]		paymentDates                The payment dates
	*  @param [in]		discountFactors		        The new discount factors
    *  @param [in]		setCorrespondingForwards	Set the corresponding forwards (for STD curves only)
	*/
    AQLString setCurveDiscountFactors( const AQLString& curveCollection,
                                      const AQLString& curveIndex,
                                      const DateVector& paymentDates,
                                      const DoubleVector& discountFactors,
                                      const bool setCorrespondingForwards )
    {
        DiscountFactorTable dfInputTable;
        dfInputTable.paymentDates_      = paymentDates;
        dfInputTable.terms_             = convertCurveDatesToTerms( curveCollection, paymentDates ); 
        dfInputTable.discountFactors_   = discountFactors;
       
        AQ_REQUIRE( dfInputTable.paymentDates_.size() ==  dfInputTable.terms_.size(),     "Invalid Curve Inputs - Inconsistent number of dates and terms" );
        AQ_REQUIRE( dfInputTable.terms_.size() ==  dfInputTable.discountFactors_.size(),  "Invalid Curve Inputs - Inconsistent number of terms and discount factors" );
        AQ_REQUIRE( !dfInputTable.discountFactors_.empty(),                               "Invalid Curve Inputs - Missing Discount Factors, there are no discount factors to set" )

        // Check Terms Data is sorted with no duplicates
        AQ_REQUIRE( isTermsDataSortedWithNoDuplicates( dfInputTable.terms_ ), "Invalid Payment Dates - Payment Dates must be sorted in ascending order with no duplicates" )

        // Check that we have no negative terms date year fractions
        for( size_t i=0; i < dfInputTable.terms_.size(); ++i )
        {
            AQ_REQUIRE( AQ_IS_GREATER_THAN_OR_EQUAL_TO_ZERO( dfInputTable.terms_[i] ), "Invalid Discount Factors - Cannot set discount factors that are in the past, before the curve 'AsOfDate'." )
        }

        // Set the Discount Factors to the Curve - No dates are used here, just terms and discount factors.
        const AQLStringVector curveIndices = curveIndexAliasList( curveCollection, curveIndex );
        etrading::AQLCurveCalibrationHelpers::setCurveDiscountFactorTable( etrading::getDataInstance(), curveCollection, curveIndices,  dfInputTable );

        // Imply and Set Equivalent Forward Rates on STD Curve
        const bool isSTDSwapCurve = etrading::isSTDCurve( curveCollection, curveIndex );
        if ( isSTDSwapCurve )
        {
            if ( setCorrespondingForwards )
            {
                implyAndSetForwardRatesFromDiscountFactors( curveCollection, curveIndex, paymentDates, discountFactors );
            }
            else
            {
                // Set-Dummy Forward Rates of 0.0% corresponding to Discount Factors of 1.0
                DoubleVector dummyDiscountFactors( discountFactors.size(), 1.0 );
                implyAndSetForwardRatesFromDiscountFactors( curveCollection, curveIndex, paymentDates, dummyDiscountFactors );
            }
        }

		// Set Discount Factors in the Curve Results Object
		if( etrading::isEnabledCurveResults() && etrading::doesExistCurveResultsDiscountFactors( curveCollection.c_str(), curveIndex.c_str() ) )
		{
			for( AQLString thisIndex : curveIndices )
			{
				etrading::CurveResultsContainer::getInstance().getCurveResults( curveCollection.c_str(), thisIndex.c_str() )->discountFactorResults()->setDiscountFactors( paymentDates, discountFactors );
			}
		}

        AQLString result =  "Discount Factors set for curve " + curveCollection + ", curve index " + curveIndex;
        return result;
    }

    /* @brief			setCurveDiscountFactorsToOne function, which sets all discount factors to one
	*  @param [in]		curveCollection		The curveCollection to use when accessing the curveIndices
	*  @param [in]		curveIndex			The curveindex within the curve
	*/
    AQLString setCurveDiscountFactorsToOne( const AQLString& curveCollection, const AQLString& curveIndex )
    {
		// Get the existing discount factor dates and values, then reset the values to one
        DiscountFactorTable results = etrading::AQLCurveCalibrationHelpers::getCurveDiscountFactorTable( etrading::getDataInstance(), curveCollection, curveIndex );
        
        // Reset the Existing Discount Factors to One
        DoubleVector newDiscountFactors( results.terms_.size(), 1.0 );
        results.discountFactors_ = newDiscountFactors;
        setCurveDiscountFactors( curveCollection, curveIndex, results.paymentDates_, results.discountFactors_ );

		// No Need to Set Discount Factors in the Curve Results Object since this is done in the underlying setCurveDiscountFactors() method
		
        AQLString result = "Discount Factors set to ONE for curve " + curveCollection + ", curve index " + curveIndex;
        return result;
    }
    
    /* @brief			getCurveDiscountFactors function, which retrieves all discount factors
	*  @param [in]		curveCollection		    The curveCollection to use when accessing the curveIndices
	*  @param [in]		curveIndex			    The curveindex within the curve
	*  @param [out]		DiscountFactorTable     A discount factor table structure that contains paymentDates_ and discountFactors_
    */
    DiscountFactorTable getCurveDiscountFactors( const AQLString& curveCollection, const AQLString& curveIndex )
    {
		// Discount Factor Results are returned as an of array terms and discount factors 
        DiscountFactorTable results = etrading::AQLCurveCalibrationHelpers::getCurveDiscountFactorTable( etrading::getDataInstance(), curveCollection, curveIndex );
        return results;
    }

    /* @brief			getCurveForwardRates function, which retrieves all forward rates
	*  @param [in]		curveCollection		    The curveCollection to use when accessing the curveIndices
	*  @param [in]		curveIndex			    The curveindex within the curve
	*  @param [out]		ForwardRateTable        A forward rate table structure that contains fixingDates_ and forwardRates_
    */
    ForwardRateTable getCurveForwardRates( const AQLString& curveCollection, const AQLString& curveIndex )
    {
		// Forward Rate Results are returned as an array of terms and forward rates
        ForwardRateTable results = etrading::AQLCurveCalibrationHelpers::getCurveForwardRateTable( etrading::getDataInstance(), curveCollection, curveIndex );
        return results;
    }

    /* @brief			This function approximates a discount factor from a forward rate using numerical integration
    *                   and the formula df = exp[ - Integral{0,T}( Forward Rates ~du ) ]
	*  @param [in]		terms		    A vector of terms date year fractions representing fixing dates
	*  @param [in]		forwards	    A vector of forward rates
	*  @param [out]		Approximate discount factors
    */
    DoubleVector approximateDiscountFactorsByIntegratingForwards( const DoubleVector & terms, const DoubleVector & forwards, const size_t & nSteps, const InterpolationEnum & interpolationMethod )
    {
        AQ_REQUIRE( forwards.size()>0, "Inconsistent number of terms and forward values.")
        AQ_REQUIRE( terms.size() == forwards.size(), "Missing terms and forward data")
        
        size_t nResults = forwards.size();
        DoubleVector discFactorResults( nResults );
        
        // Numerically Integrate the forward rates which gives ZeroRate x Time
        DoubleVector forwardIntegrals( nResults );
        DoubleVector lowerBounds( nResults, 0.0); // set all integration lowerbounds = 0.0

        // Set target function and integrand
        etrading::SimpsonsRule::TargetFunction target( terms, forwards, interpolationMethod );
        etrading::SimpsonsRuleIntegrand integrand( target );
        
        forwardIntegrals = integrand.integrate( lowerBounds, terms, nSteps, true ); // optimize = true i.e. allow OMP threading

        // Calculate the Approximate Discount Factors
        // df = exp[ - Integral{0,T}( Forward Rates ~du ) ]
        for(size_t i=0; i <nResults; ++i )
        {
            // Bounds checks are above to prevent access violation errors
            discFactorResults[i] = std::exp( - forwardIntegrals[i] );
        }

        return discFactorResults;
    }

    // Helper Functions
    // ----------------------------------------------------------------------------

    // Helper Function to convert discount factors to the log of discount factors
    DoubleVector toLogDiscountFactors( const DoubleVector & terms, const DoubleVector & discountFactors )
    {
        DoubleVector logDiscountFactors( discountFactors.size(), 0.0 );
        for( size_t i = 0; i<discountFactors.size(); ++i )
        {
            logDiscountFactors[i] = std::log( discountFactors[i] );
        }
        return logDiscountFactors;
    }

    // Helper Function to convert Zero Rate x Time to discount factors
    DoubleVector fromLogDiscountFactors( const DoubleVector & terms, const DoubleVector & logDiscountFactors, const bool applyZeroFloor  )
    {
        DoubleVector discountFactors( logDiscountFactors.size(), 0.0 );
        for( size_t i = 0; i<logDiscountFactors.size(); ++i )
        {
            discountFactors[i] = std::exp( logDiscountFactors[i] );
            
            const bool isGreaterThanZero = AQ_IS_GREATER_THAN_ZERO( discountFactors[i] );

            // Apply the zero floor or throw if discount factors are negative or zero
            if ( !isGreaterThanZero )
            {
                if ( applyZeroFloor )
                {
                    discountFactors[i] = AQ_EPSILON;
                }
                else
                {
                    AQ_THROW( "Unable to solve for Discount Factors. Discount Factor results cannot be less than or equal to zero" )
                }
            }
            
        }
        return discountFactors;
    }

    // Function to convert discount factors to zero rate
    DoubleVector toZeroRate( const DoubleVector & terms, const DoubleVector & discountFactors )
    {
        DoubleVector zeroRates( discountFactors.size(), 0.0 );
        for( size_t i = 0; i<discountFactors.size(); ++i )
        {
            zeroRates[i] = -1.0 * std::log( discountFactors[i] ) / terms[i] ;
        }
        return zeroRates;
    }

    // Function to convert zero rate to discount factors
    DoubleVector fromZeroRate( const DoubleVector & terms, const DoubleVector & zeroRates, const bool applyZeroFloor )
    {
        DoubleVector discountFactors( zeroRates.size(), 0.0 );
        for( size_t i = 0; i<zeroRates.size(); ++i )
        {
            discountFactors[i] = std::exp( -1.0 * zeroRates[i] * terms[i] );
            
            const bool isGreaterThanZero = AQ_IS_GREATER_THAN_ZERO( discountFactors[i] );

            // Apply the zero floor or throw if discount factors are negative or zero
            if ( !isGreaterThanZero )
            {
                if ( applyZeroFloor )
                {
                    discountFactors[i] = AQ_EPSILON;
                }
                else
                {
                    AQ_THROW( "Unable to solve for Discount Factors. Discount Factor results cannot be less than or equal to zero" )
                }
            }
            
        }
        return discountFactors;
    }

    // Helper Function to convert fixing terms to approximate payment terms for our equivalent Discount Factor solving
    DoubleVector toApproxPaymentTerms( const DoubleVector & fixingTerms, const double & curveFrequencyTerm )
    {
        DoubleVector paymentTerms( fixingTerms.size(), 0.0 );
        for( size_t i = 0; i<paymentTerms.size(); ++i )
        {
            paymentTerms[i] = fixingTerms[i] + curveFrequencyTerm;
        }
        return paymentTerms;
    }

    // Helper Function to convert fixing terms to check if terms data is sorted
    bool isTermsDataSortedWithNoDuplicates( const DoubleVector & terms )
    {
        // Return true if only one value
        if ( terms.size() == 1 )
        {
            return true;
        }

        // Check each value and exit immediately if the next value is less than or equal to the previous value
        for( size_t i = 1; i<terms.size(); ++i )
        {
            // Note: Base index i = 1 required
            if ( AQ_IS_LESS_THAN_OR_EQUAL( terms[i], terms[i-1] ) )
            {
                return false;
            }
        }
        return true;
    }

    // ----------------------------------------------------------------------------

    /* @brief			Function to Convert a vector of forward rates to a vector of discount factors and set them to the curve
	* @param [in]		fixingDates	        fixing or reset dates
    * @param [in]		forwardRates        forward rates
    * @param [in]		curveCollection     curve collection
    * @param [in]		curveIndex          curve index
	* @output			ImpliedDiscountFactorResults struct containing impliedDiscountFactors_, numberOfInterations_ and epsilon_
	*/
    EquivalentDiscountFactors::SolverResults setForwardRateEquivalentDiscountFactors( const DateVector & fixingDates, const DoubleVector & targetForwardRates, const AQLString & curveCollection, const AQLString & curveIndex )
    {
        // Validate Input Dimensions
        AQ_REQUIRE( fixingDates.size() == targetForwardRates.size(), "Convert Forwards to Discount Factors: Invalid Input - Inconsistent number of fixing dates and forward rates" )
        AQ_REQUIRE( targetForwardRates.size() > 0, "Convert Forwards to Discount Factors - No forward rates provided" )
        AQ_REQUIRE( targetForwardRates.size() >= 3, "Convert Forwards to Discount Factors - At least 3 forward rates are required" )

        // Check Curve Exists
        getCurveStaticDataTableName( curveCollection, curveIndex );

        // Convert fixing dates to payment terms date year fractions
        DoubleVector fixingTerms = etrading::convertCurveDatesToTerms( curveCollection, fixingDates );
        DoubleVector paymentTerms( fixingTerms.size() ); 

        AQ_REQUIRE( fixingDates.size() == fixingTerms.size(), "Convert Forwards to Discount Factors: Invalid Input - Inconsistent fixing dates and fixing terms date year fractions" )
        AQ_REQUIRE( fixingTerms.size() == paymentTerms.size(), "Convert Forwards to Discount Factors: Invalid Input - Inconsistent fixing- and payment termss date year fractions" )
        
		AQ_REQUIRE( fixingTerms[0] >= 0.0, "Invalid Curve Forward Dates: Curve forward dates cannot be in the past, before the curve 'AsOfDate'. Forwards in the past should be in the fixing table instead.")

        const double curveFrequency = etrading::getCurveFrequencyAsYearFraction( curveCollection, curveIndex );
        for( size_t i = 0; i < paymentTerms.size(); ++i )
        {
            paymentTerms[i] = fixingTerms[i] + curveFrequency;
        }
        
        // Check Payment Terms Data is sorted with no duplicates
        // Derived Data from Fixing Dates
        AQ_REQUIRE( isTermsDataSortedWithNoDuplicates( paymentTerms ), "Invalid Fixing Dates - Fixing Dates must be sorted in ascending order with no duplicates" )

        // Calculate a good initial guess for discount factors
        const size_t numberOfNumericalIntegrationSteps = 100;
        DoubleVector initialGuessDiscFactors = approximateDiscountFactorsByIntegratingForwards( paymentTerms, targetForwardRates, numberOfNumericalIntegrationSteps, etrading::SPLINE_INTERPOLATION );
        AQ_REQUIRE ( fixingTerms.size() == initialGuessDiscFactors.size(), "Unable to solve for Discount Factors. Invalid solver initial guess" )
        
        // Initialize Solver Discount Factor State Variable
        // This is used to store our initial guess of DFs and to store the solution result
        DiscountFactorTable solverDiscountFactorTable;
        solverDiscountFactorTable.terms_            = paymentTerms;
        solverDiscountFactorTable.paymentDates_     = etrading::convertCurveTermsToDates( curveCollection, paymentTerms );
        solverDiscountFactorTable.discountFactors_  = initialGuessDiscFactors;

        // Transform Initial Guess to Log Discount Factors
        DoubleVector initialGuessLogDiscountFactors = toLogDiscountFactors( paymentTerms, initialGuessDiscFactors );

        /* @brief The target objective function used by the multivariate Newton-Raphson solver. This function
	    *  calculates the residual difference between the target forwards and the current forwards implied by
        *  the current discount factor state variables
	    *
	    *  Note that the targetFunction lambda contains a capture-list [&] which captures all locals by reference.
	    *  It uses this feature to update the discount factor state variables with the current solution guess, which
        *  on convergence will contain our discount factor solution.
	    * 
	    *  @param[in]	The current estimate of the equivalent discount factors that correspond to the forwards we are solving for
	    *  @returns		A vector of calibration residual errors. The difference between the implied- and target forwards
	    */
        auto targetFunction = [ & ]( const DoubleVector& logDiscountFactors ) -> DoubleVector
        {
			// Transform the zeroRateTimesTime state variables to raw discount factors & Update the solver discount factor table
			solverDiscountFactorTable.discountFactors_ = fromLogDiscountFactors( paymentTerms, logDiscountFactors, true ); // apply zero floor = true

			// For each iteration set the estimated discount factors to the curve to update the implied forwards
			// Only Set a single curveIndex, not the entire curve index alias list, we update all index aliases at the end of the routine only rather than on every solver iteration
			etrading::AQLCurveCalibrationHelpers::setCurveDiscountFactorTable( etrading::getDataInstance(), curveCollection, AQLStringVector( 1, curveIndex ), solverDiscountFactorTable );

			// Set Discount Factors in the Curve Results Object since required for implied forward rates
			// Only Set a single curveIndex, not the entire curve index alias list, we update all index aliases at the end of the routine only rather than on every solver iteration
			if( etrading::isEnabledCurveResults() && etrading::doesExistCurveResultsDiscountFactors( curveCollection.c_str(), curveIndex.c_str() ) )
			{
				etrading::CurveResultsContainer::getInstance().getCurveResults( curveCollection.c_str(), curveIndex.c_str() )->discountFactorResults()->setDiscountFactorsUsingTerms( paymentTerms, solverDiscountFactorTable.discountFactors_ );
			}

            // Imply Forward Rates from given Discount Factors: Use IsFwdInter = False
            DoubleVector impliedForwardRates = etrading::getCurveForwardRates( fixingDates, curveCollection, curveIndex, etrading::NO_CHANGE, "", etrading::FALSE_BOOL ); // Null Calendar: "", isFwdInter = FALSE

            // Compare Calulated Forwards Rates against the Target
            const size_t nForwards = impliedForwardRates.size();
            DoubleVector residuals( nForwards );

            AQ_REQUIRE( impliedForwardRates.size() == targetForwardRates.size(), "Unable to convert forwards to discount factors - Solver result has inconsistent dimensions" )
            AQ_REQUIRE( nForwards > 0, "Unable to convert forwards to discount factors - Solver results are empty" )

            for ( size_t i=0; i < nForwards; ++i )
            {
                residuals[i] = impliedForwardRates[i] - targetForwardRates[i];
            }

            return residuals;
        };

        // Set Solver Parameters
        // ----------------------
        
        // Discount Factor bumpsize - Discount factors need to have accuracy of at least 9 decimal places ...
        // ... in order to imply forwards with 5 d.p. accuracy. Note calibration accuracy is to 9 d.p. so we
        // ... can't increase accuracy beyond this
        const double initialBumpSize            = 1.0e-2;
        const double bumpSize                   = 1.0e-9;
        
        // Forward Rate tolerance - Forwards quote to 5 decimal places in percent i.e. minimum tolerance 1e-8
        const double tolerance                  = 1.0e-8;
        
        const double gradientTolerance          = 1.0e-15;
        const double maxIterations              = 1000;

        // Store the original DFs to restore curve on failure
        DiscountFactorTable originalDFs = getCurveDiscountFactors( curveCollection, curveIndex );
        etrading::solvers::MultiVariateSolverResults results;

        try
        {
            // Note the solver discount factor input contains our initial guess, which on each iteration is updated until
            // we converge to a solution, when it then contains the solver result
            results = etrading::solvers::multiVariateNewtonRaphson( targetFunction,
                                                                    initialGuessLogDiscountFactors,
                                                                    0,          // stateVariableOffset
                                                                    solverDiscountFactorTable.discountFactors_.size(),
                                                                    tolerance,
                                                                    gradientTolerance,
                                                                    maxIterations,
                                                                    initialBumpSize,
                                                                    bumpSize,
                                                                    false );    // useInverseJacobian
        }
        catch(...)
		{
			// Restore Original DFs and Throw 
            // Only Reset the single solver curveIndex, not the entire curve index alias list, since we only used one curveIndex for solving purposes
            etrading::AQLCurveCalibrationHelpers::setCurveDiscountFactorTable( etrading::getDataInstance(), curveCollection, AQLStringVector( 1, curveIndex ), originalDFs );

			// Set Discount Factors in the Curve Results Object
			if( etrading::isEnabledCurveResults() && etrading::doesExistCurveResultsDiscountFactors( curveCollection.c_str(), curveIndex.c_str() ) )
			{
				etrading::CurveResultsContainer::getInstance().getCurveResults( curveCollection.c_str(), curveIndex.c_str() )->discountFactorResults()->setDiscountFactorsUsingTerms( originalDFs.terms_, originalDFs.discountFactors_ );
			}

            AQ_THROW("Unable to Set Curve - Solver failed to converge or find a solution")
        }

		// Set the results for the entire curve index alias list
		const AQLStringVector curveIndices = curveIndexAliasList( curveCollection, curveIndex );
		etrading::AQLCurveCalibrationHelpers::setCurveDiscountFactorTable( etrading::getDataInstance(), curveCollection, curveIndices, solverDiscountFactorTable );

		// Set Discount Factors in the Curve Results Object for the entire curve index alias list
		if( etrading::isEnabledCurveResults() && etrading::doesExistCurveResultsDiscountFactors( curveCollection.c_str(), curveIndex.c_str() ) )
		{
			for( AQLString thisIndex:curveIndices )
			{
				etrading::CurveResultsContainer::getInstance().getCurveResults( curveCollection.c_str(), thisIndex.c_str() )->discountFactorResults()->setDiscountFactorsUsingTerms( solverDiscountFactorTable.terms_, solverDiscountFactorTable.discountFactors_ );
			}
		}

        // Return the Discount Factor Results
        EquivalentDiscountFactors::SolverResults solverResults;
		
		solverResults.paymentTerms_					= paymentTerms;
        solverResults.discountFactors_				= results.solution;
        solverResults.numberOfIterations_			= results.numberOfIterations;
        solverResults.epsilon_						= results.epsilon;

        return solverResults;
    }


    /* @brief			Function to set curve forward rates that are equivalent to discount factors provided
	*  @param [in]		curveCollection		The curveCollection to use when accessing the curveIndices
	*  @param [in]		curveIndex			The curveindex within the curve
	*  @param [in]		paymentDates        Discount Factor Payment Dates
	*  @param [in]		discountFactors		Discount Factor Values
	*/
    void implyAndSetForwardRatesFromDiscountFactors( const AQLString& curveCollection, const AQLString& curveIndex, const DateVector& paymentDates, const DoubleVector& discountFactors )
    {
		AQ_REQUIRE( discountFactors.size() >= 3, "Invalid Discount Factors - At least 3 discount factors required" )
        
        // Build the ForwardRatesTable Struct
        DoubleVector termEnds = convertCurveDatesToTerms( curveCollection, paymentDates ); 
        
        // Calculate Term Starts
        DoubleVector termStarts( termEnds.size() );
        const double curveFrequency = etrading::getCurveFrequencyAsYearFraction( curveCollection, curveIndex );
        for( size_t i = 0; i < termStarts.size(); ++i )
        {
            termStarts[i] = termEnds[i] - curveFrequency;
        }

        // Get the Implied Forward Rates - using isFwdInter = false
        DateVector fixingDates = convertCurveTermsToDates( curveCollection, termStarts );
        DoubleVector impliedForwardRates = etrading::getCurveForwardRates( fixingDates, curveCollection, curveIndex, etrading::NO_CHANGE, "", etrading::FALSE_BOOL ); // Calendar: "", isFwdInter = false

        ForwardRateTable forwardRateTable;
        forwardRateTable.forwardStartTerms_ = termStarts;
        forwardRateTable.forwardEndTerms_   = termEnds;
        forwardRateTable.fixingDates_       = fixingDates;
        forwardRateTable.forwardRates_      = impliedForwardRates;

        // Set Implied Forward Rates on STD Curve
        const AQLStringVector curveIndices = curveIndexAliasList( curveCollection, curveIndex );
        etrading::AQLCurveCalibrationHelpers::setCurveForwardRateTable( etrading::getDataInstance(), curveCollection, curveIndices, forwardRateTable );
    }

}