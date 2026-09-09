#pragma once
#include "AQLCoreTemplateType.h"

namespace validation
{

    /* @brief			Validation interface for tryAqToolTermsToDates function, which retrieves curve payment dates given terms year fractions
	*  @param [in]		asOfDate		        The asOfDate
	*  @param [in]		terms			        A vector of terms year fractions
	*  @param [out]		paymentDates            A vector of corresponding payment dates
    */
    DateVector tryAqToolTermsToDates( const AQLDate& asOfDate, const DoubleVector terms );

    /* @brief			Validation interface for tryAqToolDatesToTerms function, which retrieves curve payment dates given terms year fractions
	*  @param [in]		asOfDate		        The asOfDate
	*  @param [in]		paymentDates			A vector of corresponding payment dates
	*  @param [out]		terms                   A vector of corresponding terms year fractions
    */
    DoubleVector tryAqToolDatesToTerms( const AQLDate& asOfDate, const DateVector paymentDates );
}
