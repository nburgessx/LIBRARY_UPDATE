#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "LAPriceSwapRateCalculator.h"
#include "LAMathDateUtilities.h"
#include "LAFunctionUtilities.h"
#include "LAMathCurveFuncUtility.h"
#include "LAPriceSwaptionCalculator.h"
#include "LAMathAnalyticalFormula.h"
#include "LAMathOptionTools.h"

SwapRateInfo::SwapRateInfo(LADataInstance* dataInstance, const LAString& ccy, const LAString& tenor_, CurveInfo discCurveInfo_, CurveInfo cmsCurveInfo_,
                           LAStringMatrix cmsScheduler, ReplicationConfig repConfig, double shift)
{
    mDataInstance = dataInstance;
    mTenor = tenor_;
    mDiscCurveInfo = discCurveInfo_;
    mCmsCurveInfo = cmsCurveInfo_;
    mRepConfig = repConfig;
    mShift = shift;
    mFixedLegFreq = LAFunctionUtilities::findElement(cmsScheduler, "FixedLegFrequency");
    mIndexCalendar = LAFunctionUtilities::findElement(cmsScheduler, "IndexCalendar");
    mIndexDaycount = LAFunctionUtilities::findElement(cmsScheduler, "IndexDaycount");
    mIndexSlidingRule = LAFunctionUtilities::findElement(cmsScheduler, "IndexSlidingRule");
    mPaySlidingRule.convertFromString(LAFunctionUtilities::findElement(cmsScheduler, "PaymentSlidingRule"));
    mPayCalendar.convertFromString(LAFunctionUtilities::findElement(cmsScheduler, "PaymentCalendar"));

    // Parameter vectors
    mSabrIDs = LAStringVector(MLIB_SABR_NAMES.size());
    for (size_t k = 0; k < MLIB_SABR_NAMES.size(); k++)
        mSabrIDs[k] = LAPriceCMSObject::MatrixID("_" + MLIB_SABR_NAMES[k] + "_", ccy);
    mTailIDs = LAStringVector(MLIB_TAIL_NAMES.size());
    for (size_t k = 0; k < MLIB_TAIL_NAMES.size(); k++)
        mTailIDs[k] = LAPriceCMSObject::MatrixID("_" + MLIB_TAIL_NAMES[k] + "_", ccy);

    // For CMSs
    mTau = LAPriceCMSObject::TermToTau(FrequencyToTerm(mFixedLegFreq));
    double cmsLength = TermToYearLength(mTenor);
    mNCashFlows = cmsLength / mTau;
}

double SwapRateInfo::ForwardSwapRate(LADate settlDate)
{
    LAString swapTerm = mTenor;
    CurveInfo fwdCurveInfo = mCmsCurveInfo;
    CurveInfo discCurveInfo = mDiscCurveInfo;
    LAString fixedLegFreq = mFixedLegFreq;
    LAString indexDaycount = mIndexDaycount;
    LAString indexSlidingRule = mIndexSlidingRule;
    LAString indexCalendar = mIndexCalendar;
    LAString ratetype = LAString("PAR");
    LAString interpolation = LAString("SPLINE");
    LAString roll_convention = LAString("EOM");
    bool eomroll = true;
    return LAMathCurveFuncUtility::getRate(settlDate, swapTerm, fwdCurveInfo.dataInstance, fwdCurveInfo.curveID, ratetype,
                                         fixedLegFreq, indexDaycount, indexSlidingRule, indexCalendar, interpolation,
                                         fwdCurveInfo.curveName, discCurveInfo.curveName,
                                         fwdCurveInfo.isFwdInterpolated,
                                         eomroll && is_last_business_day_temp(settlDate, indexCalendar) ? &roll_convention : NULL);
}

double SwapRateInfo::Annuity(LADate settlDate)
{
    LAString swapTerm = mTenor;
    CurveInfo curveInfo = mDiscCurveInfo;
    LAString fixedLegFreq = mFixedLegFreq;
    LAString indexDaycount = mIndexDaycount;
    LAString indexSlidingRule = mIndexSlidingRule;
    LAString indexCalendar = mIndexCalendar;
    LAPriceDataSlidingRule paySlidingRule = mPaySlidingRule;
    LAPriceDataCalendar payCalendar = mPayCalendar;

    LADate mtyDate = CalendarAdvance(settlDate, swapTerm, paySlidingRule, payCalendar);
    LADate* fodd = NULL;
    LADate* lodd = NULL;
    int* day = NULL;
    LAString interpolation = LAString("SPLINE");

    return LAMathCurveFuncUtility::getAnnuity(settlDate, mtyDate, curveInfo.dataInstance, curveInfo.curveID, fixedLegFreq,
                                            indexSlidingRule, indexCalendar,
                                            fodd, lodd, day, indexDaycount, interpolation, curveInfo.curveName);
}

