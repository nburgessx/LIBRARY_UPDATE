#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "LAPriceCMSSpreadTools.h"
#include "LAPriceCMSTools.h"
#include "LAFunctionUtilities.h"
#include "LAMathParameterUtility.h"
#include "LAMathOptionTools.h"
#include "LAPriceCopulaCMSSpread.h"
#include "LAMathDateUtilities.h"
#include "LAOptimumBrent.h"

//================ Pricing ===================================
double LAPriceCMSSpreadUtility::CMSSpreadMLATM(LADataInstance* dataInstance, const LAString& tenor1, const LAString& tenor2,
                                          const LAString& expiryTerm, const LAStringMatrix& pricingConfig,
                                          const LAStringMatrix& legScheduler, const LAStringMatrix& cmsScheduler,
                                          const LAString& proxySpreadID)
{
    LAPriceCMSMLATMTarget pricer = WarmUpMLPrice(dataInstance, tenor1, tenor2, expiryTerm, pricingConfig, legScheduler, cmsScheduler,
                                            proxySpreadID);

    // Set optimizer
    double init = 0.005, lwBound = -0.05, upBound = 0.05, tol = 0.0000001;
    size_t maxIter = 1000;
    LAOptimumBrent minimizer(init, lwBound, upBound, maxIter, tol);
    // Solve
    DoubleVector solution(1, init);
    minimizer.findMinimum(pricer, solution);
    return solution[0];
}

