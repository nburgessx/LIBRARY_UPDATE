#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "AQLPriceCMSTools.h"
#include "AQLDateSchedule.h"
#include "AQLFunctionUtilities.h"
#include "AQLMathCurveFuncUtility.h"
#include "AQLMathInterpolationUtilities.h"
#include "AQLMathSwaptionVolUtility.h"

#include "AQLMathCurveFuncUtility.h"
#include "AQLMathDefine.h"
#include "AQLDataBasics.h"
#include "AQLDataReference.h"
#include "AQLCoreComponentManager.h"

AQLCurveWrap::AQLCurveWrap(AQLDataInstance* dataInstance, const AQLString& curveID, const AQLString& curveName)
{
    AQLString interpolation = AQLString("SPLINE");
    AQLString calendar = AQLString("TKB:LNB");
    AQLString slidingRule = AQLString("NO_CHANGE");
    AQLString daycount = AQLString("ACT/365_ISDA");

    mpCoreCurve = &AQLMathCurveFuncUtility::getYieldCurveForCurveID(dataInstance, curveID);
    mBaseDate = dynamic_cast<const AQLDataDate&> ((mpCoreCurve->getYieldData().get().get().getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL)).get()).get();

    AQLString inter =  AQLCoreComponentManager::getInterpolation(interpolation);
    AQLString dc = AQLCoreComponentManager::getDayCount(daycount);
    mpCoreCurve->setInterpolation(inter);
    mpCoreCurve->getDayCount().setDayCount(dc);
    mpCoreCurve->getSlidingRule().convertFromString(slidingRule);
    AQLMathCurveFuncUtility::setCalendarForCurveID(*mpCoreCurve, calendar);
    mCurveName = curveName;
}

double AQLCurveWrap::P(AQLDate date)
{
    mpCoreCurve->setCurveType(mCurveName);
    return mpCoreCurve->getDF(mBaseDate, date);
}

DoubleVector AQLPriceCMSObject::InterpolateParameterMatrix(AQLDataInstance* dataInstance, AQLDate expDate, const AQLString& tenor, const AQLStringVector& paramIDs)
{
    size_t nParameters = paramIDs.size();
    DoubleVector p(nParameters);
    for (size_t i = 0; i < nParameters; i++)
        p[i] = AQLMathSwaptionVolUtility::lookUpSwapGrid(dataInstance, paramIDs[i], expDate, tenor);

    return p;
}

//================ Curve-based calculations ===================================
double AQLPriceCMSObject::DiscountFactor(CurveInfo curveInfo, AQLDate valDate, AQLDate expiry)
{
    DateVector fromDateVec(1); fromDateVec[0] = valDate;
    DateVector toDateVec(1); toDateVec[0] = expiry;
    AQLString interpolation = AQLString("SPLINE");
    AQLString calendar = AQLString("TKB:LNB");
    AQLString slidingRule = AQLString("NO_CHANGE");
    AQLString daycount = AQLString("ACT/365_ISDA");
    DoubleArray a = AQLMathCurveFuncUtility::getMultiDF(fromDateVec, toDateVec, curveInfo.dataInstance, curveInfo.curveID,
                                                     daycount, slidingRule, calendar, interpolation,
                                                     curveInfo.isBasis, curveInfo.curveName);
    return a[0];
}

double AQLPriceCMSObject::ForwardLibor(CurveInfo curveInfo, AQLDate startDate, AQLDate endDate, AQLStringMatrix liborScheduler)
{
    DateVector fromDateVec(1); fromDateVec[0] = startDate;
    DateVector toDateVec(1); toDateVec[0] = endDate;
    AQLString frequency = AQLString("Simple");
    AQLString daycount = AQLFunctionUtilities::findElement(liborScheduler, "Daycount");
    AQLString slidingRule = AQLFunctionUtilities::findElement(liborScheduler, "SlidingRule");
    AQLString calendar = AQLFunctionUtilities::findElement(liborScheduler, "IndexCalendar");
    AQLString interpolation = AQLString("SPLINE");
    bool useFwdData = false;
    DoubleArray a = AQLMathCurveFuncUtility::getMultiForwardRate(fromDateVec, toDateVec, curveInfo.dataInstance, curveInfo.curveID,
                                                              frequency, daycount, slidingRule, calendar, interpolation,
                                                              curveInfo.curveName, curveInfo.isFwdInterpolated,
                                                              useFwdData);
    return a[0];
}

