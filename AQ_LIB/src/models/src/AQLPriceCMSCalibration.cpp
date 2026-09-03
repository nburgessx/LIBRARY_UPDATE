#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "AQLPriceCMSCalibration.h"
#include "AQLMathDateUtilities.h"
#include "AQLFunctionUtilities.h"
#include "AQLMathInterpolationUtilities.h"
#include "AQLOptimumBrent.h"
#include "AQLMathParameterUtility.h"
#include "AQLMathSwaptionVolUtility.h"

//================ Calibration Procedure ===================================
AQLString AQLPriceCMSCalibration::Calibrate(AQLDataInstance* dataInstance, AQLStringMatrix calibrationConfig,
                                     AQLStringMatrix structLegScheduler, AQLStringMatrix cmsScheduler,
                                     AQLStringMatrix fundLegScheduler, AQLStringMatrix liborScheduler,
                                     const AQLStringVector& tenors, const AQLStringVector& expiryTerms,
                                     const DoubleMatrix& quotes)
{
    // Calibration config
    AQLDate valDate = LAStringToDate(AQLFunctionUtilities::findElement(calibrationConfig, "AsOfDate"));
    AQLString ccy = AQLFunctionUtilities::findElement(calibrationConfig, "Currency");
    AQLString colCcy = ccy;
    AQLString convID = AQLFunctionUtilities::findElement(calibrationConfig, "ConventionID");
    double lwBound = AQLFunctionUtilities::findElement(calibrationConfig, "LowerBound").getDoubleValue();
    double upBound = AQLFunctionUtilities::findElement(calibrationConfig, "UpperBound").getDoubleValue();
    double init = AQLFunctionUtilities::findElement(calibrationConfig, "InitialPoint").getDoubleValue();
    double defaultTail1 = AQLFunctionUtilities::findElement(calibrationConfig, "DefaultTail1").getDoubleValue();
    double defaultTail3 = AQLFunctionUtilities::findElement(calibrationConfig, "DefaultTail3").getDoubleValue();
    double defaultTail4 = AQLFunctionUtilities::findElement(calibrationConfig, "DefaultTail4").getDoubleValue();
    int maxIter = AQLFunctionUtilities::findElement(calibrationConfig, "MaxIterations").getIntValue();
    double tol = AQLFunctionUtilities::findElement(calibrationConfig, "Tolerance").getDoubleValue();

    // Curve info
    CurveInfo discCurveInfo = AQLPriceCMSObject::DiscountCurveInfo(dataInstance, ccy, colCcy);
    AQLString fundingFreq = AQLFunctionUtilities::findElement(fundLegScheduler, "Frequency");
    CurveInfo fundLiborCurveInfo = AQLPriceCMSObject::ForecastCurveInfo(dataInstance, ccy, colCcy, FrequencyToTerm(fundingFreq));
    AQLString cmsFloatFreq = AQLFunctionUtilities::findElement(cmsScheduler, "FloatLegFrequency");
    CurveInfo cmsCurveInfo = AQLPriceCMSObject::ForecastCurveInfo(dataInstance, ccy, colCcy, FrequencyToTerm(cmsFloatFreq));

    // Model info
    ReplicationConfig repConfig = GetReplicationConfig(calibrationConfig);
    double shift = AQLFunctionUtilities::findElement(calibrationConfig, "Shift").getDoubleValue();

    //// Calibrate ////
    // Check sizes
    size_t nTenors = tenors.size();
    size_t nExpiries = expiryTerms.size();
    if (quotes.size() != nTenors)
        throw AQLCoreInvalidData("Incompatible number of tenors and quotes", __FILE__, __LINE__);
    if (quotes[0].size() != nExpiries)
        throw AQLCoreInvalidData("Incompatible number of expiries and quotes", __FILE__, __LINE__);

    // Cache data that does not need optimization, create parameter grid
    DoubleVector tGrid(nExpiries), expGrid(nExpiries);
    vector<vector<AQLPriceCMSCalibrationTarget> > targets(nTenors, vector<AQLPriceCMSCalibrationTarget>(nExpiries));
    AQLPriceDataSlidingRule modelSlidingRule = AQLMathScheduleUtility::ModelSlidingRule();
    AQLPriceDataCalendar modelCalendar = AQLMathScheduleUtility::ModelCalendar();
    vector<SwapRateInfo*> rateInfos(nTenors);
    AQLStringVector sabrIDs(AQ_SABR_NAMES.size());
    for (size_t k = 0; k < AQ_SABR_NAMES.size(); k++)
        sabrIDs[k] = AQLPriceCMSObject::MatrixID("_" + AQ_SABR_NAMES[k] + "_", ccy);
    for (size_t j = 0; j < nTenors; j++)
    {
        AQLString tenor = tenors[j];
        // Get SABR matrix for this index
        rateInfos[j] = new SwapRateInfo(dataInstance, ccy, tenor, discCurveInfo, cmsCurveInfo, cmsScheduler, repConfig, shift);
        for (size_t i = 0; i < nExpiries; i++)
        {
            AQLString expiryTerm = expiryTerms[i];
            double quote = quotes[j][i];
            targets[j][i] = AQLPriceCMSCalibrationTarget(valDate, tenor, expiryTerm, structLegScheduler, cmsScheduler,
                                                   fundLegScheduler, liborScheduler, discCurveInfo, fundLiborCurveInfo,
                                                   cmsCurveInfo, rateInfos[j], quote, i, dataInstance, sabrIDs,
                                                   defaultTail1, defaultTail3, defaultTail4);
            if (j == 0)
            {
                tGrid[i] = targets[0][i].LastFixing();
                AQLDate expDate = CalendarAdvance(valDate, expiryTerm, modelSlidingRule, modelCalendar);
                expGrid[i] = ModelTime(valDate, expDate);
            }
        }
    }

    // Set optimizer
    AQLOptimumBrent minimizer(init, lwBound, upBound, maxIter, tol);

    // Optimize
    DoubleMatrix parameters(nExpiries, DoubleVector(nTenors)), objectives(nExpiries, DoubleVector(nTenors));
    etrading::InterpolationEnum interpolation = etrading::LINEAR_INTERPOLATION;
    for (size_t j = 0; j < nTenors; j++)
    {
        DoubleVector params(nExpiries, init);
        DoubleVector tgt(nExpiries);
        for (size_t i = 0; i < nExpiries; i++)
        {
            AQLPriceCMSCalibrationTarget target = targets[j][i];
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
            parameters[i][j] = AQLMathInterpolationUtilities::interpolate(tGrid, params, expGrid[i], interpolation);
            objectives[i][j] = tgt[i];
        }
    }

    // Load results in memory
    AQLString paramsID = AQ_TAIL2_OUT;
    AQLString targetsID = AQ_CMS_TGT_OUT;
    AQLStringVector outTerms(nExpiries);
    for (size_t i = 0; i < nExpiries; i++)
        outTerms[i] = expiryTerms[i];

    AQLStringMatrix paramsOut, targetsOut;
    AQLMathParameterObject::SetMatrixAxis(paramsID, outTerms, tenors, paramsOut);
    AQLMathParameterObject::SetMatrixAxis(targetsID, outTerms, tenors, targetsOut);
    AQLMathParameterObject::SetMatrixData(parameters, paramsOut);
    AQLMathParameterObject::SetMatrixData(objectives, targetsOut);
    AQLMathParameterObject::SetParameterMatrix(dataInstance, AQLPriceCMSObject::MatrixID(paramsID, ccy), convID, paramsOut);
    AQLMathParameterObject::SetParameterMatrix(dataInstance, AQLPriceCMSObject::MatrixID(targetsID, ccy), convID, targetsOut);

    // Release memory
    for (size_t i = 0; i < nTenors; i++)
        delete rateInfos[i];

    return "Success";
}

