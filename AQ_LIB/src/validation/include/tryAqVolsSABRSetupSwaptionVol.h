#pragma once

#include "AQLCoreTemplateType.h"

namespace validation
{
	
	/* @brief			validation interface for aqVolsSABRSetupSwaptionVol
    *  @param [in]			gridID			ID that identifies swaption vol matrix
    *  @param [inout]		volMat			Matrix of swaption vols
	*  @param [inout]		strikeMat		Matrix of strikes
	*  @param [inout]		signMat			Matrix of signs
    */
	void tryAqVolsSABRSetupSwaptionVol(const AQLString& gridID, AQLStringMatrix& volMat, AQLStringMatrix& strikeMat, AQLStringMatrix& signMat);

}