double LAPriceCMSSpreadUtility::CMSSpreadMLPrice(LADataInstance* dataInstance, const LAString& tenor1, const LAString& tenor2,
                                            const LAString& expiryTerm, double strike, const LAString& optionType,
                                            const LAStringMatrix& pricingConfig,
                                            const LAStringMatrix& legScheduler, const LAStringMatrix& cmsScheduler,
                                            const LAString& proxySpreadID)
{
    LAPriceCMSMLATMTarget pricer = WarmUpMLPrice(dataInstance, tenor1, tenor2, expiryTerm, pricingConfig, legScheduler, cmsScheduler,
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
        throw LACoreInvalidData("Invalid price type", __FILE__, __LINE__);

    return price;
}

LAPriceCMSMLATMTarget LAPriceCMSSpreadUtility::WarmUpMLPrice(LADataInstance* dataInstance, const LAString& tenor1, const LAString& tenor2,
                                                   const LAString& expiryTerm, const LAStringMatrix& pricingConfig,
                                                   const LAStringMatrix& legScheduler, const LAStringMatrix& cmsScheduler,
                                                   const LAString& proxySpreadID)
{
    SwapRateInfo rate1, rate2;
    WarmUpCMSSpread(dataInstance, tenor1, tenor2, expiryTerm, pricingConfig, legScheduler, cmsScheduler, rate1, rate2);

    // Fixed Inputs
    double confidence = 10.0;
    string copType = "PowerGaussian";

    // Other inputs
    LAString ccy = LAFunctionUtilities::findElement(pricingConfig, "Currency");
    LAString colCcy = ccy;
    LADate valDate = LAStringToDate(LAFunctionUtilities::findElement(pricingConfig, "AsOfDate"));

    // Curves
    CurveInfo discCurveInfo = LAPriceCMSObject::DiscountCurveInfo(dataInstance, ccy, colCcy);

    // Interpret forward expiry term
    size_t xPos;
    if (!TryFind(LAString("x"), expiryTerm, xPos) || xPos < 1 || xPos > expiryTerm.size() - 1)
        throw LACoreInvalidData("Invalid expiry format for Multi-Look", __FILE__, __LINE__);
    LAString startTerm = expiryTerm.subString(0, xPos - 1);
    LAString endTerm = expiryTerm.subString(xPos + 1, expiryTerm.size() - 1);

    // Cash-flow
    vector<CashFlowTiming> schedule = LAMathScheduleUtility::LegSchedule(valDate, endTerm, legScheduler, cmsScheduler);

    // Generate month vector from fixing dates
    LAString frequency = LAFunctionUtilities::findElement(legScheduler, "Frequency");
    LAString freqTerm = FrequencyToTerm(frequency);
    int monthLength = TermToMonthLength(freqTerm);
    size_t nFlows = schedule.size();
    vector<size_t> months(nFlows);
    months[0] = 0;
    for (size_t i = 1; i < nFlows; i++)
        months[i] = months[i - 1] + monthLength;

    int startLength = TermToMonthLength(startTerm);
    int endLength = TermToMonthLength(endTerm);

    return LAPriceCMSMLATMTarget(dataInstance, ccy, valDate, rate1, rate2, proxySpreadID, schedule, months, startLength, endLength,
                            discCurveInfo, copType, confidence);
}

void LAPriceCMSSpreadUtility::WarmUpCMSSpread(LADataInstance* dataInstance, const LAString& tenor1, const LAString& tenor2,
                                         const LAString& expiryTerm, const LAStringMatrix& pricingConfig,
                                         const LAStringMatrix& legScheduler, const LAStringMatrix& cmsScheduler,
                                         SwapRateInfo& rate1, SwapRateInfo& rate2)
{
    // Fixed Inputs
    double confidence = 10.0;
    string copType = "PowerGaussian";

    // Other inputs
    LAString ccy = LAFunctionUtilities::findElement(pricingConfig, "Currency");
    LAString colCcy = ccy;
    ReplicationConfig repConfig = GetReplicationConfig(pricingConfig);
    double shift = LAFunctionUtilities::findElement(pricingConfig, "Shift").getDoubleValue();
    LADate valDate = LAStringToDate(LAFunctionUtilities::findElement(pricingConfig, "AsOfDate"));

    // Curves
    CurveInfo discCurveInfo = LAPriceCMSObject::DiscountCurveInfo(dataInstance, ccy, colCcy);
    LAString cmsFloatFreq = LAFunctionUtilities::findElement(cmsScheduler, "FloatLegFrequency");
    CurveInfo cmsCurveInfo = LAPriceCMSObject::ForecastCurveInfo(dataInstance, ccy, colCcy, FrequencyToTerm(cmsFloatFreq));

    // Swap rate definitions and parameters
    rate1 = SwapRateInfo(dataInstance, ccy, tenor1, discCurveInfo, cmsCurveInfo, cmsScheduler, repConfig, shift);
    rate2 = SwapRateInfo(dataInstance, ccy, tenor2, discCurveInfo, cmsCurveInfo, cmsScheduler, repConfig, shift);
}

double LAPriceCMSSpreadUtility::CMSSpreadSLATM(LADataInstance* dataInstance, const LAString& tenor1, const LAString& tenor2,
                                          const LAString& expiryTerm, const LAStringMatrix& pricingConfig,
                                          const LAStringMatrix& legScheduler, const LAStringMatrix& cmsScheduler)
{
    SwapRateInfo rate1, rate2;
    WarmUpCMSSpread(dataInstance, tenor1, tenor2, expiryTerm, pricingConfig, legScheduler, cmsScheduler, rate1, rate2);

    LADate valDate = LAStringToDate(LAFunctionUtilities::findElement(pricingConfig, "AsOfDate"));
    CashFlowTiming cf = LAMathScheduleUtility::CashFlowSchedule(valDate, expiryTerm, legScheduler, cmsScheduler);
    cf.accrual = 0.0;

    double cms1 = rate1.CMSForward(valDate, cf);
    double cms2 = rate2.CMSForward(valDate, cf);
    return cms1 - cms2;
}

double LAPriceCMSSpreadUtility::CMSSpreadSLPrice(LADataInstance* dataInstance, const LAString& tenor1, const LAString& tenor2,
                                            const LAString& expiryTerm, double strike, const LAString& optionType,
                                            const LAStringMatrix& pricingConfig,
                                            const LAStringMatrix& legScheduler, const LAStringMatrix& cmsScheduler,
                                            const LAString& proxySpreadID)
{
    // Fixed Inputs
    double confidence = 10.0;
    string copType = "PowerGaussian";

    // Other inputs
    LAString ccy = LAFunctionUtilities::findElement(pricingConfig, "Currency");
    LAString colCcy = ccy;
    LADate valDate = LAStringToDate(LAFunctionUtilities::findElement(pricingConfig, "AsOfDate"));

    // Curves
    CurveInfo discCurveInfo = LAPriceCMSObject::DiscountCurveInfo(dataInstance, ccy, colCcy);

    SwapRateInfo rate1, rate2;
    WarmUpCMSSpread(dataInstance, tenor1, tenor2, expiryTerm, pricingConfig, legScheduler, cmsScheduler, rate1, rate2);

    // Cash-flow
    CashFlowTiming cf = LAMathScheduleUtility::CashFlowSchedule(valDate, expiryTerm, legScheduler, cmsScheduler);
    cf.accrual = 0.0;

    // Calibrate marginals
    double CMS1, CMS2, vol1, vol2;
    rate1.CMSDistribution(valDate, cf, CMS1, vol1);
    rate2.CMSDistribution(valDate, cf, CMS2, vol2);

    // Copula parameters
    double t = ModelTime(valDate, cf.fixing);
    LAString pairID = proxySpreadID;
    size_t nCopParams = 3;
    if (AQ_COP_NAMES.size() < nCopParams)
        throw LACoreInvalidData("Invalid copula parameter index", __FILE__, __LINE__);

    LAString theta1ID = LAPriceCMSObject::MatrixID("_" + AQ_COP_NAMES[0] + "_", ccy);
    double theta1 = LAMathParameterObject::LookUpParameterMatrix(dataInstance, theta1ID, cf.fixing, pairID, "Linear");
    LAString theta2ID = LAPriceCMSObject::MatrixID("_" + AQ_COP_NAMES[1] + "_", ccy);
    double theta2 = LAMathParameterObject::LookUpParameterMatrix(dataInstance, theta2ID, cf.fixing, pairID, "Linear");
    LAString rhoID = LAPriceCMSObject::MatrixID("_" + AQ_COP_NAMES[2] + "_", ccy);
    double rho = LAMathParameterObject::LookUpParameterMatrix(dataInstance, rhoID, cf.fixing, pairID, "Linear");
    DoubleVector copParams(nCopParams);
    copParams[0] = theta1; copParams[1] = theta2; copParams[2] = rho;

    // Spread
    LAPriceCopulaCMSSpread spread(confidence);
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
        throw LACoreInvalidData("Invalid price type", __FILE__, __LINE__);

    //// Spot premium
    //double df = LAPriceCMSObject::DiscountFactor(discCurveInfo, valDate, cf.payment);
    //return df * fwdPrice;

    // Forward premium
    return fwdPrice;
}

//================ Tools ===================================
DoubleVector LAPriceCMSSpreadUtility::InterpolateParameters(LADataInstance* dataInstance, const DateVector& dates, const LAString& index,
                                                       const LAString& id, const LAString& currency)
{
    LAString matrixID = LAPriceCMSObject::MatrixID(id, currency);
    size_t nDates = dates.size();
    DoubleVector result(nDates);
    for (size_t i = 0; i < nDates; i++)
        result[i] = LAMathParameterObject::LookUpParameterMatrix(dataInstance, matrixID, dates[i], index, "Linear");

    return result;
}

void LAPriceCMSSpreadUtility::CheckInitialParameters(LADataInstance* dataInstance, const LAString& ccy, LAStringVector& expiryTerms,
                                                   LAStringVector& indexes)
{
    LAString theta1ID = LAPriceCMSObject::MatrixID(AQ_THETA1_IN, ccy);
    LAStringVector theta1Terms = LAMathParameterObject::ParameterMatrixTerms(dataInstance, theta1ID);
    LAStringVector theta1Indexes = LAMathParameterObject::ParameterMatrixIndexes(dataInstance, theta1ID);

    LAString theta2ID = LAPriceCMSObject::MatrixID(AQ_THETA2_IN, ccy);
    LAStringVector theta2Terms = LAMathParameterObject::ParameterMatrixTerms(dataInstance, theta2ID);
    LAStringVector theta2Indexes = LAMathParameterObject::ParameterMatrixIndexes(dataInstance, theta2ID);
    CheckStringVectors(theta2Terms, theta1Terms);
    CheckStringVectors(theta2Indexes, theta1Indexes);

    LAString rhoID = LAPriceCMSObject::MatrixID(AQ_COPRHO_IN, ccy);
    LAStringVector rhoTerms = LAMathParameterObject::ParameterMatrixTerms(dataInstance, rhoID);
    LAStringVector rhoIndexes = LAMathParameterObject::ParameterMatrixIndexes(dataInstance, rhoID);
    CheckStringVectors(rhoTerms, theta2Terms);
    CheckStringVectors(rhoIndexes, theta2Indexes);

    expiryTerms = theta1Terms;
    indexes = theta1Indexes;
}

void LAPriceCMSSpreadUtility::ReadSLGrid(LADataInstance* dataInstance, const LAString& currency, LAStringVector& slTerms)
{
    slTerms = LAMathParameterObject::ParameterMatrixTerms(dataInstance, LAPriceCMSObject::MatrixID(AQ_SL_ATM, currency));
}

void LAPriceCMSSpreadUtility::CheckIndexes(LADataInstance* dataInstance, const LAString& ccy, bool calibrate,
                                      const LAStringVector& pairIDs, const LAStringVector& initIDs)
{
    if (calibrate)
    {
        // Check that pairIDs and SL indexes are the same
        LAStringVector slVec = LAMathParameterObject::ParameterMatrixIndexes(dataInstance, LAPriceCMSObject::MatrixID(AQ_SL_ATM, ccy));
        CheckStringVectors(slVec, pairIDs);

        // Check that all pairIDs have initial parameters
        size_t idx;
        for (size_t i = 0; i < pairIDs.size(); i++)
        {
            if (!TryFind(pairIDs[i], initIDs, idx))
                throw LACoreInvalidData("Target index does not have initial parameters", __FILE__, __LINE__);
        }

        // What about the case where initial parameters exist for IDs which are not in pairIDs?
        // Currently these will be left out of the calibration result. Should we include them or not?
    }
}

bool LAPriceCMSSpreadUtility::CheckModes(const LAStringVector& modes)
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
            throw LACoreInvalidData("Invalid calibration mode", __FILE__, __LINE__);
    }

    return calibrate;
}

