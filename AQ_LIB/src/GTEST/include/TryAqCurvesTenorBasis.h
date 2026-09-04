#pragma once

#include "InitializeGoogleTest.h"
#include "TryAqCurvesAccessors.h"

namespace google_test
{
    class TryAqCurvesTenorBasis
        : public virtual google_test::InitializeGoogleTest
        , public TryAqCurvesAccessors
    {
    public:
        TryAqCurvesTenorBasis( const AQLString& inputFile );
    };

	/* 
	*  @brief			Set up tenor basis curve
	*  @param [in]		inputFile	File representation of the curve
	*/
	void setUpAqTenorBasisCurve(const AQLString& inputFile);
}


