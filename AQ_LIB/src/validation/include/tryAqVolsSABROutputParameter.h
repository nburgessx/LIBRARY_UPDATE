#pragma once

#include "AQLCoreTemplateType.h"

namespace validation
{
	
	/* @brief			validation interface for aqVolsSABROutputParameter
    *  @param [in]		gridID		ID identifying data matrix
	*  @param [out]		ret			The matrix of data being returned
	*  @param [out]		row			Row count of the data matrix
	*  @param [out]		column		Column count of the data matrix
    */
	void tryAqVolsSABROutputParameter(const AQLString& gridID, DoubleVector& ret, size_t& row, size_t& column);

}
