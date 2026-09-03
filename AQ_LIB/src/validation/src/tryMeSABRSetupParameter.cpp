#include <boost/date_time.hpp>

#include "tryMeSABRSetupParameter.h"

#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "AQLMathSwaptionVolUtility.h"
#include "InitializeAQETrading.h"
#include "RecordMacros.h"

using namespace etrading;

namespace validation
{
	/* @brief			validation interface for meSABRSetupParameter
	*  @param [in]		gridID			ID of the parameter being set up
	*  @param [in]		conventionID	ID of convention data
	*  @param [in]		gridData		Data of the parameter being set up
	*/
	void tryMeSABRSetupParameter(const AQLString& gridID, const AQLString& conventionID, AQLStringMatrix& gridData)
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		RECORD_INPUTS(gridID, conventionID, gridData);
		
		AQLMathSwaptionVolUtility::setUpSABRGrid(etrading::InitializeAQETrading::instance().dataInstance(), gridID, conventionID, gridData);

		VALID_EXCEPTION_END
	}
}
