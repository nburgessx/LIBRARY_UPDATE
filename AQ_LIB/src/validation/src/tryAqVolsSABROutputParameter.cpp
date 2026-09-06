#include "tryAqVolsSABROutputParameter.h"

#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "AQLMathSwaptionVolUtility.h"
#include "InitializeETrading.h"
#include <algorithm>
#include "RecordMacros.h"

using namespace etrading;

namespace validation
{
	/* @brief			validation interface for aqVolsSABROutputParameter
	*  @param [in]		gridID		ID identifying data matrix
	*  @param [out]		ret			The matrix of data being returned
	*  @param [out]		row			Row count of the data matrix
	*  @param [out]		column		Column count of the data matrix
	*/
	void tryAqVolsSABROutputParameter(const AQLString& gridID, DoubleVector& ret, size_t& row, size_t& column)
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		AQ_RECORD_INPUTS(gridID);

		AQLMathSwaptionVolUtility::outPutSABRGrid(etrading::InitializeETrading::instance().dataInstance(), gridID, ret, row, column);

		// Record results
		if (CreateDataFile::recordEnabled())
		{
			CreateDataFile file(decorateFilename("tryAqVolsSABRGrid_outputs"));
			file.write("Row count", row);
			file.write("Column count", column);
			file.write("Data", ret);
		}

		VALID_EXCEPTION_END
	}
}
