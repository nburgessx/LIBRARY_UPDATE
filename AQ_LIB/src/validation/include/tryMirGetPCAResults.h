#pragma once

#include "LACoreTemplateType.h"
#include "LADataInstance.h"

namespace validation_api
{

    /* @brief			validation interface for the mirGetPCAResults method
    *  @param [in]		id			ID used to identify a PCA analysis
    *  @param [in]		update		A number that tracks the nth calculation of the same PCA analysis
    *  @return			PCA results
    */
    LAStringMatrix tryMirGetPCAResults( LADataInstance* dataInstance,
                                      const LAString& id,
                                      int update );

}

