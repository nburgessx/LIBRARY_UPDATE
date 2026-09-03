/*
 * @brief			Class which represents a basic Monte Carlo Path.
 *					It contains the simulated state-variable. Other more complex paths may be derived from this class.
 * @Created:		24 Jan 2019
 * @Author:			Ian Castleton
 * @Department:		Quant Research & Analytics
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

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