#ifndef LAPriceCMSTools_h
#define LAPriceCMSTools_h

#ifdef __GNUG__
#pragma interface
#endif

#include "AQLDataInstance.h"
#include "LAMathCashFlowSchedules.h"
#include "LAMathYieldCurve.h"
#include <utility>

#ifndef AQ_SABR_NAMES
#define AQ_SABR_NAMES AQLStringVector { "Alpha", "Beta", "Nu", "Rho"}
#endif

#ifndef AQ_TAIL_NAMES
#define AQ_TAIL_NAMES AQLStringVector { "Tail1", "Tail2", "Tail3", "Tail4"}
#endif

#ifndef AQ_TAIL2_OUT
#define AQ_TAIL2_OUT "_Tail2_Out_"
#endif

#ifndef AQ_CMS_TGT_OUT
#define AQ_CMS_TGT_OUT "_CMS_Targets_Out_"
#endif

//================ Data structures ===================================
struct CurveInfo
{
    AQLDataInstance* dataInstance;
    AQLString curveID;
    AQLString curveName;
    bool isBasis;
    bool isFwdInterpolated;
};

class MVCurveWrap
{
public:
    MVCurveWrap(AQLDataInstance* dataInstance, const AQLString& curveID, const AQLString& curveName);

    double P(AQLDate date);

    AQLDate BaseDate() { return mBaseDate; }

private:
    AQLDate mBaseDate;
    AQLString mCurveName;
    LAMathYieldCurve* mpCoreCurve;
};

class LAPriceCMSObject
{
public:
    //================ Parameters ========================================
    //static DoubleMatrix GetParameterMatrix(AQLDataInstance* dataInstance, AQLDate valDate, const AQLString& currency,
    //                                       const AQLString& tenor, const AQLStringVector& mtyTerms,
    //                                       const AQLStringVector& paramNames);

    //static DoubleVector InterpolateParameterMatrix(double t, const DoubleMatrix& matrix, AQLString interpolation);

    static DoubleVector InterpolateParameterMatrix(AQLDataInstance* dataInstance, AQLDate expDate, const AQLString& tenor, const AQLStringVector& paramIDs);

    //================ Curve-based calculations ===================================
    static double DiscountFactor(CurveInfo curveInfo, AQLDate valDate, AQLDate expiry);

    static double ForwardLibor(CurveInfo curveInfo, AQLDate startDate, AQLDate endDate, AQLStringMatrix liborScheduler);

    static void CalculateFundingLeg(AQLDate valDate, const vector<CashFlowTiming>& schedule,
                                    CurveInfo discCurveInfo, CurveInfo fwdCurveInfo,
                                    AQLStringMatrix indexScheduler, double& pv, double& annuity);

    //================ Conversions ===================================
    static double TermToTau(AQLString term);

    static void ParseTenors(const AQLString& pairID, AQLString& tenor1, AQLString& tenor2);

    //================ Others ===================================
    static AQLString MatrixID(const AQLString& id, const AQLString& currency);

    static AQLString CurveID(const AQLString& ccy, const AQLString& colCcy);

    static CurveInfo DiscountCurveInfo(AQLDataInstance* dataInstance, const AQLString& ccy, const AQLString& colCcy);

    static CurveInfo ForecastCurveInfo(AQLDataInstance* dataInstance, const AQLString& ccy, const AQLString& colCcy, const AQLString& term);
};

template<typename T>
bool leq(const pair<AQLDate, T>& x, const pair<AQLDate, T>& y)
{
    return x.first < y.first;
}

#endif 
