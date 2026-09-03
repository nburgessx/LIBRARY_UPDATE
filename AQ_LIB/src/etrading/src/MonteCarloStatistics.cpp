#include "MonteCarloStatistics.h"
#include "ExceptionMacros.h"


namespace etrading
{
	
	MonteCarloStatistics::MonteCarloStatistics() : numThreads_( 0 )
	{
	}

	/* @brief		Used to initialize the data members and set the number of threads that are in use by the Monte-Carlo.
	*				A seperate set of data is maintained per thread, so that there is no chance of conflict between threads.
	*
	*  @param[in]	numThreads	The number of threads in use. If this value is 1, then we are running in single-threaded mode.
	*/
	void MonteCarloStatistics::initialize( const int numThreads )
	{
		numThreads_ = numThreads;

		// Maintain a separate results vector for each thread
		results_.resize( numThreads, DoubleVector() );

		// Maintain a separate sum for each thread
		sum_.resize( numThreads, 0. );
	}

	/* @brief		This method is called by the Monte-Carlo engine after evaluating the PV of each path. It records the result of 1 simulation.
	*
	*  @param[in]	result		The PV result of the simulation of one path.
	*  @param[in]	threadId	Optional argument. If missing, this defaults to 0.
	*							In multi-threaded mode, the caller must specify the threadId which generated the simulation result.
	*/
	void MonteCarloStatistics::addOneResult( const double result, const int threadId )
	{
		// threadId starts at 0
		if ( threadId < numThreads_ )
		{
			results_[ threadId ].push_back( result );
			sum_[ threadId ] += result;
		}
		else
		{
			AQ_REQUIRE( numThreads_ > 0, "MonteCarloStatistics object has not been initialized ");

			AQ_THROW( "Invalid threadId passed into MonteCarloStatistics::addOneResult()" );
		}
	}

	/* @brief		Calculates and returns the mean PV of all the simulation results.
	*
	* returns	the mean of all the simulations
	*/
	double MonteCarloStatistics::getMean() const
	{
		AQ_REQUIRE( numThreads_ > 0, "MonteCarloStatistics object has not been initialized ");

		size_t numResults = 0;
		double sum = 0.0;

		// Sum over the results accumulated by thread
		for (int threadId = 0; threadId < numThreads_; threadId++ )
		{
			numResults += results_[ threadId ].size();
			sum += sum_[ threadId ];
		}

		const double mean = sum / (double) numResults;
		return mean;
	}

	/* @brief		Calculates and returns the Monte-Carlo standard error estimate of the simulation results.
	*				This is the sample standard deviation divided by sqrt( numPaths )
	*
	* returns	the standard error of all the simulations
	*/
	double MonteCarloStatistics::getStandardError() const
	{
		size_t numResults = 0;
		double sumSquaredDeviations = 0.0;
		const double mean = getMean();

		// Sum over the results accumulated by thread
		for (int threadId = 0; threadId < numThreads_; threadId++ )
		{
			const DoubleVector& results = results_[ threadId ];
			const size_t resultsSize = results.size();
			numResults += resultsSize;

			for ( size_t i = 0; i < resultsSize; i++ )
			{
				const double deviation = ( results[ i ] - mean );
				const double deviationSquared = deviation * deviation;
				sumSquaredDeviations += deviationSquared;
			}
		}

		const double sampleStandardDeviation = sqrt( sumSquaredDeviations / (double) (numResults - 1) );
		const double monteCarloStandardError = sampleStandardDeviation / sqrt( (double) numResults );

		return monteCarloStandardError;
	}

}

