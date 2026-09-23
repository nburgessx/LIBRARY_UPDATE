// aqMathStatistics.cpp

/*
 * @brief			Swig interface for the aqMathNormalDistribution* functions
 */

#include "aqMathStatistics.h"
#include "TypeUtilities.h"
#include "tryAqMathNormalDistribution.h"
#include "APISetUp.h"               // AQ_API_START and AQ_API_END Macros

double aqMathNormalDistributionStandard( const double z )
{
    AQ_API_START

    double result = validation::tryAqMathNormalDistributionStandard( z );
    return result;

    AQ_API_END
}

double aqMathNormalDistributionStandardPDF( const double z )
{
    AQ_API_START

    double result = validation::tryAqMathNormalDistributionStandardPDF( z );
    return result;

    AQ_API_END
}

double aqMathNormalDistributionStandardInverse( const double phi )
{
    AQ_API_START

    double result = validation::tryAqMathNormalDistributionStandardInverse( phi );
    return result;

    AQ_API_END
}

double aqMathNormalDistribution( const double x, const double mean, const double variance )
{
    AQ_API_START

    double result = validation::tryAqMathNormalDistribution( x, mean, variance );
    return result;

    AQ_API_END
}

double aqMathNormalDistributionPDF( const double x, const double mean, const double variance )
{
    AQ_API_START

    double result = validation::tryAqMathNormalDistributionPDF( x, mean, variance );
    return result;

    AQ_API_END
}

double aqMathNormalDistributionInverse( const double phi, const double mean, const double variance )
{
    AQ_API_START

    double result = validation::tryAqMathNormalDistributionInverse( phi, mean, variance );
    return result;

    AQ_API_END
}
