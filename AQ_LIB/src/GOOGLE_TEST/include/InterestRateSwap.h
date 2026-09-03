#pragma once

#include "LADataInstance.h"
#include "LAString.h"
#include "ReadDataFile.h"
#include "CreateDataFile.h"

#include "LAUpdateStaticDataManager.h"
#include "LAMultiSwapPricer.h"
#include "LACurveForwardRateHelpers.h"
#include "LACurveCalibrationHelpers.h"

using etrading::ReadDataFile;
using etrading::CreateDataFile;

namespace google_test
{
    //
    // CLASS        InterestRateSwap
    //
    // PURPOSE      Interest Rate Swap for testing
    //
    //              This class reads swap inputs from a test *.csv file and calls the swap par rate and pricing functions using the file parameters.
    //              It is possible to create a swap using the swap par rate function or swap pv function inputs. Optional fields are populated with
    //              default values if not specified or provided in the input file.
    //

    class InterestRateSwap
    {
    public:

        InterestRateSwap() {};
        virtual ~InterestRateSwap() {};

        InterestRateSwap( LADataInstance* dataInstance, const ReadDataFile::Load& inputFile );

        virtual const double parRate();
        virtual const double pv();
        virtual const double pv( const double& parRate );  // Used to test par swap PVs where the fixedRate = parRate
		virtual const double pv01();

    protected:

        LADataInstance*		dataInstance_;

        LAString	effectDt_;
        LAString	maturity_;
        LAString	xFirstStub_;
        LAString	xLastStub_;
        LAString	xRollDay_;
        LAString	tFirstStub_;
        LAString	tLastStub_;
        LAString	tRollDay_;
        LAString	xFreq_;
        LAString	tFreq_;
        bool		eomRoll_;
        LAString	xRollCnv_;
        LAString	xCalendar_;
        LAString	tRollCnv_;
        LAString	tCalendar_;
        LAString	xStub_;
        LAString	tStub_;
        LAString	xPayLag_;
        LAString	tPayLag_;
        LAString	tFixLag_;

        // parRate Parameters: Mandatory
        LAString	curveID_;
        LAString	tDayCount_;
        LAString	xDayCount_;
        double		tFirstFix_;
        double		tLastFix_;
        LAString	interpolation_;
        LAString	forecastCurve_;
        LAString	discountCurve_;
        bool		interpFwds_;

        // parRate Parameters: Optional
        double      tSpd_;
        bool        useFwdData_;
        bool        isOIS_;
        LAString    oisCompoundingType_;
        LAString    calendar_;
        LAString    rollConvention_;
        LAString    slidingRule_;

        // swapPV Parameters: Mandatory
        LAString	payRec_;
        double      notional_;
        double      fixedRate_;

        // swapPV Parameters: Optional
        double      floatSpreadInBasisPoints_;
        LAString	compoundingMethod_;
        LAString	floatCalendar_;
        LAString	floatRollConv_;

    };
}

