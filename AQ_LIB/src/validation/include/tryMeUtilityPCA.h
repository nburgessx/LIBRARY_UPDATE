#pragma once

#include "LACoreTemplateType.h"

namespace validation
{

    /* @brief			validation interface for the meUtilityPCA method
    *  @param [in]		key						Key used to identify a PCA analysis
    *  @param [in]		data					Data under analysis
    *  @param [in]		useCorrelationMatrix	True to use correlation matrix. False to use covariance matrix. Default to False
    *  @param [in]		nFactors				The number of PCA factors
    *  @return			A LAStringMatrix for the PCA results
    */
    LAStringMatrix tryMeUtilityPCA( const LAString& key, const DoubleMatrix& data, bool useCorrelationMatrix, int nFactors );

}
