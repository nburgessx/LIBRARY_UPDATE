#pragma once

#include "InitializeGoogleTest.h"
#include "TryAqCurvesAccessors.h"

namespace google_test
{
    class TryAqCurvesFwdFxConst
        : public virtual google_test::InitializeGoogleTest
        , public TryAqCurvesAccessors
    {
    public:
        TryAqCurvesFwdFxConst( const AQLString& inputFile );
    };

	/* 
	*  @brief			Set up fwdfxconst curve
	*  @param [in]		inputFile	File representation of the curve
	*/
	void setUpAqFwdFxConstCurveCurve( const AQLString& inputFile );
}

