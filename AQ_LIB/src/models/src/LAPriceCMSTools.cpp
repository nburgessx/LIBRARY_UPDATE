#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "LAPriceCMSTools.h"
#include "LAMathDateUtilities.h"
#include "LAFunctionUtilities.h"
#include "LAMathCurveFuncUtility.h"
#include "LAMathInterpolationUtilities.h"
#include "LAMathSwaptionVolUtility.h"

#include "LAMathCurveFuncUtility.h"
#include "LAMathDefine.h"
#include "LADataBasics.h"
#include "LADataReference.h"
#include "LACoreComponentManager.h"

MVCurveWrap::MVCurveWrap(LADataInstance* dataInstance, const LAString& curveID, const LAString& curveName)
{
    LAString interpolation = LAString("SPLINE");
    LAString calendar = LAString("TKB:LNB");
    LAString slidingRule = LAString("NO_CHANGE");
    LAString daycount = LAString("ACT/365_ISDA");

    mpCoreCurve = &LAMathCurveFuncUtility::getYieldCurveForCurveID(dataInstance, curveID);
    mBaseDate = dynamic_cast<const LADataDate&> ((mpCoreCurve->getYieldData().get().get().getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL)).get()).get();

    LAString inter =  LACoreComponentManager::getInterpolation(interpolation);
    LAString dc = LACoreComponentManager::getDayCount(daycount);
    mpCoreCurve->setInterpolation(inter);
    mpCoreCurve->getDayCount().setDayCount(dc);
    mpCoreCurve->getSlidingRule().convertFromString(slidingRule);
    LAMathCurveFuncUtility::setCalendarForCurveID(*mpCoreCurve, calendar);
    mCurveName = curveName;
}

double MVCurveWrap::P(LADate date)
{
    mpCoreCurve->setCurveType(mCurveName);
    return mpCoreCurve->getDF(mBaseDate, date);
}

DoubleVector LAPriceCMSObject::InterpolateParameterMatrix(LADataInstance* dataInstance, LADate expDate, const LAString& tenor, const LAStringVector& paramIDs)
{
    size_t nParameters = paramIDs.size();
    DoubleVector p(nParameters);
    for (size_t i = 0; i < nParameters; i++)
        p[i] = LAMathSwaptionVolUtility::lookUpSwapGrid(dataInstance, paramIDs[i], expDate, tenor);

    return p;
}

//================ Curve-based calculations ===================================
double LAPriceCMSObject::DiscountFactor(CurveInfo curveInfo, LADate valDate, LADate expiry)
{
    DateVector fromDateVec(1); fromDateVec[0] = valDate;
    DateVector toDateVec(1); toDateVec[0] = expiry;
    LAString interpolation = LAString("SPLINE");
    LAString calendar = LAString("TKB:LNB");
    LAString slidingRule = LAString("NO_CHANGE");
    LAString daycount = LAString("ACT/365_ISDA");
    DoubleArray a = LAMathCurveFuncUtility::getMultiDF(fromDateVec, toDateVec, curveInfo.dataInstance, curveInfo.curveID,
                                                     daycount, slidingRule, calendar, interpolation,
                                                     curveInfo.isBasis, curveInfo.curveName);
    return a[0];
}

double LAPriceCMSObject::ForwardLibor(CurveInfo curveInfo, LADate startDate, LADate endDate, LAStringMatrix liborScheduler)
{
    DateVector fromDateVec(1); fromDateVec[0] = startDate;
    DateVector toDateVec(1); toDateVec[0] = endDate;
    LAString frequency = LAString("Simple");
    LAString daycount = LAFunctionUtilities::findElement(liborScheduler, "Daycount");
    LAString slidingRule = LAFunctionUtilities::findElement(liborScheduler, "SlidingRule");
    LAString calendar = LAFunctionUtilities::findElement(liborScheduler, "IndexCalendar");
    LAString interpolation = LAString("SPLINE");
    bool useFwdData = false;
    DoubleArray a = LAMathCurveFuncUtility::getMultiForwardRate(fromDateVec, toDateVec, curveInfo.dataInstance, curveInfo.curveID,
                                                              frequency, daycount, slidingRule, calendar, interpolation,
                                                              curveInfo.curveName, curveInfo.isFwdInterpolated,
                                                              useFwdData);
    return a[0];
}

void LAPriceCMSObject::CalculateFundingLeg(LADate valDate, const vector<CashFlowTiming>& schedule, CurveInfo discCurveInfo,
                         CurveInfo fwdCurveInfo, LAStringMatrix indexScheduler, double& pv, double& annuity)
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
double LAPriceCMSObject::TermToTau(LAString term)
{
    if (term == "12M")
        return 1.0;
    else if (term == "6M")
        return 0.5;
    else if (term == "3M")
        return 0.25;
    else
        throw LACoreInvalidData("Unknown term in conversion to Tau", __FILE__, __LINE__);
}

void LAPriceCMSObject::ParseTenors(const LAString& pairID, LAString& tenor1, LAString& tenor2)
{
    int slashIdx = pairID.findString("/");
    if (slashIdx < 1)
        throw LACoreInvalidData(LAString("Invalid spread pair: " + pairID).getCString(), __FILE__, __LINE__);
    else
    {
        tenor1 = pairID.subString(0, slashIdx - 1) + "Y";
        tenor2 = pairID.subString(slashIdx + 1, pairID.size() - 1) + "Y";
    }
}

LAString LAPriceCMSObject::MatrixID(const LAString& id, const LAString& currency)
{
    LAString suffix = LAString(currency.subString(0, 0) + "col");
    return LAString(currency + id + suffix);
}

LAString LAPriceCMSObject::CurveID(const LAString& ccy, const LAString& colCcy)
{
    return LAString(ccy + "_ConsFwdFXCurve_" + colCcy.subString(0, 0) + "col");
}

CurveInfo LAPriceCMSObject::DiscountCurveInfo(LADataInstance* dataInstance, const LAString& ccy, const LAString& colCcy)
{
    LAString curveID = CurveID(ccy, colCcy);
    LAString curveName;
    if (ccy == "JPY" || ccy == "USD" || ccy == "AUD" || ccy == "EUR" || ccy == "GBP")
        curveName = LAString(ccy + "DISCOUNT");
    else
        curveName = LAString(ccy + "OIS");

    bool isBasis = false;
    bool isFwdInterpolated = false;
    return CurveInfo{ dataInstance, curveID, curveName, isBasis, isFwdInterpolated };
}

CurveInfo LAPriceCMSObject::ForecastCurveInfo(LADataInstance* dataInstance, const LAString& ccy, const LAString& colCcy, const LAString& term)
{
    LAString curveID = CurveID(ccy, colCcy);
    LAString fundLiborCurveName = LAString(ccy + term + "LFORECAST");
    bool isBasis = false;
    bool isFwdInterpolated = (ccy == "AUD" || ccy == "CAD" || ccy == "EUR" || ccy == "GBP" || ccy == "HKD" ||
                              ccy == "JPY" || ccy == "MXN" || ccy == "NZD" || ccy == "RUB" || ccy == "SGD" ||
                              ccy == "USD" || ccy == "ZAR" || ccy == "CHF");
    return CurveInfo{ dataInstance, curveID, fundLiborCurveName, isBasis, isFwdInterpolated };
}
