#include "tryAqToolDate.h"

#include "CreateDataFile.h"
#include "CurveValidation.h"
#include "Environment.h"
#include "AQLCurveForwardRateHelpers.h"
#include "AQLDateScheduleHelpers.h"
#include "ParameterValidation.h"
#include "RecordMacros.h"
#include "StructuredExceptionHandler.h"
#include "tryAqCurveDiscountFactor.h" // needed for the utility functions
#include "tryAqDate.h"

using etrading::CreateDataFile;
using etrading::decorateCurvename;

namespace validation
{

    
    /* @brief			Validation interface for tryAqToolTermsToDates function, which retrieves curve payment dates given terms year fractions
	*  @param [in]		asOfDate		        The asOfDate
	*  @param [in]		terms			        A vector of terms year fractions
	*  @param [out]		paymentDates            A vector of corresponding payment dates
    */
    DateVector tryAqToolTermsToDates( const AQLDate& asOfDate, const DoubleVector terms )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
		AQ_RECORD_INPUTS( asOfDate, terms );

        DateVector result = etrading::convertCurveTermsToDates( asOfDate, terms );

        // Record Outputs and Return the result
        AQ_RECORD_OUTPUTS_AND_RETURN_RESULT( result );

        VALID_EXCEPTION_END
    }

    /* @brief			Validation interface for tryAqToolDatesToTerms function, which retrieves curve payment dates given terms year fractions
	*  @param [in]		asOfDate		        The asOfDate
	*  @param [in]		paymentDates			A vector of corresponding payment dates
	*  @param [out]		terms                   A vector of corresponding terms year fractions
    */
    DoubleVector tryAqToolDatesToTerms( const AQLDate& asOfDate, const DateVector paymentDates )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
		AQ_RECORD_INPUTS( asOfDate, paymentDates );

        DoubleVector result = etrading::convertCurveDatesToTerms( asOfDate, paymentDates );

        // Record Outputs and Return the result
        AQ_RECORD_OUTPUTS_AND_RETURN_RESULT( result );

        VALID_EXCEPTION_END
    }
}