void AQLPriceCMSObject::CalculateFundingLeg(AQLDate valDate, const vector<CashFlowTiming>& schedule, CurveInfo discCurveInfo,
                         CurveInfo fwdCurveInfo, AQLStringMatrix indexScheduler, double& pv, double& annuity)
{
    size_t nFlows = schedule.size();
    pv = 0.0, annuity = 0.0;
    for (size_t i = 0; i < nFlows; i++)
    {
        double acc = schedule[i].accrual;
        double df = DiscountFactor(discCurveInfo, valDate, schedule[i].payment);
        double fwd = ForwardLibor(fwdCurveInfo, schedule[i].start, schedule[i].end, indexScheduler);
        pv += fwd * acc * df;
        annuity += acc * df;
    }
}

//================ Conversions ===================================
double AQLPriceCMSObject::TermToTau(AQLString term)
{
    if (term == "12M")
        return 1.0;
    else if (term == "6M")
        return 0.5;
    else if (term == "3M")
        return 0.25;
    else
        throw AQLCoreInvalidData("Unknown term in conversion to Tau", __FILE__, __LINE__);
}

void AQLPriceCMSObject::ParseTenors(const AQLString& pairID, AQLString& tenor1, AQLString& tenor2)
{
    int slashIdx = pairID.findString("/");
    if (slashIdx < 1)
        throw AQLCoreInvalidData(AQLString("Invalid spread pair: " + pairID).getCString(), __FILE__, __LINE__);
    else
    {
        tenor1 = pairID.subString(0, slashIdx - 1) + "Y";
        tenor2 = pairID.subString(slashIdx + 1, pairID.size() - 1) + "Y";
    }
}

AQLString AQLPriceCMSObject::MatrixID(const AQLString& id, const AQLString& currency)
{
    AQLString suffix = AQLString(currency.subString(0, 0) + "col");
    return AQLString(currency + id + suffix);
}

AQLString AQLPriceCMSObject::CurveID(const AQLString& ccy, const AQLString& colCcy)
{
    return AQLString(ccy + "_ConsFwdFXCurve_" + colCcy.subString(0, 0) + "col");
}

CurveInfo AQLPriceCMSObject::DiscountCurveInfo(AQLDataInstance* dataInstance, const AQLString& ccy, const AQLString& colCcy)
{
    AQLString curveID = CurveID(ccy, colCcy);
    AQLString curveName;
    if (ccy == "JPY" || ccy == "USD" || ccy == "AUD" || ccy == "EUR" || ccy == "GBP")
        curveName = AQLString(ccy + "DISCOUNT");
    else
        curveName = AQLString(ccy + "OIS");

    bool isBasis = false;
    bool isFwdInterpolated = false;
    return CurveInfo{ dataInstance, curveID, curveName, isBasis, isFwdInterpolated };
}

CurveInfo AQLPriceCMSObject::ForecastCurveInfo(AQLDataInstance* dataInstance, const AQLString& ccy, const AQLString& colCcy, const AQLString& term)
{
    AQLString curveID = CurveID(ccy, colCcy);
    AQLString fundLiborCurveName = AQLString(ccy + term + "LFORECAST");
    bool isBasis = false;
    bool isFwdInterpolated = (ccy == "AUD" || ccy == "CAD" || ccy == "EUR" || ccy == "GBP" || ccy == "HKD" ||
                              ccy == "JPY" || ccy == "MXN" || ccy == "NZD" || ccy == "RUB" || ccy == "SGD" ||
                              ccy == "USD" || ccy == "ZAR" || ccy == "CHF");
    return CurveInfo{ dataInstance, curveID, fundLiborCurveName, isBasis, isFwdInterpolated };
}
