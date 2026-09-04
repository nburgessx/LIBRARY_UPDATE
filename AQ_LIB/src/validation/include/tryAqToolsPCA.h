#pragma once

#include "AQLCoreTemplateType.h"

namespace validation
{

    /* @brief			validation interface for the aqToolsPCA method
    *  @param [in]		key						Key used to identify a PCA analysis
    *  @param [in]		data					Data under analysis
    *  @param [in]		useCorrelationMatrix	True to use correlation matrix. False to use covariance matrix. Default to False
    *  @param [in]		nFactors				The number of PCA factors
    *  @return			A AQLStringMatrix for the PCA results
    */
    AQLStringMatrix tryAqToolsPCA( const AQLString& key, const DoubleMatrix& data, bool useCorrelationMatrix, int nFactors );

}
