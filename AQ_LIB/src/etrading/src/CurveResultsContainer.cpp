// CurveResultsContainer.cpp

/*
 * @brief			Curve Results Container Class
 * @Created:		15th April 2019
 * @Author:			Nicholas Burgess
 * @Department:		Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */
#include "CurveResultsContainer.h"
#include "CurveValidation.h"			// CurveAliasList method
#include "ExceptionMacros.h"

namespace etrading
{
    // ----------------------------- HELPER METHODS -----------------------------------------

    // Helper Method to Check if Curve Results are enabled
    bool isEnabledCurveResults()
    {
        return etrading::CurveResultsContainer::getInstance().isEnabled();
    }
    
    // Helper Method to Check if Curve Results exist for the curve index
    bool doesExistCurveResults( const std::string & curveCollection, const std::string & curveIndex )
    {
		// Don't throw here if curve results disabled, since this check determines whether to use the legacy object pool instead
		if( !isEnabledCurveResults() )
		{
			return false;
		}

        return etrading::CurveResultsContainer::getInstance().doesExist( curveCollection, curveIndex );
    }
    
	// Helper Method to Check if Curve Group Exists
	bool doesExistCurveGroup( const std::string & curveGroup )
	{
		// Don't throw here if curve results disabled, since this check determines whether to use the legacy object pool instead
		if( !isEnabledCurveResults() )
		{
			return false;
		}

        bool doesExistCurveGroup = false;
        if ( doesExistCurveResults( curveGroup, "" ) )
        {
            doesExistCurveGroup = etrading::CurveResultsContainer::getInstance().getCurveResults( curveGroup, "" )->doesExistCurveGroup();
        }
        return doesExistCurveGroup;
	}

    // Helper Method to Check if Curve Result the Description Table exists for the curve index
    bool doesExistCurveResultsDescription(const std::string & curveCollection, const std::string & curveIndex )
    {
		// Don't throw here if curve results disabled, since this check determines whether to use the legacy object pool instead
		if( !isEnabledCurveResults() )
		{
			return false;
		}

        bool doesExistCurveDescription = false;
        if ( doesExistCurveResults( curveCollection, curveIndex ) )
        {
            doesExistCurveDescription = etrading::CurveResultsContainer::getInstance().getCurveResults( curveCollection, curveIndex )->doesExistCurveDescription();
        }
        return doesExistCurveDescription;
    }
   
	// Helper Method to Check if Curve Result the Conventions and Market Data exists for the curve index
	bool doesExistCurveResultsConventionsAndMarketData( const std::string & curveCollection, const std::string & curveIndex )
	{
		// Don't throw here if curve results disabled, since this check determines whether to use the legacy object pool instead
		if( !isEnabledCurveResults() )
		{
			return false;
		}

		bool doesExistCurveConventionsAndMarketData = false;
		if( doesExistCurveResults( curveCollection, curveIndex ) )
		{
			doesExistCurveConventionsAndMarketData = etrading::CurveResultsContainer::getInstance().getCurveResults( curveCollection, curveIndex )->doesExistCurveConventionsAndMarketData();
		}
		return doesExistCurveConventionsAndMarketData;
	}

    // Helper Method to Check if Curve Result Discount Factors exist for the curve index
    bool doesExistCurveResultsDiscountFactors( const std::string & curveCollection, const std::string & curveIndex )
    {
		// Don't throw here if curve results disabled, since this check determines whether to use the legacy object pool instead
		if( !isEnabledCurveResults() )
		{
			return false;
		}

        bool doesExistDiscountFactors = false;
        if ( doesExistCurveResults( curveCollection, curveIndex ) )
        {
            doesExistDiscountFactors = etrading::CurveResultsContainer::getInstance().getCurveResults( curveCollection, curveIndex )->doesExistDiscountFactorResults();
        }
        return doesExistDiscountFactors;
    }

    // Helper Method to Check if Curve Result Jacobian exists for the curve index
    bool doesExistCurveResultsJacobianByDiscountFactor( const std::string & curveCollection, const std::string & curveIndex )
    {
		// Don't throw here if curve results disabled, since this check determines whether to use the legacy object pool instead
		if( !isEnabledCurveResults() )
		{
			return false;
		}

        bool doesExistJacobian = false;
        if ( doesExistCurveResults( curveCollection, curveIndex ) )
        {
            doesExistJacobian = etrading::CurveResultsContainer::getInstance().getCurveResults( curveCollection, curveIndex )->doesExistJacobianResultsByDiscountFactor();
        }
        return doesExistJacobian;
    }

