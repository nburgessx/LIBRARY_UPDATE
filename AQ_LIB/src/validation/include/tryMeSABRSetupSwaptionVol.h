#pragma once

#include "LACoreTemplateType.h"

namespace validation_api
{
	
	/* @brief			validation interface for meSABRSetupSwaptionVol
    *  @param [in]			gridID			ID that identifies swaption vol matrix
    *  @param [inout]		volMat			Matrix of swaption vols
	*  @param [inout]		strikeMat		Matrix of strikes
	*  @param [inout]		signMat			Matrix of signs
    */
	void tryMeSABRSetupSwaptionVol(const LAString& gridID, LAStringMatrix& volMat, LAStringMatrix& strikeMat, LAStringMatrix& signMat);

}
