#ifndef AQLPriceCMSCalibration_h
#define AQLPriceCMSCalibration_h

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLFunction.h"
#include "AQLPriceCMSTools.h"
#include "AQLMathCashFlowSchedules.h"
#include "AQLPriceReplication.h"
#include "AQLPriceSwapRateCalculator.h"

//================ Calibration Procedure ===================================
class AQLPriceCMSCalibration
{
public:

    static AQLString Calibrate(AQLDataInstance* dataInstance, AQLStringMatrix calibrationConfig,
                              AQLStringMatrix structLegScheduler, AQLStringMatrix cmsScheduler,
                              AQLStringMatrix fundLegScheduler, AQLStringMatrix liborScheduler,
                              const AQLStringVector& tenors, const AQLStringVector& mtyTerms,
                              const DoubleMatrix& quotes);
};

//================ CMSCalibrationTarget ===================================
class AQLPriceCMSCalibrationTarget : public AQLFunction
{
public:
    AQLPriceCMSCalibrationTarget() {};

    AQLPriceCMSCalibrationTarget(AQLDate valDate, AQLString tenor, AQLString maturity,
                           AQLStringMatrix structLegScheduler, AQLStringMatrix cmsScheduler,
                           AQLStringMatrix fundLegScheduler, AQLStringMatrix liborScheduler,
                           CurveInfo discCurveInfo, CurveInfo fundLiborCurveInfo,
                           CurveInfo cmsCurveInfo, SwapRateInfo* rateInfo, double quote,
                           size_t parameterIdx, AQLDataInstance* dataInstance, const AQLStringVector& sabrIDs,
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
    AQLString mIndexTenor, mSwapMaturity;
    DoubleVector mAccs, mFwdSwapRates, mAnnuities, mDFs, mTFix;
    DoubleVector mtGrid, mpGrid;
    DoubleMatrix mSABR;
    size_t mNStructFlows, mParameterIdx;
};

#endif 
