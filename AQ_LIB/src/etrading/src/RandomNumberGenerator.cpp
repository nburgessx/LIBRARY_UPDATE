#include "RandomNumberGenerator.h"
#include "ExceptionMacros.h"
#include "LabelValueBlockValidation.h"


namespace etrading
{
	
	/* @brief		Constructor of the RandomNumberGenerator class.
	*  @param[in]	mcParametersLVB		A label-value-block containing Monte-Carlo / Random number parameters
	*  @param[in]	minSample			For uniform distributions, a lower bound on the random number.
	*  @param[in]	maxSample			For uniform distributions, an upper bound on the random number.
	*  @param[in]	mean				For normal distributions, the mean of the distribution
	*  @param[in]	stdDeviation		For normal distributions, the standard deviation of the distribution
	*/
	RandomNumberGenerator::RandomNumberGenerator( const LabelValueBlock& mcParametersLVB, const double minSample, const double maxSample, const double mean, const double stdDeviation )
	{
		const bool validateKeys = true;
		const std::string propertyName = "MonteCarloParameters";
		validateKeysForLVB( montecarlo_parameters_lvbKeys(), mcParametersLVB.getKeys(), validateKeys, propertyName );

		generatorEnum_			= toRandomNumberGeneratorEnum( mcParametersLVB.getCompulsoryValue( MONTE_CARLO_PROPERTIES_KEYS::RANDOM_NUMBER_GENERATOR ) );
		distributionEnum_		= toDistributionEnum( mcParametersLVB.getCompulsoryValue( MONTE_CARLO_PROPERTIES_KEYS::DISTRIBUTION ) );
		useAntitheticSamples_	= mcParametersLVB.getOptionalValueAsBool( MONTE_CARLO_PROPERTIES_KEYS::ANTITHETIC_SAMPLING, false );
		evaluatePathsInParallel_= mcParametersLVB.getOptionalValueAsBool( MONTE_CARLO_PROPERTIES_KEYS::EVALUATE_IN_PARALLEL, false );
		nSamples_				= (int) mcParametersLVB.getCompulsoryValueAsDouble( MONTE_CARLO_PROPERTIES_KEYS::NUMBER_OF_PATHS );
		nStateVariables_		= (int) mcParametersLVB.getOptionalValueAsUnsignedInt( MONTE_CARLO_PROPERTIES_KEYS::NUMBER_OF_ASSETS, 1);

		switch ( generatorEnum_ )
		{
			case MERSENNE_TWISTER_GENERATOR:
			{
				switch ( distributionEnum_ )
				{
					case UNIFORM_DISTRIBUTION:
						implBase_ = std::unique_ptr<RandomImplBase>( new MersenneTwisterUniformImpl( minSample, maxSample, useAntitheticSamples_, nStateVariables_ ) );
						break;

					case NORMAL_DISTRIBUTION:
						implBase_ = std::unique_ptr<RandomImplBase>( new MersenneTwisterNormalImpl( mean, stdDeviation, useAntitheticSamples_, nStateVariables_ ) );
						break;

					default:
						MLIB_THROW( "Unsupported distribution type " + toString( distributionEnum_ ) + " with MersenneTwister generator");
						break;
				}
				break;
			}
			case SOBOL_GENERATOR:
			{
				switch ( distributionEnum_ )
				{
					case UNIFORM_DISTRIBUTION:
						implBase_ = std::unique_ptr<RandomImplBase>( new SobolUniformImpl( minSample, maxSample, nStateVariables_ ) );
						break;

					default:
						MLIB_THROW( "Unsupported distribution type " + toString( distributionEnum_ ) + " with Sobol generator");
						break;
				}
				break;
			}
			default:
				MLIB_THROW( "Only MersenneTwister or Sobol generator is currently supported." );
		}

	}

