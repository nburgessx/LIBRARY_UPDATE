
#ifndef AQLMultiSwapPricer_h
#define AQLMultiSwapPricer_h

#ifdef __GNUG__
#pragma interface
#endif

#if defined (WIN32) || defined (WIN64)
#include <windows.h>
#else
#include <pthread.h>
#endif


#include "AQLDate.h"
#include "AQLCoreAppError.h"
#include "AQLCoreTemplateType.h"
#include "AQLDataInstance.h"
#include "AQLString.h"

class AQLMathObjectValue;

class AQLMultiSwapPricer
{
public:
	
	static DoubleArray convergentPlainVanillaTrade(AQLDataInstance* dataInstance, std::map<AQLString, AQLString> maptrade);
	
	static void calcPlainVanillaTrades(AQLDataInstance* dataInstance, std::map<AQLString, AQLString> maptrade, DoubleVector& pvResultVec);

	static void calcPlainVanillaTrades(AQLDataInstance* dataInstance, std::map<AQLString, AQLString> maptrade, const AQLStringVector& calcTargetVector, 
									   DoubleMatrix& pvResultMatrix);
	
	static void setUpPreparetionForcalcTrades(AQLDataInstance* dataInstance, std::map<AQLString, AQLString> mapcalcinfo);

	static void setUpStoreBasisCurve(AQLDataInstance* dataInstance,
                                const AQLString& curveID, 
                                const AQLString& basisCurveName, 
                                const AQLStringMatrix& basisMkt, 
                                const AQLStringMatrix& basisConv,
								const AQLStringMatrix& generateProp,
								const AQLStringMatrix& moneyConv);

	static void setUpStoreSwapCurve(AQLDataInstance* dataInstance,
								 const AQLString& curveID,
								 const AQLString& genCurveName,
								 const AQLStringMatrix& generateProp, 
								 const AQLStringMatrix& moneyConv,
								 const AQLStringMatrix& liborRates, 
								 const AQLStringMatrix& liborConv,
								 const AQLStringMatrix& swapRates, 
								 const AQLStringMatrix& swapConv,
								 const AQLStringMatrix& fra3mRates,
								 const AQLStringMatrix& fra6mRates,
								 const AQLStringMatrix& fraConv,
								 const AQLStringMatrix& futureRates, 
								 const AQLStringMatrix& futureConv,
								 const AQLStringMatrix& adjustSwapConv,
								 const AQLStringMatrix& adjustSwapRates);

	static void setUpStoreOISCurve(AQLDataInstance* dataInstance,
								const AQLString& curveID,
								const AQLString& genCurveName,
								const AQLStringMatrix& generateProp, 
								const AQLStringMatrix& oisRates, 
								const AQLStringMatrix& oisConv,
								const AQLStringMatrix& histRates,
								const AQLStringMatrix& lobasisRates = AQLStringMatrix(), 
								const AQLStringMatrix& lobasisConv = AQLStringMatrix(), 
								const AQLStringMatrix& swapRates = AQLStringMatrix(), 
								const AQLStringMatrix& swapConv = AQLStringMatrix());

	static void shiftMarketRate(AQLStringMatrix& rateMat, double shiftval, bool ispara = true, unsigned int shiftpos = 0);

	static AQLMathObjectValue* createSingleTrade(AQLDataInstance* dataInstance, std::map<AQLString, AQLString> maptrade, AQLString orgtradeid, bool isduplicatemode = false, AQLString key_suffix = "", bool istypicalpardeal = false);
	
	static void createOfferBidAdjustEntity(AQLDataInstance* dataInstance, std::map<AQLString, AQLString> maptrade, AQLStringVector orgtradevec, bool isduplicatemode = false, AQLString key_suffix = "");


	static void calcRiskAndPV(AQLDataInstance* dataInstance, const AQLStringVector& tradeVec, DoubleVector& pvResultVec);

	static void createSourceDeltaRiskEntity(AQLDataInstance* dataInstance, AQLString scenarioname, AQLStringVector riskDetail1sordelta, AQLStringVector riskDetail2sordelta, bool isbumpgridauto, AQLString paraName = "");
	
	static void createZeroDeltaRiskEntity(AQLDataInstance* dataInstance, AQLString scenarioname, AQLStringVector riskDetail1zerodelta, AQLStringVector riskDetail2zerodelta, bool isbumpgridauto, AQLString paraName = "");

	static void calcPortfolioRiskAndPV(AQLDataInstance* dataInstance, std::map<AQLString, AQLString> mapriskinfo);

	static double getPortfolioRiskResult(AQLDataInstance* dataInstance, AQLString marketkey, AQLString marketgrid, AQLString risktype = "SOURCEBUMP");

	static void storePastRates(AQLDataInstance* dataInstance, const AQLStringMatrix& IndexInfo, const AQLStringMatrix& PastRates);

	static void setUpFixingRateFromPastRates(AQLDataInstance* dataInstance, AQLString copytradeid, const AQLDate& asOfDate);

	static AQLString setUpMarketParamsAndGlobalShift(AQLDataInstance* dataInstance, std::map<AQLString, AQLString> mapcalcinfo, AQLString curveID);

	static void setUpLineParams(AQLDataInstance* dataInstance, std::map<AQLString, AQLString> mapcalcinfo);

	static AQLString getDiscountCurveName(AQLDataInstance* dataInstance, AQLString curveID);

	static AQLString getCurveIDfromTradeReference(AQLDataInstance* dataInstance, AQLString tradeID);

	static AQLString getMarketParamfromTradeReference(AQLDataInstance* dataInstance, AQLString tradeID);

	static AQLStringVector getCurveNamesFromSetUpOrder(AQLDataInstance* dataInstance, AQLString curveID);

	static void createShiftCurves(AQLDataInstance* dataInstance, AQLObject& eorgCurve, AQLString targetMarketName, AQLString newCurveID, bool isshiftarget, bool ispara, 
											double shiftval, unsigned int shiftpos, AQLString market, bool isglobalscenario);

};
#endif