	// Helper Method to Check if Curve Result Jacobian exists for the curve index
    bool doesExistCurveResultsJacobianByForwardRate( const std::string & curveCollection, const std::string & curveIndex )
    {
		// Don't throw here if curve results disabled, since this check determines whether to use the legacy object pool instead
		if( !isEnabledCurveResults() )
		{
			return false;
		}

        bool doesExistJacobian = false;
        if ( doesExistCurveResults( curveCollection, curveIndex ) )
        {
            doesExistJacobian = etrading::CurveResultsContainer::getInstance().getCurveResults( curveCollection, curveIndex )->doesExistJacobianResultsByForwardRate();
        }
        return doesExistJacobian;
    }

	// Helper Method to Check if Curve Result Jacobian exists for the curve index
    bool doesExistCurveResultsJacobianByCompoundRate( const std::string & curveCollection, const std::string & curveIndex )
    {
		// Don't throw here if curve results disabled, since this check determines whether to use the legacy object pool instead
		if( !isEnabledCurveResults() )
		{
			return false;
		}

        bool doesExistJacobian = false;
        if ( doesExistCurveResults( curveCollection, curveIndex ) )
        {
            doesExistJacobian = etrading::CurveResultsContainer::getInstance().getCurveResults( curveCollection, curveIndex )->doesExistJacobianResultsByCompoundRate();
        }
        return doesExistJacobian;
    }

    // Helper Method to get Discount Factors from the Curve Results Container
    std::shared_ptr<CurveDescription> getCurveDescriptionFromCurveResultsObject( const std::string & curveCollection, const std::string & curveIndex )
    {
		AQ_REQUIRE( isEnabledCurveResults(), "Curve Results have been Disabled" )
        const std::shared_ptr<CurveDescription> curveDesription = CurveResultsContainer::getInstance().getCurveResults( curveCollection, curveIndex )->curveDescription();
        return curveDesription;
    }

    // Helper Method to get the *** OPTIONAL *** Curve Results, Conventions & Market Data
    std::shared_ptr<CurveConventionsAndMarketData> getOptionalCurveResultsConventionsAndMarketData( const std::string & curveCollection, const std::string & curveIndex )
    {
        // No Throw - return nullptr if not found
        std::shared_ptr<CurveConventionsAndMarketData> conventionsAndMarketData = nullptr;
        if ( isEnabledCurveResults() )
        {
            std::shared_ptr<CurveResults> curveResults = CurveResultsContainer::getInstance().getCurveResultsNoThrow(curveCollection, curveIndex);
            
            // Check if found
            if ( curveResults != nullptr )
            {
                curveResults->curveConventionsAndMarketData();
            }
        }
        return conventionsAndMarketData;
    }

	// Helper Method to get Discount Factors from the Curve Results Container
    DoubleVector getDiscountFactorsFromCurveResultsObject( const std::string & curveCollection,
                                                           const std::string & curveIndex,
                                                           const DateVector & paymentDates )
    {
		AQ_REQUIRE( isEnabledCurveResults(), "Curve Results have been Disabled" )
        const DoubleVector discountFactors = CurveResultsContainer::getInstance().getCurveResults( curveCollection, curveIndex )->discountFactorResults()->getDiscountFactors( paymentDates );
        return discountFactors;
    }

    // Helper Method to get Discount Factors from the Curve Results Container using Payment Date Year Fractions
    DoubleVector getDiscountFactorsUsingYearFractionsFromCurveResultsObject( const std::string & curveCollection,
                                                                             const std::string & curveIndex,
                                                                             const DoubleVector & paymentDateYearFractions )
    {
		AQ_REQUIRE( isEnabledCurveResults(), "Curve Results have been Disabled" )
        const DoubleVector discountFactors = CurveResultsContainer::getInstance().getCurveResults( curveCollection, curveIndex )->discountFactorResults()->getDiscountFactors( paymentDateYearFractions );
        return discountFactors;
    }

