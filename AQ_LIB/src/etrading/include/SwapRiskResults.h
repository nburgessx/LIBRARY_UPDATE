// SwapRiskResults.h

/*
 * @brief			Swap Risk Results Class
 */
#pragma once
#include <vector>
#include <memory>

#include "AQLDate.h"
#include "Variant.h"

namespace etrading
{
	// Typedefs
    typedef std::vector<double> VectorDouble;
    typedef std::vector<AQLDate> VectorDate;
    typedef std::vector<double> DoubleVector;   // For Backwards compatibility
	typedef std::vector<AQLDate> DateVector;     // For Backwards compatibility

	// =============================== HELPER DATA CLASSES ==================================================================
	
	class SwapDiscountRiskData
	{
		public:
			SwapDiscountRiskData( const DateVector & paymentDates, const DoubleVector & coupons );
			DateVector paymentDates_;
			DoubleVector coupons_;
	};

	class SwapForwardRiskData
	{
		public:
			SwapForwardRiskData( const DateVector & fixingDates, const DoubleVector & annuities );
			DateVector fixingDates_;
			DoubleVector annuities_;
	};


	// =============================== SWAP RISK RESULTS CLASS ==================================================================

    // Class to store and retrieve discount factors
    class SwapRiskResults
    {

    public:
        SwapRiskResults() {};
        ~SwapRiskResults() {};

        // Main Constructor(s) 
		// =======================================================

		// Discount Risk Constructor
        SwapRiskResults( const AQLDate & asOfDate, const SwapDiscountRiskData & swapDiscountRiskData );
		
		// Forward Risk Constructor
		SwapRiskResults( const AQLDate & asOfDate, const SwapForwardRiskData & swapForwardRiskData );
		
		// Discount & Forward Risk Constuctor
		SwapRiskResults( const AQLDate & asOfDate, const SwapDiscountRiskData & swapDiscountRiskData, const SwapForwardRiskData & swapForwardRiskData );
		
		// =======================================================

        // Copy Constructor
        SwapRiskResults( const SwapRiskResults& rhs );

        // Assignment Operator
        SwapRiskResults & operator=( const SwapRiskResults & rhs );

		// Clone
		std::shared_ptr<SwapRiskResults> clone() const;

        // Accessors
		// =======================================================

        AQLDate asOfDate() const				{ return asOfDate_;		}
        DateVector paymentDates() const		{ return paymentDates_;	}
		DoubleVector coupons() const		{ return coupons_;		}
        DateVector fixingDates() const		{ return fixingDates_;	}
		DoubleVector annuities() const		{ return annuities_;	}
        
    private:
        
        AQLDate                  asOfDate_;
		DateVector				paymentDates_;
        DoubleVector            coupons_;
		DateVector				fixingDates_;
		DoubleVector            annuities_;
    };

}
