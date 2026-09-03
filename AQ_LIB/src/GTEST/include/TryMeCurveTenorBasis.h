#pragma once

#include "InitializeGoogleTest.h"
#include "TryMeCurveAccessors.h"

namespace google_test
{
    class TryMeCurveTenorBasis
        : public virtual google_test::InitializeGoogleTest
        , public TryMeCurveAccessors
    {
    public:
        TryMeCurveTenorBasis( const AQLString& inputFile );
    };

	/* 
	*  @brief			Set up tenor basis curve
	*  @param [in]		inputFile	File representation of the curve
	*/
	void setUpMeTenorBasisCurve(const AQLString& inputFile);
}


