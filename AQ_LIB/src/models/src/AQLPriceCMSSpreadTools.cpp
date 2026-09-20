#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "AQLPriceCMSSpreadTools.h"
#include "AQLPriceCMSTools.h"
#include "AQLFunctionUtilities.h"
#include "AQLMathParameterUtility.h"
#include "AQLMathOptionTools.h"
#include "AQLPriceCopulaCMSSpread.h"
#include "AQLDateSchedule.h"
#include "AQLOptimumBrent.h"

//================ Pricing ===================================
double AQLPriceCMSSpreadUtility::CMSSpreadMLATM(AQLDataInstance* dataInstance, const AQLString& tenor1, const AQLString& tenor2,
                                          const AQLString& expiryTerm, const AQLStringMatrix& pricingConfig,
                                          const AQLStringMatrix& legScheduler, const AQLStringMatrix& cmsScheduler,
                                          const AQLString& proxySpreadID)
{
    AQLPriceCMSMLATMTarget pricer = WarmUpMLPrice(dataInstance, tenor1, tenor2, expiryTerm, pricingConfig, legScheduler, cmsScheduler,
                                            proxySpreadID);

    // Set optimizer
    double init = 0.005, lwBound = -0.05, upBound = 0.05, tol = 0.0000001;
    size_t maxIter = 1000;
    AQLOptimumBrent minimizer(init, lwBound, upBound, maxIter, tol);
    // Solve
    DoubleVector solution(1, init);
    minimizer.findMinimum(pricer, solution);
    return solution[0];
}

double AQLPriceCMSSpreadUtility::CMSSpreadMLPrice(AQLDataInstance* dataInstance, const AQLString& tenor1, const AQLString& tenor2,
                                            const AQLString& expiryTerm, double strike, const AQLString& optionType,
                                            const AQLStringMatrix& pricingConfig,
                                            const AQLStringMatrix& legScheduler, const AQLStringMatrix& cmsScheduler,
                                            const AQLString& proxySpreadID)
{
    AQLPriceCMSMLATMTarget pricer = WarmUpMLPrice(dataInstance, tenor1, tenor2, expiryTerm, pricingConfig, legScheduler, cmsScheduler,
                                            proxySpreadID);

    double price;
    if (optionType == "Cap")
        price = pricer.Price(strike, true);
    else if (optionType == "Floor")
        price = pricer.Price(strike, false);
    else if (optionType == "Forward")
        price = pricer.Forward();
    else if (optionType == "Straddle")
    {
        double cap = pricer.Price(strike, true);
        double floor = pricer.Price(strike, false);
        price = cap + floor;
    }
    else
        throw AQLCoreInvalidData("Invalid price type", __FILE__, __LINE__);

    return price;
}

AQLPriceCMSMLATMTarget AQLPriceCMSSpreadUtility::WarmUpMLPrice(AQLDataInstance* dataInstance, const AQLString& tenor1, const AQLString& tenor2,
                                                   const AQLString& expiryTerm, const AQLStringMatrix& pricingConfig,
                                                   const AQLStringMatrix& legScheduler, const AQLStringMatrix& cmsScheduler,
                                                   const AQLString& proxySpreadID)
{
    SwapRateInfo rate1, rate2;
    WarmUpCMSSpread(dataInstance, tenor1, tenor2, expiryTerm, pricingConfig, legScheduler, cmsScheduler, rate1, rate2);

    // Fixed Inputs
    double confidence = 10.0;
    string copType = "PowerGaussian";

    // Other inputs
    AQLString ccy = AQLFunctionUtilities::findElement(pricingConfig, "Currency");
    AQLString colCcy = ccy;
    AQLDate valDate = etrading::AQLStringToDate(AQLFunctionUtilities::findElement(pricingConfig, "AsOfDate"));

    // Curves
    CurveInfo discCurveInfo = AQLPriceCMSObject::DiscountCurveInfo(dataInstance, ccy, colCcy);

    // Interpret forward expiry term
    size_t xPos;
    if (!TryFind(AQLString("x"), expiryTerm, xPos) || xPos < 1 || xPos > expiryTerm.size() - 1)
        throw AQLCoreInvalidData("Invalid expiry format for Multi-Look", __FILE__, __LINE__);
    AQLString startTerm = expiryTerm.subString(0, xPos - 1);
    AQLString endTerm = expiryTerm.subString(xPos + 1, expiryTerm.size() - 1);

    // Cash-flow
    vector<CashFlowTiming> schedule = AQLMathScheduleUtility::LegSchedule(valDate, endTerm, legScheduler, cmsScheduler);

    // Generate month vector from fixing dates
    AQLString frequency = AQLFunctionUtilities::findElement(legScheduler, "Frequency");
    AQLString freqTerm = etrading::FrequencyToTerm(frequency);
    int monthLength = etrading::TermToMonthLength(freqTerm);
    size_t nFlows = schedule.size();
    vector<size_t> months(nFlows);
    months[0] = 0;
    for (size_t i = 1; i < nFlows; i++)
        months[i] = months[i - 1] + monthLength;

    int startLength = etrading::TermToMonthLength(startTerm);
    int endLength = etrading::TermToMonthLength(endTerm);

    return AQLPriceCMSMLATMTarget(dataInstance, ccy, valDate, rate1, rate2, proxySpreadID, schedule, months, startLength, endLength,
                            discCurveInfo, copType, confidence);
}