	/* @brief		Helper static method which ensures a Uniform Random Number Generator is constructed correctly.
	*  @param[in]	mcParametersLVB		A label-value-block containing Monte-Carlo / Random number parameters
	*  @param[in]	minSample			For uniform distributions, a lower bound on the random number.
	*  @param[in]	maxSample			For uniform distributions, an upper bound on the random number.
	*  @returns		An initialized instance of the RandomNumberGenerator class
	*/
	RandomNumberGenerator RandomNumberGenerator::buildUniformGenerator( const LabelValueBlock& mcParametersLVB, const double minSample, const double maxSample )
	{
		auto distributionEnum = toDistributionEnum( mcParametersLVB.getCompulsoryValue( MONTE_CARLO_PROPERTIES_KEYS::DISTRIBUTION ) );
		MLIB_REQUIRE( distributionEnum == UNIFORM_DISTRIBUTION, "Incorrect setup of Random Number Generator. Cannot build a Uniform generator from this config." );

		const double unusedMean = std::numeric_limits<double>::quiet_NaN();
		const double unusedStdDeviation = std::numeric_limits<double>::quiet_NaN();
		return RandomNumberGenerator( mcParametersLVB, minSample, maxSample, unusedMean, unusedStdDeviation );
	}

	/* @brief		Helper static method which ensures a Normal Random Number Generator is constructed correctly.
	*  @param[in]	mcParametersLVB		A label-value-block containing Monte-Carlo / Random number parameters
	*  @param[in]	mean				For normal distributions, the mean of the distribution
	*  @param[in]	stdDeviation		For normal distributions, the standard deviation of the distribution
	*  @returns		An initialized instance of the RandomNumberGenerator class
	*/
	RandomNumberGenerator RandomNumberGenerator::buildNormalGenerator( const LabelValueBlock& mcParametersLVB, const double mean, const double stdDeviation )
	{
		auto distributionEnum = toDistributionEnum( mcParametersLVB.getCompulsoryValue( MONTE_CARLO_PROPERTIES_KEYS::DISTRIBUTION ) );
		MLIB_REQUIRE( distributionEnum == NORMAL_DISTRIBUTION, "Incorrect setup of Random Number Generator. Cannot build a Normal generator from this config." );

		const double unusedMinSample = std::numeric_limits<double>::quiet_NaN();
		const double unusedMaxSample = std::numeric_limits<double>::quiet_NaN();
		return RandomNumberGenerator( mcParametersLVB, unusedMinSample, unusedMaxSample, mean, stdDeviation );
	}
	
	/* @brief		Generates a vector of random numbers.
	*  @param[in]	nSamples		The number of random numbers to generate
	*
	*  @returns		A vector containing the random numbers
	*/
	DoubleVector RandomNumberGenerator::getRandomNumbers( const size_t nSamples )
	{
		return implBase_->getRandomNumbers( nSamples );
	}

	/* @brief		Generates a vector of sequences.
	*				Each sequence contains a random number per dimension (asset)
	*  @param[in]	nSamples		The number of sample sequences to generate
	*
	*  @returns		A vector containing the sequences
	*/
	std::vector<DoubleVector> RandomNumberGenerator::getMultiDimRandomNumbers( const size_t nSamples )
	{
		return implBase_->getMultiDimRandomNumbers( nSamples );
	}

	// --------- Inner implementation classes ---------

	/* @brief	Base class constructor for all inner implementation classes.
	*			This class specifies the API for generating random numbers
	*  @param[in]	nStateVariables		The size of the random number sequence vector.
	*									This roughly translates into the number of assets to simulate.
	*									For single-asset problems, nStateVariables = 1.
	*									This parameter is important for random numbers generated by Sobol.
	*/
	RandomNumberGenerator::RandomImplBase::RandomImplBase( const int nStateVariables )
		: nStateVariables_( nStateVariables )
	{}

	/* @brief Constructor for Antithetic random number generators
	*  @param[in]	useAntitheticSamples	Whether to generate antithetic samples
	*  @param[in]	nStateVariables			The size of the random number sequence vector.
	*										This roughly translates into the number of assets to simulate.
	*										For single-asset problems, nStateVariables = 1.
	*										This parameter is important for random numbers generated by Sobol.		
	*/
	RandomNumberGenerator::AntitheticRandomImplBase::AntitheticRandomImplBase( const bool useAntitheticSamples, const int nStateVariables )
		: RandomImplBase( nStateVariables ),
		useAntitheticSamples_( useAntitheticSamples )
	{}

