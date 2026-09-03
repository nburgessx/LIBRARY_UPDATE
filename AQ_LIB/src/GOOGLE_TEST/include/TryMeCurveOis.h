#pragma once

#include "InitializeMLibGoogleTest.h"
#include "TryMeCurveAccessors.h"

namespace google_test
{
    class TryMeCurveOis
        : public virtual google_test::InitializeMLibGoogleTest
        , public TryMeCurveAccessors
    {
    public:
        TryMeCurveOis( const LAString& inputFile );
    };

	/* 
	*  @brief			Set up OIS curve
	*  @param [in]		inputFile	File representation of the curve
	*/
	void setUpMeOISCurve(const LAString& inputFile);
}

