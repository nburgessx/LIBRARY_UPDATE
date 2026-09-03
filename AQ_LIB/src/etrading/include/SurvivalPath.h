#pragma once

#include "MonteCarloPath.h"
#include "AQLDate.h"

namespace etrading
{
	class SurvivalPath : public MonteCarloPath
	{
	public:
		SurvivalPath();
		SurvivalPath( const double survivalProbability, const AQLDate& stoppingDate, const double discountFactorAtStoppingDate );
		virtual ~SurvivalPath();

		/* @brief	Returns the stopping date when the underlying index defaults.
		*/
		AQLDate getStoppingDate() const;

		/* @brief	Returns the discount factor on the stopping date. Used for payments which occur on default.
		*/
		double getDiscountFactorAtStoppingDate() const;

	private:
		AQLDate stoppingDate_;
		double discountFactorAtStoppingDate_;
	};

}
