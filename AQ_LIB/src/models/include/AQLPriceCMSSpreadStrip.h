#ifndef AQLPriceCMSSpreadStrip_h
#define AQLPriceCMSSpreadStrip_h

#ifdef __GNUG__
#pragma interface
#endif

#include <AQLCoreTemplateType.h>
#include "AQLFunction.h"
#include <AQLPriceCMSCalibration.h>
#include <vector>

using namespace std;

/// Strip CMS Spread cap/floor quotes (multi-look) into single-look prices by assuming
/// Normal prices for single-looks with interpolated correlations.
class AQLPriceCMSSpreadStrip
{
public:

    static void Strip(AQLDataInstance* dataInstance, AQLString convID, AQLString currency,
                      AQLDate valDate, const AQLString& pairID, const AQLStringMatrix& legScheduler, const AQLStringMatrix& cmsScheduler,
                      const AQLStringMatrix& quoteMatrix, const vector<bool>& isCalls,
                      CurveInfo discCurveInfo, CurveInfo cmsCurveInfo,
                      ReplicationConfig repConfig, double shift,
                      const AQLStringVector& slTerms, const DateVector& slDates,
                      DoubleVector& smileStrikes, DoubleMatrix& smilePrices);
};

//================ CMSCalibrationTarget ===================================
class AQLPriceCMSSpreadStripTarget : public AQLFunction
{
public:
    AQLPriceCMSSpreadStripTarget() {};

    AQLPriceCMSSpreadStripTarget(AQLDate valDate, AQLString mtyTerm,
                           SwapRateInfo rateInfo1, SwapRateInfo rateInfo2,
                           AQLStringMatrix legScheduler, AQLStringMatrix cmsScheduler, CurveInfo discCurveInfo,
                           size_t parameterIdx);

    void SetMarket(double quote, double strike, bool isCall);

    void SetParameterGrids(const DoubleVector& tGrid, const DoubleVector& pGrid);

    double operator()(double x) const;

    double InterpolateCorrelations(double t, const DoubleVector& pGrid) const;

    AQLDate LastFixing();

    DoubleVector UpdatedParameterGrid(double x) const;

private:
    vector<CashFlowTiming> mStructLegSchedule;
    double mQuote, mStrike;
    bool mIsCall;
    AQLString mIndexTenor, mSwapMaturity;
    DoubleVector mAccs, mS1s, mS2s, mStDev1s, mStDev2s, mDFs, mTFix;
    DoubleVector mtGrid, mpGrid;
    size_t mNFlows, mParameterIdx;
    AQLDate mLastFixing;
};

#endif 
