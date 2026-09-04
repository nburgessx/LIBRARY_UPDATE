#pragma once

#include "InitializeGoogleTest.h"
#include "TryAqCurvesAccessors.h"

namespace google_test
{
    class TryAqCurvesOis
        : public virtual google_test::InitializeGoogleTest
        , public TryAqCurvesAccessors
    {
    public:
        TryAqCurvesOis( const AQLString& inputFile );
    };

	/* 
	*  @brief			Set up OIS curve
	*  @param [in]		inputFile	File representation of the curve
	*/
	void setUpAqOISCurve(const AQLString& inputFile);
}

