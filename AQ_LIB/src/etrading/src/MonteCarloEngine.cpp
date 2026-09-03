/*
 * @brief			Class which defines the Monte Carlo Engine.
 * @Created:		14 Jan 2019
 * @Author:			Ian Castleton
 * @Department:		Quant Research & Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */


#include "MonteCarloEngine.h"
#include "MonteCarloPath.h"
#include "SurvivalPath.h"

#include "CoreEnumerations.h"



#include <omp.h>

namespace etrading
{
	/* @brief		Initializes the Monte-Carlo engine
	*  @param[in]	payOffFunction			A 1D lambda function which calculates the payoff for each path	
	*  @param[in]	evaluatePathsInParallel	Whether to invoke the payoffFunction from multiple threads
	*/
	template<class PathType>
	MonteCarloEngine<PathType>::MonteCarloEngine( std::function<double (PathType)> payoffFunction, const bool evaluatePathsInParallel )
		: payoffFunction_( payoffFunction ), evaluatePathsInParallel_( evaluatePathsInParallel )
	{
		numThreads_ = evaluatePathsInParallel ? omp_get_max_threads() : 1;
		monteCarloStatistics_.initialize( numThreads_ );
	}

	/* @brief		Runs the Monte-Carlo simulation for the specified paths. Gathers statistics in the specified monteCarloStatistics parameter.
	*				Provided the payoff function is thread-safe, this method is able to run the paths in parallel.
	*  @param[in]	paths					A vector containing the set of paths to use in the simulation
	*  @param[in]	monteCarloStatistics	A statistics object which collects information about the simulation.
	*										In particular, the statistics object will hold the current mean value (PV) and standard error of the paths so far.
	*/
	template<class PathType>
	void MonteCarloEngine<PathType>::runSimulation( const std::vector<PathType>& paths )
	{
		double pv = 0.;
		const size_t numberOfPaths = paths.size();

		if ( evaluatePathsInParallel_ )
		{
			#pragma omp parallel for num_threads( numThreads_ )
			for ( int i=0; i< (int) numberOfPaths; i++ )
			{
				const PathType& path = paths[i];
				const double pvForPath = payoffFunction_( path );
				monteCarloStatistics_.addOneResult( pvForPath, omp_get_thread_num() );
			}
		}
		else
		{
			for ( size_t i=0; i<numberOfPaths; i++ )
			{
				const double pvForPath = payoffFunction_( paths[ i ] );
				monteCarloStatistics_.addOneResult( pvForPath );
			}
		}
	}

	template<class PathType>
	const MonteCarloStatistics& MonteCarloEngine<PathType>::getMonteCarloStatistics() const
	{
		return monteCarloStatistics_;
	}

	// explicit instantiation of the MonteCarloEngine for various path types
	template class MonteCarloEngine<MonteCarloPath>;
	template class MonteCarloEngine<SurvivalPath>;
}