    // Helper Method to imply Forward Rates from Discount Factors using the Curve Results Container
    DoubleVector implyForwardRatesFromCurveResultsObject( const std::string & curveCollection,
                                                          const std::string & curveIndex,
                                                          const DateVector & fixingDates,
                                                          const BusinessDayAdjustmentEnum& fixingBusinessDayAdj,
									                      const StandardString & fixingCalendar,
														  const bool isFwdInter,
                                                          const CompoundingFrequencyEnum & compoundFrequency )
    {
		AQ_REQUIRE( isEnabledCurveResults(), "Curve Results have been Disabled" )
        const DoubleVector impliedForwardRates = CurveResultsContainer::getInstance().getCurveResults( curveCollection, curveIndex )->discountFactorResults()->implyForwardRates( fixingDates, fixingBusinessDayAdj, fixingCalendar, isFwdInter, compoundFrequency );
        return impliedForwardRates;
    }
    
    // Helper Method to imply Forward Rates from Discount Factors using the Curve Results Container
    DoubleVector implyForwardRatesFromCurveResultsObject( const std::string & curveCollection,
                                                          const std::string & curveIndex,
                                                          const DateVector & fromDates,
                                                          const DateVector & toDates,
														  const bool isFwdInter,
                                                          const CompoundingFrequencyEnum & compoundFrequency)
    {
		AQ_REQUIRE( isEnabledCurveResults(), "Curve Results have been Disabled" )
        const DoubleVector impliedForwardRates = CurveResultsContainer::getInstance().getCurveResults( curveCollection, curveIndex )->discountFactorResults()->implyForwardRates( fromDates, toDates, isFwdInter, compoundFrequency );
        return impliedForwardRates;
    }

	std::shared_ptr<etrading::JacobianResults> getJacobianResults( const std::string & curveCollection, const std::string & curveIndex, const RiskTypeEnum & riskType )
	{
		switch( riskType )
		{
			case DISCOUNT_FACTOR_RISK_TYPE:
			{
				AQ_REQUIRE( etrading::doesExistCurveResultsJacobianByDiscountFactor( curveCollection, curveIndex ), "Jacobian Results by DISCOUNT_FACTORS for Curve Collection '" + curveCollection + "' and/or Curve Index '" + curveIndex + "' do not exist"  )
				return etrading::CurveResultsContainer::getInstance().getCurveResults( curveCollection, curveIndex )->jacobianResultsByDiscountFactor();
			}
			case FORWARD_RATE_RISK_TYPE:
			{
				AQ_REQUIRE( etrading::doesExistCurveResultsJacobianByForwardRate( curveCollection, curveIndex ), "Jacobian Results by FORWARD_RATES for Curve Collection '" + curveCollection + "' and/or Curve Index '" + curveIndex + "' do not exist"  )
				return etrading::CurveResultsContainer::getInstance().getCurveResults( curveCollection, curveIndex )->jacobianResultsByForwardRate();
			}
			case COMPOUND_RATE_RISK_TYPE:
			{
				AQ_REQUIRE( etrading::doesExistCurveResultsJacobianByCompoundRate( curveCollection, curveIndex ), "Jacobian Results by COMPOUND_RATES for Curve Collection '" + curveCollection + "' and/or Curve Index '" + curveIndex + "' do not exist"  )
				return etrading::CurveResultsContainer::getInstance().getCurveResults( curveCollection, curveIndex )->jacobianResultsByCompoundRate();
			}
			default:
			{
				AQ_THROW("Invalid Jacobian Results: Risk Type must be 'DISCOUNT_FACTORS', 'FORWARD_RATES' or 'COMPOUND_RATES'.")
			}
		}

		AQ_THROW("Invalid Jacobian Results: Risk Type must be 'DISCOUNT_FACTORS', 'FORWARD_RATES' or 'COMPOUND_RATES'.")
	}

    // ----------------------------- CLASS METHODS -----------------------------------------

    // Method to get the Curve Results Shared Pointer
    std::shared_ptr<CurveResults> CurveResultsContainer::getCurveResults( const std::string & curveCollection, const std::string & curveIndex ) const
    {
		AQ_REQUIRE( isEnabledCurveResults(), "Curve Results have been Disabled" )

        // Mutex Required for Thread-Safety - Results can get deleted or overwritten
        boost::shared_lock<boost::shared_mutex> lock( resultsAccess_ );

        std::pair<std::string, std::string> searchKey( curveCollection, curveIndex );
        auto it = curveResultsContainer_.find(searchKey);
        AQ_REQUIRE( it != curveResultsContainer_.end(), "Curve Collection '" + curveCollection + "' and/or Curve Index '" + curveIndex + "' does not exist" )
        return it->second;
    }
    
