// tryMeUtilityDataFilter.h

/*
 * @brief			Function to filter a vector of input data and remove duplicates
 * @Created:		22 September 2017
 * @Author:			Nicholas Burgess
 * @Department:		ISD Quant Research and Analytics
 *
 * The copyright to the computer program(s) herein is the property of Mizuho International.
 */

#pragma once
#include "DataUtilities.h"

namespace validation_api
{
    using etrading::VariantMatrix;
    using etrading::VariantVector;

    /* @brief			Function to filter a vector of input data and remove duplicates
    *  @param [in]		dataVector		            input vector data
    *  @param [in]      displayByRow		        True = Display by Row, False = Display by Column, Default is False
    *  @return			Returns a filtered data list with duplicates removed
    */
    const VariantMatrix tryMeUtilityDataFilter( const VariantVector & dataVector, const bool displayByRow = false );

}