void AQLPriceCMSSpreadUtility::WarmUpCMSSpread(AQLDataInstance* dataInstance, const AQLString& tenor1, const AQLString& tenor2,
                                         const AQLString& expiryTerm, const AQLStringMatrix& pricingConfig,
                                         const AQLStringMatrix& legScheduler, const AQLStringMatrix& cmsScheduler,
                                         SwapRateInfo& rate1, SwapRateInfo& rate2)
{
    // Fixed Inputs
    double confidence = 10.0;
    string copType = "PowerGaussian";

    // Other inputs
    AQLString ccy = AQLFunctionUtilities::findElement(pricingConfig, "Currency");
    AQLString colCcy = ccy;
    ReplicationConfig repConfig = GetReplicationConfig(pricingConfig);
    double shift = AQLFunctionUtilities::findElement(pricingConfig, "Shift").getDoubleValue();
    AQLDate valDate = etrading::AQLStringToDate(AQLFunctionUtilities::findElement(pricingConfig, "AsOfDate"));

    // Curves
    CurveInfo discCurveInfo = AQLPriceCMSObject::DiscountCurveInfo(dataInstance, ccy, colCcy);
    AQLString cmsFloatFreq = AQLFunctionUtilities::findElement(cmsScheduler, "FloatLegFrequency");
    CurveInfo cmsCurveInfo = AQLPriceCMSObject::ForecastCurveInfo(dataInstance, ccy, colCcy, etrading::FrequencyToTerm(cmsFloatFreq));

    // Swap rate definitions and parameters
    rate1 = SwapRateInfo(dataInstance, ccy, tenor1, discCurveInfo, cmsCurveInfo, cmsScheduler, repConfig, shift);
    rate2 = SwapRateInfo(dataInstance, ccy, tenor2, discCurveInfo, cmsCurveInfo, cmsScheduler, repConfig, shift);
}

double AQLPriceCMSSpreadUtility::CMSSpreadSLATM(AQLDataInstance* dataInstance, const AQLString& tenor1, const AQLString& tenor2,
                                          const AQLString& expiryTerm, const AQLStringMatrix& pricingConfig,
                                          const AQLStringMatrix& legScheduler, const AQLStringMatrix& cmsScheduler)
{
    SwapRateInfo rate1, rate2;
    WarmUpCMSSpread(dataInstance, tenor1, tenor2, expiryTerm, pricingConfig, legScheduler, cmsScheduler, rate1, rate2);

    AQLDate valDate = etrading::AQLStringToDate(AQLFunctionUtilities::findElement(pricingConfig, "AsOfDate"));
    CashFlowTiming cf = AQLMathScheduleUtility::CashFlowSchedule(valDate, expiryTerm, legScheduler, cmsScheduler);
    cf.accrual = 0.0;

    double cms1 = rate1.CMSForward(valDate, cf);
    double cms2 = rate2.CMSForward(valDate, cf);
    return cms1 - cms2;
}

