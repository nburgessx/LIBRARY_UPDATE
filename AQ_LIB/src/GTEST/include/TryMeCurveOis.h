#pragma once

#include "InitializeGoogleTest.h"
#include "TryMeCurveAccessors.h"

namespace google_test
{
    class TryMeCurveOis
        : public virtual google_test::InitializeGoogleTest
        , public TryMeCurveAccessors
    {
    public:
        TryMeCurveOis( const AQLString& inputFile );
    };

	/* 
	*  @brief			Set up OIS curve
	*  @param [in]		inputFile	File representation of the curve
	*/
	void setUpMeOISCurve(const AQLString& inputFile);
}

