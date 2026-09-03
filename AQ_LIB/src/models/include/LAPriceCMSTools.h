#ifndef LAPriceCMSTools_h
#define LAPriceCMSTools_h

#ifdef __GNUG__
#pragma interface
#endif

#include "LADataInstance.h"
#include "LAMathCashFlowSchedules.h"
#include "LAMathYieldCurve.h"
#include <utility>

#ifndef MLIB_SABR_NAMES
#define MLIB_SABR_NAMES LAStringVector { "Alpha", "Beta", "Nu", "Rho"}
#endif

#ifndef MLIB_TAIL_NAMES
#define MLIB_TAIL_NAMES LAStringVector { "Tail1", "Tail2", "Tail3", "Tail4"}
#endif

#ifndef MLIB_TAIL2_OUT
#define MLIB_TAIL2_OUT "_Tail2_Out_"
#endif

#ifndef MLIB_CMS_TGT_OUT
#define MLIB_CMS_TGT_OUT "_CMS_Targets_Out_"
#endif

//================ Data structures ===================================
struct CurveInfo
{
    LADataInstance* dataInstance;
    LAString curveID;
    LAString curveName;
    bool isBasis;
    bool isFwdInterpolated;
};

class MVCurveWrap
{
public:
    MVCurveWrap(LADataInstance* dataInstance, const LAString& curveID, const LAString& curveName);

    double P(LADate date);

    LADate BaseDate() { return mBaseDate; }

private:
    LADate mBaseDate;
    LAString mCurveName;
    LAMathYieldCurve* mpCoreCurve;
};

class LAPriceCMSObject
{
public:
    //================ Parameters ========================================
    //static DoubleMatrix GetParameterMatrix(LADataInstance* dataInstance, LADate valDate, const LAString& currency,
    //                                       const LAString& tenor, const LAStringVector& mtyTerms,
    //                                       const LAStringVector& paramNames);

    //static DoubleVector InterpolateParameterMatrix(double t, const DoubleMatrix& matrix, LAString interpolation);

    static DoubleVector InterpolateParameterMatrix(LADataInstance* dataInstance, LADate expDate, const LAString& tenor, const LAStringVector& paramIDs);

    //================ Curve-based calculations ===================================
    static double DiscountFactor(CurveInfo curveInfo, LADate valDate, LADate expiry);

    static double ForwardLibor(CurveInfo curveInfo, LADate startDate, LADate endDate, LAStringMatrix liborScheduler);

    static void CalculateFundingLeg(LADate valDate, const vector<CashFlowTiming>& schedule,
                                    CurveInfo discCurveInfo, CurveInfo fwdCurveInfo,
                                    LAStringMatrix indexScheduler, double& pv, double& annuity);

    //================ Conversions ===================================
    static double TermToTau(LAString term);

    static void ParseTenors(const LAString& pairID, LAString& tenor1, LAString& tenor2);

    //================ Others ===================================
    static LAString MatrixID(const LAString& id, const LAString& currency);

    static LAString CurveID(const LAString& ccy, const LAString& colCcy);

    static CurveInfo DiscountCurveInfo(LADataInstance* dataInstance, const LAString& ccy, const LAString& colCcy);

    static CurveInfo ForecastCurveInfo(LADataInstance* dataInstance, const LAString& ccy, const LAString& colCcy, const LAString& term);
};

template<typename T>
bool leq(const pair<LADate, T>& x, const pair<LADate, T>& y)
{
    return x.first < y.first;
}

#endif 