void LAPriceCMSSpreadUtility::CheckStringVectors(const LAStringVector& terms1, const LAStringVector& terms2)
{
    size_t n1 = terms1.size();
    if (terms2.size() != n1)
        throw LACoreInvalidData("Mismatched array sizes in string vector comparison", __FILE__, __LINE__);

    for (size_t i = 0; i < n1; i++)
    {
        if (terms1[i] != terms2[i])
            throw LACoreInvalidData("Mismatched array content in string vector comparison", __FILE__, __LINE__);
    }
}

double LAPriceCMSSpreadUtility::CMSSpreadCoupon(double strike, bool isCall, double S1, double S2,
                                           double stDev1, double stDev2, double rho)
{
    double stDev = sqrt(stDev1 * stDev1 + stDev2 * stDev2 - 2.0 * rho * stDev1 * stDev2);
    double fwd = S1 - S2;
    return BachelierPrice(strike, isCall, fwd, stDev);
}

//================ Targets ===================================
LAPriceCMSMLATMTarget::LAPriceCMSMLATMTarget(LADataInstance* dataInstance, const LAString& ccy, LADate valDate,
                                   SwapRateInfo rate1, SwapRateInfo rate2, const LAString& pairID,
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
        throw LACoreInvalidData("Invalid copula parameter index", __FILE__, __LINE__);

    mTheta1ID = LAPriceCMSObject::MatrixID("_" + AQ_COP_NAMES[0] + "_", ccy);
    mTheta2ID = LAPriceCMSObject::MatrixID("_" + AQ_COP_NAMES[1] + "_", ccy);
    mRhoID = LAPriceCMSObject::MatrixID("_" + AQ_COP_NAMES[2] + "_", ccy);

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
            double theta1 = LAMathParameterObject::LookUpParameterMatrix(mDataInstance, mTheta1ID, cf.fixing, mPairID, "Linear");
            double theta2 = LAMathParameterObject::LookUpParameterMatrix(mDataInstance, mTheta2ID, cf.fixing, mPairID, "Linear");
            double rho = LAMathParameterObject::LookUpParameterMatrix(mDataInstance, mRhoID, cf.fixing, mPairID, "Linear");
            DoubleVector copParams(3);
            copParams[0] = theta1; copParams[1] = theta2; copParams[2] = rho;

            double t = ModelTime(valDate, cf.fixing);
            double df = LAPriceCMSObject::DiscountFactor(mDiscCurveInfo, mValDate, cf.payment);

            mCMS1.push_back(CMS1); mVol1.push_back(vol1); mCMS2.push_back(CMS2); mVol2.push_back(vol2);
            mCopParams.push_back(copParams); mDF.push_back(df); mT.push_back(t); mAcc.push_back(cf.accrual);
        }
    }
}

double LAPriceCMSMLATMTarget::operator()(double x) const
{
    double call = Price(x, true);
    double put = Price(x, false);
    return fabs(call - put);
}

double LAPriceCMSMLATMTarget::Forward()
{
    LAPriceCopulaCMSSpread spread(mConfidence);
    size_t nEffFlows = mT.size();
    double pv = 0.0;
    for (size_t i = 0; i < nEffFlows; i++)
        pv += mDF[i] * (mCMS1[i] - mCMS2[i]) * mAcc[i];

    return pv;
}

double LAPriceCMSMLATMTarget::Price(double strike, bool isCall) const
{
    LAPriceCopulaCMSSpread spread(mConfidence);
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
