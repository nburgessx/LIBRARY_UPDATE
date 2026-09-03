/*
* @brief			validation interface for meSABROutputParameter
* @Created:			29 October 2018
* @Author:			Joseph Ye
* @Department:		ISD Front Office Development
*
* The copyright to the computer program(s) herein is the property of Mizuho International.
*/

#include "tryMeSABROutputParameter.h"

#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "LAMathSwaptionVolUtility.h"
#include "InitializeMLibETrading.h"
#include <algorithm>
#include "RecordMacros.h"

using namespace etrading;

namespace validation_api
{
	/* @brief			validation interface for meSABROutputParameter
	*  @param [in]		gridID		ID identifying data matrix
	*  @param [out]		ret			The matrix of data being returned
	*  @param [out]		row			Row count of the data matrix
	*  @param [out]		column		Column count of the data matrix
	*/
	void tryMeSABROutputParameter(const LAString& gridID, DoubleVector& ret, size_t& row, size_t& column)
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		RECORD_INPUTS(gridID);

		LAMathSwaptionVolUtility::outPutSABRGrid(etrading::InitializeMLibETrading::instance().dataInstance(), gridID, ret, row, column);

		// Record results
		if (CreateDataFile::recordEnabled())
		{
			CreateDataFile file(decorateFilename("tryMeOutPutSABRGrid_outputs"));
			file.write("Row count", row);
			file.write("Column count", column);
			file.write("Data", ret);
		}

		VALID_EXCEPTION_END
	}
}
