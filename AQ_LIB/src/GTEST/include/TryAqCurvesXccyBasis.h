#pragma once

#include "InitializeGoogleTest.h"
#include "TryAqCurvesAccessors.h"

namespace google_test
{
    class TryAqCurvesXccyBasis
        : public virtual google_test::InitializeGoogleTest
        , public TryAqCurvesAccessors
    {
    public:
        TryAqCurvesXccyBasis( const AQLString& inputFile );
    };

	/* 
	*  @brief			Set up xccy basis curve
	*  @param [in]		inputFile	File representation of the curve
	*/
	void setUpAqXccyBasisCurve(const AQLString& inputFile);
}

