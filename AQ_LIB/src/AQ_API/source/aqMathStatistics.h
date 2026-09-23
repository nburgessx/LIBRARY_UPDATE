// aqMathStatistics.h

/*
 * @brief			Swig interface for the aqMathNormalDistribution* functions
 */

#pragma once

/* @brief			swig interface for aqMathNormalDistributionStandard
*  @param [in]		z			The Standard Normal Variate z
*  @return			phi			The Standard Normal Probability phi
*/
double aqMathNormalDistributionStandard( const double z );

/* @brief			swig interface for aqMathNormalDistributionStandardPDF
*  @param [in]		z			The Standard Normal Variate z
*  @return			phi			The Standard Normal Probability Density Function value, PDF
*/
double aqMathNormalDistributionStandardPDF( const double z );

/* @brief			swig interface for aqMathNormalDistributionStandardInverse
*  @param [in]		phi			The Standard Normal Probability phi
*  @return			z			The Standard Normal Variate z
*/
double aqMathNormalDistributionStandardInverse( const double phi );

/* @brief			swig interface for aqMathNormalDistribution
*  @param [in]		x			The non-Standard Normal Variate x
*  @param [in]		mean		The Normal Distribution mean
*  @param [in]		variance	The Normal Distribution variance
*  @return			phi			The Normal Probability phi
*/
double aqMathNormalDistribution( const double x, const double mean, const double variance );

/* @brief			swig interface for aqMathNormalDistributionPDF
*  @return			phi			The Normal Probability Density Function value, PDF
*/
double aqMathNormalDistributionPDF( const double x, const double mean, const double variance );

/* @brief			swig interface for aqMathNormalDistributionInverse
*  @param [in]		phi			The Normal Probability phi
*  @return			x			The non-Standard Normal Variate x
*/
double aqMathNormalDistributionInverse( const double phi, const double mean, const double variance );
