/*
 * @brief			validation interface for the mirSetupPCA method
 * @Created:		27 March 2016
 * @Author:			Joseph Ye
 * @Department:		ISD Front Office Development
 *
 * The copyright to the computer program(s) herein is the property of AlgoQuantHub.
 */


#pragma once

#include "LACoreTemplateType.h"
#include "LADataInstance.h"

namespace validation_api
{

    /* @brief			validation interface for the mirSetupPCA method
    *  @param [in]		dataInstance			Pointer to the object pool
    *  @param [in]		id				ID used to identify a PCA analysis
    *  @param [in]		Data			Data under analysis
    *  @param [in]		isScale			True means using correlation matrix. False means using covariance matrix. Default to false
    *  @param [in]		factorNum		The number of PCA factors to show results for
    *  @return			A string that identifies the current PCA analysis
    */
    const LAString tryMirSetupPCA( LADataInstance* dataInstance,
                                   const LAString& ID,
                                   const DoubleMatrix& data,
                                   bool isScale,
                                   int factorNum );

}
