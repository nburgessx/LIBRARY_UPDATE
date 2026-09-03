// SwapRiskResults.h

/*
 * @brief			Swap Risk Results Class
 * @Created:		31st December 2019
 * @Author:			Nicholas Burgess
 * @Department:		Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */
#pragma once
#include <vector>
#include <memory>

#include "LADate.h"
#include "Variant.h"

namespace etrading
{
	// Typedefs
    typedef std::vector<double> VectorDouble;
    typedef std::vector<LADate> VectorDate;
    typedef std::vector<double> DoubleVector;   // For Backwards compatibility
	typedef std::vector<LADate> DateVector;     // For Backwards compatibility

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
        SwapRiskResults( const LADate & asOfDate, const SwapDiscountRiskData & swapDiscountRiskData );
		
		// Forward Risk Constructor
		SwapRiskResults( const LADate & asOfDate, const SwapForwardRiskData & swapForwardRiskData );
		
		// Discount & Forward Risk Constuctor
		SwapRiskResults( const LADate & asOfDate, const SwapDiscountRiskData & swapDiscountRiskData, const SwapForwardRiskData & swapForwardRiskData );
		
		// =======================================================

        // Copy Constructor
        SwapRiskResults( const SwapRiskResults& rhs );

        // Assignment Operator
        SwapRiskResults & operator=( const SwapRiskResults & rhs );

		// Clone
		std::shared_ptr<SwapRiskResults> clone() const;

        // Accessors
		// =======================================================

        LADate asOfDate() const				{ return asOfDate_;		}
        DateVector paymentDates() const		{ return paymentDates_;	}
		DoubleVector coupons() const		{ return coupons_;		}
        DateVector fixingDates() const		{ return fixingDates_;	}
		DoubleVector annuities() const		{ return annuities_;	}
        
    private:
        
        LADate                  asOfDate_;
		DateVector				paymentDates_;
        DoubleVector            coupons_;
		DateVector				fixingDates_;
		DoubleVector            annuities_;
    };

}