	/* @brief	Method to generate a 1-dimensional vector of random numbers
	*			i.e. for single asset monte carlo
	* @param[in]	nSamples		The number of random numbers to generate
	*
	*  @returns		A vector containing the random numbers
	*/
	DoubleVector RandomNumberGenerator::AntitheticRandomImplBase::getRandomNumbers( const size_t nSamples )
	{
		DoubleVector samples( nSamples );

		if ( useAntitheticSamples_ )
		{
			// Check if nSamples is odd
			if ( (nSamples % 2) == 1 )
			{
				// We have been asked to use antithetic samples, which generates an even number of sample points
				// However nSamples is odd, so we extend the samples vector by 1.
				samples.resize( nSamples + 1 );
			}

			for ( size_t i=0; i<nSamples; i+=2 )
			{
				const double sample = getSample();
				samples[i] = sample;

				// ... now calculate the corresponding antithetic sample
				const double antitheticSample = getAntitheticSample( sample );
				samples[i+1] = antitheticSample;
			}
		}
		else
		{
			// Not using antithetic samples
			for ( size_t i=0; i<nSamples; i++ )
			{
				const double sample = getSample();
				samples[i] = sample;
			}
		}
		return samples;
	}

	/* @brief	Method to generate an n-dimensional sequence of random numbers
	*			The sequence size is set in the constructor.
	* @param[in]	nSamples		The number of sequences to generate. Each sequence is an n-dimensional vector
	*								i.e. for n-dimensional monte-carlo
	*
	*  @returns		A vector containing the sequences
	*/
	std::vector<DoubleVector> RandomNumberGenerator::AntitheticRandomImplBase::getMultiDimRandomNumbers( const size_t nSamples )
	{
		std::vector<DoubleVector> samples( nSamples );

		if ( useAntitheticSamples_ )
		{
			// Check if nSamples is odd
			if ( (nSamples % 2) == 1 )
			{
				// We have been asked to use antithetic samples, which generates an even number of sample points
				// However nSamples is odd, so we extend the samples vector by 1.
				samples.resize( nSamples + 1 );
			}

			for ( size_t i=0; i<nSamples; i+=2 )
			{
				std::vector<double> sampleVector( nStateVariables_ );
				for ( size_t j=0; j < (size_t) nStateVariables_; j++ )
				{
					const double sample = getSample();
					sampleVector[ j ] = sample;
				}
				samples[i] = sampleVector;

				// ... now calculate the corresponding antithetic sample
				std::vector<double> antithetics( nStateVariables_ );
				for ( size_t j=0; j < (size_t) nStateVariables_; j++ )
				{
					const double antitheticSample = getAntitheticSample( sampleVector[ j ]);
					antithetics[ j ] = antitheticSample;
				}
				samples[i+1] = antithetics;
			}
		}
		else
		{
			// Not using antithetic samples
			for ( size_t i=0; i<nSamples; i++ )
			{
				std::vector<double> sequence( nStateVariables_ );
				for ( size_t j=0; j < (size_t) nStateVariables_; j++ )
				{
					const double sample = getSample();
					sequence[ j ] = sample;
				}
				samples[i] = sequence;
			}
		}
		return samples;	
	
	}

	/* Constructor for the  Uniform Distribution Mersenne Twister.
	*  @param[in]	minSample				The minimum possible value of the uniform distribution
	*  @param[in]	maxSample				The maximum possible value of the uniform distribution
	*  @param[in]	useAntitheticSamples	Whether to generate antithetic samples
	*  @param[in]	nStateVariables			The size of the random number sequence vector.
	*										This roughly translates into the number of assets to simulate.
	*										For single-asset problems, nStateVariables = 1.
	*/
	RandomNumberGenerator::MersenneTwisterUniformImpl::MersenneTwisterUniformImpl( const double minSample, const double maxSample, const bool useAntitheticSamples, const int nStateVariables )
		: AntitheticRandomImplBase( useAntitheticSamples, nStateVariables ),
		minSample_( minSample),
		maxSample_( maxSample ),
		mtGenerator_( 123 /* seed */ ),
		uniformDistribution_( minSample, maxSample )
	{}

	/* @brief Returns the next random number from a uniform distribution
	*/
	double RandomNumberGenerator::MersenneTwisterUniformImpl::getSample()
	{
		const double sample = uniformDistribution_( mtGenerator_ );
		return sample;
	}

	/* @brief	Computes the antithetic sample corresponding to the provided sample.
	*			The antithetic sample ensures the distribution of samples always has the correct mean.
	* @param[in]	sample	The random sample
	* @returns		The antithetic sample corresponding to the input sample.
	*/
	double RandomNumberGenerator::MersenneTwisterUniformImpl::getAntitheticSample( const double sample )
	{
		const double antitheticSample = minSample_ + ( maxSample_ - sample );
		return antitheticSample;
	}

