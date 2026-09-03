#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "LAPriceCMSSpreadCalibration.h"
#include "LAPriceCMSSpreadTools.h"
#include "LAMathDateUtilities.h"
#include "LAMathDateCalculations.h"
#include "LAFunctionUtilities.h"
#include "LAMathParameterUtility.h"
#include "LAMathCashFlowSchedules.h"
#include "LAMathDefine.h"
#include "LAPriceCashFlowGenerator.h"
#include "LADataBasics.h"
#include "LAPriceCopulaCMSSpread.h"
#include "LAMathAnalyticalFormula.h"
#include "LAPriceReplication.h"
#include "LAPriceSwaptionCalculator.h"
#include "LAOptimumBrent.h"
#include "LAMathInterpolationUtilities.h"
#include "LACoreUtil.h"
#include "LAPriceCMSSpreadStrip.h"
#include "LANl2sol.h"
#include <algorithm>

//================ Calibration ===================================
LAString LAPriceCMSSpreadCalibration::Calibrate(LADataInstance* dataInstance, LAStringMatrix calibrationConfig, LAStringMatrix legScheduler, LAStringMatrix cmsScheduler,
                                           LAStringMatrix inputTypes, const LAStringVector& optionTypes, const DoubleVector& smileWeights)
{
    // Calibration config
    LADate valDate = LAStringToDate(LAFunctionUtilities::findElement(calibrationConfig, "AsOfDate"));
    LAString ccy = LAFunctionUtilities::findElement(calibrationConfig, "Currency");
    LAString colCcy = ccy;
    LAString convID = LAFunctionUtilities::findElement(calibrationConfig, "ConventionID");
    LAObject conventions = dataInstance->getObjectPool().getObject(convID, ENCHKTYPE_ISDEFINED ).get();
    LAPriceDataCalendar pCalendar = dynamic_cast<const LAPriceDataCalendar& >(conventions.getData(PRICING_DATA_FIXINGCALENDAR, ISDEFINED).get());
    LAPriceDataSlidingRule pSlidingRule = dynamic_cast<const LAPriceDataSlidingRule& >(conventions.getData(CALIBRATION_DATA_SLIDINGRULE, ISDEFINED).get());
    LADate pAsOfDate = dynamic_cast<const LADataDate& >(conventions.getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL).get()).get();
    LAString smileType = LAFunctionUtilities::findElement(calibrationConfig, "SmileType");

    // Read pairs to calibrate
    size_t nPairs = inputTypes.size();
    LAStringVector pairIDs(nPairs), modes(nPairs);
    for (int i = 0; i < nPairs; i++)
    {
        pairIDs[i] = inputTypes[i][0];
        modes[i] = inputTypes[i][1];
    }
    // Check initial parameter consistency
    LAStringVector initExpiryTerms, initIndexes;
    LAPriceCMSSpreadUtility::CheckInitialParameters(dataInstance, ccy, initExpiryTerms, initIndexes);
    // Check index consistency
    bool calibrate = LAPriceCMSSpreadUtility::CheckModes(modes);
    LAPriceCMSSpreadUtility::CheckIndexes(dataInstance, ccy, calibrate, pairIDs, initIndexes);

    //// Create time grid ////
    // Get time grid of SLs
    LAStringVector slTerms;
    LAPriceCMSSpreadUtility::ReadSLGrid(dataInstance, ccy, slTerms);

    // Merge input parameter grid and SL grid if needed
    LAStringVector gridTerms = initExpiryTerms;
    if (calibrate)
    {
        gridTerms = MergeVectors<LAString>(initExpiryTerms, slTerms);
        gridTerms = EliminateDuplicates<LAString>(gridTerms);
    }

    // Transform into dates and sort
    size_t nGrid = gridTerms.size();
    vector<pair<LADate, LAString> > grid(nGrid);
    for (size_t i = 0; i < nGrid; i++)
    {
        LAString term = gridTerms[i];
        LADate date = CalendarAdvance(valDate, term, pSlidingRule, pCalendar);
        grid[i] = make_pair(date, term);
    }
    sort(grid.begin(), grid.end(), leq<LAString>);
    // Retrieve sorted grids
    DateVector gridDates(nGrid);
    DoubleVector gridTimes(nGrid);
    gridTerms.resize(nGrid);
    for (size_t i = 0; i < nGrid; i++)
    {
        gridTerms[i] = grid[i].second;
        gridDates[i] = grid[i].first;
        gridTimes[i] = ModelTime(valDate, gridDates[i]);
    }

    // Curves and schedules
    CurveInfo discCurveInfo = LAPriceCMSObject::DiscountCurveInfo(dataInstance, ccy, colCcy);
    LAString cmsFloatFreq = LAFunctionUtilities::findElement(cmsScheduler, "FloatLegFrequency");
    CurveInfo cmsCurveInfo = LAPriceCMSObject::ForecastCurveInfo(dataInstance, ccy, colCcy, FrequencyToTerm(cmsFloatFreq));

    // Model info
    ReplicationConfig repConfig = GetReplicationConfig(calibrationConfig);
    double shift = LAFunctionUtilities::findElement(calibrationConfig, "Shift").getDoubleValue();

    // Option types (for smile calibration)
    size_t nStrikes = optionTypes.size();
    if (smileWeights.size() != nStrikes)
        throw LACoreInvalidData("Invalid smile weight size", __FILE__, __LINE__);

    vector<bool> isCalls(nStrikes);
    for (size_t i = 0; i < nStrikes; i++)
        isCalls[i] = LAFunctionUtilities::StringToBool(optionTypes[i]);

    //// Calibration ////
    DoubleMatrix theta1(nPairs, DoubleVector(nGrid)), theta2(nPairs, DoubleVector(nGrid)), rho(nPairs, DoubleVector(nGrid));
    DoubleMatrix targets(nPairs, DoubleVector(nGrid, 0.0));
    for (size_t pairIdx = 0; pairIdx < nPairs; pairIdx++)
    {
        LAString pairID = pairIDs[pairIdx];
        LAString mode = modes[pairIdx];
        LAString tenor1, tenor2;
        LAPriceCMSObject::ParseTenors(pairID, tenor1, tenor2);

        theta1[pairIdx] = LAPriceCMSSpreadUtility::InterpolateParameters(dataInstance, gridDates, pairID, MLIB_THETA1_IN, ccy);
        theta2[pairIdx] = LAPriceCMSSpreadUtility::InterpolateParameters(dataInstance, gridDates, pairID, MLIB_THETA2_IN, ccy);
        rho[pairIdx] = LAPriceCMSSpreadUtility::InterpolateParameters(dataInstance, gridDates, pairID, MLIB_COPRHO_IN, ccy);
        // Calibrate
        if (mode == "Fixed")
        {
            // Do nothing
        }
        else if (mode == "ATM" || mode == "Smile")
        {
            // SL time grid
            size_t nSLTimes = slTerms.size();
            DoubleVector slTimes(nSLTimes);
            DateVector slDates(nSLTimes);
            LAString spotLag = LAFunctionUtilities::findElement(legScheduler, "SpotLag");
            LADate startDate = CalendarAdvance(valDate, spotLag, pSlidingRule, pCalendar);
            LAPriceDataSlidingRule noChangeSlidingRule;
            noChangeSlidingRule.convertFromString("no_change");
            LAPriceDataCalendar noChangeCalendar;
            noChangeCalendar.convertFromString("");
            for (size_t timeIdx = 0; timeIdx < nSLTimes; timeIdx++)
            {
                slDates[timeIdx] = CalendarAdvance(valDate, slTerms[timeIdx], pSlidingRule, pCalendar);
                slTimes[timeIdx] = ModelTime(valDate, slDates[timeIdx]);
            }

            // Set SL-sized containers
            DoubleVector rhoSL(nSLTimes, 0.0), targetsSL(nSLTimes);

            // Interpolate parameters to SL time grid
            DoubleVector theta1Init, theta2Init, rhoInit;
            theta1Init = LAPriceCMSSpreadUtility::InterpolateParameters(dataInstance, slDates, pairID, MLIB_THETA1_IN, ccy);
            theta2Init = LAPriceCMSSpreadUtility::InterpolateParameters(dataInstance, slDates, pairID, MLIB_THETA2_IN, ccy);
            rhoInit = LAPriceCMSSpreadUtility::InterpolateParameters(dataInstance, slDates, pairID, MLIB_COPRHO_IN, ccy);
            SwapRateInfo rateInfo1(dataInstance, ccy, tenor1, discCurveInfo, cmsCurveInfo, cmsScheduler, repConfig, shift);
            SwapRateInfo rateInfo2(dataInstance, ccy, tenor2, discCurveInfo, cmsCurveInfo, cmsScheduler, repConfig, shift);

            bool atmIsCall = true;
            etrading::InterpolationEnum interpolation = etrading::LINEAR_INTERPOLATION;
            if (mode == "ATM")
            {
                // Calibrate on the SL times
                for (size_t timeIdx = 0; timeIdx < nSLTimes; timeIdx++)
                {
                    // Define target
                    CashFlowTiming cf = LAMathScheduleUtility::CashFlowSchedule(valDate, slTerms[timeIdx], legScheduler, cmsScheduler);
                    double quote = LAMathParameterObject::LookUpParameterMatrix(dataInstance, LAPriceCMSObject::MatrixID(MLIB_SL_ATM, ccy), slDates[timeIdx],
                                                                             pairID, "Linear");

                    double theta1_ = theta1Init[timeIdx];
                    double theta2_ = theta2Init[timeIdx];
                    LAPriceCMSSpreadATMTarget target(valDate, cf, discCurveInfo, rateInfo1, rateInfo2, quote, atmIsCall,
                                                theta1_, theta2_);

                    // Optimize
                    double tol = 0.000001;
                    int maxIter = 1000;
                    double init = rhoInit[timeIdx];
                    double lwBound = -0.9999;
                    double upBound = 0.9999;
                    LAOptimumBrent minimizer(init, lwBound, upBound, maxIter, tol);
                    DoubleVector solution(1, init);
                    minimizer.findMinimum(target, solution);

                    // Store solution
                    rhoSL[timeIdx] = solution[0];
                    targetsSL[timeIdx] = target(solution[0]);
                }

                // Interpolate results on SL times to total grid
                for (size_t gridIdx = 0; gridIdx < nGrid; gridIdx++)
                {
                    rho[pairIdx][gridIdx] = LAMathInterpolationUtilities::interpolate(slTimes, rhoSL, gridTimes[gridIdx], interpolation);
                    targets[pairIdx][gridIdx] = LAMathInterpolationUtilities::interpolate(slTimes, targetsSL, gridTimes[gridIdx], interpolation);
                }
            }
            else // It's Smile
            {
                DoubleMatrix smilePrices;
                DoubleVector smileStrikes;
                if (smileType == "ML") // Strip SL from ML
                {
                    LAString quoteMatrixID = LAPriceCMSObject::MatrixID(LAString(MLIB_ML_SMILE + pairID + "_"), ccy);
                    LAStringMatrix quoteMatrix = LAMathParameterObject::ParameterMatrix(dataInstance, quoteMatrixID);
                    LAPriceCMSSpreadStrip::Strip(dataInstance, convID, ccy,  valDate, pairID, legScheduler, cmsScheduler, quoteMatrix, isCalls,
                                            discCurveInfo, cmsCurveInfo, repConfig, shift, slTerms, slDates, smileStrikes, smilePrices);
                }
                else if (smileType == "SL") // Just read SL
                {
                    LAString quoteMatrixID = LAPriceCMSObject::MatrixID(LAString(MLIB_SL_SMILE + pairID + "_"), ccy);
                    LAStringMatrix quoteMatrix = LAMathParameterObject::ParameterMatrix(dataInstance, quoteMatrixID);
                    ParseSLSmile(quoteMatrix, slTerms, isCalls, smileStrikes, smilePrices);
                }
                else
                    throw LACoreInvalidData("Invalid smile type", __FILE__, __LINE__);

                // Group sl and ml prices
                DoubleVector allQuotes(1 + nStrikes), allStrikes(1 + nStrikes), allWeights(1 + nStrikes);
                vector<bool> allIsCall(1 + nStrikes);
                LAStringVector indexes(11 + nStrikes);
                indexes[0] = "T"; indexes[1] = "S1"; indexes[2] = "Vol1"; indexes[3] = "S2"; indexes[4] = "Vol2";
                indexes[5] = "Theta1"; indexes[6] = "Theta2"; indexes[7] = "Rho"; indexes[8] = "Target";
                indexes[9] = "DF"; indexes[10] = "ATM_Price";
                allWeights[0] = 1.0; // ATM
                allIsCall[0] = atmIsCall; // ATM
                for (size_t k = 0; k < nStrikes; k++)
                {
                    allStrikes[k + 1] = smileStrikes[k];
                    allWeights[k + 1] = smileWeights[k];
                    allIsCall[k + 1] = isCalls[k];
                    indexes[11 + k] = LAString(n2s(smileStrikes[k]).c_str());
                }
                LAStringMatrix outMatrix;
                DoubleMatrix dataMatrix(nSLTimes, DoubleVector(11 + nStrikes));
                LAString qID = "_AllSLQuotes_" + pairID + "_";
                LAMathParameterObject::SetMatrixAxis(qID, slTerms, indexes, outMatrix);

                // Calibrate on the SL times
                DoubleVector theta1SL(nSLTimes, 1.0), theta2SL(nSLTimes, 1.0);
                for (size_t timeIdx = 0; timeIdx < nSLTimes; timeIdx++)
                {
                    allQuotes[0] = LAMathParameterObject::LookUpParameterMatrix(dataInstance, LAPriceCMSObject::MatrixID(MLIB_SL_ATM, ccy), slDates[timeIdx],
                                                                             pairID, "Linear");
                    for (size_t k = 0; k < nStrikes; k++)
                        allQuotes[k + 1] = smilePrices[k][timeIdx];

                    CashFlowTiming cf = LAMathScheduleUtility::CashFlowSchedule(valDate, slTerms[timeIdx], legScheduler, cmsScheduler);
                    LAPriceCMSSpreadSmileTarget target(valDate, cf, discCurveInfo, rateInfo1, rateInfo2,
                                                  allQuotes, allStrikes, allIsCall, allWeights);

                    target.RetrieveCache(dataMatrix[timeIdx]);
                    for (size_t k = 0; k < nStrikes + 1; k++)
                        dataMatrix[timeIdx][10 + k] = allQuotes[k];

                    NL2SOL solver(target);
                    DoubleVector solution(3);
                    solution[0] = 1.0; solution[1] = 1.0; solution[2] = 0.8;
                    solver.tryToSolve(solution);
                    theta1SL[timeIdx] = solution[0];
                    theta2SL[timeIdx] = solution[1];
                    rhoSL[timeIdx] = solution[2];

                    dataMatrix[timeIdx][5] = solution[0];
                    dataMatrix[timeIdx][6] = solution[1];
                    dataMatrix[timeIdx][7] = solution[2];
                    DoubleVector tgt(3);
                    target(tgt, solution);
                    dataMatrix[timeIdx][8] = tgt[0];
                }

                // Output fit information
                LAMathParameterObject::SetMatrixData(dataMatrix, outMatrix);
                LAMathParameterObject::SetParameterMatrix(dataInstance, LAPriceCMSObject::MatrixID(qID, ccy), convID, outMatrix);

                // Interpolate results on SL times to total grid
                for (size_t gridIdx = 0; gridIdx < nGrid; gridIdx++)
                {
                    theta1[pairIdx][gridIdx] = LAMathInterpolationUtilities::interpolate(slTimes, theta1SL, gridTimes[gridIdx], interpolation);
                    theta2[pairIdx][gridIdx] = LAMathInterpolationUtilities::interpolate(slTimes, theta2SL, gridTimes[gridIdx], interpolation);
                    rho[pairIdx][gridIdx] = LAMathInterpolationUtilities::interpolate(slTimes, rhoSL, gridTimes[gridIdx], interpolation);
                }
            }
        }
        else
            throw LACoreInvalidData("Invalid calibration mode", __FILE__, __LINE__);
    }

    // Load results in memory
    /// \todo Put this in a helper function
    LAStringMatrix theta1Out(nGrid + 1, LAStringVector(nPairs + 1));
    LAStringMatrix theta2Out(nGrid + 1, LAStringVector(nPairs + 1));
    LAStringMatrix rhoOut(nGrid + 1, LAStringVector(nPairs + 1));
    LAStringMatrix targetsOut(nGrid + 1, LAStringVector(nPairs + 1));
    for (size_t timeIdx = 0; timeIdx < nGrid + 1; timeIdx++)
    {
        if (timeIdx == 0)
        {
            theta1Out[0][0] = MLIB_THETA1_OUT;
            theta2Out[0][0] = MLIB_THETA2_OUT;
            rhoOut[0][0] = MLIB_COPRHO_OUT;
            targetsOut[0][0] = MLIB_CMSSPRD_TGT_OUT;
            for (size_t pairIdx = 0; pairIdx < nPairs; pairIdx++)
            {
                LAString pairID = pairIDs[pairIdx];
                theta1Out[0][pairIdx + 1] = pairID;
                theta2Out[0][pairIdx + 1] = pairID;
                rhoOut[0][pairIdx + 1] = pairID;
                targetsOut[0][pairIdx + 1] = pairID;
            }
        }
        else
        {
            LAString term = gridTerms[timeIdx - 1];
            theta1Out[timeIdx][0] = term;
            theta2Out[timeIdx][0] = term;
            rhoOut[timeIdx][0] = term;
            targetsOut[timeIdx][0] = term;
            for (size_t pairIdx = 0; pairIdx < nPairs; pairIdx++)
            {
                theta1Out[timeIdx][pairIdx + 1] = LAString(n2s(theta1[pairIdx][timeIdx - 1]).c_str());
                theta2Out[timeIdx][pairIdx + 1] = LAString(n2s(theta2[pairIdx][timeIdx - 1]).c_str());
                rhoOut[timeIdx][pairIdx + 1] = LAString(n2s(rho[pairIdx][timeIdx - 1]).c_str());
                targetsOut[timeIdx][pairIdx + 1] = LAString(n2s(targets[pairIdx][timeIdx - 1]).c_str());
            }
        }
    }
    LAMathParameterObject::SetParameterMatrix(dataInstance, LAPriceCMSObject::MatrixID(theta1Out[0][0], ccy), convID, theta1Out);
    LAMathParameterObject::SetParameterMatrix(dataInstance, LAPriceCMSObject::MatrixID(theta2Out[0][0], ccy), convID, theta2Out);
    LAMathParameterObject::SetParameterMatrix(dataInstance, LAPriceCMSObject::MatrixID(rhoOut[0][0], ccy), convID, rhoOut);
    LAMathParameterObject::SetParameterMatrix(dataInstance, LAPriceCMSObject::MatrixID(targetsOut[0][0], ccy), convID, targetsOut);

    //// Output result ////
    return "Success";
}

