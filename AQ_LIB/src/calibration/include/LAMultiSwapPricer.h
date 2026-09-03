
#ifndef LAMultiSwapPricer_h
#define LAMultiSwapPricer_h

#ifdef __GNUG__
#pragma interface
#endif

#if defined (WIN32) || defined (WIN64)
#include <windows.h>
#else
#include <pthread.h>
#endif


#include "LADate.h"
#include "LACoreAppError.h"
#include "LACoreTemplateType.h"
#include "LADataInstance.h"
#include "LAString.h"

class LAMathObjectValue;

class LAMultiSwapPricer
{
public:
	
	static DoubleArray convergentPlainVanillaTrade(LADataInstance* dataInstance, std::map<LAString, LAString> maptrade);
	
	static void calcPlainVanillaTrades(LADataInstance* dataInstance, std::map<LAString, LAString> maptrade, DoubleVector& pvResultVec);

	static void calcPlainVanillaTrades(LADataInstance* dataInstance, std::map<LAString, LAString> maptrade, const LAStringVector& calcTargetVector, 
									   DoubleMatrix& pvResultMatrix);
	
	static void setUpPreparetionForcalcTrades(LADataInstance* dataInstance, std::map<LAString, LAString> mapcalcinfo);

	static void setUpStoreBasisCurve(LADataInstance* dataInstance,
                                const LAString& curveID, 
                                const LAString& basisCurveName, 
                                const LAStringMatrix& basisMkt, 
                                const LAStringMatrix& basisConv,
								const LAStringMatrix& generateProp,
								const LAStringMatrix& moneyConv);

	static void setUpStoreSwapCurve(LADataInstance* dataInstance,
								 const LAString& curveID,
								 const LAString& genCurveName,
								 const LAStringMatrix& generateProp, 
								 const LAStringMatrix& moneyConv,
								 const LAStringMatrix& liborRates, 
								 const LAStringMatrix& liborConv,
								 const LAStringMatrix& swapRates, 
								 const LAStringMatrix& swapConv,
								 const LAStringMatrix& fra3mRates,
								 const LAStringMatrix& fra6mRates,
								 const LAStringMatrix& fraConv,
								 const LAStringMatrix& futureRates, 
								 const LAStringMatrix& futureConv,
								 const LAStringMatrix& adjustSwapConv,
								 const LAStringMatrix& adjustSwapRates);

	static void setUpStoreOISCurve(LADataInstance* dataInstance,
								const LAString& curveID,
								const LAString& genCurveName,
								const LAStringMatrix& generateProp, 
								const LAStringMatrix& oisRates, 
								const LAStringMatrix& oisConv,
								const LAStringMatrix& histRates,
								const LAStringMatrix& lobasisRates = LAStringMatrix(), 
								const LAStringMatrix& lobasisConv = LAStringMatrix(), 
								const LAStringMatrix& swapRates = LAStringMatrix(), 
								const LAStringMatrix& swapConv = LAStringMatrix());

	static void shiftMarketRate(LAStringMatrix& rateMat, double shiftval, bool ispara = true, unsigned int shiftpos = 0);

	static LAMathObjectValue* createSingleTrade(LADataInstance* dataInstance, std::map<LAString, LAString> maptrade, LAString orgtradeid, bool isduplicatemode = false, LAString key_suffix = "", bool istypicalpardeal = false);
	
	static void createOfferBidAdjustEntity(LADataInstance* dataInstance, std::map<LAString, LAString> maptrade, LAStringVector orgtradevec, bool isduplicatemode = false, LAString key_suffix = "");


	static void calcRiskAndPV(LADataInstance* dataInstance, const LAStringVector& tradeVec, DoubleVector& pvResultVec);

	static void createSourceDeltaRiskEntity(LADataInstance* dataInstance, LAString scenarioname, LAStringVector riskDetail1sordelta, LAStringVector riskDetail2sordelta, bool isbumpgridauto, LAString paraName = "");
	
	static void createZeroDeltaRiskEntity(LADataInstance* dataInstance, LAString scenarioname, LAStringVector riskDetail1zerodelta, LAStringVector riskDetail2zerodelta, bool isbumpgridauto, LAString paraName = "");

	static void calcPortfolioRiskAndPV(LADataInstance* dataInstance, std::map<LAString, LAString> mapriskinfo);

	static double getPortfolioRiskResult(LADataInstance* dataInstance, LAString marketkey, LAString marketgrid, LAString risktype = "SOURCEBUMP");

	static void storePastRates(LADataInstance* dataInstance, const LAStringMatrix& IndexInfo, const LAStringMatrix& PastRates);

	static void setUpFixingRateFromPastRates(LADataInstance* dataInstance, LAString copytradeid, const LADate& asOfDate);

	static LAString setUpMarketParamsAndGlobalShift(LADataInstance* dataInstance, std::map<LAString, LAString> mapcalcinfo, LAString curveID);

	static void setUpLineParams(LADataInstance* dataInstance, std::map<LAString, LAString> mapcalcinfo);

	static LAString getDiscountCurveName(LADataInstance* dataInstance, LAString curveID);

	static LAString getCurveIDfromTradeReference(LADataInstance* dataInstance, LAString tradeID);

	static LAString getMarketParamfromTradeReference(LADataInstance* dataInstance, LAString tradeID);

	static LAStringVector getCurveNamesFromSetUpOrder(LADataInstance* dataInstance, LAString curveID);

	static void createShiftCurves(LADataInstance* dataInstance, LAObject& eorgCurve, LAString targetMarketName, LAString newCurveID, bool isshiftarget, bool ispara, 
											double shiftval, unsigned int shiftpos, LAString market, bool isglobalscenario);

};
#endif