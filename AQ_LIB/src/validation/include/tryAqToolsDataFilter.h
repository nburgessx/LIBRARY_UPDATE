// tryAqToolsDataFilter.h

/*
 * @brief			Function to filter a vector of input data and remove duplicates
 */

#pragma once
#include "DataUtilities.h"

namespace validation
{
    using etrading::VariantMatrix;
    using etrading::VariantVector;

    /* @brief			Function to filter a vector of input data and remove duplicates
    *  @param [in]		dataVector		            input vector data
    *  @param [in]      displayByRow		        True = Display by Row, False = Display by Column, Default is False
    *  @return			Returns a filtered data list with duplicates removed
    */
    const VariantMatrix tryAqToolsDataFilter( const VariantVector & dataVector, const bool displayByRow = false );

}