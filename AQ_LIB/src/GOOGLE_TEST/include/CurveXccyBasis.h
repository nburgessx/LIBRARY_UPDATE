#pragma once

#include "InitializeAQGoogleTest.h"
#include "CurveAccessors.h"
#include "LAString.h"

namespace google_test
{
    // wrapper for LAUpdateStaticDataManager::setUpBasisCurve (used for Xccy basis)
    class CurveXccyBasis
        : public virtual google_test::InitializeAQGoogleTest
        , public CurveAccessors
    {
    public:
        CurveXccyBasis( const LAString& inputOis1 );
    };

	/* 
	*  @brief			Set up xccy basis curve
	*  @param [in]		inputFile	File representation of the curve
    */
	void setUpXccyBasisCurve(const LAString& inputFile);
}

