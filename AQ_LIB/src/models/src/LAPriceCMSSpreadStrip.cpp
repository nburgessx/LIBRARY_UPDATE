#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "LAPriceCMSSpreadStrip.h"
#include "LAPriceCMSSpreadCalibration.h"
#include "LAMathCashFlowSchedules.h"
#include "LAFunctionUtilities.h"
#include "LAMathDateUtilities.h"
#include "LAMathInterpolationUtilities.h"
#include "LAOptimumBrent.h"
#include "LAMathParameterUtility.h"
#include "LACoreUtil.h"
#include "LAPriceCMSSpreadTools.h"

void LAPriceCMSSpreadStrip::Strip(LADataInstance* dataInstance, LAString convID, LAString currency,
                             LADate valDate, const LAString& pairID, const LAStringMatrix& legScheduler, const LAStringMatrix& cmsScheduler,
                             const LAStringMatrix& quoteMatrix, const vector<bool>& isCalls,
                             CurveInfo discCurveInfo, CurveInfo cmsCurveInfo,
                             ReplicationConfig repConfig, double shift,
                             const LAStringVector& slTerms, const DateVector& slDates,
                             DoubleVector& smileStrikes, DoubleMatrix& slPrices)
{
    // Expiries
    size_t nExpiries = quoteMatrix.size() - 1;
    LAStringVector expiryTerms(nExpiries);
    for (size_t i = 0; i < nExpiries; i++)
        expiryTerms[i] = quoteMatrix[i + 1][0];

    // Strikes
    size_t nStrikes = quoteMatrix[0].size() - 2; // First column is ATM info
    DoubleVector strikes(nStrikes);
    for (size_t j = 0; j < nStrikes; j++)
        strikes[j] = quoteMatrix[0][j + 2].getDoubleValue() / 100.0;
    if (isCalls.size() != nStrikes)
        throw LACoreInvalidData("Inconsistent strike sizes in spread strip", __FILE__, __LINE__);

    // Quotes
    DoubleMatrix quotes(nExpiries, DoubleVector(nStrikes));
    for (size_t i = 0; i < nExpiries; i++)
        for (size_t j = 0; j < nStrikes; j++)
            quotes[i][j] = quoteMatrix[i + 1][j + 2].getDoubleValue();

    // Index information
    LAString tenor1, tenor2;
    LAPriceCMSObject::ParseTenors(pairID, tenor1, tenor2);
    SwapRateInfo rateInfo1(dataInstance, currency, tenor1, discCurveInfo, cmsCurveInfo, cmsScheduler, repConfig, shift);
    SwapRateInfo rateInfo2(dataInstance, currency, tenor2, discCurveInfo, cmsCurveInfo, cmsScheduler, repConfig, shift);

    // For each expiry, generate a schedule
    vector<LAPriceCMSSpreadStripTarget> targets(nExpiries);
    DateVector stripDates(nExpiries);
    DoubleVector stripTimes(nExpiries);
    for (size_t i = 0; i < nExpiries; i++)
    {
        targets[i] = LAPriceCMSSpreadStripTarget(valDate, expiryTerms[i], rateInfo1, rateInfo2,
                                            legScheduler, cmsScheduler, discCurveInfo, i);
        stripDates[i] = targets[i].LastFixing();
        stripTimes[i] = ModelTime(valDate, stripDates[i]);
    }

    // Set optimizer
    double init = 0.50, lwBound = -0.9999, upBound = 0.9999, tol = 0.0000001;
    size_t maxIter = 1000;
    LAOptimumBrent minimizer(init, lwBound, upBound, maxIter, tol);

    // Strip for each strike
    DoubleMatrix rawStrip(nStrikes, DoubleVector(nExpiries));
    for (size_t j = 0; j < nStrikes; j++)
    {
        // Strip at fixed strike
        double strike = strikes[j];
        bool isCall = isCalls[j];
        DoubleVector params(nExpiries, init);
        for (size_t i = 0; i < nExpiries; i++)
        {
            targets[i].SetMarket(quotes[i][j], strike, isCall);
            targets[i].SetParameterGrids(stripTimes, params);
            // Optimize
            DoubleVector solution(1, params[i]);
            minimizer.findMinimum(targets[i], solution);
            params[i] = solution[0];
        }
        rawStrip[j] = params;
    }

    // Interpolate to specified SL time grid
    size_t nSlDates = slDates.size();
    slPrices = DoubleMatrix(nStrikes, DoubleVector(nSlDates));
    //double deltaT = 0.0;
    etrading::InterpolationEnum interpolation = etrading::LINEAR_INTERPOLATION;
    LAStringMatrix stripRhoOut(nSlDates + 1, LAStringVector(nStrikes + 1));
    LAStringMatrix stripPriceOut(nSlDates + 1, LAStringVector(nStrikes + 1));
    LAString stripRhoID = "_StripRho_" + pairID + "_";
    LAString stripPriceID = "_StripPrice_" + pairID + "_";
    stripRhoOut[0][0] = stripRhoID;
    stripPriceOut[0][0] = stripPriceID;
    for (size_t i = 0; i < nSlDates; i++)
    {
        CashFlowTiming timing = LAMathScheduleUtility::CashFlowSchedule(valDate, slTerms[i], legScheduler, cmsScheduler);
        double expiry = ModelTime(valDate, timing.fixing);
        double df = LAPriceCMSObject::DiscountFactor(discCurveInfo, valDate, timing.payment);
        double S1, S2, stDev1, stDev2;
        timing.accrual = 0.0;
        rateInfo1.CMSDistribution(valDate, timing, S1, stDev1);
        rateInfo2.CMSDistribution(valDate, timing, S2, stDev2);
        double sqrtt = sqrt(expiry);
        stDev1 *= sqrtt;
        stDev2 *= sqrtt;
        for (size_t j = 0; j < nStrikes; j++)
        {
            double rho = LAMathInterpolationUtilities::interpolate(stripTimes, rawStrip[j], expiry, interpolation);
            slPrices[j][i] = df * LAPriceCMSSpreadUtility::CMSSpreadCoupon(strikes[j], isCalls[j], S1, S2, stDev1, stDev2, rho);
            stripRhoOut[1 + i][1 + j] = LAString(n2s(rho).c_str());
            stripPriceOut[1 + i][1 + j] = LAString(n2s(slPrices[j][i]).c_str());
            if (j == 0)
            {
                stripRhoOut[1 + i][0] = slTerms[i];
                stripPriceOut[1 + i][0] = slTerms[i];
            }
        }

         //Load
        if (i == 0)
        {
            for (size_t j = 0; j < nStrikes; j++)
            {
                stripRhoOut[0][1 + j] = LAString(n2s(strikes[j]).c_str());
                stripPriceOut[0][1 + j] = LAString(n2s(strikes[j]).c_str());
            }
        }
    }
    smileStrikes = strikes;

    // Load to memory
    LAMathParameterObject::SetParameterMatrix(dataInstance, LAPriceCMSObject::MatrixID(stripRhoID, currency), convID, stripRhoOut);
    LAMathParameterObject::SetParameterMatrix(dataInstance, LAPriceCMSObject::MatrixID(stripPriceID, currency), convID, stripPriceOut);
}

