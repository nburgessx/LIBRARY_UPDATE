// SwapRiskResults.cpp

/*
 * @brief			Swap Risk Results Class
 */
#include "SwapRiskResults.h"
#include "ExceptionMacros.h"


namespace etrading
{
	// =============================== HELPER DATA CLASSES ==================================================================
	
	SwapDiscountRiskData::SwapDiscountRiskData( const DateVector & paymentDates, const DoubleVector & coupons )
		: paymentDates_(paymentDates), coupons_(coupons)
	{
		AQ_REQUIRE( paymentDates.size() == coupons.size(), "Invalid SwapDiscountRisk Data: Number of payment dates and coupons must match")
	}
			

	SwapForwardRiskData::SwapForwardRiskData( const DateVector & fixingDates, const DoubleVector & annuities )
		: fixingDates_(fixingDates), annuities_(annuities)
	{
		AQ_REQUIRE( fixingDates.size() == annuities.size(), "Invalid SwapForwardRisk Data: Number of fixing dates and annuities must match")
	}


	// =============================== SWAP RISK RESULTS CLASS ==================================================================
    
    // Discount Risk Constructor
    SwapRiskResults::SwapRiskResults( const AQLDate & asOfDate, const SwapDiscountRiskData & swapDiscountRiskData )
		: asOfDate_( asOfDate ),
          paymentDates_(swapDiscountRiskData.paymentDates_),
		  coupons_(swapDiscountRiskData.coupons_),
		  fixingDates_(),
          annuities_()
    {
        // Dimension Check(s)
        AQ_REQUIRE( coupons_.size() > 0, "Invalid Swap Risk Results: The Swap Discount Risk Results are empty")
    }

	// Forward Risk Constructor
	SwapRiskResults::SwapRiskResults( const AQLDate & asOfDate, const SwapForwardRiskData & swapForwardRiskData )
		: asOfDate_( asOfDate ),
          paymentDates_(),
		  coupons_(),
		  fixingDates_(swapForwardRiskData.fixingDates_),
          annuities_(swapForwardRiskData.annuities_)
    {
        // Dimension Check(s)
        AQ_REQUIRE( annuities_.size() > 0, "Invalid Swap Risk Results: The Swap Forward Risk Results are empty")
    }
	
	// Discount & Forward Risk Constructor
	SwapRiskResults::SwapRiskResults( const AQLDate & asOfDate, const SwapDiscountRiskData & swapDiscountRiskData, const SwapForwardRiskData & swapForwardRiskData )
		: asOfDate_( asOfDate ),
          paymentDates_(swapDiscountRiskData.paymentDates_),
		  coupons_(swapDiscountRiskData.coupons_),
		  fixingDates_(swapForwardRiskData.fixingDates_),
          annuities_(swapForwardRiskData.annuities_)
    {
        // Dimension Check(s)
        AQ_REQUIRE( coupons_.size() > 0, "Invalid Swap Risk Results: The Swap Discount Risk Results are empty")
		AQ_REQUIRE( annuities_.size() > 0, "Invalid Swap Risk Results: The Swap Forward Risk Results are empty")
		AQ_REQUIRE( coupons_.size() == annuities_.size(), "Invalid Swap Risk Results: The number of discount risk and forward risk parameters must match")
    }

    // Copy Constructor
    SwapRiskResults::SwapRiskResults( const SwapRiskResults & rhs ) 
        : asOfDate_(rhs.asOfDate_),
          paymentDates_(rhs.paymentDates_),
		  coupons_(rhs.coupons_),
		  fixingDates_(rhs.fixingDates_),
          annuities_(rhs.annuities_)
	{
	}

    // Assignment Operator
    SwapRiskResults & SwapRiskResults::operator=( const SwapRiskResults & rhs )
    {
		// For Performance
		if ( & rhs == this )
		{
			return *this;
		}

        // For Exception Safety
            
        // 1. Make a temp copy
        SwapRiskResults temp( rhs );

        // 2. Swap Data Members with the temp copy
        std::swap( asOfDate_,		temp.asOfDate_ );
        std::swap( paymentDates_,	temp.paymentDates_ );
		std::swap( coupons_,		temp.coupons_ );
		std::swap( fixingDates_,	temp.fixingDates_ );
        std::swap( annuities_,		temp.annuities_ );
        
        return *this;
    }

	// Clone
	std::shared_ptr<SwapRiskResults> SwapRiskResults::clone() const
	{
		return std::make_shared<SwapRiskResults>( SwapRiskResults( *this ) );
	}

}
