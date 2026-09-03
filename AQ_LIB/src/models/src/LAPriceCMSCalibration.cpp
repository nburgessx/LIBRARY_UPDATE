#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "LAPriceCMSCalibration.h"
#include "LAMathDateUtilities.h"
#include "LAFunctionUtilities.h"
#include "LAMathInterpolationUtilities.h"
#include "LAOptimumBrent.h"
#include "LAMathParameterUtility.h"
#include "LAMathSwaptionVolUtility.h"

//================ Calibration Procedure ===================================
LAString LAPriceCMSCalibration::Calibrate(LADataInstance* dataInstance, LAStringMatrix calibrationConfig,
                                     LAStringMatrix structLegScheduler, LAStringMatrix cmsScheduler,
                                     LAStringMatrix fundLegScheduler, LAStringMatrix liborScheduler,
                                     const LAStringVector& tenors, const LAStringVector& expiryTerms,
                                     const DoubleMatrix& quotes)
{
    // Calibration config
    LADate valDate = LAStringToDate(LAFunctionUtilities::findElement(calibrationConfig, "AsOfDate"));
    LAString ccy = LAFunctionUtilities::findElement(calibrationConfig, "Currency");
    LAString colCcy = ccy;
    LAString convID = LAFunctionUtilities::findElement(calibrationConfig, "ConventionID");
    double lwBound = LAFunctionUtilities::findElement(calibrationConfig, "LowerBound").getDoubleValue();
    double upBound = LAFunctionUtilities::findElement(calibrationConfig, "UpperBound").getDoubleValue();
    double init = LAFunctionUtilities::findElement(calibrationConfig, "InitialPoint").getDoubleValue();
    double defaultTail1 = LAFunctionUtilities::findElement(calibrationConfig, "DefaultTail1").getDoubleValue();
    double defaultTail3 = LAFunctionUtilities::findElement(calibrationConfig, "DefaultTail3").getDoubleValue();
    double defaultTail4 = LAFunctionUtilities::findElement(calibrationConfig, "DefaultTail4").getDoubleValue();
    int maxIter = LAFunctionUtilities::findElement(calibrationConfig, "MaxIterations").getIntValue();
    double tol = LAFunctionUtilities::findElement(calibrationConfig, "Tolerance").getDoubleValue();

    // Curve info
    CurveInfo discCurveInfo = LAPriceCMSObject::DiscountCurveInfo(dataInstance, ccy, colCcy);
    LAString fundingFreq = LAFunctionUtilities::findElement(fundLegScheduler, "Frequency");
    CurveInfo fundLiborCurveInfo = LAPriceCMSObject::ForecastCurveInfo(dataInstance, ccy, colCcy, FrequencyToTerm(fundingFreq));
    LAString cmsFloatFreq = LAFunctionUtilities::findElement(cmsScheduler, "FloatLegFrequency");
    CurveInfo cmsCurveInfo = LAPriceCMSObject::ForecastCurveInfo(dataInstance, ccy, colCcy, FrequencyToTerm(cmsFloatFreq));

    // Model info
    ReplicationConfig repConfig = GetReplicationConfig(calibrationConfig);
    double shift = LAFunctionUtilities::findElement(calibrationConfig, "Shift").getDoubleValue();

    //// Calibrate ////
    // Check sizes
    size_t nTenors = tenors.size();
    size_t nExpiries = expiryTerms.size();
    if (quotes.size() != nTenors)
        throw LACoreInvalidData("Incompatible number of tenors and quotes", __FILE__, __LINE__);
    if (quotes[0].size() != nExpiries)
        throw LACoreInvalidData("Incompatible number of expiries and quotes", __FILE__, __LINE__);

    // Cache data that does not need optimization, create parameter grid
    DoubleVector tGrid(nExpiries), expGrid(nExpiries);
    vector<vector<LAPriceCMSCalibrationTarget> > targets(nTenors, vector<LAPriceCMSCalibrationTarget>(nExpiries));
    LAPriceDataSlidingRule modelSlidingRule = LAMathScheduleUtility::ModelSlidingRule();
    LAPriceDataCalendar modelCalendar = LAMathScheduleUtility::ModelCalendar();
    vector<SwapRateInfo*> rateInfos(nTenors);
    LAStringVector sabrIDs(AQ_SABR_NAMES.size());
    for (size_t k = 0; k < AQ_SABR_NAMES.size(); k++)
        sabrIDs[k] = LAPriceCMSObject::MatrixID("_" + AQ_SABR_NAMES[k] + "_", ccy);
    for (size_t j = 0; j < nTenors; j++)
    {
        LAString tenor = tenors[j];
        // Get SABR matrix for this index
        rateInfos[j] = new SwapRateInfo(dataInstance, ccy, tenor, discCurveInfo, cmsCurveInfo, cmsScheduler, repConfig, shift);
        for (size_t i = 0; i < nExpiries; i++)
        {
            LAString expiryTerm = expiryTerms[i];
            double quote = quotes[j][i];
            targets[j][i] = LAPriceCMSCalibrationTarget(valDate, tenor, expiryTerm, structLegScheduler, cmsScheduler,
                                                   fundLegScheduler, liborScheduler, discCurveInfo, fundLiborCurveInfo,
                                                   cmsCurveInfo, rateInfos[j], quote, i, dataInstance, sabrIDs,
                                                   defaultTail1, defaultTail3, defaultTail4);
            if (j == 0)
            {
                tGrid[i] = targets[0][i].LastFixing();
                LADate expDate = CalendarAdvance(valDate, expiryTerm, modelSlidingRule, modelCalendar);
                expGrid[i] = ModelTime(valDate, expDate);
            }
        }
    }

    // Set optimizer
    LAOptimumBrent minimizer(init, lwBound, upBound, maxIter, tol);

    // Optimize
    DoubleMatrix parameters(nExpiries, DoubleVector(nTenors)), objectives(nExpiries, DoubleVector(nTenors));
    etrading::InterpolationEnum interpolation = etrading::LINEAR_INTERPOLATION;
    for (size_t j = 0; j < nTenors; j++)
    {
        DoubleVector params(nExpiries, init);
        DoubleVector tgt(nExpiries);
        for (size_t i = 0; i < nExpiries; i++)
        {
            LAPriceCMSCalibrationTarget target = targets[j][i];
            target.SetParameterGrids(tGrid, params);
            // Optimize
            DoubleVector solution(1, params[i]);
            minimizer.findMinimum(target, solution);
            params[i] = solution[0];
            tgt[i] = 10000.0 * target(params[i]);
        }
        for (size_t i = 0; i < nExpiries; i++)
        {
            //parameters[i][j] = params[i]; // if direct time grid
            parameters[i][j] = LAMathInterpolationUtilities::interpolate(tGrid, params, expGrid[i], interpolation);
            objectives[i][j] = tgt[i];
        }
    }

    // Load results in memory
    LAString paramsID = AQ_TAIL2_OUT;
    LAString targetsID = AQ_CMS_TGT_OUT;
    LAStringVector outTerms(nExpiries);
    for (size_t i = 0; i < nExpiries; i++)
        outTerms[i] = expiryTerms[i];

    LAStringMatrix paramsOut, targetsOut;
    LAMathParameterObject::SetMatrixAxis(paramsID, outTerms, tenors, paramsOut);
    LAMathParameterObject::SetMatrixAxis(targetsID, outTerms, tenors, targetsOut);
    LAMathParameterObject::SetMatrixData(parameters, paramsOut);
    LAMathParameterObject::SetMatrixData(objectives, targetsOut);
    LAMathParameterObject::SetParameterMatrix(dataInstance, LAPriceCMSObject::MatrixID(paramsID, ccy), convID, paramsOut);
    LAMathParameterObject::SetParameterMatrix(dataInstance, LAPriceCMSObject::MatrixID(targetsID, ccy), convID, targetsOut);

    // Release memory
    for (size_t i = 0; i < nTenors; i++)
        delete rateInfos[i];

    return "Success";
}

