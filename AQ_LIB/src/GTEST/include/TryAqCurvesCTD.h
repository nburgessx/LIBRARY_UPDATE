#pragma once

#include "InitializeGoogleTest.h"
#include "TryAqCurvesAccessors.h"

namespace google_test
{
	/* 
	*  @brief			Set up CTD curve
	*  @param [in]		inputFile	File representation of the curve
	*/
	void setUpAqCTDCurve(const AQLString& inputFile);
}

