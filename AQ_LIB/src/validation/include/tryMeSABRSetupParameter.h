#pragma once

#include "LACoreTemplateType.h"

namespace validation_api
{
	
	/* @brief			validation interface for meSABRSetupParameter
    *  @param [in]		gridID			ID of the parameter being set up
    *  @param [in]		conventionID	ID of convention data
	*  @param [in]		gridData		Data of the parameter being set up
    */
	void tryMeSABRSetupParameter(const LAString& gridID, const LAString& conventionID, LAStringMatrix& gridData );

}
