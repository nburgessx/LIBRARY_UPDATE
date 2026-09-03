#pragma once

#include "InitializeAQGoogleTest.h"
#include "CurveAccessors.h"
#include "AQLString.h"

namespace google_test
{
    // wrapper for LAUpdateStaticDataManager::setUpBasisCurve
    class CurveTenorBasis
        : public virtual google_test::InitializeAQGoogleTest
        , public CurveAccessors
    {
    public:
        CurveTenorBasis( const AQLString& inputFile );
    };

	/* 
	*  @brief			Set up tenor basis curve
	*  @param [in]		inputFile	File representation of the curve
    */
	void setUpTenorBasisCurve(const AQLString& inputFile);
}


