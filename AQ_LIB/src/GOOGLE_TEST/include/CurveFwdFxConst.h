#pragma once

#include "InitializeMLibGoogleTest.h"
#include "CurveAccessors.h"
#include "LAString.h"

namespace google_test
{
    // wrapper for LAUpdateStaticDataManager::setUpFwdFXConstantCurve
    class CurveFwdFxConst
        : public virtual google_test::InitializeMLibGoogleTest
        , public CurveAccessors
    {
    public:
        CurveFwdFxConst( const LAString& inputFile );
    };
}