//================ CMSCalibrationTarget ===================================
LAPriceCMSSpreadStripTarget::LAPriceCMSSpreadStripTarget(LADate valDate, LAString mtyTerm,
                                               SwapRateInfo rateInfo1, SwapRateInfo rateInfo2,
                                               LAStringMatrix legScheduler, LAStringMatrix cmsScheduler,
                                               CurveInfo discCurveInfo, size_t parameterIdx)
{
    vector<CashFlowTiming> schedule = LAMathScheduleUtility::LegSchedule(valDate, mtyTerm, legScheduler, cmsScheduler);
    mParameterIdx = parameterIdx;

    ////// Cache data ////
    mNFlows = schedule.size();
    mAccs = DoubleVector(mNFlows), mS1s = DoubleVector(mNFlows), mS2s = DoubleVector(mNFlows);
    mStDev1s = DoubleVector(mNFlows), mStDev2s = DoubleVector(mNFlows);
    mDFs = DoubleVector(mNFlows), mTFix = DoubleVector(mNFlows);
    for (size_t i = 0; i < mNFlows; i++)
    {
        CashFlowTiming timing = schedule[i];
        double t = ModelTime(valDate, timing.fixing);
        mTFix[i] = t;
        mAccs[i] = timing.accrual;
        mDFs[i] = LAPriceCMSObject::DiscountFactor(discCurveInfo, valDate, timing.payment);
        // S1
        rateInfo1.CMSDistribution(valDate, timing, mS1s[i], mStDev1s[i]);
        mStDev1s[i] *= sqrt(t);
        // S2
        rateInfo2.CMSDistribution(valDate, timing, mS2s[i], mStDev2s[i]);
        mStDev2s[i] *= sqrt(t);

        if (fabs(t) < 0.0001)
        {
            mStDev1s[i] = 0.0;
            mStDev2s[i] = 0.0;
        }
    }

    mLastFixing = schedule.back().fixing;
}

double LAPriceCMSSpreadStripTarget::operator()(double x) const
{
    // Set new parameters
    DoubleVector currentpGrid = UpdatedParameterGrid(x);

    // Calculate PV of structured leg
    double pv = 0.0;
    for (size_t i = 1; i < mNFlows; i++) // First flow not included
    {
        double t = mTFix[i];
        double rho = InterpolateCorrelations(t, currentpGrid);
        double coupon = LAPriceCMSSpreadUtility::CMSSpreadCoupon(mStrike, mIsCall, mS1s[i], mS2s[i],
                                                            mStDev1s[i], mStDev2s[i], rho);
        pv += mDFs[i] * mAccs[i] * coupon;
    }

    // Calculate target
    return fabs(pv - mQuote);
}

void LAPriceCMSSpreadStripTarget::SetMarket(double quote, double strike, bool isCall)
{
    mQuote = quote; mStrike = strike; mIsCall = isCall;
}

LADate LAPriceCMSSpreadStripTarget::LastFixing()
{
    return mLastFixing;
}

void LAPriceCMSSpreadStripTarget::SetParameterGrids(const DoubleVector& tGrid, const DoubleVector& pGrid)
{
    mtGrid = DoubleVector(tGrid);
    mpGrid = DoubleVector(pGrid);
}

DoubleVector LAPriceCMSSpreadStripTarget::UpdatedParameterGrid(double x) const
{
    DoubleVector p(mpGrid);
    p[mParameterIdx] = x;
    return p;
}

double LAPriceCMSSpreadStripTarget::InterpolateCorrelations(double t, const DoubleVector& pGrid) const
{
    etrading::InterpolationEnum interpolation = etrading::LINEAR_INTERPOLATION;
    return LAMathInterpolationUtilities::interpolate(mtGrid, pGrid, t, interpolation);
}
