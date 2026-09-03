#pragma once

#include "InitializeAQGoogleTest.h"
#include "CurveAccessors.h"
#include "LAString.h"

namespace google_test
{
    // wrapper for LAUpdateStaticDataManager::setUpFwdFXConstantCurve
    class CurveFwdFxConst
        : public virtual google_test::InitializeAQGoogleTest
        , public CurveAccessors
    {
    public:
        CurveFwdFxConst( const LAString& inputFile );
    };
}

