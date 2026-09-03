// CurveResultsContainer.h

/*
 * @brief			Curve Results Container Class
 * @Created:		15th April 2019
 * @Author:			Nicholas Burgess
 * @Department:		Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once
#include <memory>           // Shared Pointer
#include <boost/thread.hpp> // shared_mutex
#include <map>
#include "Singleton.h"      // Singleton template
#include "CurveResults.h"
#include "CoreEnumerations.h"

// Macro to Clear Curve Results Container - Only Single LWO Curves Generate Curve Results, so we must clear the Cache for Other Curve Types
#define MLIB_CLEAR_CURVE_RESULTS_CACHE etrading::CurveResultsContainer::getInstance().deleteAllCurveResults(); 

namespace etrading
{
    // ----------------------------- HELPER METHODS -----------------------------------------

    // Helper Method to Check if Curve Results are enabled
    bool isEnabledCurveResults();
    
    // Helper Method to Check if Curve Results exist for the curve index
    bool doesExistCurveResults( const std::string & curveCollection, const std::string & curveIndex );
    
	// Helper Method to Check if Curve Group Exists
    bool doesExistCurveGroup( const std::string & curveGroup );

    // Helper Method to Check if Curve Result the Description Table exists for the curve index
    bool doesExistCurveResultsDescription( const std::string & curveCollection, const std::string & curveIndex );
    
	// Helper Method to Check if Curve Result the Conventions and Market Data exists for the curve index
	bool doesExistCurveResultsConventionsAndMarketData( const std::string & curveCollection, const std::string & curveIndex );

    // Helper Method to Check if Curve Result Discount Factors exist for the curve index
    bool doesExistCurveResultsDiscountFactors( const std::string & curveCollection, const std::string & curveIndex );

	// Helper Method to Check if Curve Result Jacobian exists for the curve index
    bool doesExistCurveResultsJacobianByDiscountFactor( const std::string & curveCollection, const std::string & curveIndex );

	// Helper Method to Check if Curve Result Jacobian exists for the curve index
    bool doesExistCurveResultsJacobianByForwardRate( const std::string & curveCollection, const std::string & curveIndex );

	// Helper Method to Check if Curve Result Jacobian exists for the curve index
    bool doesExistCurveResultsJacobianByCompoundRate( const std::string & curveCollection, const std::string & curveIndex );

    // Helper Method to get Discount Factors from the Curve Results Container
    std::shared_ptr<CurveDescription> getCurveDescriptionFromCurveResultsObject( const std::string & curveCollection, const std::string & curveIndex );

    // Helper Method to get the *** OPTIONAL *** Curve Results, Conventions & Market Data
    std::shared_ptr<CurveConventionsAndMarketData> getOptionalCurveResultsConventionsAndMarketData( const std::string & curveCollection, const std::string & curveIndex );

	// Helper Method to get Discount Factors from the Curve Results Container
    DoubleVector getDiscountFactorsFromCurveResultsObject( const std::string & curveCollection,
                                                           const std::string & curveIndex,
                                                           const DateVector & paymentDates );
    
    // Helper Method to get Discount Factors from the Curve Results Container using Payment Date Year Fractions
    DoubleVector getDiscountFactorsUsingYearFractionsFromCurveResultsObject( const std::string & curveCollection,
                                                                             const std::string & curveIndex,
                                                                             const DoubleVector & paymentDateYearFractions );
    
    // Helper Method to imply Forward Rates from Discount Factors using the Curve Results Container
    DoubleVector implyForwardRatesFromCurveResultsObject( const std::string & curveCollection,
                                                          const std::string & curveIndex,
                                                          const DateVector & fixingDates,
                                                          const BusinessDayAdjustmentEnum& fixingBusinessDayAdj = NONE_BUSINESS_DAY_ADJ,
									                      const StandardString & fixingCalendar = "",
														  const bool isFwdInter = false,
                                                          const CompoundingFrequencyEnum & compoundFrequency = SIMPLE_COMPOUNDING );

    // Helper Method to imply Forward Rates from Discount Factors using the Curve Results Container
    DoubleVector implyForwardRatesFromCurveResultsObject( const std::string & curveCollection,
                                                          const std::string & curveIndex,
                                                          const DateVector & fromDates,
                                                          const DateVector & toDates,
														  const bool isFwdInter = false,
                                                          const CompoundingFrequencyEnum & compoundFrequency = SIMPLE_COMPOUNDING );

	// Helper Method to get the Jacobian Results for the correct Risk Type
	std::shared_ptr<etrading::JacobianResults> getJacobianResults( const std::string & curveCollection, const std::string & curveIndex, const RiskTypeEnum & riskType );
	
    // ----------------------------- CLASS METHODS -----------------------------------------

    // CurveResultsContainer, A singleton that holds all the curve results for each curve collection, which is accessible via the getInstance() method - Uses lazy instantiation
    class CurveResultsContainer : public Singleton<CurveResultsContainer>
    {

    public:

        // Method to get the Curve Results Shared Pointer
        std::shared_ptr<CurveResults> getCurveResults( const std::string & curveCollection, const std::string & curveIndex ) const;

        // *** OPTIONAL *** Method to get the Curve Results Shared Pointer that *** DOES NOT THROW *** and returns a nullptr if not found 
        std::shared_ptr<CurveResults> getCurveResultsNoThrow( const std::string & curveCollection, const std::string & curveIndex ) const;

        // Method to add a curve results object to the curve results container/cache
        void addCurveResults( const std::string & curveCollection, const std::string & curveIndex, std::shared_ptr<CurveResults> curveResults );
        
        // Method to delete a curve results object from the curve results container/cache, will return true if successful and false otherwise if not found in cache or something similar
        bool deleteCurveResults( const std::string & curveCollection, const std::string & curveIndex );
        
        // Method to delete all curve results objects from the curve results container/cache
        void deleteAllCurveResults();

        // Method to check if curve results object exists
        bool doesExist( const std::string & curveCollection, const std::string & curveIndex  ) const;

        // Set Enable flag to Enable/Disable Curve Results
        void enableCurveResults( const bool enable );
        
        // Check if Curve Results are Enabled
        bool isEnabled() const { return isEnabled_; }

    private:

        // Private Constructur - Singleton
        // Enable the curve results by default
        CurveResultsContainer() : isEnabled_( true ) {};
        
        // Singleton friend class
        friend Singleton<CurveResultsContainer>;

        // Copy Constructor and Assignment Operators should never be called
        CurveResultsContainer( const CurveResultsContainer& ) = delete;
        CurveResultsContainer & operator=( const CurveResultsContainer& ) = delete;

        // Mutex for results access when adding and removing curve results
        mutable boost::shared_mutex resultsAccess_;
        
        // Curve Results Map: Key = Pair( CurveCollection, CurveIndex ), Value = CurveResults
        std::map<std::pair< std::string, std::string>, std::shared_ptr<CurveResults> > curveResultsContainer_;

        bool isEnabled_;
    };

	// ----------------------------- HELPER CLASS -----------------------------------------

	// Helper Class to Disable Curve Results
	// Constructor disables the curve results and once out of scope the destructor will re-enable the curve results object
	// Use as follows: "DisableCurveResults d;"
	class DisableCurveResults
	{
		public:
			DisableCurveResults()	{ CurveResultsContainer::getInstance().enableCurveResults( false ); }
			~DisableCurveResults()	{ CurveResultsContainer::getInstance().enableCurveResults( true );	}
	};
}