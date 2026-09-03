#pragma once

#include "InitializeAQGoogleTest.h"
#include "TryMeCurveAccessors.h"

namespace google_test
{
    class TryMeCurveFwdFxConst
        : public virtual google_test::InitializeAQGoogleTest
        , public TryMeCurveAccessors
    {
    public:
        TryMeCurveFwdFxConst( const AQLString& inputFile );
    };

	/* 
	*  @brief			Set up fwdfxconst curve
	*  @param [in]		inputFile	File representation of the curve
	*/
	void setUpMeFwdFxConstCurveCurve( const AQLString& inputFile );
}