double AQLPriceCMSSpreadUtility::CMSSpreadSLPrice(AQLDataInstance* dataInstance, const AQLString& tenor1, const AQLString& tenor2,
                                            const AQLString& expiryTerm, double strike, const AQLString& optionType,
                                            const AQLStringMatrix& pricingConfig,
                                            const AQLStringMatrix& legScheduler, const AQLStringMatrix& cmsScheduler,
                                            const AQLString& proxySpreadID)
{
    // Fixed Inputs
    double confidence = 10.0;
    string copType = "PowerGaussian";

    // Other inputs
    AQLString ccy = AQLFunctionUtilities::findElement(pricingConfig, "Currency");
    AQLString colCcy = ccy;
    AQLDate valDate = etrading::AQLStringToDate(AQLFunctionUtilities::findElement(pricingConfig, "AsOfDate"));

    // Curves
    CurveInfo discCurveInfo = AQLPriceCMSObject::DiscountCurveInfo(dataInstance, ccy, colCcy);

    SwapRateInfo rate1, rate2;
    WarmUpCMSSpread(dataInstance, tenor1, tenor2, expiryTerm, pricingConfig, legScheduler, cmsScheduler, rate1, rate2);

    // Cash-flow
    CashFlowTiming cf = AQLMathScheduleUtility::CashFlowSchedule(valDate, expiryTerm, legScheduler, cmsScheduler);
    cf.accrual = 0.0;

    // Calibrate marginals
    double CMS1, CMS2, vol1, vol2;
    rate1.CMSDistribution(valDate, cf, CMS1, vol1);
    rate2.CMSDistribution(valDate, cf, CMS2, vol2);

    // Copula parameters
    double t = etrading::ModelTime(valDate, cf.fixing);
    AQLString pairID = proxySpreadID;
    size_t nCopParams = 3;
    if (AQ_COP_NAMES.size() < nCopParams)
        throw AQLCoreInvalidData("Invalid copula parameter index", __FILE__, __LINE__);

    AQLString theta1ID = AQLPriceCMSObject::MatrixID("_" + AQ_COP_NAMES[0] + "_", ccy);
    double theta1 = AQLMathParameterObject::LookUpParameterMatrix(dataInstance, theta1ID, cf.fixing, pairID, "Linear");
    AQLString theta2ID = AQLPriceCMSObject::MatrixID("_" + AQ_COP_NAMES[1] + "_", ccy);
    double theta2 = AQLMathParameterObject::LookUpParameterMatrix(dataInstance, theta2ID, cf.fixing, pairID, "Linear");
    AQLString rhoID = AQLPriceCMSObject::MatrixID("_" + AQ_COP_NAMES[2] + "_", ccy);
    double rho = AQLMathParameterObject::LookUpParameterMatrix(dataInstance, rhoID, cf.fixing, pairID, "Linear");
    DoubleVector copParams(nCopParams);
    copParams[0] = theta1; copParams[1] = theta2; copParams[2] = rho;

    // Spread
    AQLPriceCopulaCMSSpread spread(confidence);
    double fwdPrice;
    if (optionType == "Cap")
        fwdPrice = spread.Price(t, strike, true, 1.0, 1.0, CMS1, CMS2, vol1, vol2, copType, copParams);
    else if (optionType == "Floor")
        fwdPrice = spread.Price(t, strike, false, 1.0, 1.0, CMS1, CMS2, vol1, vol2, copType, copParams);
    else if (optionType == "Straddle")
    {
        double cap = spread.Price(t, strike, true, 1.0, 1.0, CMS1, CMS2, vol1, vol2, copType, copParams);
        double floor = spread.Price(t, strike, false, 1.0, 1.0, CMS1, CMS2, vol1, vol2, copType, copParams);
        fwdPrice = cap + floor;
    }
    else
        throw AQLCoreInvalidData("Invalid price type", __FILE__, __LINE__);

    //// Spot premium
    //double df = AQLPriceCMSObject::DiscountFactor(discCurveInfo, valDate, cf.payment);
    //return df * fwdPrice;

    // Forward premium
    return fwdPrice;
}

//================ Tools ===================================
DoubleVector AQLPriceCMSSpreadUtility::InterpolateParameters(AQLDataInstance* dataInstance, const DateVector& dates, const AQLString& index,
                                                       const AQLString& id, const AQLString& currency)
{
    AQLString matrixID = AQLPriceCMSObject::MatrixID(id, currency);
    size_t nDates = dates.size();
    DoubleVector result(nDates);
    for (size_t i = 0; i < nDates; i++)
        result[i] = AQLMathParameterObject::LookUpParameterMatrix(dataInstance, matrixID, dates[i], index, "Linear");

    return result;
}

