/*
 * @brief			validation interface for meUtilityDate method(s)
 * @Created:		5th June 2018
 * @Author:			Nicholas Burgess
 * @Department:		Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */

#pragma once
#include "LACoreTemplateType.h"

namespace validation_api
{

    /* @brief			Validation interface for tryMeUtilityTermsToDates function, which retrieves curve payment dates given terms year fractions
	*  @param [in]		asOfDate		        The asOfDate
	*  @param [in]		terms			        A vector of terms year fractions
	*  @param [out]		paymentDates            A vector of corresponding payment dates
    */
    DateVector tryMeUtilityTermsToDates( const LADate& asOfDate, const DoubleVector terms );

    /* @brief			Validation interface for tryMeUtilityDatesToTerms function, which retrieves curve payment dates given terms year fractions
	*  @param [in]		asOfDate		        The asOfDate
	*  @param [in]		paymentDates			A vector of corresponding payment dates
	*  @param [out]		terms                   A vector of corresponding terms year fractions
    */
    DoubleVector tryMeUtilityDatesToTerms( const LADate& asOfDate, const DateVector paymentDates );
}
