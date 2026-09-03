/*
 * @brief			validation interface for Polynomial Interpolation Method(s)
 * @Created:		8th January 2020
 * @Author:			Nicholas Burgess
 * @Department:		Quant Research
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#pragma once

#include <vector>

namespace validation_api
{
	// @brief		Function to peform polynomial INTERPOLATION for a single x value
	// @param [in]	xValues		- Polynomial Time Vector
	// @param [in]	yValues		- Polynomial Value Vector
	// @param [in]	degree		- Polynomial Degree
	// @param [in]	x			- Interpolation Time Value
	// @return The interpolated y value at time x
    double tryMeMathPolynomialInterpolation( const std::vector<double>& xValues, const std::vector<double>& yValues, const unsigned int degree, const double x );
	
	// @brief		Function to peform polynomial INTERPOLATION for mulitple x values
	// @param [in]	xValues		- Polynomial Time Vector
	// @param [in]	yValues		- Polynomial Value Vector
	// @param [in]	degree		- Polynomial Degree
	// @param [in]	x			- Interpolation Time Value(s)
	// @return The interpolated y value(s) at time(s) x
	std::vector<double> tryMeMathPolynomialInterpolations( const std::vector<double>& xValues, const std::vector<double>& yValues, const unsigned int degree, const std::vector<double> & x );

	// @brief		Function to peform polynomial INTEGRATION for single integration bounds
	// @param [in]	xValues		- Polynomial Time Vector
	// @param [in]	yValues		- Polynomial Value Vector
	// @param [in]	degree		- Polynomial Degree
	// @param [in]	lowerBound	- Integration Lower Bound
	// @param [in]	upperBound	- Integration Upper Bound
	// @return The integration value over the lower- and upper bound
    double tryMeMathPoynomialIntegration( const std::vector<double>& xValues, const std::vector<double>& yValues, const unsigned int degree, const double lowerBound, const double upperBound );

	// @brief		Function to peform polynomial INTEGRATION for mulitple integration bounds
	// @param [in]	xValues		- Polynomial Time Vector
	// @param [in]	yValues		- Polynomial Value Vector
	// @param [in]	degree		- Polynomial Degree
	// @param [in]	lowerBounds - Integration Lower Bound(s)
	// @param [in]	upperBounds - Integration Upper Bound(s)
	// @return The integration value(s) over the lower- and upper bound(s)
	std::vector<double> tryMeMathPoynomialIntegrations( const std::vector<double>& xValues, const std::vector<double>& yValues, const unsigned int degree, const std::vector<double> & lowerBounds, const std::vector<double> & upperBounds );

}
                               