void LAPriceCMSSpreadCalibration::ParseSLSmile(const LAStringMatrix& quoteMatrix, const LAStringVector& refTerms,
                                          const vector<bool>& isCalls,
                                          DoubleVector& smileStrikes, DoubleMatrix& smilePrices)
{
    // Expiries
    size_t nExpiries = quoteMatrix.size() - 1;
    LAStringVector expiryTerms(nExpiries);
    for (size_t i = 0; i < nExpiries; i++)
        expiryTerms[i] = quoteMatrix[i + 1][0];

    LAPriceCMSSpreadUtility::CheckStringVectors(refTerms, expiryTerms);

    // Strikes
    size_t nStrikes = quoteMatrix[0].size() - 1;
    DoubleVector strikes(nStrikes);
    for (size_t j = 0; j < nStrikes; j++)
        strikes[j] = quoteMatrix[0][j + 1].getDoubleValue() / 100.0;

    if (isCalls.size() != nStrikes)
        throw LACoreInvalidData("Inconsistent strike sizes in spread strip", __FILE__, __LINE__);

    // Return result
    smileStrikes = strikes;
    smilePrices = DoubleMatrix(nStrikes, DoubleVector(nExpiries));
    for (size_t i = 0; i < nExpiries; i++)
        for (size_t j = 0; j < nStrikes; j++)
            smilePrices[j][i] = quoteMatrix[i + 1][j + 1].getDoubleValue();
}

