#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "AQLPriceSwapRateCalculator.h"
#include "AQLMathDateUtilities.h"
#include "AQLFunctionUtilities.h"
#include "AQLMathCurveFuncUtility.h"
#include "AQLPriceSwaptionCalculator.h"
#include "AQLMathAnalyticalFormula.h"
#include "AQLMathOptionTools.h"

SwapRateInfo::SwapRateInfo(AQLDataInstance* dataInstance, const AQLString& ccy, const AQLString& tenor_, CurveInfo discCurveInfo_, CurveInfo cmsCurveInfo_,
                           AQLStringMatrix cmsScheduler, ReplicationConfig repConfig, double shift)
{
    mDataInstance = dataInstance;
    mTenor = tenor_;
    mDiscCurveInfo = discCurveInfo_;
    mCmsCurveInfo = cmsCurveInfo_;
    mRepConfig = repConfig;
    mShift = shift;
    mFixedLegFreq = AQLFunctionUtilities::findElement(cmsScheduler, "FixedLegFrequency");
    mIndexCalendar = AQLFunctionUtilities::findElement(cmsScheduler, "IndexCalendar");
    mIndexDaycount = AQLFunctionUtilities::findElement(cmsScheduler, "IndexDaycount");
    mIndexSlidingRule = AQLFunctionUtilities::findElement(cmsScheduler, "IndexSlidingRule");
    mPaySlidingRule.convertFromString(AQLFunctionUtilities::findElement(cmsScheduler, "PaymentSlidingRule"));
    mPayCalendar.convertFromString(AQLFunctionUtilities::findElement(cmsScheduler, "PaymentCalendar"));

    // Parameter vectors
    mSabrIDs = AQLStringVector(AQ_SABR_NAMES.size());
    for (size_t k = 0; k < AQ_SABR_NAMES.size(); k++)
        mSabrIDs[k] = AQLPriceCMSObject::MatrixID("_" + AQ_SABR_NAMES[k] + "_", ccy);
    mTailIDs = AQLStringVector(AQ_TAIL_NAMES.size());
    for (size_t k = 0; k < AQ_TAIL_NAMES.size(); k++)
        mTailIDs[k] = AQLPriceCMSObject::MatrixID("_" + AQ_TAIL_NAMES[k] + "_", ccy);

    // For CMSs
    mTau = AQLPriceCMSObject::TermToTau(FrequencyToTerm(mFixedLegFreq));
    double cmsLength = TermToYearLength(mTenor);
    mNCashFlows = cmsLength / mTau;
}

double SwapRateInfo::ForwardSwapRate(AQLDate settlDate)
{
    AQLString swapTerm = mTenor;
    CurveInfo fwdCurveInfo = mCmsCurveInfo;
    CurveInfo discCurveInfo = mDiscCurveInfo;
    AQLString fixedLegFreq = mFixedLegFreq;
    AQLString indexDaycount = mIndexDaycount;
    AQLString indexSlidingRule = mIndexSlidingRule;
    AQLString indexCalendar = mIndexCalendar;
    AQLString ratetype = AQLString("PAR");
    AQLString interpolation = AQLString("SPLINE");
    AQLString roll_convention = AQLString("EOM");
    bool eomroll = true;
    return AQLMathCurveFuncUtility::getRate(settlDate, swapTerm, fwdCurveInfo.dataInstance, fwdCurveInfo.curveID, ratetype,
                                         fixedLegFreq, indexDaycount, indexSlidingRule, indexCalendar, interpolation,
                                         fwdCurveInfo.curveName, discCurveInfo.curveName,
                                         fwdCurveInfo.isFwdInterpolated,
                                         eomroll && is_last_business_day_temp(settlDate, indexCalendar) ? &roll_convention : NULL);
}

