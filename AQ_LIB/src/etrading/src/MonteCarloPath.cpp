#include "MonteCarloPath.h"
#include <limits>


namespace etrading
{
	MonteCarloPath::MonteCarloPath()
		: stateVariable_( std::numeric_limits<double>::quiet_NaN() )
	{}

	MonteCarloPath::MonteCarloPath( const double stateVariable )
		: stateVariable_( stateVariable )
	{}

	MonteCarloPath::~MonteCarloPath()
	{}

	/* @brief	Returns the state variable for the path.
	*/
	double MonteCarloPath::getStateVariable() const
	{
		return stateVariable_;
	}

}