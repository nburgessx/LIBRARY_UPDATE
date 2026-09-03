#ifndef LAMathCurveGenerateFuncUtility_h
#define LAMathCurveGenerateFuncUtility_h

#ifdef __GNUG__
#pragma interface
#endif

#include "LADate.h"
#include "LACoreAppError.h"
#include "LACoreTemplateType.h"
#include "LADataInstance.h"
#include "LAString.h"
#include "LAMathYieldCurve.h"
#include "ConstantDeclarations.h"
#include "LATime.h"
//fu
#include "LAPriceDataDayCount.h"

//////////////////////////

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

class LAMathCurveGenerateFuncUtility
{
//curve
public:	
	static LAString getCurveGenerateTime(LADataInstance* dataInstance, const LAString& curveid, const LAString& curvename);

	static void  setUpCurveFromMarket(LADataInstance* dataInstance,const LAString& CurveID, 
						LAStringMatrix mgrid, LAStringMatrix lgrid, 
						LAStringMatrix fgrid, LAStringMatrix sgrid,
						LAStringMatrix mdata, LAStringMatrix ldata, 
						LAStringMatrix fdata, LAStringMatrix sdata,
						LAStringMatrix ylddata, bool isCheckDF = false, bool isAUDadjust = false);

	static void setUpXccyBasisCurve(LADataInstance* dataInstance,const LAString& BCurveID, LAStringMatrix ylddata, 
						LAStringMatrix bgrid, LAStringMatrix bdata);

    static void setUpBasisCurve(LADataInstance* dataInstance,
                                const LAString& stdCurveID, 
                                const LAString& basisCurveID, 
                                LAStringMatrix ylddata, 
                                LAStringMatrix bgrid, 
                                LAStringMatrix bdata);

	static void setUpBasisCurveFromMarket(LADataInstance* dataInstance,const LAString& CurveID, 
						LAStringMatrix mgrid, LAStringMatrix lgrid, LAStringMatrix fgrid, 
						LAStringMatrix sgrid,LAStringMatrix mdata, LAStringMatrix ldata, 
						LAStringMatrix fdata, LAStringMatrix sdata,LAStringMatrix ylddata,
						LAStringMatrix bgrid, LAStringMatrix bdata, bool isCheckDF = false, bool isAUDadjust = false);
	
	static void	setUpCurve2(LADataInstance* dataInstance, const LAString& curveid, const LADate& asofdate, const DoubleMatrix& mat, 
							const LAString& curveName);
	static void	setUpCurve(LADataInstance* dataInstance, const LAString& curveid, const LADate& asofdate, const DoubleMatrix& mat, 
						   const LAString& curveName);
	//furuytos20120813//
	static void	setUpCurve3(LADataInstance* dataInstance, const LAString& curveid, const LADate& asofdate, const DoubleMatrix& mat, const LAStringMatrix& conv, 
						   const LAString& curveName);
	/////////////////
	static void setUpCurve(LADataInstance* dataInstance, const LAString& curveid, const LADate& asofdate,
						   DoubleArray& termarray, DoubleArray& dfarray, const LAString& curveName);

    static void setUpArbFreeCurveFromMarket(LADataInstance* dataInstance, const LAString& CurveID, 
                                            LAStringMatrix monGrid, LAStringMatrix monConv, //money market
                                            LAStringMatrix libGrid, LAStringMatrix libConv, //libor market
                                            LAStringMatrix swapGrid, LAStringMatrix swapConv, //swap market
                                            LAStringMatrix currGrid, LAStringMatrix currConv, //currency basis swap market
                                            LAStringMatrix libBasisGrid, LAStringMatrix libBasisConv, //3M6M libor basis swap market
                                            LAStringMatrix fraGrid3M, LAStringMatrix fraGrid6M, LAStringMatrix fraConv, //FRA market
                                            LAStringMatrix yldData, LAString& warning);

    static void setUpForecastCurveFromMarket(LADataInstance* dataInstance,
                                             const LAString& forecastCurveID, 
                                             LAStringMatrix bgrid, 
                                             LAStringMatrix bdata);

    static void setUpForecastCurve(LADataInstance* dataInstance,
                                   const LAString& arbFreeCurveID, 
                                   const LAString& forecastCurveID, 
                                   const LADate& asofdate,
                                   const DoubleMatrix& ratesArray);

    static void setUpCdtDFCurve(LADataInstance* dataInstance, LAString curveID, LAStringMatrix data);

    static DoubleArray outPutCurveFromYieldData(LADataInstance* dataInstance, const LAString& curveid, const LAString& curveName = STD);
    
    static DoubleArray outPutCurveFromYieldData2(LADataInstance* dataInstance, const LAString& curveidID, const LAString& curveidID2, 
												 const LAString& curveName = STD, const LAString& curveName2 = STD);

    /* @brief			Function to get the discount factors from a curve returning a discountFactorTable structure containing dates, terms and discount factors
	*  @param [in]		dataInstance		            The object pool dataInstance std::map
    *  @param [in]		curveid		            The curveCollection to use when accessing the curveIndices
    *  @param [in]		curveName               The curveindex within the curve
    *  @param [out]     DiscountFactorTable     The discount factor table results
	*/
    static DiscountFactorTable getCurveDiscountFactorTable( LADataInstance* dataInstance, const LAString& curveid, const LAString& curveName );

    /* @brief			Function to set the discount factors on a curve
	*  @param [in]		dataInstance		            The object pool dataInstance std::map
    *  @param [in]		curveid		            The curveCollection to use when accessing the curveIndices
    *  @param [in]		curveNames              A vector of curveindex names to set within the curve
    *  @param [in]      discountFactorTable     The discount factor table inputs
	*/
    static void setCurveDiscountFactorTable( LADataInstance* dataInstance, const LAString& curveid, const std::vector<LAString>& curveNames, const DiscountFactorTable& discountFactorTable );

    /* @brief			Function to get the forward rates from a curve returning a forwardRateTable structure containing dates, terms and forward rates
	*  @param [in]		dataInstance		            The object pool dataInstance std::map
    *  @param [in]		curveid		            The curveCollection to use when accessing the curveIndices
    *  @param [in]		curveName               The curveindex within the curve
    *  @param [out]     ForwardRateTable        The forward rate table results
	*/
    static ForwardRateTable getCurveForwardRateTable( LADataInstance* dataInstance, const LAString& curveid, const LAString& curveName );

    /* @brief			Function to set the forward rates on a curve
    *  @param [in]		dataInstance		            The object pool dataInstance std::map
    *  @param [in]		curveid		            The curveCollection to use when accessing the curveIndices
    *  @param [in]		curveNames              A vector of curveindex names to set within the curve
    *  @param [in]      ForwardRateTable        The forward rate table inputs
    */
    static void setCurveForwardRateTable( LADataInstance* dataInstance, const LAString& curveid, const std::vector<LAString>& curveNames, const ForwardRateTable& forwardRateTable );

	// reset marketdata use libor
	static void resetMarketDataUseL(LAMathYieldCurvePro &curve, const LAString &ccy, LAStringMatrix ldata);
	// restor swaprate froml
	static void restoreSwapRateFromL(LAMathYieldCurvePro &curve, const LAString &ccy, const std::map<LAString, double> &sRateMap, LAStringMatrix sdata);
	
private:
    LAMathCurveGenerateFuncUtility(void);
    ~LAMathCurveGenerateFuncUtility(void);
    LAMathCurveGenerateFuncUtility(const LAMathCurveGenerateFuncUtility &rhs);
    LAMathCurveGenerateFuncUtility &operator=(const LAMathCurveGenerateFuncUtility &rhs);



};

#endif
