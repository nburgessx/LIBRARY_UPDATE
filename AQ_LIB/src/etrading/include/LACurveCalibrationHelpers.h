//
// LACurveCalibrationHelpers.h  
// *** This file used to be called "LAMathCurveGenerateFuncUti1ity.h" ***
//
#pragma once

#ifdef __GNUG__
#pragma interface
#endif


#include "AQLDate.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQLDataInstance.h"
#include "AQLString.h"
#include "LACurvePricingObject.h"
#include "LATime.h"
#include "AQLPriceDataDayCount.h"

// TODO - Try to remove this file. It contains lots of #defines that clash with external binaries
#include "ConstantDeclarations.h"

class LAMathYieldCurvePro;

// Structure for Discount Factor Payment Dates and Values
struct DiscountFactorTable
{
    DoubleVector    terms_;
    DateVector      paymentDates_;
    DoubleVector    discountFactors_;
};

// Structure for Forward Rate Fixing Dates and Values
struct ForwardRateTable
{
    DoubleVector    forwardStartTerms_;
    DoubleVector    forwardEndTerms_;
    DateVector      fixingDates_;
    DoubleVector    forwardRates_;
};

namespace etrading
{

    class LACurveCalibrationHelpers
    {

        //curve
    public:
        static AQLString getCurveGenerateTime(AQLDataInstance* dataInstance, const AQLString& curveid, const AQLString& curvename);

        static void  setUpCurveFromMarket(AQLDataInstance* dataInstance, const AQLString& CurveID,
            AQLStringMatrix mgrid, AQLStringMatrix lgrid,
            AQLStringMatrix fgrid, AQLStringMatrix sgrid,
            AQLStringMatrix mdata, AQLStringMatrix ldata,
            AQLStringMatrix fdata, AQLStringMatrix sdata,
            AQLStringMatrix ylddata, bool isCheckDF = false, bool isAUDadjust = false);

        static void setUpXccyBasisCurve(AQLDataInstance* dataInstance, const AQLString& BCurveID, AQLStringMatrix ylddata,
            AQLStringMatrix bgrid, AQLStringMatrix bdata);

        static void setUpBasisCurve(AQLDataInstance* dataInstance,
            const AQLString& stdCurveID,
            const AQLString& basisCurveID,
            AQLStringMatrix ylddata,
            AQLStringMatrix bgrid,
            AQLStringMatrix bdata);

        static void setUpBasisCurveFromMarket(AQLDataInstance* dataInstance, const AQLString& CurveID,
            AQLStringMatrix mgrid, AQLStringMatrix lgrid, AQLStringMatrix fgrid,
            AQLStringMatrix sgrid, AQLStringMatrix mdata, AQLStringMatrix ldata,
            AQLStringMatrix fdata, AQLStringMatrix sdata, AQLStringMatrix ylddata,
            AQLStringMatrix bgrid, AQLStringMatrix bdata, bool isCheckDF = false, bool isAUDadjust = false);

        static void	setUpCurve2(AQLDataInstance* dataInstance, const AQLString& curveid, const AQLDate& asofdate, const DoubleMatrix& mat,
            const AQLString& curveName);

        static void	setUpCurve(AQLDataInstance* dataInstance, const AQLString& curveid, const AQLDate& asofdate, const DoubleMatrix& mat,
            const AQLString& curveName);

        static void	setUpCurve3(AQLDataInstance* dataInstance, const AQLString& curveid, const AQLDate& asofdate, const DoubleMatrix& mat, const AQLStringMatrix& conv,
            const AQLString& curveName);

        static void setUpCurve(AQLDataInstance* dataInstance, const AQLString& curveid, const AQLDate& asofdate,
            DoubleArray& termarray, DoubleArray& dfarray, const AQLString& curveName);

        static void setUpArbFreeCurveFromMarket(AQLDataInstance* dataInstance, const AQLString& CurveID,
            AQLStringMatrix monGrid, AQLStringMatrix monConv, //money market
            AQLStringMatrix libGrid, AQLStringMatrix libConv, //libor market
            AQLStringMatrix swapGrid, AQLStringMatrix swapConv, //swap market
            AQLStringMatrix currGrid, AQLStringMatrix currConv, //currency basis swap market
            AQLStringMatrix libBasisGrid, AQLStringMatrix libBasisConv, //3M6M libor basis swap market
            AQLStringMatrix fraGrid3M, AQLStringMatrix fraGrid6M, AQLStringMatrix fraConv, //FRA market
            AQLStringMatrix yldData, AQLString& warning);

