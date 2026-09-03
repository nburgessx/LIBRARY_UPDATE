#include <boost/date_time.hpp>

#include "tryMeSABRSetupConvention.h"

#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "AQLMathSwaptionVolUtility.h"
#include "InitializeAQETrading.h"
#include "RecordMacros.h"

using namespace etrading;

namespace validation
{
	/* @brief			validation interface for meSABRSetupConvention
	*  @param [in]		conventionID	ID of convention data
	*  @param [in]		convData		Convention data in label value block
	*/
	void tryMeSABRSetupConvention(const AQLString& conventionID, AQLStringMatrix& convData)
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		RECORD_INPUTS(conventionID, convData);
		
		AQLMathSwaptionVolUtility::setUpConvention(etrading::InitializeAQETrading::instance().dataInstance(), conventionID, convData);

		VALID_EXCEPTION_END
	}
}
