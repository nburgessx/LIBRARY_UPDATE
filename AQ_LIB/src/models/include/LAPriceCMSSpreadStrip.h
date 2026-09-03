#ifndef LAPriceCMSSpreadStrip_h
#define LAPriceCMSSpreadStrip_h

#ifdef __GNUG__
#pragma interface
#endif

#include <LACoreTemplateType.h>
#include "LAFunction.h"
#include <LAPriceCMSCalibration.h>
#include <vector>

using namespace std;

/// Strip CMS Spread cap/floor quotes (multi-look) into single-look prices by assuming
/// Normal prices for single-looks with interpolated correlations.
class LAPriceCMSSpreadStrip
{
public:

    static void Strip(LADataInstance* dataInstance, LAString convID, LAString currency,
                      LADate valDate, const LAString& pairID, const LAStringMatrix& legScheduler, const LAStringMatrix& cmsScheduler,
                      const LAStringMatrix& quoteMatrix, const vector<bool>& isCalls,
                      CurveInfo discCurveInfo, CurveInfo cmsCurveInfo,
                      ReplicationConfig repConfig, double shift,
                      const LAStringVector& slTerms, const DateVector& slDates,
                      DoubleVector& smileStrikes, DoubleMatrix& smilePrices);
};

//================ CMSCalibrationTarget ===================================
class LAPriceCMSSpreadStripTarget : public LAFunction
{
public:
    LAPriceCMSSpreadStripTarget() {};

    LAPriceCMSSpreadStripTarget(LADate valDate, LAString mtyTerm,
                           SwapRateInfo rateInfo1, SwapRateInfo rateInfo2,
                           LAStringMatrix legScheduler, LAStringMatrix cmsScheduler, CurveInfo discCurveInfo,
                           size_t parameterIdx);

    void SetMarket(double quote, double strike, bool isCall);

    void SetParameterGrids(const DoubleVector& tGrid, const DoubleVector& pGrid);

    double operator()(double x) const;

    double InterpolateCorrelations(double t, const DoubleVector& pGrid) const;

    LADate LastFixing();

    DoubleVector UpdatedParameterGrid(double x) const;

private:
    vector<CashFlowTiming> mStructLegSchedule;
    double mQuote, mStrike;
    bool mIsCall;
    LAString mIndexTenor, mSwapMaturity;
    DoubleVector mAccs, mS1s, mS2s, mStDev1s, mStDev2s, mDFs, mTFix;
    DoubleVector mtGrid, mpGrid;
    size_t mNFlows, mParameterIdx;
    LADate mLastFixing;
};

#endif 
