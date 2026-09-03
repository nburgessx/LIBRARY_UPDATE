#ifndef LAPriceCMSCalibration_h
#define LAPriceCMSCalibration_h

#ifdef __GNUG__
#pragma interface
#endif

#include "LAFunction.h"
#include "LAPriceCMSTools.h"
#include "LAMathCashFlowSchedules.h"
#include "LAPriceReplication.h"
#include "LAPriceSwapRateCalculator.h"

//================ Calibration Procedure ===================================
class LAPriceCMSCalibration
{
public:

    static LAString Calibrate(LADataInstance* dataInstance, LAStringMatrix calibrationConfig,
                              LAStringMatrix structLegScheduler, LAStringMatrix cmsScheduler,
                              LAStringMatrix fundLegScheduler, LAStringMatrix liborScheduler,
                              const LAStringVector& tenors, const LAStringVector& mtyTerms,
                              const DoubleMatrix& quotes);
};

//================ CMSCalibrationTarget ===================================
class LAPriceCMSCalibrationTarget : public LAFunction
{
public:
    LAPriceCMSCalibrationTarget() {};

    LAPriceCMSCalibrationTarget(LADate valDate, LAString tenor, LAString maturity,
                           LAStringMatrix structLegScheduler, LAStringMatrix cmsScheduler,
                           LAStringMatrix fundLegScheduler, LAStringMatrix liborScheduler,
                           CurveInfo discCurveInfo, CurveInfo fundLiborCurveInfo,
                           CurveInfo cmsCurveInfo, SwapRateInfo* rateInfo, double quote,
                           size_t parameterIdx, LADataInstance* dataInstance, const LAStringVector& sabrIDs,
                           double defaultTail1, double defaultTail3, double defaultTail4);

    double operator()(double x) const;

    void SetParameterGrids(const DoubleVector& tGrid, const DoubleVector& pGrid);

    DoubleVector InterpolateParameters(double t, const DoubleVector& pGrid) const;

    DoubleVector UpdatedParameterGrid(double x) const;

    double LastFixing();

private:
    vector<CashFlowTiming> mStructLegSchedule, mFundLegSchedule;
    SwapRateInfo* mRateInfo;
    double mQuote, mFundLegPV, mFundLegAnnuity;
    double mDefaultTail1, mDefaultTail3, mDefaultTail4;
    LAString mIndexTenor, mSwapMaturity;
    DoubleVector mAccs, mFwdSwapRates, mAnnuities, mDFs, mTFix;
    DoubleVector mtGrid, mpGrid;
    DoubleMatrix mSABR;
    size_t mNStructFlows, mParameterIdx;
};

#endif 