//================ Calibration Targets ===================================
LAPriceCMSSpreadATMTarget::LAPriceCMSSpreadATMTarget(LADate valDate, CashFlowTiming cf, CurveInfo discCurveInfo,
                                           const SwapRateInfo& swapRate1, const SwapRateInfo& swapRate2,
                                           double quote, bool isCall, double theta1, double theta2)
{
    mQuote = quote;
    mExpiry = ModelTime(valDate, cf.fixing);
    mIsCall = isCall;
    mTheta1 = theta1;
    mTheta2 = theta2;
    mCopulaType = "PowerGaussian";
    mConfidence = 10.0;
    mDfPay = LAPriceCMSObject::DiscountFactor(discCurveInfo, valDate, cf.payment);
    mSwapRate1 = swapRate1;
    mSwapRate2 = swapRate2;
    cf.accrual = 0.0;
    mSwapRate1.CMSDistribution(valDate, cf, mS1, mVol1);
    mSwapRate2.CMSDistribution(valDate, cf, mS2, mVol2);

    // Calculate spread option ATM strike
    mStrike = mS1 - mS2;
}

double LAPriceCMSSpreadATMTarget::operator()(double x) const
{
    // Set parameter
    DoubleVector params(3);
    params[0] = mTheta1;
    params[1] = mTheta2;
    params[2] = x;
    // Calculate PV
    LAPriceCopulaCMSSpread calculator(mConfidence);
    double pv = calculator.Price(mExpiry, mStrike, mIsCall, 1.0, 1.0, mS1, mS2, mVol1, mVol2, mCopulaType, params);
    pv *= mDfPay;

    // Calculate target
    return fabs(pv - mQuote);
}

