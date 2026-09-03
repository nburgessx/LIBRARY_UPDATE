#include "SurvivalPath.h"
#include <limits>

namespace etrading
{

	SurvivalPath::SurvivalPath()
		: MonteCarloPath(), stoppingDate_(), discountFactorAtStoppingDate_( std::numeric_limits<double>::quiet_NaN() )
	{}

	SurvivalPath::SurvivalPath( const double survivalProbability, const LADate& stoppingDate, const double discountFactorAtStoppingDate )
		: MonteCarloPath( survivalProbability ), stoppingDate_( stoppingDate ), discountFactorAtStoppingDate_( discountFactorAtStoppingDate )
	{}

	SurvivalPath::~SurvivalPath()
	{}

	/* @brief	Returns the stopping date when the underlying index defaults.
	*/
	LADate SurvivalPath::getStoppingDate() const
	{
		return stoppingDate_;
	}

	/* @brief	Returns the discount factor on the stopping date. Used for payments which occur on default.
	*/
	double SurvivalPath::getDiscountFactorAtStoppingDate() const
	{
		return discountFactorAtStoppingDate_;
	}
}