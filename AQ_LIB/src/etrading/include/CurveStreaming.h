// CurveStreaming.cpp

/*
 * @brief			Curve Streaming Methods
 * @Created:		14th June 2018
 * @Author:			Nicholas Burgess
 * @Department:		Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#pragma once

#include "AQLCoreTemplateType.h"
#include "AQLDataInstance.h"
#include <boost/lexical_cast.hpp>
#include "LabelValueBlock.h"
#include "LACurveCalibrationHelpers.h"
#include <vector>

using etrading::LabelValueBlock;

namespace etrading
{
    // Annonymous Namespace
    namespace EquivalentDiscountFactors
    {
        struct SolverResults
        {
			DoubleVector	paymentTerms_;
            DoubleVector    discountFactors_;
            size_t          numberOfIterations_;
            DoubleVector    epsilon_;
        };
    }


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
                                   const bool setCorrespondingDiscountFactors = true );

    /* @brief			setCurveDiscountFactors function, which overrides curve discount factors
	*  @param [in]		curveCollection		        The curveCollection to use when accessing the curveIndices
	*  @param [in]		curveIndex			        The curveindex within the curve
	*  @param [in]		paymentDates                The payment dates corresponding to our discount factors
	*  @param [in]		discountFactors		        The new discount factors
    *  @param [in]		setCorrespondingForwards	Set the corresponding forwards (for STD curves only)
	*/
    AQLString setCurveDiscountFactors( const AQLString& curveCollection,
                                      const AQLString& curveIndex,
                                      const DateVector& paymentDates,
                                      const DoubleVector& discountFactors,
                                      const bool setCorrespondingForwards = false );

    /* @brief			setCurveDiscountFactorsToOne function, which sets all discount factors to one
	*  @param [in]		curveCollection		The curveCollection to use when accessing the curveIndices
	*  @param [in]		curveIndex			The curveindex within the curve
	*/
    AQLString setCurveDiscountFactorsToOne( const AQLString& curveCollection, const AQLString& curveIndices );

    /* @brief			getCurveDiscountFactors function, which retrieves all discount factors
	*  @param [in]		curveCollection		    The curveCollection to use when accessing the curveIndices
	*  @param [in]		curveIndex			    The curveindex within the curve
	*  @param [out]		DiscountFactorTable     A discount factor table structure that contains paymentDates_ and discountFactors_
    */
    DiscountFactorTable getCurveDiscountFactors( const AQLString& curveCollection, const AQLString& curveIndex );

    /* @brief			getCurveForwardRates function, which retrieves all forward rates
	*  @param [in]		curveCollection		    The curveCollection to use when accessing the curveIndices
	*  @param [in]		curveIndex			    The curveindex within the curve
	*  @param [out]		ForwardRateTable        A forward rate table structure that contains fixingDates_ and forwardRates_
    */
    ForwardRateTable getCurveForwardRates( const AQLString& curveCollection, const AQLString& curveIndex );

    /* @brief			This function approximates a discount factor from a forward rate using numerical integration
    *                   and the formula df = exp[ - Integral{0,T}( Forward Rates ~du ) ]
	*  @param [in]		terms		    A vector of terms date year fractions representing fixing dates
	*  @param [in]		forwards	    A vector of forward rates
    *  @param [in]		nSteps	        [Optional] Number of numerical integration steps; defaults to 10 integration steps
	*  @param [out]		Approximate discount factors
    */
    DoubleVector approximateDiscountFactorsByIntegratingForwards( const DoubleVector & terms, const DoubleVector & forwards, const size_t & nSteps = 100, const InterpolationEnum & interpolationMethod = etrading::LINEAR_INTERPOLATION );

    // State Variable Transformations
    // ------------------------------

    // Helper Function to convert discount factors to the log of discount factors
    DoubleVector toLogDiscountFactors( const DoubleVector & terms, const DoubleVector & discountFactors );

    // Helper Function to convert Log Discount Factors to discount factors
    DoubleVector fromLogDiscountFactors( const DoubleVector & terms, const DoubleVector & logDiscountFactors, const bool applyZeroFloor = false  );

    // Helper Function to convert discount factors to Zero Rate
    DoubleVector toZeroRate( const DoubleVector & terms, const DoubleVector & discountFactors );
    
    // Helper Function to convert Zero Rate to discount factors
    DoubleVector fromZeroRate( const DoubleVector & terms, const DoubleVector & zeroRates, const bool applyZeroFloor = false  );
    
    // ------------------------------

    // Helper Function to convert fixing terms to approximate payment terms for our equivalent Discount Factor solving
    DoubleVector toApproxPaymentTerms( const DoubleVector & fixingTerms, const double & curveFrequencyTerm );

    // Helper Function to convert fixing terms to check if terms data is sorted
    bool isTermsDataSortedWithNoDuplicates( const DoubleVector & terms );

    /* @brief			Function to Convert a vector of forward rates to a vector of discount factors and set them to the curve
	* @param [in]		fixingDates	        fixing or reset dates
    * @param [in]		targetForwardRates  target forward rates
    * @param [in]		curveCollection     curve collection
    * @param [in]		curveIndex          curve index
	* @output			EquivalentDiscountFactors::SolverResults struct containing discountFactors_, numberOfInterations_ and epsilon_
	*/
    EquivalentDiscountFactors::SolverResults setForwardRateEquivalentDiscountFactors( const DateVector & fixingDates, const DoubleVector & targetForwardRates, const AQLString & curveCollection, const AQLString & curveIndex );

    /* @brief			Function to set curve forward rates that are equivalent to discount factors provided
	*  @param [in]		curveCollection		The curveCollection to use when accessing the curveIndices
	*  @param [in]		curveIndex			The curveindex within the curve
	*  @param [in]		paymentDates        Discount Factor Payment Dates
	*  @param [in]		discountFactors		Discount Factor Values
	*/
    void implyAndSetForwardRatesFromDiscountFactors( const AQLString& curveCollection, const AQLString& curveIndex, const DateVector& paymentDates, const DoubleVector& discountFactors );
}
