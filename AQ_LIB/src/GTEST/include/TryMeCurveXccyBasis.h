#pragma once

#include "InitializeAQGoogleTest.h"
#include "TryMeCurveAccessors.h"

namespace google_test
{
    class TryMeCurveXccyBasis
        : public virtual google_test::InitializeAQGoogleTest
        , public TryMeCurveAccessors
    {
    public:
        TryMeCurveXccyBasis( const AQLString& inputFile );
    };

	/* 
	*  @brief			Set up xccy basis curve
	*  @param [in]		inputFile	File representation of the curve
	*/
	void setUpMeXccyBasisCurve(const AQLString& inputFile);
}

