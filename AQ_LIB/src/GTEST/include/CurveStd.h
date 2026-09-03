#pragma once

#include "InitializeGoogleTest.h"
#include "CurveAccessors.h"
#include "AQLString.h"
#include "ReadDataFile.h"

namespace google_test
{
    // wrapper for LAUpdateStaticDataManager::setUpSwapCurve
    class CurveStd
        : public virtual google_test::InitializeGoogleTest
        , public CurveAccessors
    {
    public:
        CurveStd( const AQLString& inputFile );

        // return input swap rates as an ReadTestData table
        virtual const etrading::ReadDataFile& swapRates() const
        {
            return inputFile_["swapRates"];
        }
    };

	/* 
	*  @brief			Set up STD swap curve
	*  @param [in]		inputFile	File representation of the curve
    */
	void setUpSTDCurve(const AQLString& stdInputFile);
}
