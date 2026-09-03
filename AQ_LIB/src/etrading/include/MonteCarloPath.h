/*
 * @brief			Class which represents a basic Monte Carlo Path.
 *					It contains the simulated state-variable. Other more complex paths may be derived from this class.
 * @Created:		24 Jan 2019
 * @Author:			Ian Castleton
 * @Department:		Quant Research & Analytics
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */


#pragma once

namespace etrading
{
	class MonteCarloPath
	{
	public:
		MonteCarloPath();
		MonteCarloPath( const double stateVariable );
		virtual ~MonteCarloPath();

		/* @brief	Returns the state variable for the path.
		*/
		double getStateVariable() const;

	private:
		double stateVariable_;

	};
}
