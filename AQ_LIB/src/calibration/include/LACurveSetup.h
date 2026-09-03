#ifndef LA_CURVE_SETUP_H
#define LA_CURVE_SETUP_H

#ifdef __GNUG__
#pragma interface
#endif

#if defined (WIN32) || defined (WIN64)
#include <windows.h>
#else
#include <pthread.h>
#endif


#include <AQLDate.h>
#include <AQLCoreAppError.h>
#include <AQLCoreTemplateType.h>
#include <AQLDataInstance.h>
#include <AQLString.h>

#include <map>

class LAStaticData;

class LACurveSetup
{
public:
    enum COMPOUND_TYPE {COMPOUND_NORMAL, COMPOUND_FLAT, COMPOUND_SIMPLE, AVERAGE};

	//==============================================================================
	static void setUpBasisCurve(AQLDataInstance* dataInstance,
                                const AQLString& curveID, 
                                const AQLString& marketName, 
                                const AQLStringMatrix& basisRates, 
                                const AQLStringMatrix& basisConv,
								const AQLStringMatrix& fwdFXs, 
								const AQLStringMatrix& fwdConv,
								const AQLStringMatrix& spotFXs, 
								const AQLStringMatrix& generateProp,
								const AQLStringMatrix& moneyConv,
								const AQLString& curveNames);

	//==============================================================================
	static void setUpFwdFXConstantCurve(AQLDataInstance* dataInstance,
                                const AQLString& curveID, 
                                const AQLString& marketName, 
                                const AQLStringMatrix& fwdfxconstConv,
								const AQLStringMatrix& generateProp,
								const AQLString& curveNames);
	//==============================================================================
	static void setUpSwapCurve(AQLDataInstance* dataInstance,
								const AQLString& curveID,
								const AQLString& marketName,
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
								const AQLStringMatrix& adjustSwapRates,
								const AQLString& curveNames,
								const AQLString& curveName_DF2);
	//==============================================================================
	static void SetUpArbFreeCurve(AQLDataInstance* dataInstance,
								const AQLString& curveID,
								const AQLStringMatrix& generateProp, 
								const AQLStringMatrix& moneyConv,
								const AQLStringMatrix& liborRates, 
								const AQLStringMatrix& liborConv,
								const AQLStringMatrix& swapRates, 
								const AQLStringMatrix& swapConv,
								const AQLStringMatrix& fra3mRates,
								const AQLStringMatrix& fra6mRates,
								const AQLStringMatrix& fraConv,
								const AQLStringMatrix& xccyBasisRates, 
								const AQLStringMatrix& xccyBasisConv,
								const AQLStringMatrix& threeSixRates,
								const AQLStringMatrix& threeSixConv,
								const AQLStringMatrix& futureRates, 
								const AQLStringMatrix& futureConv,
								const AQLStringMatrix& adjustData,
								const AQLString& curveNames_3ML,
								const AQLString& curveNames_6ML,
								const AQLString& curveNames_DF,
								const AQLString& curveName_DF2);
	//==============================================================================
	static void setUpOISCurve(AQLDataInstance* dataInstance,
								const AQLString& curveID,
								const AQLString& marketName,
								const AQLStringMatrix& generateProp, 
								const AQLStringMatrix& oisRates, 
								const AQLStringMatrix& oisConv,
								const AQLString& curveNames,
								const AQLStringMatrix& histRates,
								const AQLStringMatrix& lobasisRates = AQLStringMatrix(), 
								const AQLStringMatrix& lobasisConv = AQLStringMatrix(), 
								const AQLStringMatrix& swapRates = AQLStringMatrix(), 
								const AQLStringMatrix& swapConv = AQLStringMatrix());
	//==============================================================================
	static void setUpFloater(AQLDataInstance* dataInstance,
							 const AQLString& curveID,
							 const AQLString& discountCurveName,
							 const AQLString& forecastCurveName,
							 const AQLStringMatrix& generateProp, 
							 const AQLStringMatrix& basisMkt, 
							 const AQLStringMatrix& basisConv,
							 const AQLStringMatrix& swapConv,
							 const AQLStringMatrix& adjustData);
	//==============================================================================
	static void setUpForIRServer(void);
	//==============================================================================
	static void setUpDefaultIRStaticData(AQLDataInstance& dataInstance);
	static void setUpDefaultIRStaticData(AQLDataInstance& dataInstance, AQLString filePath);
	static void setUpDefaultCalibStaticData(AQLDataInstance& dataInstance);
	static void setUpDefaultCalibStaticData(AQLDataInstance& dataInstance, AQLString filePath);
	static AQLString getIRStaticData(const AQLString& key);
	//==============================================================================
	static void resetService();
	//==============================================================================
	static void SetUpFundingSpread(AQLDataInstance* dataInstance, 
								   const AQLStringMatrix &fundingSpread);

