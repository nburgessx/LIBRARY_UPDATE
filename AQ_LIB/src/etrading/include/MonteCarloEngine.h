/*
 * @brief			Class which defines the Monte Carlo Engine
 * @Created:		14 Jan 2019
 * @Author:			Ian Castleton
 * @Department:		Quant Research & Analytics
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once

#include "MonteCarloStatistics.h"
#include "LACoreTemplateType.h"

#include <functional>


namespace etrading
{
	template< class PathType>
	class MonteCarloEngine
	{
	public:
		/* @brief		Initializes the Monte-Carlo engine
		*  @param[in]	payOffFunction			A 1D lambda function which calculates the payoff for each path
		*  @param[in]	evaluatePathsInParallel	Whether to invoke the payoffFunction from multiple threads
		*/
		MonteCarloEngine( std::function<double (PathType)> payoffFunction, const bool evaluatePathsInParallel );

		/* @brief		Runs the Monte-Carlo simulation for the specified paths. Gathers statistics in the specified monteCarloStatistics parameter.
		*				Provided the payoff function is thread-safe, this method is able to run the paths in parallel.
		*  @param[in]	paths					A vector containing the set of paths to use in the simulation
		*  @param[in]	monteCarloStatistics	A statistics object which collects information about the simulation.
		*										In particular, the statistics object will hold the current mean value (PV) and standard error of the paths so far.
		*/
		void runSimulation( const std::vector<PathType>& paths );

		const MonteCarloStatistics& getMonteCarloStatistics() const;
 
	private:
		std::function<double (PathType)> payoffFunction_;
		bool evaluatePathsInParallel_;
		int numThreads_;
		MonteCarloStatistics monteCarloStatistics_;

	};
}