void AQLPriceCMSSpreadUtility::CheckInitialParameters(AQLDataInstance* dataInstance, const AQLString& ccy, AQLStringVector& expiryTerms,
                                                   AQLStringVector& indexes)
{
    AQLString theta1ID = AQLPriceCMSObject::MatrixID(AQ_THETA1_IN, ccy);
    AQLStringVector theta1Terms = AQLMathParameterObject::ParameterMatrixTerms(dataInstance, theta1ID);
    AQLStringVector theta1Indexes = AQLMathParameterObject::ParameterMatrixIndexes(dataInstance, theta1ID);

    AQLString theta2ID = AQLPriceCMSObject::MatrixID(AQ_THETA2_IN, ccy);
    AQLStringVector theta2Terms = AQLMathParameterObject::ParameterMatrixTerms(dataInstance, theta2ID);
    AQLStringVector theta2Indexes = AQLMathParameterObject::ParameterMatrixIndexes(dataInstance, theta2ID);
    CheckStringVectors(theta2Terms, theta1Terms);
    CheckStringVectors(theta2Indexes, theta1Indexes);

    AQLString rhoID = AQLPriceCMSObject::MatrixID(AQ_COPRHO_IN, ccy);
    AQLStringVector rhoTerms = AQLMathParameterObject::ParameterMatrixTerms(dataInstance, rhoID);
    AQLStringVector rhoIndexes = AQLMathParameterObject::ParameterMatrixIndexes(dataInstance, rhoID);
    CheckStringVectors(rhoTerms, theta2Terms);
    CheckStringVectors(rhoIndexes, theta2Indexes);

    expiryTerms = theta1Terms;
    indexes = theta1Indexes;
}

void AQLPriceCMSSpreadUtility::ReadSLGrid(AQLDataInstance* dataInstance, const AQLString& currency, AQLStringVector& slTerms)
{
    slTerms = AQLMathParameterObject::ParameterMatrixTerms(dataInstance, AQLPriceCMSObject::MatrixID(AQ_SL_ATM, currency));
}

void AQLPriceCMSSpreadUtility::CheckIndexes(AQLDataInstance* dataInstance, const AQLString& ccy, bool calibrate,
                                      const AQLStringVector& pairIDs, const AQLStringVector& initIDs)
{
    if (calibrate)
    {
        // Check that pairIDs and SL indexes are the same
        AQLStringVector slVec = AQLMathParameterObject::ParameterMatrixIndexes(dataInstance, AQLPriceCMSObject::MatrixID(AQ_SL_ATM, ccy));
        CheckStringVectors(slVec, pairIDs);

        // Check that all pairIDs have initial parameters
        size_t idx;
        for (size_t i = 0; i < pairIDs.size(); i++)
        {
            if (!TryFind(pairIDs[i], initIDs, idx))
                throw AQLCoreInvalidData("Target index does not have initial parameters", __FILE__, __LINE__);
        }

        // What about the case where initial parameters exist for IDs which are not in pairIDs?
        // Currently these will be left out of the calibration result. Should we include them or not?
    }
}

bool AQLPriceCMSSpreadUtility::CheckModes(const AQLStringVector& modes)
{
    bool calibrate;
    size_t nPairs = modes.size();
    for (int i = 0; i < nPairs; i++)
    {
        if (modes[i] == "Fixed")
            calibrate = false;
        else if (modes[i] == "ATM" || modes[i] == "Smile")
        {
            calibrate = true;
            break;
        }
        else
            throw AQLCoreInvalidData("Invalid calibration mode", __FILE__, __LINE__);
    }

    return calibrate;
}

void AQLPriceCMSSpreadUtility::CheckStringVectors(const AQLStringVector& terms1, const AQLStringVector& terms2)
{
    size_t n1 = terms1.size();
    if (terms2.size() != n1)
        throw AQLCoreInvalidData("Mismatched array sizes in string vector comparison", __FILE__, __LINE__);

    for (size_t i = 0; i < n1; i++)
    {
        if (terms1[i] != terms2[i])
            throw AQLCoreInvalidData("Mismatched array content in string vector comparison", __FILE__, __LINE__);
    }
}

double AQLPriceCMSSpreadUtility::CMSSpreadCoupon(double strike, bool isCall, double S1, double S2,
                                           double stDev1, double stDev2, double rho)
{
    double stDev = sqrt(stDev1 * stDev1 + stDev2 * stDev2 - 2.0 * rho * stDev1 * stDev2);
    double fwd = S1 - S2;
    return BachelierPrice(strike, isCall, fwd, stDev);
}

