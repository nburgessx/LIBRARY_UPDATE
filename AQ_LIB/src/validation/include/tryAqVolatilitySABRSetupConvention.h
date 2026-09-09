#pragma once

#include "AQLCoreTemplateType.h"

namespace validation
{
	
	/* @brief			validation interface for aqVolatilitySABRSetupConvention
	*  @param [in]		conventionID	ID of convention data
	*  @param [in]		convData		Convention data in label value block
	*/
	void tryAqVolatilitySABRSetupConvention(const AQLString& conventionID, AQLStringMatrix& convData);

}