    // *** OPTIONAL *** Method to get the Curve Results Shared Pointer that *** DOES NOT THROW *** and returns a nullptr if not found 
    std::shared_ptr<CurveResults> CurveResultsContainer::getCurveResultsNoThrow( const std::string & curveCollection, const std::string & curveIndex ) const
    {
        // No Throw - return nullptr if no result
        std::shared_ptr<CurveResults> curveResults = nullptr;
		if( isEnabledCurveResults() )
        {
            // Mutex Required for Thread-Safety - Results can get deleted or overwritten
            boost::shared_lock<boost::shared_mutex> lock( resultsAccess_ );

            std::pair<std::string, std::string> searchKey( curveCollection, curveIndex );
            auto it = curveResultsContainer_.find(searchKey);
            
            // Check if found
            if ( it != curveResultsContainer_.end() )
            {
                curveResults = it->second;
            }
        }
        return curveResults;
    }

    // Method to add a curve results object to the curve results container/cache
    void CurveResultsContainer::addCurveResults( const std::string & curveCollection, const std::string & curveIndex, std::shared_ptr<CurveResults> curveResults )
    {
		AQ_REQUIRE( isEnabledCurveResults(), "Curve Results have been Disabled" )

        // Mutex Required to Write - Exclusive Access via unique_lock
        boost::unique_lock<boost::shared_mutex> uniqueLock( resultsAccess_ );

        std::pair<std::string, std::string> searchKey( curveCollection, curveIndex );
        curveResultsContainer_[searchKey] = curveResults;
    }
    
    // Method to delete a curve results object from the curve results container/cache, will return true if successful and false otherwise if not found in cache or something similar
    bool CurveResultsContainer::deleteCurveResults( const std::string & curveCollection, const std::string & curveIndex )
    {
        // Mutex Required to Delete - Exclusive Access via unique_lock
        boost::unique_lock<boost::shared_mutex> uniqueLock( resultsAccess_ );
        
		bool curveResultsDeleted = false;

		// 1. Delete Curve Results Built Independent of the Curve Object Pool
		std::pair<std::string, std::string> searchKey( curveCollection, curveIndex );
		auto it = curveResultsContainer_.find(searchKey);

		if (it != curveResultsContainer_.end())
		{
			curveResultsContainer_.erase(it);
			curveResultsDeleted = true;
		}

		// 2. Delete Curve Results Built via Curve Object Pool
		const AQLStringVector curveIndexAliasList = etrading::curveIndexAliasList(curveCollection.c_str(), curveIndex.c_str(), false ); // don't throw on error, but return empty list
		for (auto thisCurveIndex : curveIndexAliasList)
		{
			std::pair<std::string, std::string> searchKey(curveCollection, thisCurveIndex.c_str());
			auto it = curveResultsContainer_.find(searchKey);

			if (it != curveResultsContainer_.end())
			{
				curveResultsContainer_.erase(it);
				curveResultsDeleted = true;
			}
		}

        return curveResultsDeleted;
    }

    // Method to delete all curve results objects from the curve results container/cache
    void CurveResultsContainer::deleteAllCurveResults()
    {
        // Mutex Required to Delete - Exclusive Access via unique_lock
        boost::unique_lock<boost::shared_mutex> uniqueLock( resultsAccess_ );
        
        curveResultsContainer_.clear();
    }

    // Method to check if curve results object exists
    bool CurveResultsContainer::doesExist( const std::string & curveCollection, const std::string & curveIndex  ) const
    {
        // Mutex Required for Thread-Safety - Results can get deleted or overwritten
        boost::shared_lock<boost::shared_mutex> lock( resultsAccess_ );

        std::pair<std::string, std::string> searchKey( curveCollection, curveIndex );
        auto it = curveResultsContainer_.find(searchKey);
        bool doesCurveExist =  it!=curveResultsContainer_.end();
        return doesCurveExist;
    }

    // Enable/Disable Curve Results
    void CurveResultsContainer::enableCurveResults( const bool enable )
    {
        // Mutex Required to Write - Allow Shared Read Access but Prevent Read Mutex Upgrades to Write
        boost::upgrade_lock<boost::shared_mutex> lock( resultsAccess_ );

        isEnabled_ = enable;
    }

}