double SwapRateInfo::Annuity(AQLDate settlDate)
{
    AQLString swapTerm = mTenor;
    CurveInfo curveInfo = mDiscCurveInfo;
    AQLString fixedLegFreq = mFixedLegFreq;
    AQLString indexDaycount = mIndexDaycount;
    AQLString indexSlidingRule = mIndexSlidingRule;
    AQLString indexCalendar = mIndexCalendar;
    AQLPriceDataSlidingRule paySlidingRule = mPaySlidingRule;
    AQLPriceDataCalendar payCalendar = mPayCalendar;

    AQLDate mtyDate = CalendarAdvance(settlDate, swapTerm, paySlidingRule, payCalendar);
    AQLDate* fodd = NULL;
    AQLDate* lodd = NULL;
    int* day = NULL;
    AQLString interpolation = AQLString("SPLINE");

    return AQLMathCurveFuncUtility::getAnnuity(settlDate, mtyDate, curveInfo.dataInstance, curveInfo.curveID, fixedLegFreq,
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
        AQLPriceSwaptionCalculator* swpnCalculator = GetSwaptionCalculator(mRepConfig.swaptionModel, fwd + mShift,
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

double SwapRateInfo::CMSForward(AQLDate valDate, CashFlowTiming cf)
{
    double fwd = ForwardSwapRate(cf.indexSettlement);
    double annuity = Annuity(cf.indexSettlement);
    double expiry = ModelTime(valDate, cf.fixing);
    double dfPay = AQLPriceCMSObject::DiscountFactor(mDiscCurveInfo, valDate, cf.payment);
    double tau = Tau();
    double nCashFlows = NCashFlows();
    double deltaT = cf.accrual;
    DoubleVector sabr = AQLPriceCMSObject::InterpolateParameterMatrix(mDataInstance, cf.fixing, mTenor, mSabrIDs);
    DoubleVector tails = AQLPriceCMSObject::InterpolateParameterMatrix(mDataInstance, cf.fixing, mTenor, mTailIDs);

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
        AQLPriceSwaptionCalculator* swpnCalculator = GetSwaptionCalculator(mRepConfig.swaptionModel, fwd + mShift, sabr, tails);
        MVReplication* rep = GetReplicationMethod(mRepConfig.replicationModel, swpnCalculator, mShift, annuity, dfPay, tau,
            nCashFlows, mRepConfig.confidence, mRepConfig.nPoints);

        cmsFwd = rep->Forward(expiry, deltaT, fwd);
        delete swpnCalculator;
        delete rep;
    }

    return cmsFwd;
}

void SwapRateInfo::CMSDistribution(AQLDate valDate, CashFlowTiming cf, double& cmsFwd, double& cmsVol)
{
    double fwd = ForwardSwapRate(cf.indexSettlement);
    double annuity = Annuity(cf.indexSettlement);
    double expiry = ModelTime(valDate, cf.fixing);
    double dfPay = AQLPriceCMSObject::DiscountFactor(mDiscCurveInfo, valDate, cf.payment);
    double tau = Tau();
    double nCashFlows = NCashFlows();
    double deltaT = cf.accrual;
    DoubleVector sabr = AQLPriceCMSObject::InterpolateParameterMatrix(mDataInstance, cf.fixing, mTenor, mSabrIDs);
    DoubleVector tails = AQLPriceCMSObject::InterpolateParameterMatrix(mDataInstance, cf.fixing, mTenor, mTailIDs);
    if (mRepConfig.swaptionModel == "CMSMarketN")
    {
        double vol = tails[1];
        double stDev = vol * sqrt(expiry);
        double correction = MVReplication::R(fwd, tau, deltaT, nCashFlows) * stDev * stDev;
        cmsFwd = fwd + correction;
        double atmStrike = cmsFwd;
        double swpnCap = BachelierPrice(expiry, atmStrike, true, fwd, vol);
        double fwdATMCap = swpnCap + correction * AQLDist::normsdist((fwd - atmStrike) / stDev);
        cmsVol = AQLMathAnalyticalFormula::BlackImplVolDD(fwdATMCap, cmsFwd, atmStrike, 1.0, 0) / sqrt(expiry);
    }
    else if (mRepConfig.swaptionModel == "CMSMarketLogN")
    {
        throw AQLCoreInvalidData("CMSMarketLogN not available for CMS spreads yet", __FILE__, __LINE__);
    }
    else
    {
        AQLPriceSwaptionCalculator* swpnCalculator = GetSwaptionCalculator(mRepConfig.swaptionModel, fwd + mShift, sabr, tails);
        MVReplication* rep = GetReplicationMethod(mRepConfig.replicationModel, swpnCalculator, mShift, annuity, dfPay, tau,
                                                  nCashFlows, mRepConfig.confidence, mRepConfig.nPoints);

        cmsFwd = rep->Forward(expiry, deltaT, fwd);
        double fwdATMCap = rep->Option(expiry, deltaT, cmsFwd, true, fwd);
        cmsVol = AQLMathAnalyticalFormula::BlackImplVolDD(fwdATMCap, cmsFwd, cmsFwd, 1.0, 0) / sqrt(expiry);

        delete swpnCalculator;
        delete rep;
    }
}
