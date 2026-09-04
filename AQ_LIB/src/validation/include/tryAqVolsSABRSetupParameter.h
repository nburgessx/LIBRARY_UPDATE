#pragma once

#include "AQLCoreTemplateType.h"

namespace validation
{
	
	/* @brief			validation interface for aqVolsSABRSetupParameter
    *  @param [in]		gridID			ID of the parameter being set up
    *  @param [in]		conventionID	ID of convention data
	*  @param [in]		gridData		Data of the parameter being set up
    */
	void tryAqVolsSABRSetupParameter(const AQLString& gridID, const AQLString& conventionID, AQLStringMatrix& gridData );

}
