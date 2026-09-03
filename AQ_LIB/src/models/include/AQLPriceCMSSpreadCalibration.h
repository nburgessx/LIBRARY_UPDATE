#ifndef AQLPriceCMSSpreadCalibration_h
#define AQLPriceCMSSpreadCalibration_h

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLString.h"
#include "AQLDate.h"
#include "AQLDataInstance.h"
#include <utility>
#include "AQLFunction.h"
#include "AQLPriceCMSCalibration.h"
#include "AQLFunctionVector.h"

using namespace std;

//================ Calibration ===================================
class AQLPriceCMSSpreadCalibration
{
public:
    static AQLString Calibrate(AQLDataInstance* dataInstance, AQLStringMatrix calibrationConfig, AQLStringMatrix legScheduler, AQLStringMatrix cmsScheduler,
                              AQLStringMatrix inputTypes, const AQLStringVector& optionTypes, const DoubleVector& smileWeights);

    static void ParseSLSmile(const AQLStringMatrix& quoteMatrix, const AQLStringVector& refTerms,
                             const vector<bool>& isCalls,
                             DoubleVector& smileStrikes, DoubleMatrix& smilePrices);
};

//================ Calibration Targets ===================================
class AQLPriceCMSSpreadATMTarget : public AQLFunction
{
public:
    AQLPriceCMSSpreadATMTarget(AQLDate valDate, CashFlowTiming cf, CurveInfo discCurveInfo,
                         const SwapRateInfo& swapRate1, const SwapRateInfo& swapRate2,
                         double quote, bool isCall, double theta1, double theta2);

    double operator()(double x) const;

private:
    SwapRateInfo mSwapRate1, mSwapRate2;
    double mQuote, mExpiry, mStrike, mS1, mS2, mVol1, mVol2, mConfidence, mDfPay;
    double mTheta1, mTheta2;
    bool mIsCall;
    string mCopulaType = "Power Gaussian";
};

class AQLPriceCMSSpreadSmileTarget : public AQLFunctionVector
{
public:
    AQLPriceCMSSpreadSmileTarget(AQLDate valDate, CashFlowTiming cf, CurveInfo discCurveInfo,
                           const SwapRateInfo& swapRate1, const SwapRateInfo& swapRate2,
                           const DoubleVector& quotes, const DoubleVector& strikes, const vector<bool>& isCall,
                           const DoubleVector& weights);

    virtual ~AQLPriceCMSSpreadSmileTarget() { }

    unsigned long lengthOfArgumentVector( ) { return mArgSize; }

    unsigned long lengthOfFunctionVector( ) { return mArgSize; }

    bool constraintsAreViolated(const DoubleVector& x);

    unsigned long maximumNumberOfIterations() { return 1000; }

    void operator()(DoubleVector& f, const DoubleVector& x);

    double ATMStrike() { return mATMStrike; }

    void RetrieveCache(DoubleVector& v);

private:
    SwapRateInfo mSwapRate1, mSwapRate2;
    DoubleVector mLwBound, mUpBound, mQuotes, mWeights, mStrikes;
    vector<bool> mIsCall;
    size_t mArgSize = 3, mNStrikes;

    double mExpiry, mS1, mS2, mVol1, mVol2, mConfidence, mDfPay;
    double mWeightTotal, mATMStrike;
    string mCopulaType = "Power Gaussian";
};

#endif 
