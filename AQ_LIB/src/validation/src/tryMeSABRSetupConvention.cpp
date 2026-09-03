#include <boost/date_time.hpp>

#include "tryMeSABRSetupConvention.h"

#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "LAMathSwaptionVolUtility.h"
#include "InitializeAQETrading.h"
#include "RecordMacros.h"

using namespace etrading;

namespace validation
{
	/* @brief			validation interface for meSABRSetupConvention
	*  @param [in]		conventionID	ID of convention data
	*  @param [in]		convData		Convention data in label value block
	*/
	void tryMeSABRSetupConvention(const LAString& conventionID, LAStringMatrix& convData)
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		RECORD_INPUTS(conventionID, convData);
		
		LAMathSwaptionVolUtility::setUpConvention(etrading::InitializeAQETrading::instance().dataInstance(), conventionID, convData);

		VALID_EXCEPTION_END
	}
}
