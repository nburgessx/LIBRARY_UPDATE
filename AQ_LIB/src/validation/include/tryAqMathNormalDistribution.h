#pragma once

#include "NormalDistribution.h"

namespace validation
{

    /* @brief			Validation method for aqMathNormalDistributionStandard
    *  @param [in]		z			The Standard Normal Variate z
    *  @return			phi         The Standard Normal Probability phi
    */
    const double tryAqMathNormalDistributionStandard( const double & z );



    /* @brief			Validation method for aqMathNormalDistributionStandardPDF
    *  @param [in]		z			The Standard Normal Variate z
    *  @return			phi         The Standard Normal Probability Density Function value, PDF
    */
    const double tryAqMathNormalDistributionStandardPDF( const double & z );


    /* @brief			Validation method for aqMathNormalDistributionStandardInverse
    *  @param [in]		phi         The Standard Normal Probability phi
    *  @return			z			The Standard Normal Variate z
    */
    const double tryAqMathNormalDistributionStandardInverse( const double & phi );


    /* @brief			Validation method for aqMathNormalDistribution
    *  @param [in]		x			The Non-Standard Normal Variate x
    *  @param [in]		mean		The Normal Distribution mean
    *  @param [in]		variance	The Normal Distribution variance
    *  @return			phi         The Standard Normal Probability phi
    */
    const double tryAqMathNormalDistribution( const double & x, const double & mean, const double & variance );


    /* @brief			Validation method for aqMathNormalDistributionPDF
    *  @param [in]		x			The Non-Standard Normal Variate x
    *  @param [in]		mean		The Normal Distribution mean
    *  @param [in]		variance	The Normal Distribution variance
    *  @return			phi         The Standard Normal Probability Density Function value PDF
    */
    const double tryAqMathNormalDistributionPDF( const double & x, const double & mean, const double & variance );


    /* @brief			Validation method for aqMathNormalDistributionInverse
    *  @param [in]		phi         The Normal Probability phi
    *  @param [in]		mean		The Normal Distribution mean
    *  @param [in]		variance	The Normal Distribution variance
    *  @return			x			The non-Standard Normal Variate x
    */
    const double tryAqMathNormalDistributionInverse( const double & phi, const double & mean, const double & variance );
}
                               