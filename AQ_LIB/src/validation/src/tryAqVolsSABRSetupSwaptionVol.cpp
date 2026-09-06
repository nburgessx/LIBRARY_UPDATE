#include <boost/date_time.hpp>

#include "tryAqVolsSABRSetupSwaptionVol.h"

#include "CreateDataFile.h"
#include "StructuredExceptionHandler.h"
#include "AQLMathSwaptionVolUtility.h"
#include "InitializeETrading.h"
#include "RecordMacros.h"

using namespace etrading;

namespace validation
{
	/* @brief			validation interface for aqVolsSABRSetupSwaptionVol
	*  @param [in]			gridID			ID that identifies swaption vol matrix
	*  @param [inout]		volMat			Matrix of swaption vols
	*  @param [inout]		strikeMat		Matrix of strikes
	*  @param [inout]		signMat			Matrix of signs
	*/
	void tryAqVolsSABRSetupSwaptionVol(const AQLString& gridID, AQLStringMatrix& volMat, AQLStringMatrix& strikeMat, AQLStringMatrix& signMat)
	{
		VALID_EXCEPTION_START

		// Record Inputs for logs, tests and playback
		AQ_RECORD_INPUTS(gridID, volMat, strikeMat, signMat);
		
		AQLMathSwaptionVolUtility::setUpSwaptionVol(etrading::InitializeETrading::instance().dataInstance(), gridID, volMat, strikeMat, signMat);

		VALID_EXCEPTION_END
	}
}
