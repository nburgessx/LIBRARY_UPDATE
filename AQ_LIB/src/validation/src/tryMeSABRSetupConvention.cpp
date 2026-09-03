/*
* @brief			validation interface for meSABRSetupConvention
* @Created:			05 November 2018
* @Author:			Joseph Ye
* @Department:		ISD Front Office Development
*
* The copyright to the computer program(s) herein is the property of Mizuho International.
*/

#include <boost/date_time.hpp>

#include "tryMeSABRSetupConvention.h"

#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "LAMathSwaptionVolUtility.h"
#include "InitializeMLibETrading.h"
#include "RecordMacros.h"

using namespace etrading;

namespace validation_api
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
		
		LAMathSwaptionVolUtility::setUpConvention(etrading::InitializeMLibETrading::instance().dataInstance(), conventionID, convData);

		VALID_EXCEPTION_END
	}
}
