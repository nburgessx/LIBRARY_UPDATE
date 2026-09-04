#pragma once

#include "InitializeGoogleTest.h"
#include "CurveAccessors.h"
#include "AQLString.h"

namespace google_test
{
    // wrapper for AQLUpdateStaticDataManager::setUpFwdFXConstantCurve
    class CurveFwdFxConst
        : public virtual google_test::InitializeGoogleTest
        , public CurveAccessors
    {
    public:
        CurveFwdFxConst( const AQLString& inputFile );
    };
}

