#pragma once

#include "AQLCoreTemplateType.h"

namespace validation
{
	
	/* @brief			validation interface for aqVolsSABRSetupConvention
	*  @param [in]		conventionID	ID of convention data
	*  @param [in]		convData		Convention data in label value block
	*/
	void tryAqVolsSABRSetupConvention(const AQLString& conventionID, AQLStringMatrix& convData);

}
