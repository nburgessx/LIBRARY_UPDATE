#pragma once

#include "AQLCoreTemplateType.h"
#include "AQLDataInstance.h"

namespace validation
{

    /* @brief			validation interface for the mirSetupPCA method
    *  @param [in]		dataInstance			Pointer to the object pool
    *  @param [in]		id				ID used to identify a PCA analysis
    *  @param [in]		Data			Data under analysis
    *  @param [in]		isScale			True means using correlation matrix. False means using covariance matrix. Default to false
    *  @param [in]		factorNum		The number of PCA factors to show results for
    *  @return			A string that identifies the current PCA analysis
    */
    const AQLString tryMirSetupPCA( AQLDataInstance* dataInstance,
                                   const AQLString& ID,
                                   const DoubleMatrix& data,
                                   bool isScale,
                                   int factorNum );

}