//================ CMSCalibrationTarget ===================================
LAPriceCMSCalibrationTarget::LAPriceCMSCalibrationTarget(LADate valDate, LAString tenor, LAString maturity,
                                               LAStringMatrix structLegScheduler, LAStringMatrix cmsScheduler,
                                               LAStringMatrix fundLegScheduler, LAStringMatrix liborScheduler,
                                               CurveInfo discCurveInfo, CurveInfo fundLiborCurveInfo,
                                               CurveInfo cmsCurveInfo, SwapRateInfo* rateInfo, double quote,
                                               size_t parameterIdx, LADataInstance* dataInstance, const LAStringVector& paramIDs,
                                               double defaultTail1, double defaultTail3, double defaultTail4)
{
    mStructLegSchedule = LAMathScheduleUtility::LegSchedule(valDate, maturity, structLegScheduler, cmsScheduler);
    mFundLegSchedule = LAMathScheduleUtility::LegSchedule(valDate, maturity, fundLegScheduler, liborScheduler);
    mQuote = quote;
    mParameterIdx = parameterIdx;
    mDefaultTail1 = defaultTail1;
    mDefaultTail3 = defaultTail3;
    mDefaultTail4 = defaultTail4;
    mRateInfo = rateInfo;

    //// Cache data ////
    // Funding leg
    LAPriceCMSObject::CalculateFundingLeg(valDate, mFundLegSchedule, discCurveInfo, fundLiborCurveInfo, liborScheduler,
                                      mFundLegPV, mFundLegAnnuity);
    // Structured leg
    mNStructFlows = mStructLegSchedule.size();
    mAccs = DoubleVector(mNStructFlows), mFwdSwapRates = DoubleVector(mNStructFlows);
    mAnnuities = DoubleVector(mNStructFlows), mDFs = DoubleVector(mNStructFlows);
    mTFix = DoubleVector(mNStructFlows);
    mSABR = DoubleMatrix(mNStructFlows, DoubleVector(paramIDs.size()));
    for (size_t i = 0; i < mNStructFlows; i++)
    {
        CashFlowTiming timing = mStructLegSchedule[i];
        mAccs[i] = timing.accrual;
        mDFs[i] = LAPriceCMSObject::DiscountFactor(discCurveInfo, valDate, timing.payment);
        mFwdSwapRates[i] = mRateInfo->ForwardSwapRate(timing.indexSettlement);
        mAnnuities[i] = mRateInfo->Annuity(timing.indexSettlement);
        mTFix[i] = ModelTime(valDate, timing.fixing);
        for (size_t j = 0; j < paramIDs.size(); j++)
            mSABR[i][j] = LAMathSwaptionVolUtility::lookUpSwapGrid(dataInstance, paramIDs[j], timing.fixing, tenor);
    }
}

