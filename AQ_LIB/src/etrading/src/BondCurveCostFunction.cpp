#include "BondCurveCostFunction.h"
#include "BondCurves.h"
#include "ExceptionMacros.h"
 
#include <ql/math/optimization/endcriteria.hpp>
#include <ql/math/optimization/levenbergmarquardt.hpp>

namespace etrading
{
	/* @brief Constructor of the BondCurve CostFunction which is initialized using bond market data
	 * @param[in]	bondYields		A vector of bond yields observed in the market 
	 * @param[in]	bondMaturities	A vector of bond maturities observed in the market, one per bond yield
	 */
	BondCurveCostFunction::BondCurveCostFunction ( const DoubleVector& bondMaturities, const DoubleVector& bondYields )
		: bondMaturities_( bondMaturities ), bondYields_( bondYields )
	{
		MLIB_REQUIRE( bondYields.size() == bondMaturities.size(), "Number of bond yields should match the number of bond maturities. ");
	}

	/* @brief Copy constructor
	 */
	BondCurveCostFunction::BondCurveCostFunction( const BondCurveCostFunction& rhs ) 
		: bondMaturities_( rhs.bondMaturities_ ), bondYields_( rhs.bondYields_ )
	{
	}

	/* @brief Calculates the sum of squares of the model fit for the specified model parameters
	 * @params[in]	modelParams	The model calibration parameters to use
	 * @returns		a single value representing how good the model fits the market data
	 */
	QuantLib::Real BondCurveCostFunction::value( const QuantLib::Array & modelParams ) const 
	{
		QuantLib::Array residuals = values( modelParams );
		QuantLib::Real diffSqTotal = 0.0;

		for(size_t i = 0; i < residuals.size(); ++i)
		{
			double residual = residuals[i];
			double diffSq = residual * residual;
			diffSqTotal += diffSq;
		}

		return  diffSqTotal;
	}

	/* @brief Calculates an array of model reprice errors (residuals) for the specified model parameters
	 * @params[in]	modelParams	The model calibration parameters to use
	 * @returns		An array of residuals, one per bond
	 */
	QuantLib::Array BondCurveCostFunction::values( const QuantLib::Array& modelParams ) const
	{
		DoubleVector calculatedYields = getModelValues( modelParams );

		size_t nBonds = calculatedYields.size();

		QuantLib::Array residuals( nBonds );

		for(size_t i = 0; i < nBonds; ++i)
		{
			auto calculatedYield = calculatedYields[i];
			auto targetBondValue = bondYields_[i];
			residuals[i] = calculatedYield - targetBondValue;
		}

		return  residuals;
	}

}
