#pragma once

#include "InitializeGoogleTest.h"
#include "TryAqCurvesAccessors.h"

namespace google_test
{
    class TryAqCurvesStd
        : public virtual google_test::InitializeGoogleTest
        , public TryAqCurvesAccessors
    {
    public:
        TryAqCurvesStd( const AQLString& inputFile );
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
	void setUpAqSTDCurve(const AQLString& stdInputFile);
}