    static double compound(AQLDataInstance* dataInstance,
                           const AQLString& curveID,
                           const AQLString& forecastCurveName,
                           const AQLDate& start_date,
                           const AQLDate& end_date,
                           const double spread,
                           const AQLString& frequency_,
                           const bool is_start_roll,
                           const AQLString& roll_convention_,
                           const AQLString& calendar_,
                           const AQLString& sliding_rule_,
                           const AQLString& day_count_,
                           const AQLString& interpolation_,
                           const AQLString& compound_type_,
                           const AQLDate* first_odd,
                           const AQLDate* last_odd);
	//==============================================================================
	static void CalcMeanAndCovariance(const DoubleMatrix& data, DoubleArray& mean, DoubleMatrix& covar);

	static void SetUpPCA(AQLDataInstance* dataInstance, const DoubleMatrix& corr, const size_t no_factors, const AQLString& id);

	static DoubleMatrix GetPCAResult(AQLDataInstance* dataInstance, const AQLString& type, const AQLString& id);
	//==============================================================================
//#if defined(VISUAL_STUDIO_2010_ANALYTICS) || defined(ETRADINGADDIN)
	static AQLStringMatrix LoadStaticDataObject( const AQLString& prefix, const AQLString& suffix );
//#endif
	//==============================================================================

//#if defined(VISUAL_STUDIO_2010_ANALYTICS) || defined(ETRADINGADDIN)
	static int setUpHazardRate(AQLDataInstance* dataInstance,
                               const AQLString& cdsRefEntity, 
                               const AQLStringMatrix& cdsRates, 
                               const AQLStringMatrix& cdsConv,
							   const AQLStringMatrix& generateProp,
							   const AQLStringMatrix& riskScenarioProp);

	static int registerHazardRate(AQLDataInstance* dataInstance,
                                  const AQLString& cdsRefEntity, 
                                  const DoubleArray& hazardTerms, 
                                  const DoubleArray& hazardRates,
								  const AQLStringMatrix& generateProp);

	static void outPutHazardRate(AQLDataInstance* dataInstance,
                                 const AQLString& cdsRefEntity,
								 DoubleArray& hazardRate);

	static double calcSurvivalProbability(AQLDataInstance* dataInstance,
                                          const AQLString& cdsRefEntity,
								          const double t);

	static double calcHazardRate(AQLDataInstance* dataInstance,
                                 const AQLString& cdsRefEntity,
								 const double t);

	static int calcCDSSingleTrade(AQLDataInstance* dataInstance,
								  const AQLDate& baseDate,
								  const std::map<AQLString, AQLString>& infoMap);

	static void getCDSSingleTradeResult(AQLDataInstance* dataInstance, 
										const AQLString& cds_name,
										const AQLStringVector& resultAttrNames,
										DoubleVector& resultValues);

#endif

protected:
    static std::istringstream* createFutureStream(const AQLStringMatrix& future_rates, AQLString& usegrid_future);
	static void setStaticDataValue(LAStaticData &staticData, const AQLString &key, const AQLString &val, const bool is_override = false);
};
//#endif