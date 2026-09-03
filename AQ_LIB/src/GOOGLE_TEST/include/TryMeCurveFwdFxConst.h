#pragma once

#include "InitializeMLibGoogleTest.h"
#include "TryMeCurveAccessors.h"

namespace google_test
{
    class TryMeCurveFwdFxConst
        : public virtual google_test::InitializeMLibGoogleTest
        , public TryMeCurveAccessors
    {
    public:
        TryMeCurveFwdFxConst( const LAString& inputFile );
    };

	/* 
	*  @brief			Set up fwdfxconst curve
	*  @param [in]		inputFile	File representation of the curve
	*/
	void setUpMeFwdFxConstCurveCurve( const LAString& inputFile );
}

