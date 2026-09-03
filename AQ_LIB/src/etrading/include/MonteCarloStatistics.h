/*
 * @brief			Class which defines a simple statistics-gathering class, used by the Monte Carlo engine.
 * @Created:		14 Jan 2019
 * @Author:			Ian Castleton
 * @Department:		Quant Research & Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#pragma once

#include "LACoreTemplateType.h"

namespace etrading
{
	
	class MonteCarloStatistics
	{
	public:
		MonteCarloStatistics();

		/* @brief		Used to initialize the data members and set the number of threads that are in use by the Monte-Carlo.
		*				A seperate set of data is maintained per thread, so that there is no chance of conflict between threads.
		*
		*  @param[in]	numThreads	The number of threads in use. If this value is 1, then we are running in single-threaded mode.
		*/
		void initialize( const int numThreads );

		/* @brief		This method is called by the Monte-Carlo engine after evaluating the PV of each path. It records the result of 1 simulation.
		*
		*  @param[in]	result		The PV result of the simulation of one path.
		*  @param[in]	threadId	Optional argument. If missing, this defaults to 0.
		*							In multi-threaded mode, the caller must specify the threadId which generated the simulation result.
		*/
		void addOneResult( const double result, const int threadId = 0 );

		/* @brief		Calculates and returns the mean PV of all the simulation results.
		*
		* returns	the mean of all the simulations
		*/
		double getMean() const;

		/* @brief		Calculates and returns the Monte-Carlo standard error estimate of the simulation results.
		*				This is the sample standard deviation divided by sqrt( numPaths )
		*
		* returns	the standard error of all the simulations
		*/
		double getStandardError() const;

	private:
		int numThreads_;

		// Maintain a separate results vector for each thread. i.e. this std::vector is indexed by threadId
		std::vector<DoubleVector> results_;
		
		// Maintain a separate sum for each thread. i.e. this std::vector is indexed by threadId
		std::vector<double> sum_;

	};

}
