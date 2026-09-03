#pragma once

#include "InitializeMLibGoogleTest.h"
#include "TryMeCurveAccessors.h"

namespace google_test
{
    class TryMeCurveXccyBasis
        : public virtual google_test::InitializeMLibGoogleTest
        , public TryMeCurveAccessors
    {
    public:
        TryMeCurveXccyBasis( const LAString& inputFile );
    };

	/* 
	*  @brief			Set up xccy basis curve
	*  @param [in]		inputFile	File representation of the curve
	*/
	void setUpMeXccyBasisCurve(const LAString& inputFile);
}

