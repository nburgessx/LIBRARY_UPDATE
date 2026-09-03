#include "tryMeUtilityDate.h"

#include "CreateDataFile.h"
#include "CurveValidation.h"
#include "Environment.h"
#include "LACurveForwardRateHelpers.h"
#include "LADateScheduleHelpers.h"
#include "ParameterValidation.h"
#include "RecordMacros.h"
#include "StructuredExceptionHandler.h"
#include "tryMeCurveDiscountFactor.h" // needed for the utility functions
#include "tryMeDate.h"

using etrading::CreateDataFile;
using etrading::decorateCurvename;

namespace validation_api
{

    
    /* @brief			Validation interface for tryMeUtilityTermsToDates function, which retrieves curve payment dates given terms year fractions
	*  @param [in]		asOfDate		        The asOfDate
	*  @param [in]		terms			        A vector of terms year fractions
	*  @param [out]		paymentDates            A vector of corresponding payment dates
    */
    DateVector tryMeUtilityTermsToDates( const LADate& asOfDate, const DoubleVector terms )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
		RECORD_INPUTS( asOfDate, terms );

        DateVector result = etrading::convertCurveTermsToDates( asOfDate, terms );

        // Record Outputs and Return the result
        RECORD_OUTPUTS_AND_RETURN_RESULT( result );

        VALID_EXCEPTION_END
    }

    /* @brief			Validation interface for tryMeUtilityDatesToTerms function, which retrieves curve payment dates given terms year fractions
	*  @param [in]		asOfDate		        The asOfDate
	*  @param [in]		paymentDates			A vector of corresponding payment dates
	*  @param [out]		terms                   A vector of corresponding terms year fractions
    */
    DoubleVector tryMeUtilityDatesToTerms( const LADate& asOfDate, const DateVector paymentDates )
    {
        VALID_EXCEPTION_START

        // Record Inputs for logs, tests and playback
		RECORD_INPUTS( asOfDate, paymentDates );

        DoubleVector result = etrading::convertCurveDatesToTerms( asOfDate, paymentDates );

        // Record Outputs and Return the result
        RECORD_OUTPUTS_AND_RETURN_RESULT( result );

        VALID_EXCEPTION_END
    }
}
