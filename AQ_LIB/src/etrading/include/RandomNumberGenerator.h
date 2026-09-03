/*
 * @brief			Class which defines a Random Number Generator, used in Monte-Carlo pricing.
 * @Created:		14 Jan 2019
 * @Author:			Ian Castleton
 * @Department:		Quant Research & Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#pragma once

#include "CoreEnumerations.h"
#include "LabelValueBlock.h"
#include "CommonConstants.h"

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_real_distribution.hpp>
#include <boost/random/normal_distribution.hpp>
#include <ql/math/randomnumbers/sobolrsg.hpp>

namespace etrading
{
	class RandomNumberGenerator
	{

	public:

		/* @brief		Helper static method which ensures a Uniform Random Number Generator is constructed correctly.
		*  @param[in]	mcParametersLVB		A label-value-block containing Monte-Carlo / Random number parameters
		*  @param[in]	minSample			For uniform distributions, a lower bound on the random number.
		*  @param[in]	maxSample			For uniform distributions, an upper bound on the random number.
		*  @returns		An initialized instance of the RandomNumberGenerator class
		*/
		static RandomNumberGenerator buildUniformGenerator( const LabelValueBlock& mcParametersLVB, const double minSample, const double maxSample );

		/* @brief		Helper static method which ensures a Normal Random Number Generator is constructed correctly.
		*  @param[in]	mcParametersLVB		A label-value-block containing Monte-Carlo / Random number parameters
		*  @param[in]	mean				For normal distributions, the mean of the distribution
		*  @param[in]	stdDeviation		For normal distributions, the standard deviation of the distribution
		*  @returns		An initialized instance of the RandomNumberGenerator class
		*/
		static RandomNumberGenerator buildNormalGenerator( const LabelValueBlock& mcParametersLVB, const double mean, const double stdDeviation );


		/* @brief		Constructor of the RandomNumberGenerator class.
		*  @param[in]	mcParametersLVB		A label-value-block containing Monte-Carlo / Random number parameters
		*  @param[in]	minSample			For uniform distributions, a lower bound on the random number.
		*  @param[in]	maxSample			For uniform distributions, an upper bound on the random number.
		*  @param[in]	mean				For normal distributions, the mean of the distribution
		*  @param[in]	stdDeviation		For normal distributions, the standard deviation of the distribution
		*/
		RandomNumberGenerator( const LabelValueBlock& mcParametersLVB, const double minSample, const double maxSample, const double mean, const double stdDeviation );


		/* @brief		Generates a vector of random numbers.
		*  @param[in]	nSamples		The number of random numbers to generate
		*
		*  @returns		A vector containing the random numbers
		*/
		DoubleVector getRandomNumbers(const size_t nSamples );

		/* @brief		Generates a vector of (vector of random numbers).
		*				Each item in the outer vector contains a random number per dimension (asset)
		*  @param[in]	nSamples		The number of sample sequences to generate
		*
		*  @returns		A vector containing the sequences
		*/
		std::vector<DoubleVector> getMultiDimRandomNumbers( const size_t nSamples );


		static std::vector<std::string> montecarlo_parameters_lvbKeys()
		{
			const std::string arr[] =
			{
				MONTE_CARLO_PROPERTIES_KEYS::RANDOM_NUMBER_GENERATOR
				, MONTE_CARLO_PROPERTIES_KEYS::DISTRIBUTION
				, MONTE_CARLO_PROPERTIES_KEYS::NUMBER_OF_PATHS
				, MONTE_CARLO_PROPERTIES_KEYS::ANTITHETIC_SAMPLING
				, MONTE_CARLO_PROPERTIES_KEYS::EVALUATE_IN_PARALLEL
				, MONTE_CARLO_PROPERTIES_KEYS::NUMBER_OF_ASSETS
			};

			std::vector<std::string> expectedKeys( arr, arr + sizeof( arr ) / sizeof( arr[0] ) );

			return expectedKeys;
		}

		// Returns an enum specifying the Random Number Generator ( Mersenne-Twister, Sobol etc )
		RandomNumberGeneratorEnum getGenerator() const { return generatorEnum_; }

		// Returns an enum specifying the distribution of the random numbers ( Uniform, Normal, etc )
		DistributionEnum getDistribution() const { return distributionEnum_; }

		// Returns whether anti-thetic sampling is used when generating the random numbers
		bool getUseAntitheticSamples() const { return useAntitheticSamples_; }

		// Whether the Monte-Carlo paths should be similated in parallel
		bool getEvaluatePathsInParallel() const { return evaluatePathsInParallel_; }

		// How many random numbers were originally requested in the mcParametersLVB
		int getNumberOfSamples() const { return nSamples_; }


	private:
		// The parameters that are parsed from the mcParametersLVB
		RandomNumberGeneratorEnum generatorEnum_;
		DistributionEnum distributionEnum_;
		bool useAntitheticSamples_;
		bool evaluatePathsInParallel_;
		int nSamples_;			// THis represents how many random number samples to generate per asset.
		int nStateVariables_;	// This represents the dimensionality / number of assets.

		// --------- Inner implementation classes ---------

		/* @brief	RandomImplBase specifies the API of random number generators.
		*			Sub-classes are required to implement getRandomNumbers() and getMultiDimRandomNumbers()
		*/
		class RandomImplBase
		{
		public:
			/* @brief	Base class constructor for all inner implementation classes.
			*			This class specifies the API for generating random numbers
			*  @param[in]	nStateVariables	The size of the random number sequence vector.
			*								This roughly translates into the number of assets to simulate.
			*								For single-asset problems, nStateVariables = 1.
			*								This parameter is important for random numbers generated by Sobol.
			*/
			RandomImplBase( const int nStateVariables );

			/* @brief	Declares an API to generate a 1-dimensional vector of random numbers
			*			i.e. for single asset monte carlo
			* @param[in]	nSamples		The number of random numbers to generate
			*
			*  @returns		A vector containing the random numbers
			*/
			virtual DoubleVector getRandomNumbers( const size_t nSamples ) = 0;

			/* @brief	Declares an API to generate an n-dimensional sequence of random numbers
			*			The sequence-size is set in the constructor.
			* @param[in]	nSamples		The size of the outer samples vector. Each sequence is an n-dimensional vector
			*								i.e. for n-dimensional monte-carlo
			*
			*  @returns		A vector containing the sequences
			*/
			virtual std::vector<DoubleVector> getMultiDimRandomNumbers( const size_t nSamples ) = 0;
			virtual ~RandomImplBase() {};

		protected:
			int nStateVariables_;
		};

		/* @brief	AntitheticRandomImplBase specifies the API for generators which are capable of generating
		 *			antithetic random number samples. Antithetic samples ensure that the sample mean of the distribution
		 *			is always correct.
		 */
		class AntitheticRandomImplBase : public RandomImplBase
		{
		public:
			/* @brief Constructor for Antithetic random number generators
			*  @param[in]	useAntitheticSamples	Whether to generate antithetic samples
			*  @param[in]	nStateVariables			The size of the random number sequence vector.
			*										This roughly translates into the number of assets to simulate.
			*										For single-asset problems, nStateVariables = 1.
			*										This parameter is important for random numbers generated by Sobol.		
			*/
			AntitheticRandomImplBase( const bool useAntitheticSamples, const int nStateVariables );

			/* @brief	Method to generate a 1-dimensional vector of random numbers
			*			i.e. for single asset monte carlo
			* @param[in]	nSamples		The number of random numbers to generate
			*
			*  @returns		A vector containing the random numbers
			*/
			virtual DoubleVector getRandomNumbers( const size_t nSamples );

			/* @brief	Method to generate an n-dimensional sequence of random numbers
			*			The sequence size is set in the constructor.
			* @param[in]	nSamples		The number of sequences to generate. Each sequence is an n-dimensional vector
			*								i.e. for n-dimensional monte-carlo
			*
			*  @returns		A vector containing the sequences
			*/
			virtual std::vector<DoubleVector> getMultiDimRandomNumbers( const size_t nSamples );

			/* @brief Returns the next random number computed by the random number generator
			*/
			virtual double getSample() = 0;

			/* @brief	Computes the antithetic sample corresponding to the provided sample.
			*			The antithetic sample ensures the distribution of samples always has the correct mean.
			* @param[in]	sample	The random sample
			* @returns		The antithetic sample corresponding to the input sample.
			*/
			virtual double getAntitheticSample( const double sample ) = 0;
			virtual ~AntitheticRandomImplBase() {};

		protected:
			bool useAntitheticSamples_;
		};

		// @brief Uniform distribution MersenneTwister implementation class
		class MersenneTwisterUniformImpl : public AntitheticRandomImplBase
		{
		public:
			/* Constructor for the  Uniform Distribution Mersenne Twister.
			*  @param[in]	minSample				The minimum possible value of the uniform distribution
			*  @param[in]	maxSample				The maximum possible value of the uniform distribution
			*  @param[in]	useAntitheticSamples	Whether to generate antithetic samples
			*  @param[in]	nStateVariables			The size of the random number sequence vector.
			*										This roughly translates into the number of assets to simulate.
			*										For single-asset problems, nStateVariables = 1.
			*/
			MersenneTwisterUniformImpl( const double minSample, const double maxSample, const bool useAntitheticSamples, const int nStateVariables );
			virtual ~MersenneTwisterUniformImpl() {};

			/* @brief Returns the next random number from a uniform distribution
			*/
			virtual double getSample();

			/* @brief	Computes the antithetic sample corresponding to the provided sample.
			*			The antithetic sample ensures the distribution of samples always has the correct mean.
			* @param[in]	sample	The random sample
			* @returns		The antithetic sample corresponding to the input sample.
			*/
			virtual double getAntitheticSample( const double sample );

		private:
			double minSample_;
			double maxSample_;
			
			boost::random::mt19937 mtGenerator_;
			boost::random::uniform_real_distribution<> uniformDistribution_;
		};

		// @brief Normal distribution MersenneTwister implementation class
		class MersenneTwisterNormalImpl : public AntitheticRandomImplBase
		{
		public:
			/* Constructor for the  Normally Distributed Mersenne Twister.
			*  @param[in]	mean					The minimum possible value of the uniform distribution
			*  @param[in]	standardDeviation		The maximum possible value of the uniform distribution
			*  @param[in]	useAntitheticSamples	Whether to generate antithetic samples
			*  @param[in]	nStateVariables			The size of the random number sequence vector.
			*										This roughly translates into the number of assets to simulate.
			*										For single-asset problems, nStateVariables = 1.
			*/
			MersenneTwisterNormalImpl( const double mean, const double standardDeviation, const bool useAntitheticSamples, const int nStateVariables );
			virtual ~MersenneTwisterNormalImpl() {};

			/* @brief Returns the next random number from a normal-distribution.
			*/
			virtual double getSample();

			/* @brief	Computes the antithetic sample corresponding to the provided sample.
			*			The antithetic sample ensures the distribution of samples always has the correct mean.
			* @param[in]	sample	The random sample
			* @returns		The antithetic sample corresponding to the input sample.
			*/
			virtual double getAntitheticSample( const double sample );

		private:
			double mean_;
			double stdDeviation_;

			boost::random::mt19937 mtGenerator_;
			boost::random::normal_distribution<> normalDistribution_;
		};


		// @brief Uniform Sobol sequence implementation class
		class SobolUniformImpl : public RandomImplBase
		{
		public:
			/* Constructor for the  Uniform Distribution Sobol quasi-random generator.
			*  @param[in]	minSample				The minimum possible value of the uniform distribution
			*  @param[in]	maxSample				The maximum possible value of the uniform distribution
			*  @param[in]	nStateVariables			The size of the random number sequence vector. i.e. the size of the Sobol sequence.
			*										This roughly translates into the number of assets to simulate.
			*										For single-asset problems, nStateVariables = 1.
			*/
			SobolUniformImpl( const double minSample, const double maxSample, const int nStateVariables );
			virtual ~SobolUniformImpl() {};

			/* @brief		Generates a vector of Sobol quasi-random numbers.
			*				NOTE: This generates a 1-dimensional sobol sequence (i.e. for single asset)
			*  @param[in]	nSamples		The number of sobol numbers to generate
			*
			*  @returns		A vector containing the sobol numbers
			*/
			virtual DoubleVector getRandomNumbers(const size_t nSamples );

			/* @brief		Generates a vector of Sobol sequences.
			*				Each sequence has dimension nStateVariables ( set in the constructor )
			*				and is designed to uniformly fill each dimension.
			*				
			*  @param[in]	nSamples		The number of sobol sequences to generate
			*
			*  @returns		A vector containing the sequences
			*/
			virtual std::vector<DoubleVector> getMultiDimRandomNumbers(const size_t nSamples );

		private:
			double minSample_;
			double maxSample_;
			QuantLib::SobolRsg sobolRsg_;
		};

		// The underlying implementation which generates the random sequence
		std::unique_ptr<RandomImplBase> implBase_;
	};

}


