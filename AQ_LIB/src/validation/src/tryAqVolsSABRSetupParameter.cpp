#include <boost/date_time.hpp>

#include "tryAqVolsSABRSetupParameter.h"

#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "AQLMathSwaptionVolUtility.h"
#include "InitializeETrading.h"
#include "RecordMacros.h"

using namespace etrading;

namespace validation
{
	/* @brief			validation interface for aqVolsSABRSetupParameter
	*  @param [in]		gridID			ID of the parameter being set up
	*  @param [in]		conventionID	ID of convention data
	*  @param [in]		gridData		Data of the parameter being set up
	*/
	void tryAqVolsSABRSetupParameter(const AQLString& gridID, const AQLString& conventionID, AQLStringMatrix& gridData)
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		AQ_RECORD_INPUTS(gridID, conventionID, gridData);
		
		AQLMathSwaptionVolUtility::setUpSABRGrid(etrading::InitializeETrading::instance().dataInstance(), gridID, conventionID, gridData);

		VALID_EXCEPTION_END
	}
}
