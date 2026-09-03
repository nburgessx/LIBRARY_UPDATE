
/*
 * @brief			Tests of the Monte Carlo RandomNumberGenerator class.
 *
 * @Created:		18 Feb 2019
 * @Author:			Ian Castleton
 * @Department:		Quant Research & Analytics
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 *
 */

#include "RandomNumberGenerator.h"

// Include: Google Test Library
#include <gTest/gTest.h>

#include <vector>
#include "LAString.h"

#include <numeric>

TEST( TestRandomNumberGenerator, UNIT_TestMersenneTwisterUniform )
{
	const int numberOfSamples = 100;

	std::vector<LAString> rngType		= { "RandomNumberGenerator", "MersenneTwister" };
	std::vector<LAString> distribution	= { "Distribution", "Uniform" };
	std::vector<LAString> numPaths		= { "NumberOfPaths", LAString(numberOfSamples) };
	std::vector<LAString> antithetic	= { "AntitheticSampling", "TRUE" };
	std::vector<LAString> inParallel	= { "EvaluateInParallel", "FALSE" };

	LAStringMatrix mcParametersLVB = { rngType, distribution, numPaths, antithetic, inParallel };

	const double minSample = 10.0;
	const double maxSample = 20.0;
	etrading::RandomNumberGenerator uniformGenerator = etrading::RandomNumberGenerator::buildUniformGenerator( mcParametersLVB, minSample, maxSample );

	const DoubleVector samples = uniformGenerator.getRandomNumbers( numberOfSamples );

	const double sum = std::accumulate( samples.begin(), samples.end(), 0.0 );
	const double mean = sum / (double) numberOfSamples;

	const double tolerance = 1.0e-10;
	const double expectedMean = 0.5 * ( minSample + maxSample );
	EXPECT_NEAR( mean, expectedMean, tolerance );
}

TEST( TestRandomNumberGenerator, UNIT_TestMersenneTwisterNormal )
{
	const int numberOfSamples = 100;

	std::vector<LAString> rngType		= { "RandomNumberGenerator", "MersenneTwister" };
	std::vector<LAString> distribution	= { "Distribution", "Normal" };
	std::vector<LAString> numPaths		= { "NumberOfPaths", LAString(numberOfSamples) };
	std::vector<LAString> antithetic	= { "AntitheticSampling", "TRUE" };
	std::vector<LAString> inParallel	= { "EvaluateInParallel", "FALSE" };

	LAStringMatrix mcParametersLVB = { rngType, distribution, numPaths, antithetic, inParallel };

	// --- Test Standard Normals ---
	const double standardNormalMean = 0.0;
	const double standardNormalStdDeviation = 1.0;
	etrading::RandomNumberGenerator stamdardNormalGenerator = etrading::RandomNumberGenerator::buildNormalGenerator( mcParametersLVB, standardNormalMean, standardNormalStdDeviation );
	
	const DoubleVector standardSamples = stamdardNormalGenerator.getRandomNumbers( numberOfSamples );

	const double standardSum = std::accumulate( standardSamples.begin(), standardSamples.end(), 0.0 );
	const double calculatedMean = standardSum / (double) numberOfSamples;
	
	// The mean of the standard normal distribution should be 0.0
	const double tolerance = 1.0e-10;
	EXPECT_NEAR( calculatedMean, standardNormalMean, tolerance );

	// --- Test Normals that have non standard mean ---
	const double normalMean = 125.0;
	const double normalStdDeviation = 31.0;
	etrading::RandomNumberGenerator normalGenerator = etrading::RandomNumberGenerator::buildNormalGenerator( mcParametersLVB, normalMean, normalStdDeviation );

	const DoubleVector samples = normalGenerator.getRandomNumbers( numberOfSamples );

	const double sum = std::accumulate( samples.begin(), samples.end(), 0.0 );
	const double mean = sum / (double) numberOfSamples;

	// The mean of the normal distribution should be 125.0
	EXPECT_NEAR( mean, normalMean, tolerance );
}

TEST( TestRandomNumberGenerator, UNIT_TestSobolSequence )
{
	const int numberOfSamples = 1023; // 2^n-1

	std::vector<LAString> rngType		= { "RandomNumberGenerator", "Sobol" };
	std::vector<LAString> distribution	= { "Distribution", "Uniform" };
	std::vector<LAString> numPaths		= { "NumberOfPaths", LAString(numberOfSamples) };
	std::vector<LAString> antithetic	= { "AntitheticSampling", "FALSE" };
	std::vector<LAString> inParallel	= { "EvaluateInParallel", "FALSE" };

	LAStringMatrix mcParametersLVB = { rngType, distribution, numPaths, antithetic, inParallel };

	const double minSample = 10.0;
	const double maxSample = 20.0;
	etrading::RandomNumberGenerator sobolUniform = etrading::RandomNumberGenerator::buildUniformGenerator( mcParametersLVB, minSample, maxSample );

	const DoubleVector samples = sobolUniform.getRandomNumbers( numberOfSamples );

	const double sum = std::accumulate( samples.begin(), samples.end(), 0.0 );
	const double mean = sum / (double) numberOfSamples;

	const double tolerance = 1.0e-10;
	const double expectedMean = 0.5 * ( minSample + maxSample );
	EXPECT_NEAR( mean, expectedMean, tolerance );

}
