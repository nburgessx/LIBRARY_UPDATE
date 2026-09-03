/*
 * @brief			Bond Curve coefficient Fitting
 *					This is used to fit Bond Yields to a parametric form
 * @Created:		18th June 2018
 * @Author:			Ian Castleton
 * @Department:	    Quant Research & Analytics
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once

#include "BondCurves.h"
#include "LACoreTemplateType.h"

#include <ql/math/optimization/costfunction.hpp>
#include <ql/math/optimization/constraint.hpp>

namespace etrading
{


	// Abstract base class cost function for Bond Curve fitting.
	class BondCurveCostFunction : public QuantLib::CostFunction 
	{

	public:

		BondCurveCostFunction() {}
		virtual ~BondCurveCostFunction(){}

		/* @brief Constructor of the BondCurve CostFunction which is initialized using bond market data
		 * @param[in]	bondMaturities		A vector of bond maturities observed in the market
		 * @param[in]	bondYields			A vector of bond yields observed in the market 
		*/
		BondCurveCostFunction ( const DoubleVector& bondMaturities, const DoubleVector& bondYields );

		/* @brief Copy constructor
		*/
		BondCurveCostFunction( const BondCurveCostFunction& rhs );
		
		// The following are override methods of QuantLib::CostFunction

		/* @brief Calculates the sum of squares of the model fit for the specified model parameters
		 * @params[in]	modelParams	The model calibration parameters to use
		 * @returns		a single value representing how good the model fits the market data
		 */
		virtual QuantLib::Real value ( const QuantLib::Array& modelParams ) const;

		/* @brief Calculates an array of model reprice errors (residuals) for the specified model parameters
		 * @params[in]	modelParams	The model calibration parameters to use
		 * @returns		An array of residuals, one per bond
		 */
		virtual QuantLib::Array values( const QuantLib::Array& modelParams ) const;

	protected:
		/* @brief Calculates the interpolatored bond yields, given the specified model parameters
		 *        Subclasses must implement this method by invoking the appropriate bond curve interpolator.
		 * @params[in]	modelParams	The interpolator coefficients to use
		 * @returns		An array of bond yields calculated from the model 
		 */
		virtual DoubleVector getModelValues( const QuantLib::Array& modelParams ) const = 0;

		// Input market data which the CostFunction will calibrate to
		DoubleVector bondMaturities_;
		DoubleVector bondYields_;

	};

}

