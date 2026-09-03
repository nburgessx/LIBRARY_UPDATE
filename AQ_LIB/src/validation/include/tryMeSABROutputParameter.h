#pragma once

#include "LACoreTemplateType.h"

namespace validation_api
{
	
	/* @brief			validation interface for meSABROutputParameter
    *  @param [in]		gridID		ID identifying data matrix
	*  @param [out]		ret			The matrix of data being returned
	*  @param [out]		row			Row count of the data matrix
	*  @param [out]		column		Column count of the data matrix
    */
	void tryMeSABROutputParameter(const LAString& gridID, DoubleVector& ret, size_t& row, size_t& column);

}