double LAPriceCMSCalibrationTarget::operator()(double x) const
{
    // Set new parameters
    DoubleVector currentpGrid = UpdatedParameterGrid(x);

    // Calculate PV of structured leg
    double structLegPV = 0.0;
    for (size_t i = 0; i < mNStructFlows; i++)
    {
        double fwd = mFwdSwapRates[i];
        double annuity = mAnnuities[i];
        double t = mTFix[i];
        DoubleVector tailParameters = InterpolateParameters(t, currentpGrid);
        double cmsRate = mRateInfo->CMSForward(fwd, annuity, t, mDFs[i], mAccs[i], mSABR[i], tailParameters);
        structLegPV += mDFs[i] * mAccs[i] * cmsRate;
    }

    // Calculate trade spread
    double tradeSpread = (structLegPV - mFundLegPV) / mFundLegAnnuity;

    // Calculate target
    return fabs(tradeSpread - mQuote);
}

DoubleVector LAPriceCMSCalibrationTarget::InterpolateParameters(double t, const vector<double>& pGrid) const
{
    etrading::InterpolationEnum interpolation = etrading::LINEAR_INTERPOLATION;
    DoubleVector p(4);
    p[0] = mDefaultTail1;
    p[1] = LAMathInterpolationUtilities::interpolate(mtGrid, pGrid, t, interpolation);
    p[2] = mDefaultTail3;
    p[3] = mDefaultTail4;
    return p;
}

double LAPriceCMSCalibrationTarget::LastFixing()
{
    return mTFix.back();
}

DoubleVector LAPriceCMSCalibrationTarget::UpdatedParameterGrid(double x) const
{
    DoubleVector p(mpGrid);
    p[mParameterIdx] = x;
    return p;
}

void LAPriceCMSCalibrationTarget::SetParameterGrids(const DoubleVector& tGrid, const DoubleVector& pGrid)
{
    mtGrid = DoubleVector(tGrid);
    mpGrid = DoubleVector(pGrid);
}
