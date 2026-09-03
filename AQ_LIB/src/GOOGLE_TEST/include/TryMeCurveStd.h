#pragma once

#include "InitializeMLibGoogleTest.h"
#include "TryMeCurveAccessors.h"

namespace google_test
{
    class TryMeCurveStd
        : public virtual google_test::InitializeMLibGoogleTest
        , public TryMeCurveAccessors
    {
    public:
        TryMeCurveStd( const LAString& inputFile );
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
	void setUpMeSTDCurve(const LAString& stdInputFile);
}