//================ Targets ===================================
AQLPriceCMSMLATMTarget::AQLPriceCMSMLATMTarget(AQLDataInstance* dataInstance, const AQLString& ccy, AQLDate valDate,
                                   SwapRateInfo rate1, SwapRateInfo rate2, const AQLString& pairID,
                                   const vector<CashFlowTiming>& schedule, const vector<size_t>& months,
                                   int startLength, int endLength, CurveInfo discCurveInfo,
                                   const string& copType, double confidence)
{
    mDataInstance = dataInstance;
    mValDate = valDate;
    mSchedule = schedule;
    mConfidence = confidence;
    mRate1 = rate1;
    mRate2 = rate2;
    mPairID = pairID;
    mCopType = copType;
    mMonths = months;
    mStartLength = startLength;
    mEndLength = endLength;
    mDiscCurveInfo = discCurveInfo;

    size_t nCopParams = 3;
    if (AQ_COP_NAMES.size() < nCopParams)
        throw AQLCoreInvalidData("Invalid copula parameter index", __FILE__, __LINE__);

    mTheta1ID = AQLPriceCMSObject::MatrixID("_" + AQ_COP_NAMES[0] + "_", ccy);
    mTheta2ID = AQLPriceCMSObject::MatrixID("_" + AQ_COP_NAMES[1] + "_", ccy);
    mRhoID = AQLPriceCMSObject::MatrixID("_" + AQ_COP_NAMES[2] + "_", ccy);

    size_t nFlows = mSchedule.size();
    mCopParams.clear();
    mCMS1.clear(); mVol1.clear(); mCMS2.clear(); mVol2.clear(); mDF.clear(); mT.clear(); mAcc.clear();
    for (size_t i = 0; i < nFlows; i++)
    {
        if (mMonths[i] >= mStartLength && mMonths[i] < mEndLength)
        {
            CashFlowTiming cf = mSchedule[i];

            // Calibrate marginals
            double CMS1, CMS2, vol1, vol2;
            mRate1.CMSDistribution(mValDate, cf, CMS1, vol1);
            mRate2.CMSDistribution(mValDate, cf, CMS2, vol2);

            // Copula parameters
            double theta1 = AQLMathParameterObject::LookUpParameterMatrix(mDataInstance, mTheta1ID, cf.fixing, mPairID, "Linear");
            double theta2 = AQLMathParameterObject::LookUpParameterMatrix(mDataInstance, mTheta2ID, cf.fixing, mPairID, "Linear");
            double rho = AQLMathParameterObject::LookUpParameterMatrix(mDataInstance, mRhoID, cf.fixing, mPairID, "Linear");
            DoubleVector copParams(3);
            copParams[0] = theta1; copParams[1] = theta2; copParams[2] = rho;

            double t = etrading::ModelTime(valDate, cf.fixing);
            double df = AQLPriceCMSObject::DiscountFactor(mDiscCurveInfo, mValDate, cf.payment);

            mCMS1.push_back(CMS1); mVol1.push_back(vol1); mCMS2.push_back(CMS2); mVol2.push_back(vol2);
            mCopParams.push_back(copParams); mDF.push_back(df); mT.push_back(t); mAcc.push_back(cf.accrual);
        }
    }
}

double AQLPriceCMSMLATMTarget::operator()(double x) const
{
    double call = Price(x, true);
    double put = Price(x, false);
    return fabs(call - put);
}

double AQLPriceCMSMLATMTarget::Forward()
{
    AQLPriceCopulaCMSSpread spread(mConfidence);
    size_t nEffFlows = mT.size();
    double pv = 0.0;
    for (size_t i = 0; i < nEffFlows; i++)
        pv += mDF[i] * (mCMS1[i] - mCMS2[i]) * mAcc[i];

    return pv;
}

double AQLPriceCMSMLATMTarget::Price(double strike, bool isCall) const
{
    AQLPriceCopulaCMSSpread spread(mConfidence);
    size_t nEffFlows = mT.size();
    double pv = 0.0;
    for (size_t i = 0; i < nEffFlows; i++)
    {
        DoubleVector copParams = mCopParams[i];
        double CMS1 = mCMS1[i]; double vol1 = mVol1[i]; double CMS2 = mCMS2[i]; double vol2 = mVol2[i];
        double t = mT[i];
        double epsT = 0.00001;
        double fwdPrice;
        if (isCall)
        {
            if (t < epsT)
                fwdPrice = max(CMS1 - CMS2 - strike, 0.0);
            else
                fwdPrice = spread.Price(t, strike, true, 1.0, 1.0, CMS1, CMS2, vol1, vol2, mCopType, copParams);
        }
        else
        {
            if (t < epsT)
                fwdPrice = max(strike + CMS2 - CMS1, 0.0);
            else
                fwdPrice = spread.Price(t, strike, false, 1.0, 1.0, CMS1, CMS2, vol1, vol2, mCopType, copParams);
        }

        pv += mDF[i] * fwdPrice * mAcc[i];
    }

    return pv;
}