	/* Constructor for the  Normally Distributed Mersenne Twister.
	*  @param[in]	mean					The minimum possible value of the uniform distribution
	*  @param[in]	standardDeviation		The maximum possible value of the uniform distribution
	*  @param[in]	useAntitheticSamples	Whether to generate antithetic samples
	*  @param[in]	nStateVariables			The size of the random number sequence vector.
	*										This roughly translates into the number of assets to simulate.
	*										For single-asset problems, nStateVariables = 1.
	*/
	RandomNumberGenerator::MersenneTwisterNormalImpl::MersenneTwisterNormalImpl( const double mean, const double stdDeviation, const bool useAntitheticSamples, const int nStateVariables )
		: AntitheticRandomImplBase( useAntitheticSamples, nStateVariables ),
		mean_( mean ),
		stdDeviation_( stdDeviation ),
		mtGenerator_( 123  ),
		normalDistribution_( mean, stdDeviation )
	{}

	/* @brief Returns the next random number from a normal-distribution.
	*/
	double RandomNumberGenerator::MersenneTwisterNormalImpl::getSample()
	{
		const double sample = normalDistribution_( mtGenerator_ );
		return sample;
	}

	/* @brief	Computes the antithetic sample corresponding to the provided sample.
	*			The antithetic sample ensures the distribution of samples always has the correct mean.
	* @param[in]	sample	The random sample
	* @returns		The antithetic sample corresponding to the input sample.
	*/
	double RandomNumberGenerator::MersenneTwisterNormalImpl::getAntitheticSample( const double sample )
	{
		const double antitheticSample = 2.0 * mean_ - sample;
		return antitheticSample;
	}

	/* Constructor for the  Uniform Distribution Sobol quasi-random generator.
	*  @param[in]	minSample				The minimum possible value of the uniform distribution
	*  @param[in]	maxSample				The maximum possible value of the uniform distribution
	*  @param[in]	nStateVariables			The size of the random number sequence vector. i.e. the size of the Sobol sequence.
	*										This roughly translates into the number of assets to simulate.
	*										For single-asset problems, nStateVariables = 1.
	*/
	RandomNumberGenerator::SobolUniformImpl::SobolUniformImpl( const double minSample, const double maxSample, const int nStateVariables )
		: RandomImplBase( nStateVariables ),
		minSample_( minSample ),
		maxSample_( maxSample ),
		sobolRsg_( nStateVariables /* i.e. number of assets */ )
	{}

	/* @brief		Generates a vector of Sobol quasi-random numbers.
	*				NOTE: This generates a 1-dimensional sobol sequence (i.e. for single asset)
	*  @param[in]	nSamples		The number of sobol numbers to generate
	*
	*  @returns		A vector containing the sobol numbers
	*/
	DoubleVector RandomNumberGenerator::SobolUniformImpl::getRandomNumbers( const size_t nSamples )
	{
		DoubleVector samples( nSamples );

		const double scaleFactor = ( maxSample_ - minSample_ );
		for ( size_t i=0; i<nSamples; i++ )
		{
			const double sobolUniform = sobolRsg_.nextSequence().value[0];
			const double scaledUniform = minSample_ + sobolUniform * scaleFactor;
			samples[i] = scaledUniform;
		}

		return samples;
	}

	/* @brief		Generates a vector of Sobol sequences.
	*				Each sequence has dimension nStateVariables ( set in the constructor )
	*				and is designed to uniformly fill each dimension.
	*				
	*  @param[in]	nSamples		The number of sobol sequences to generate
	*
	*  @returns		A vector containing the sequences
	*/
	std::vector<DoubleVector> RandomNumberGenerator::SobolUniformImpl::getMultiDimRandomNumbers( const size_t nSamples )
	{
		std::vector<DoubleVector> samples( nSamples );

		const double scaleFactor = ( maxSample_ - minSample_ );
		for ( size_t i=0; i<nSamples; i++ )
		{
			const std::vector<double> sobolUniforms = sobolRsg_.nextSequence().value;

			std::vector<double> sequence( sobolUniforms.size() );
			for ( size_t i=0; i<sobolUniforms.size(); i++ )
			{
				const double scaledUniform = minSample_ + sobolUniforms[ i ] * scaleFactor;
				sequence[ i ] = scaledUniform;
			}

			samples[i] = sequence;
		}
		return samples;
	}

}

