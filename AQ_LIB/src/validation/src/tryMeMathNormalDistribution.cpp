#include "tryMeMathNormalDistribution.h"
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
    /* @brief			Validation method for meMathNormalDistributionStandard
    *  @param [in]		z			The Standard Normal Variate z
    *  @return			phi         The Standard Normal Probability phi
    */
    const double tryMeMathNormalDistributionStandard( const double & z )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        RECORD_INPUTS( z );

        // Calculation
        const double phi = etrading::standardNormalDistribution( z );
        
        // Record Outputs AND Return the Result for logs, tests and playback
        RECORD_OUTPUTS_AND_RETURN_RESULT( phi );

		VALID_EXCEPTION_END
    }


    /* @brief			Validation method for meMathNormalDistributionStandardPDF
    *  @param [in]		z			The Standard Normal Variate z
    *  @return			phi         The Standard Normal Probability Density Function value, PDF
    */
    const double tryMeMathNormalDistributionStandardPDF( const double & z )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        RECORD_INPUTS( z );

        // Calculation
        const double PDF = etrading::standardNormalDistributionPDF( z );
        
        // Record Outputs AND Return the Result for logs, tests and playback
        RECORD_OUTPUTS_AND_RETURN_RESULT( PDF );

		VALID_EXCEPTION_END
    }


    /* @brief			Validation method for meMathNormalDistributionStandardInverse
    *  @param [in]		phi         The Standard Normal Probability phi
    *  @return			z			The Standard Normal Variate z
    */
    const double tryMeMathNormalDistributionStandardInverse( const double & phi )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        RECORD_INPUTS( phi );

        // Calculation
        const double z = etrading::standardNormalDistributionInverse( phi );
        
        // Record Outputs AND Return the Result for logs, tests and playback
        RECORD_OUTPUTS_AND_RETURN_RESULT( z );

		VALID_EXCEPTION_END
    }


    /* @brief			Validation method for meMathNormalDistribution
    *  @param [in]		x			The Non-Standard Normal Variate x
    *  @param [in]		mean		The Normal Distribution mean
    *  @param [in]		variance	The Normal Distribution variance
    *  @return			phi         The Standard Normal Probability phi
    */
    const double tryMeMathNormalDistribution( const double & x, const double & mean, const double & variance )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        RECORD_INPUTS( x, mean, variance );

        // Calculation
        const double phi = etrading::normalDistribution( x, mean, variance );
        
        // Record Outputs AND Return the Result for logs, tests and playback
        RECORD_OUTPUTS_AND_RETURN_RESULT( phi );

		VALID_EXCEPTION_END
    }


    /* @brief			Validation method for meMathNormalDistributionPDF
    *  @param [in]		x			The Non-Standard Normal Variate x
    *  @param [in]		mean		The Normal Distribution mean
    *  @param [in]		variance	The Normal Distribution variance
    *  @return			phi         The Standard Normal Probability Density Function value PDF
    */
    const double tryMeMathNormalDistributionPDF( const double & x, const double & mean, const double & variance )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        RECORD_INPUTS( x, mean, variance );

        // Calculation
        const double PDF = etrading::normalDistributionPDF( x, mean, variance );
        
        // Record Outputs AND Return the Result for logs, tests and playback
        RECORD_OUTPUTS_AND_RETURN_RESULT( PDF );

		VALID_EXCEPTION_END
    }


    /* @brief			Validation method for meMathNormalDistributionInverse
    *  @param [in]		phi         The Normal Probability phi
    *  @param [in]		mean		The Normal Distribution mean
    *  @param [in]		variance	The Normal Distribution variance
    *  @return			x			The non-Standard Normal Variate x
    */
    const double tryMeMathNormalDistributionInverse( const double & phi, const double & mean, const double & variance )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
        RECORD_INPUTS( phi, mean, variance );

        // Calculation
        const double x = etrading::normalDistributionInverse( phi, mean, variance );
        
        // Record Outputs AND Return the Result for logs, tests and playback
        RECORD_OUTPUTS_AND_RETURN_RESULT( x );

		VALID_EXCEPTION_END
    }
}