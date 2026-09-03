#pragma once

#include "InitializeGoogleTest.h"
#include "CurveAccessors.h"
#include "AQLString.h"

namespace google_test
{
    // wrapper for LAUpdateStaticDataManager::setUpOISCurve
    class CurveOis
        : public virtual google_test::InitializeGoogleTest
        , public CurveAccessors
    {
    public:
        CurveOis( const AQLString& inputFile );
    };

	/* 
	*  @brief			Set up OIS curve
	*  @param [in]		inputFile	File representation of the curve
    */
	void setUpOISCurve(const AQLString& inputFile);
}