LAPriceCMSSpreadSmileTarget::LAPriceCMSSpreadSmileTarget(LADate valDate, CashFlowTiming cf, CurveInfo discCurveInfo,
                                               const SwapRateInfo& swapRate1, const SwapRateInfo& swapRate2,
                                               const DoubleVector& quotes, const DoubleVector& strikes, const vector<bool>& isCall,
                                               const DoubleVector& weights)
{
    mStrikes = strikes;
    mWeights = weights;
    mIsCall = isCall;
    mQuotes = quotes;
    mSwapRate1 = swapRate1;
    mSwapRate2 = swapRate2;
    mNStrikes = strikes.size();
    if (mWeights.size() != mNStrikes || mIsCall.size() != mNStrikes || mQuotes.size() != mNStrikes)
        throw LACoreInvalidData("Invalid input size in CMS Spread Smile target", __FILE__, __LINE__);

    mWeightTotal = 0.0;
    for (size_t i = 0; i < mNStrikes; i++)
    {
        mWeights[i] = fabs(mWeights[i]);
        mWeightTotal += mWeights[i];
    }

    // Constraints
    mLwBound = DoubleVector { 0.5, 0.5, -0.9999 };
    mUpBound = DoubleVector { 1.5, 1.5, +0.9999 };

    if (mLwBound.size() != mArgSize || mUpBound.size() != mArgSize)
        throw LACoreInvalidData("Invalid constraint size in CMS Spread Smile target", __FILE__, __LINE__);

    //// Cache ////
    mExpiry = ModelTime(valDate, cf.fixing);
    mCopulaType = "PowerGaussian";
    mConfidence = 10.0;
    mDfPay = LAPriceCMSObject::DiscountFactor(discCurveInfo, valDate, cf.payment);

    // Calibration
    cf.accrual = 0.0;
    mSwapRate1.CMSDistribution(valDate, cf, mS1, mVol1);
    mSwapRate2.CMSDistribution(valDate, cf, mS2, mVol2);

    // ATM strike
    mStrikes[0] = mS1 - mS2;
    mATMStrike = mStrikes[0];
}