        static void setUpForecastCurveFromMarket(AQLDataInstance* dataInstance,
            const AQLString& forecastCurveID,
            AQLStringMatrix bgrid,
            AQLStringMatrix bdata);

        static void setUpForecastCurve(AQLDataInstance* dataInstance,
            const AQLString& arbFreeCurveID,
            const AQLString& forecastCurveID,
            const AQLDate& asofdate,
            const DoubleMatrix& ratesArray);

        static void setUpCdtDFCurve(AQLDataInstance* dataInstance, AQLString curveID, AQLStringMatrix data);

        static DoubleArray outPutCurveFromYieldData(AQLDataInstance* dataInstance, const AQLString& curveid, const AQLString& curveName = STD);

        static DoubleArray outPutCurveFromYieldData2(AQLDataInstance* dataInstance, const AQLString& curveidID, const AQLString& curveidID2,
            const AQLString& curveName = STD, const AQLString& curveName2 = STD);

        /* @brief			Function to get the discount factors from a curve returning a discountFactorTable structure containing dates, terms and discount factors
        *  @param [in]		dataInstance		            The object pool dataInstance std::map
        *  @param [in]		curveid		            The curveCollection to use when accessing the curveIndices
        *  @param [in]		curveName               The curveindex within the curve
        *  @param [out]     DiscountFactorTable     The discount factor table results
        */
        static DiscountFactorTable getCurveDiscountFactorTable(AQLDataInstance* dataInstance, const AQLString& curveid, const AQLString& curveName);

        /* @brief			Function to set the discount factors on a curve
        *  @param [in]		dataInstance		            The object pool dataInstance std::map
        *  @param [in]		curveid		            The curveCollection to use when accessing the curveIndices
        *  @param [in]		curveNames              A vector of curveindex names to set within the curve
        *  @param [in]      discountFactorTable     The discount factor table inputs
        */
        static void setCurveDiscountFactorTable(AQLDataInstance* dataInstance, const AQLString& curveid, const std::vector<AQLString>& curveNames, const DiscountFactorTable& discountFactorTable);

        /* @brief			Function to get the forward rates from a curve returning a forwardRateTable structure containing dates, terms and forward rates
        *  @param [in]		dataInstance		            The object pool dataInstance std::map
        *  @param [in]		curveid		            The curveCollection to use when accessing the curveIndices
        *  @param [in]		curveName               The curveindex within the curve
        *  @param [out]     ForwardRateTable        The forward rate table results
        */
        static ForwardRateTable getCurveForwardRateTable(AQLDataInstance* dataInstance, const AQLString& curveid, const AQLString& curveName);

        /* @brief			Function to set the forward rates on a curve
        *  @param [in]		dataInstance		            The object pool dataInstance std::map
        *  @param [in]		curveid		            The curveCollection to use when accessing the curveIndices
        *  @param [in]		curveNames              A vector of curveindex names to set within the curve
        *  @param [in]      ForwardRateTable        The forward rate table inputs
        */
        static void setCurveForwardRateTable(AQLDataInstance* dataInstance, const AQLString& curveid, const std::vector<AQLString>& curveNames, const ForwardRateTable& forwardRateTable);

        // reset marketdata use libor
        static void resetMarketDataUseL(LAMathYieldCurvePro &curve, const AQLString &ccy, AQLStringMatrix ldata);
        // restor swaprate froml
        static void restoreSwapRateFromLibor(LAMathYieldCurvePro &curve, const AQLString &ccy, const std::map<AQLString, double> &sRateMap, AQLStringMatrix sdata);

    private:
        LACurveCalibrationHelpers(void);
        ~LACurveCalibrationHelpers(void);
        LACurveCalibrationHelpers(const LACurveCalibrationHelpers &rhs);
        LACurveCalibrationHelpers &operator=(const LACurveCalibrationHelpers &rhs);
    };

}
