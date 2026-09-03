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
