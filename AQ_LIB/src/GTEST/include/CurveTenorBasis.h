#pragma once

#include "InitializeGoogleTest.h"
#include "CurveAccessors.h"
#include "AQLString.h"

namespace google_test
{
    // wrapper for AQLUpdateStaticDataManager::setUpBasisCurve
    class CurveTenorBasis
        : public virtual google_test::InitializeGoogleTest
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