//================ CMSCalibrationTarget ===================================
AQLPriceCMSCalibrationTarget::AQLPriceCMSCalibrationTarget(AQLDate valDate, AQLString tenor, AQLString maturity,
                                               AQLStringMatrix structLegScheduler, AQLStringMatrix cmsScheduler,
                                               AQLStringMatrix fundLegScheduler, AQLStringMatrix liborScheduler,
                                               CurveInfo discCurveInfo, CurveInfo fundLiborCurveInfo,
                                               CurveInfo cmsCurveInfo, SwapRateInfo* rateInfo, double quote,
                                               size_t parameterIdx, AQLDataInstance* dataInstance, const AQLStringVector& paramIDs,
                                               double defaultTail1, double defaultTail3, double defaultTail4)
{
    mStructLegSchedule = AQLMathScheduleUtility::LegSchedule(valDate, maturity, structLegScheduler, cmsScheduler);
    mFundLegSchedule = AQLMathScheduleUtility::LegSchedule(valDate, maturity, fundLegScheduler, liborScheduler);
    mQuote = quote;
    mParameterIdx = parameterIdx;
    mDefaultTail1 = defaultTail1;
    mDefaultTail3 = defaultTail3;
    mDefaultTail4 = defaultTail4;
    mRateInfo = rateInfo;

    //// Cache data ////
    // Funding leg
    AQLPriceCMSObject::CalculateFundingLeg(valDate, mFundLegSchedule, discCurveInfo, fundLiborCurveInfo, liborScheduler,
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
        mDFs[i] = AQLPriceCMSObject::DiscountFactor(discCurveInfo, valDate, timing.payment);
        mFwdSwapRates[i] = mRateInfo->ForwardSwapRate(timing.indexSettlement);
        mAnnuities[i] = mRateInfo->Annuity(timing.indexSettlement);
        mTFix[i] = ModelTime(valDate, timing.fixing);
        for (size_t j = 0; j < paramIDs.size(); j++)
            mSABR[i][j] = AQLMathSwaptionVolUtility::lookUpSwapGrid(dataInstance, paramIDs[j], timing.fixing, tenor);
    }
}

double AQLPriceCMSCalibrationTarget::operator()(double x) const
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

DoubleVector AQLPriceCMSCalibrationTarget::InterpolateParameters(double t, const vector<double>& pGrid) const
{
    etrading::InterpolationEnum interpolation = etrading::LINEAR_INTERPOLATION;
    DoubleVector p(4);
    p[0] = mDefaultTail1;
    p[1] = AQLMathInterpolationUtilities::interpolate(mtGrid, pGrid, t, interpolation);
    p[2] = mDefaultTail3;
    p[3] = mDefaultTail4;
    return p;
}

double AQLPriceCMSCalibrationTarget::LastFixing()
{
    return mTFix.back();
}

DoubleVector AQLPriceCMSCalibrationTarget::UpdatedParameterGrid(double x) const
{
    DoubleVector p(mpGrid);
    p[mParameterIdx] = x;
    return p;
}

void AQLPriceCMSCalibrationTarget::SetParameterGrids(const DoubleVector& tGrid, const DoubleVector& pGrid)
{
    mtGrid = DoubleVector(tGrid);
    mpGrid = DoubleVector(pGrid);
}
