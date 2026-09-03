#pragma once

#include "InitializeAQGoogleTest.h"
#include "CurveAccessors.h"
#include "LAString.h"

namespace google_test
{
    // wrapper for LAUpdateStaticDataManager::setUpBasisCurve
    class CurveTenorBasis
        : public virtual google_test::InitializeAQGoogleTest
        , public CurveAccessors
    {
    public:
        CurveTenorBasis( const LAString& inputFile );
    };

	/* 
	*  @brief			Set up tenor basis curve
	*  @param [in]		inputFile	File representation of the curve
    */
	void setUpTenorBasisCurve(const LAString& inputFile);
}