void LAPriceCMSSpreadSmileTarget::operator()(DoubleVector& f, const DoubleVector& x)
{
    f.resize(mArgSize);
    if (x.size() != mArgSize)
        throw LACoreInvalidData("Invalid argument size in CMS Spread Smile objective function", __FILE__, __LINE__);

    // Calculate objective
    double pv, objective = 0.0;
    for (size_t i = 0; i < mNStrikes; i++)
    {
        LAPriceCopulaCMSSpread calculator(mConfidence);
        pv = calculator.Price(mExpiry, mStrikes[i], mIsCall[i], 1.0, 1.0, mS1, mS2, mVol1, mVol2,
                              mCopulaType, x);
        pv *= mDfPay;
        objective += mWeights[i] * fabs(pv - mQuotes[i]);
    }

    f[0] = objective / mWeightTotal;
    for (size_t k = 1; k < mArgSize; k++)
        f[k] = 0.0;
}

void LAPriceCMSSpreadSmileTarget::RetrieveCache(DoubleVector& v)
{
    v[0] = mExpiry;
    v[1] = mS1;
    v[2] = mVol1;
    v[3] = mS2;
    v[4] = mVol2;
    v[9] = mDfPay;
}

bool LAPriceCMSSpreadSmileTarget::constraintsAreViolated(const DoubleVector& x)
{
    bool constraintViolated = false;
    for (size_t i = 0; i < mArgSize; i++)
    {
        if (x[i] < mLwBound[i] || x[i] > mUpBound[i])
        {
            constraintViolated = true;
            break;
        }
    }

    return constraintViolated;
}
