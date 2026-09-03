#pragma once

#include "AQLCoreTemplateType.h"
#include "AQLDataInstance.h"

namespace validation
{

    /* @brief			validation interface for the mirGetPCAResults method
    *  @param [in]		id			ID used to identify a PCA analysis
    *  @param [in]		update		A number that tracks the nth calculation of the same PCA analysis
    *  @return			PCA results
    */
    AQLStringMatrix tryMirGetPCAResults( AQLDataInstance* dataInstance,
                                      const AQLString& id,
                                      int update );

}