double SwapRateInfo::CMSForward(double fwd, double annuity, double T, double dfPay, double deltaT,
                                const DoubleVector& sabrParameters, const DoubleVector& tailParameters)
{
    string model = mRepConfig.swaptionModel;
    double result;
    if (model == "CMSMarketN")
    {
        double vol = tailParameters[1];
        double R0 = MVReplication::R(fwd, Tau(), deltaT, NCashFlows());
        return fwd + R0 * vol * vol * T;
    }
    else if (model == "CMSMarketLogN")
    {
        double vol = tailParameters[1];
        double R0 = MVReplication::R(fwd, Tau(), deltaT, NCashFlows());
        double sfwd = fwd + mShift;
        return fwd + R0 * sfwd * sfwd * (exp(vol * vol * T) - 1.0);
    }
    else
    {
        LAPriceSwaptionCalculator* swpnCalculator = GetSwaptionCalculator(mRepConfig.swaptionModel, fwd + mShift,
            sabrParameters, tailParameters);
        MVReplication* rep = GetReplicationMethod(mRepConfig.replicationModel, swpnCalculator, mShift, annuity,
            dfPay, Tau(), NCashFlows(),
            mRepConfig.confidence, mRepConfig.nPoints);

        result = rep->Forward(T, deltaT, fwd);
        delete swpnCalculator;
        delete rep;
    }

    return result;
}

double SwapRateInfo::CMSForward(LADate valDate, CashFlowTiming cf)
{
    double fwd = ForwardSwapRate(cf.indexSettlement);
    double annuity = Annuity(cf.indexSettlement);
    double expiry = ModelTime(valDate, cf.fixing);
    double dfPay = LAPriceCMSObject::DiscountFactor(mDiscCurveInfo, valDate, cf.payment);
    double tau = Tau();
    double nCashFlows = NCashFlows();
    double deltaT = cf.accrual;
    DoubleVector sabr = LAPriceCMSObject::InterpolateParameterMatrix(mDataInstance, cf.fixing, mTenor, mSabrIDs);
    DoubleVector tails = LAPriceCMSObject::InterpolateParameterMatrix(mDataInstance, cf.fixing, mTenor, mTailIDs);

    double cmsFwd;
    if (mRepConfig.swaptionModel == "CMSMarketN")
    {
        double vol = tails[1];
        double R0 = MVReplication::R(fwd, tau, deltaT, nCashFlows);
        cmsFwd = fwd + R0 * vol * vol * expiry;
    }
    else if (mRepConfig.swaptionModel == "CMSMarketLogN")
    {
        double vol = tails[1];
        double R0 = MVReplication::R(fwd, tau, deltaT, nCashFlows);
        double sfwd = fwd + mShift;
        cmsFwd = fwd + R0 * sfwd * sfwd * (exp(vol * vol * expiry) - 1.0);
    }
    else
    {
        LAPriceSwaptionCalculator* swpnCalculator = GetSwaptionCalculator(mRepConfig.swaptionModel, fwd + mShift, sabr, tails);
        MVReplication* rep = GetReplicationMethod(mRepConfig.replicationModel, swpnCalculator, mShift, annuity, dfPay, tau,
            nCashFlows, mRepConfig.confidence, mRepConfig.nPoints);

        cmsFwd = rep->Forward(expiry, deltaT, fwd);
        delete swpnCalculator;
        delete rep;
    }

    return cmsFwd;
}

void SwapRateInfo::CMSDistribution(LADate valDate, CashFlowTiming cf, double& cmsFwd, double& cmsVol)
{
    double fwd = ForwardSwapRate(cf.indexSettlement);
    double annuity = Annuity(cf.indexSettlement);
    double expiry = ModelTime(valDate, cf.fixing);
    double dfPay = LAPriceCMSObject::DiscountFactor(mDiscCurveInfo, valDate, cf.payment);
    double tau = Tau();
    double nCashFlows = NCashFlows();
    double deltaT = cf.accrual;
    DoubleVector sabr = LAPriceCMSObject::InterpolateParameterMatrix(mDataInstance, cf.fixing, mTenor, mSabrIDs);
    DoubleVector tails = LAPriceCMSObject::InterpolateParameterMatrix(mDataInstance, cf.fixing, mTenor, mTailIDs);
    if (mRepConfig.swaptionModel == "CMSMarketN")
    {
        double vol = tails[1];
        double stDev = vol * sqrt(expiry);
        double correction = MVReplication::R(fwd, tau, deltaT, nCashFlows) * stDev * stDev;
        cmsFwd = fwd + correction;
        double atmStrike = cmsFwd;
        double swpnCap = BachelierPrice(expiry, atmStrike, true, fwd, vol);
        double fwdATMCap = swpnCap + correction * LADist::normsdist((fwd - atmStrike) / stDev);
        cmsVol = LAMathAnalyticalFormula::BlackImplVolDD(fwdATMCap, cmsFwd, atmStrike, 1.0, 0) / sqrt(expiry);
    }
    else if (mRepConfig.swaptionModel == "CMSMarketLogN")
    {
        throw LACoreInvalidData("CMSMarketLogN not available for CMS spreads yet", __FILE__, __LINE__);
    }
    else
    {
        LAPriceSwaptionCalculator* swpnCalculator = GetSwaptionCalculator(mRepConfig.swaptionModel, fwd + mShift, sabr, tails);
        MVReplication* rep = GetReplicationMethod(mRepConfig.replicationModel, swpnCalculator, mShift, annuity, dfPay, tau,
                                                  nCashFlows, mRepConfig.confidence, mRepConfig.nPoints);

        cmsFwd = rep->Forward(expiry, deltaT, fwd);
        double fwdATMCap = rep->Option(expiry, deltaT, cmsFwd, true, fwd);
        cmsVol = LAMathAnalyticalFormula::BlackImplVolDD(fwdATMCap, cmsFwd, cmsFwd, 1.0, 0) / sqrt(expiry);

        delete swpnCalculator;
        delete rep;
    }
}
