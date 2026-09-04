#pragma once
#include "AQLCoreTemplateType.h"

namespace validation
{

    /* @brief			Validation interface for tryAqToolsTermsToDates function, which retrieves curve payment dates given terms year fractions
	*  @param [in]		asOfDate		        The asOfDate
	*  @param [in]		terms			        A vector of terms year fractions
	*  @param [out]		paymentDates            A vector of corresponding payment dates
    */
    DateVector tryAqToolsTermsToDates( const AQLDate& asOfDate, const DoubleVector terms );

    /* @brief			Validation interface for tryAqToolsDatesToTerms function, which retrieves curve payment dates given terms year fractions
	*  @param [in]		asOfDate		        The asOfDate
	*  @param [in]		paymentDates			A vector of corresponding payment dates
	*  @param [out]		terms                   A vector of corresponding terms year fractions
    */
    DoubleVector tryAqToolsDatesToTerms( const AQLDate& asOfDate, const DateVector paymentDates );
}
