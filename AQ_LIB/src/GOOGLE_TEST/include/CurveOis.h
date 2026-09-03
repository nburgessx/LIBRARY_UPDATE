#pragma once

#include "InitializeAQGoogleTest.h"
#include "CurveAccessors.h"
#include "LAString.h"

namespace google_test
{
    // wrapper for LAUpdateStaticDataManager::setUpOISCurve
    class CurveOis
        : public virtual google_test::InitializeAQGoogleTest
        , public CurveAccessors
    {
    public:
        CurveOis( const LAString& inputFile );
    };

	/* 
	*  @brief			Set up OIS curve
	*  @param [in]		inputFile	File representation of the curve
    */
	void setUpOISCurve(const LAString& inputFile);
}

