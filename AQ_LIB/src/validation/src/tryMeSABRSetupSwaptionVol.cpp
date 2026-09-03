/*
* @brief			validation interface for meSABRSetupSwaptionVol
* @Created:			29 October 2018
* @Author:			Joseph Ye
* @Department:		ISD Front Office Development
*
* The copyright to the computer program(s) herein is the property of AlgoQuantHub.
*/

#include <boost/date_time.hpp>

#include "tryMeSABRSetupSwaptionVol.h"

#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "LAMathSwaptionVolUtility.h"
#include "InitializeMLibETrading.h"
#include "RecordMacros.h"

using namespace etrading;

namespace validation_api
{
	/* @brief			validation interface for meSABRSetupSwaptionVol
	*  @param [in]			gridID			ID that identifies swaption vol matrix
	*  @param [inout]		volMat			Matrix of swaption vols
	*  @param [inout]		strikeMat		Matrix of strikes
	*  @param [inout]		signMat			Matrix of signs
	*/
	void tryMeSABRSetupSwaptionVol(const LAString& gridID, LAStringMatrix& volMat, LAStringMatrix& strikeMat, LAStringMatrix& signMat)
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		RECORD_INPUTS(gridID, volMat, strikeMat, signMat);
		
		LAMathSwaptionVolUtility::setUpSwaptionVol(etrading::InitializeMLibETrading::instance().dataInstance(), gridID, volMat, strikeMat, signMat);

		VALID_EXCEPTION_END
	}
}
