#pragma once

#include "InitializeGoogleTest.h"
#include "CurveAccessors.h"
#include "AQLString.h"

namespace google_test
{
    // wrapper for AQLUpdateStaticDataManager::setUpBasisCurve (used for Xccy basis)
    class CurveXccyBasis
        : public virtual google_test::InitializeGoogleTest
        , public CurveAccessors
    {
    public:
        CurveXccyBasis( const AQLString& inputOis1 );
    };

	/* 
	*  @brief			Set up xccy basis curve
	*  @param [in]		inputFile	File representation of the curve
    */
	void setUpXccyBasisCurve(const AQLString& inputFile);
}

