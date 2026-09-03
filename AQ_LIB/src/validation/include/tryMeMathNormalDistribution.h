#pragma once

#include "NormalDistribution.h"

namespace validation
{

    /* @brief			Validation method for meMathNormalDistributionStandard
    *  @param [in]		z			The Standard Normal Variate z
    *  @return			phi         The Standard Normal Probability phi
    */
    const double tryMeMathNormalDistributionStandard( const double & z );



    /* @brief			Validation method for meMathNormalDistributionStandardPDF
    *  @param [in]		z			The Standard Normal Variate z
    *  @return			phi         The Standard Normal Probability Density Function value, PDF
    */
    const double tryMeMathNormalDistributionStandardPDF( const double & z );


    /* @brief			Validation method for meMathNormalDistributionStandardInverse
    *  @param [in]		phi         The Standard Normal Probability phi
    *  @return			z			The Standard Normal Variate z
    */
    const double tryMeMathNormalDistributionStandardInverse( const double & phi );


    /* @brief			Validation method for meMathNormalDistribution
    *  @param [in]		x			The Non-Standard Normal Variate x
    *  @param [in]		mean		The Normal Distribution mean
    *  @param [in]		variance	The Normal Distribution variance
    *  @return			phi         The Standard Normal Probability phi
    */
    const double tryMeMathNormalDistribution( const double & x, const double & mean, const double & variance );


    /* @brief			Validation method for meMathNormalDistributionPDF
    *  @param [in]		x			The Non-Standard Normal Variate x
    *  @param [in]		mean		The Normal Distribution mean
    *  @param [in]		variance	The Normal Distribution variance
    *  @return			phi         The Standard Normal Probability Density Function value PDF
    */
    const double tryMeMathNormalDistributionPDF( const double & x, const double & mean, const double & variance );


    /* @brief			Validation method for meMathNormalDistributionInverse
    *  @param [in]		phi         The Normal Probability phi
    *  @param [in]		mean		The Normal Distribution mean
    *  @param [in]		variance	The Normal Distribution variance
    *  @return			x			The non-Standard Normal Variate x
    */
    const double tryMeMathNormalDistributionInverse( const double & phi, const double & mean, const double & variance );
}
                               