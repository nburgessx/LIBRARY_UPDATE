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
