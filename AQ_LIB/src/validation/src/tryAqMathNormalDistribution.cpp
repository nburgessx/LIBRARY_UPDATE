#include "tryAqMathNormalDistribution.h"
#include "NormalDistribution.h"

// Includes for Structured Exception Handling and Input/Output Logging
#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "CommonConstants.h"
#include "RecordMacros.h"

// Namespaces for Input/Output Logging
using etrading::CreateDataFile;
using etrading::decorateFilename;


namespace validation
{
    /* @brief			Validation method for aqMathNormalDistributionStandard
    *  @param [in]		z			The Standard Normal Variate z
    *  @return			phi         The Standard Normal Probability phi
    */
    const double tryAqMathNormalDistributionStandard( const double & z )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        AQ_RECORD_INPUTS( z );

        // Calculation
        const double phi = etrading::standardNormalDistribution( z );
        
        // Record Outputs AND Return the Result for logs, tests and playback
        AQ_RECORD_OUTPUTS_AND_RETURN_RESULT( phi );

		VALID_EXCEPTION_END
    }


    /* @brief			Validation method for aqMathNormalDistributionStandardPDF
    *  @param [in]		z			The Standard Normal Variate z
    *  @return			phi         The Standard Normal Probability Density Function value, PDF
    */
    const double tryAqMathNormalDistributionStandardPDF( const double & z )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        AQ_RECORD_INPUTS( z );

        // Calculation
        const double PDF = etrading::standardNormalDistributionPDF( z );
        
        // Record Outputs AND Return the Result for logs, tests and playback
        AQ_RECORD_OUTPUTS_AND_RETURN_RESULT( PDF );

		VALID_EXCEPTION_END
    }


    /* @brief			Validation method for aqMathNormalDistributionStandardInverse
    *  @param [in]		phi         The Standard Normal Probability phi
    *  @return			z			The Standard Normal Variate z
    */
    const double tryAqMathNormalDistributionStandardInverse( const double & phi )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        AQ_RECORD_INPUTS( phi );

        // Calculation
        const double z = etrading::standardNormalDistributionInverse( phi );
        
        // Record Outputs AND Return the Result for logs, tests and playback
        AQ_RECORD_OUTPUTS_AND_RETURN_RESULT( z );

		VALID_EXCEPTION_END
    }


    /* @brief			Validation method for aqMathNormalDistribution
    *  @param [in]		x			The Non-Standard Normal Variate x
    *  @param [in]		mean		The Normal Distribution mean
    *  @param [in]		variance	The Normal Distribution variance
    *  @return			phi         The Standard Normal Probability phi
    */
    const double tryAqMathNormalDistribution( const double & x, const double & mean, const double & variance )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        AQ_RECORD_INPUTS( x, mean, variance );

        // Calculation
        const double phi = etrading::normalDistribution( x, mean, variance );
        
        // Record Outputs AND Return the Result for logs, tests and playback
        AQ_RECORD_OUTPUTS_AND_RETURN_RESULT( phi );

		VALID_EXCEPTION_END
    }


    /* @brief			Validation method for aqMathNormalDistributionPDF
    *  @param [in]		x			The Non-Standard Normal Variate x
    *  @param [in]		mean		The Normal Distribution mean
    *  @param [in]		variance	The Normal Distribution variance
    *  @return			phi         The Standard Normal Probability Density Function value PDF
    */
    const double tryAqMathNormalDistributionPDF( const double & x, const double & mean, const double & variance )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        AQ_RECORD_INPUTS( x, mean, variance );

        // Calculation
        const double PDF = etrading::normalDistributionPDF( x, mean, variance );
        
        // Record Outputs AND Return the Result for logs, tests and playback
        AQ_RECORD_OUTPUTS_AND_RETURN_RESULT( PDF );

		VALID_EXCEPTION_END
    }


    /* @brief			Validation method for aqMathNormalDistributionInverse
    *  @param [in]		phi         The Normal Probability phi
    *  @param [in]		mean		The Normal Distribution mean
    *  @param [in]		variance	The Normal Distribution variance
    *  @return			x			The non-Standard Normal Variate x
    */
    const double tryAqMathNormalDistributionInverse( const double & phi, const double & mean, const double & variance )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        AQ_RECORD_INPUTS( phi, mean, variance );

        // Calculation
        const double x = etrading::normalDistributionInverse( phi, mean, variance );
        
        // Record Outputs AND Return the Result for logs, tests and playback
        AQ_RECORD_OUTPUTS_AND_RETURN_RESULT( x );

		VALID_EXCEPTION_END
    }
}