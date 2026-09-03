/*
* @brief			validation interface for meSABRSetupParameter
* @Created:			22 October 2018
* @Author:			Joseph Ye
* @Department:		ISD Front Office Development
*
* The copyright to the computer program(s) herein is the property of AlgoQuantHub.
*/

#include <boost/date_time.hpp>

#include "tryMeSABRSetupParameter.h"

#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "LAMathSwaptionVolUtility.h"
#include "InitializeMLibETrading.h"
#include "RecordMacros.h"

using namespace etrading;

namespace validation_api
{
	/* @brief			validation interface for meSABRSetupParameter
	*  @param [in]		gridID			ID of the parameter being set up
	*  @param [in]		conventionID	ID of convention data
	*  @param [in]		gridData		Data of the parameter being set up
	*/
	void tryMeSABRSetupParameter(const LAString& gridID, const LAString& conventionID, LAStringMatrix& gridData)
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		RECORD_INPUTS(gridID, conventionID, gridData);
		
		LAMathSwaptionVolUtility::setUpSABRGrid(etrading::InitializeMLibETrading::instance().dataInstance(), gridID, conventionID, gridData);

		VALID_EXCEPTION_END
	}
}
