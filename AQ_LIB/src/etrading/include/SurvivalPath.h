/*
 * @brief			Class which represents a Monte Carlo Path for a credit risky asset which defaults on the stopping date.
 *					It contains the simulated stopping date, and the corresponding discount factor for
 *					any payouts which occur on default.
 * @Created:		24 Jan 2019
 * @Author:			Ian Castleton
 * @Department:		Quant Research & Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#pragma once

#include "MonteCarloPath.h"
#include "LADate.h"

namespace etrading
{
	class SurvivalPath : public MonteCarloPath
	{
	public:
		SurvivalPath();
		SurvivalPath( const double survivalProbability, const LADate& stoppingDate, const double discountFactorAtStoppingDate );
		virtual ~SurvivalPath();

		/* @brief	Returns the stopping date when the underlying index defaults.
		*/
		LADate getStoppingDate() const;

		/* @brief	Returns the discount factor on the stopping date. Used for payments which occur on default.
		*/
		double getDiscountFactorAtStoppingDate() const;

	private:
		LADate stoppingDate_;
		double discountFactorAtStoppingDate_;
	};

}
