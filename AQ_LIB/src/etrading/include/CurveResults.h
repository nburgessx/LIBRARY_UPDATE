// CurveResults.h

/*
 * @brief			Curve Results Class
 * @Created:		3rd April 2019
 * @Author:			Nicholas Burgess
 * @Department:		Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */
#pragma once
#include <vector>
#include <memory>

#include "LADate.h"
#include "CoreEnumerations.h"
#include "LAInterpolationBase.h"
#include "Variant.h"

// Curve Result Components
#include "CurveGroup.h"
#include "CurveDescription.h"
#include "CurveConventionsAndMarketData.h"
#include "DiscountFactorResults.h"
#include "JacobianResults.h"

namespace etrading
{
    // Local anonymous namespace with typedefs for notational brevity
    namespace
    {
        // Typedefs
        typedef std::vector<double> VectorDouble;
        typedef std::vector<LADate> VectorDate;
        typedef std::vector<double> DoubleVector;   // For Backwards compatibility
        typedef std::vector<LADate> DateVector;     // For Backwards compatibility
    }

    // ================================= HELPER METHOD(S) ===================================================================
    
    // Helper Struct to Hold Curve Description and Discount Factor Results
    struct CurveDescriptionAndDiscountFactorResults
    {
        std::shared_ptr<etrading::CurveDescription> curveDescription_;
        std::shared_ptr<etrading::DiscountFactorResults> discountFactorsResults_;
    };

    // Shared Helper Method to Create Discount Factor Results Object
    CurveDescriptionAndDiscountFactorResults curveDescriptionAndDiscountFactorResults( const LAStringMatrix & curveLVB,
																					   const LAStringMatrix & discFactorParameterLVB,
																					   const LAStringMatrix & discountFactors,
																					   const StandardStringMatrix & forwardAdjustments = StandardStringMatrix() );


    // ================================= CURVE RESULTS CLASS ================================================================

    // Class to Store Curve Results; Please ensure this class remains thread-safe
    class CurveResults
    {

    public:

        // Constructor
        CurveResults() {};
        
        // Destructor
        ~CurveResults() {};
        
        // Copy Constructor
        CurveResults( const CurveResults& rhs );

		// Clone
		std::shared_ptr<CurveResults> clone() const;

        // Assignment Operator
        CurveResults & operator=( const CurveResults & rhs );

		// Curve Group Constructor
		CurveResults( const std::shared_ptr<CurveGroup> & curveGroup );

        // Curve Info Constructor
        CurveResults( const std::shared_ptr<CurveDescription> & curveDescpription );

        // Discount Factor Constructor
        CurveResults( const std::shared_ptr<CurveDescription> & curveDescpription,
					  const std::shared_ptr<CurveConventionsAndMarketData> & curveConventionsAndMarketData,
                      const std::shared_ptr<DiscountFactorResults> & discountFactorResults );

        // Jacobian Constructor 
        CurveResults( const std::shared_ptr<CurveDescription> & curveDescpription,
					  const std::shared_ptr<CurveConventionsAndMarketData> & curveConventionsAndMarketData,
                      const std::shared_ptr<DiscountFactorResults> & discountFactorResults,
                      const std::shared_ptr<JacobianResults> & jacobianResultsByDiscountFactor,
					  const std::shared_ptr<JacobianResults> & jacobianResultsByForwardRate,
					  const std::shared_ptr<JacobianResults> & jacobianResultsByCompoundRate );

        // Accessors
		std::shared_ptr<CurveGroup>							curveGroup() const;
		std::shared_ptr<CurveDescription>					curveDescription() const;
		std::shared_ptr<CurveConventionsAndMarketData>		curveConventionsAndMarketData() const;
        std::shared_ptr<DiscountFactorResults>				discountFactorResults() const;
        std::shared_ptr<JacobianResults>					jacobianResultsByDiscountFactor() const;
		std::shared_ptr<JacobianResults>					jacobianResultsByForwardRate() const;
		std::shared_ptr<JacobianResults>					jacobianResultsByCompoundRate() const;

        // Helper Methods
		bool doesExistCurveGroup() const							{ return curveGroup_ != nullptr; }
		bool doesExistCurveDescription() const                      { return curveDescription_ != nullptr; }
		bool doesExistCurveConventionsAndMarketData() const			{ return curveConventionsAndMarketData_ != nullptr; }
        bool doesExistDiscountFactorResults() const                 { return discountFactorResults_ != nullptr; }
        bool doesExistJacobianResultsByDiscountFactor() const       { return jacobianResultsByDiscountFactor_ != nullptr; }
		bool doesExistJacobianResultsByForwardRate() const          { return jacobianResultsByForwardRate_ != nullptr; }
		bool doesExistJacobianResultsByCompoundRate() const         { return jacobianResultsByCompoundRate_ != nullptr; }

    private:

		std::shared_ptr<CurveGroup> curveGroup_;
        std::shared_ptr<CurveDescription> curveDescription_;
		std::shared_ptr<CurveConventionsAndMarketData> curveConventionsAndMarketData_;
		std::shared_ptr<DiscountFactorResults> discountFactorResults_;
        std::shared_ptr<JacobianResults> jacobianResultsByDiscountFactor_;
		std::shared_ptr<JacobianResults> jacobianResultsByForwardRate_;
		std::shared_ptr<JacobianResults> jacobianResultsByCompoundRate_;
    };

}

