#pragma once

#include "AQLDataInstance.h"
#include "AQLString.h"
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

        InterestRateSwap( AQLDataInstance* dataInstance, const ReadDataFile::Load& inputFile );

        virtual const double parRate();
        virtual const double pv();
        virtual const double pv( const double& parRate );  // Used to test par swap PVs where the fixedRate = parRate
		virtual const double pv01();

    protected:

        AQLDataInstance*		dataInstance_;

        AQLString	effectDt_;
        AQLString	maturity_;
        AQLString	xFirstStub_;
        AQLString	xLastStub_;
        AQLString	xRollDay_;
        AQLString	tFirstStub_;
        AQLString	tLastStub_;
        AQLString	tRollDay_;
        AQLString	xFreq_;
        AQLString	tFreq_;
        bool		eomRoll_;
        AQLString	xRollCnv_;
        AQLString	xCalendar_;
        AQLString	tRollCnv_;
        AQLString	tCalendar_;
        AQLString	xStub_;
        AQLString	tStub_;
        AQLString	xPayLag_;
        AQLString	tPayLag_;
        AQLString	tFixLag_;

        // parRate Parameters: Mandatory
        AQLString	curveID_;
        AQLString	tDayCount_;
        AQLString	xDayCount_;
        double		tFirstFix_;
        double		tLastFix_;
        AQLString	interpolation_;
        AQLString	forecastCurve_;
        AQLString	discountCurve_;
        bool		interpFwds_;

        // parRate Parameters: Optional
        double      tSpd_;
        bool        useFwdData_;
        bool        isOIS_;
        AQLString    oisCompoundingType_;
        AQLString    calendar_;
        AQLString    rollConvention_;
        AQLString    slidingRule_;

        // swapPV Parameters: Mandatory
        AQLString	payRec_;
        double      notional_;
        double      fixedRate_;

        // swapPV Parameters: Optional
        double      floatSpreadInBasisPoints_;
        AQLString	compoundingMethod_;
        AQLString	floatCalendar_;
        AQLString	floatRollConv_;

    };
}

