#pragma once

#include "InitializeAQGoogleTest.h"
#include "TryMeCurveAccessors.h"

namespace google_test
{
    class TryMeCurveTenorBasis
        : public virtual google_test::InitializeAQGoogleTest
        , public TryMeCurveAccessors
    {
    public:
        TryMeCurveTenorBasis( const LAString& inputFile );
    };

	/* 
	*  @brief			Set up tenor basis curve
	*  @param [in]		inputFile	File representation of the curve
	*/
	void setUpMeTenorBasisCurve(const LAString& inputFile);
}


