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


#include <LADate.h>
#include <LACoreAppError.h>
#include <LACoreTemplateType.h>
#include <LADataInstance.h>
#include <LAString.h>

#include <map>

class LAStaticData;

class LACurveSetup
{
public:
    enum COMPOUND_TYPE {COMPOUND_NORMAL, COMPOUND_FLAT, COMPOUND_SIMPLE, AVERAGE};

	//==============================================================================
	static void setUpBasisCurve(LADataInstance* dataInstance,
                                const LAString& curveID, 
                                const LAString& marketName, 
                                const LAStringMatrix& basisRates, 
                                const LAStringMatrix& basisConv,
								const LAStringMatrix& fwdFXs, 
								const LAStringMatrix& fwdConv,
								const LAStringMatrix& spotFXs, 
								const LAStringMatrix& generateProp,
								const LAStringMatrix& moneyConv,
								const LAString& curveNames);

	//==============================================================================
	static void setUpFwdFXConstantCurve(LADataInstance* dataInstance,
                                const LAString& curveID, 
                                const LAString& marketName, 
                                const LAStringMatrix& fwdfxconstConv,
								const LAStringMatrix& generateProp,
								const LAString& curveNames);
	//==============================================================================
	static void setUpSwapCurve(LADataInstance* dataInstance,
								const LAString& curveID,
								const LAString& marketName,
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
								const LAStringMatrix& adjustSwapRates,
								const LAString& curveNames,
								const LAString& curveName_DF2);
	//==============================================================================
	static void SetUpArbFreeCurve(LADataInstance* dataInstance,
								const LAString& curveID,
								const LAStringMatrix& generateProp, 
								const LAStringMatrix& moneyConv,
								const LAStringMatrix& liborRates, 
								const LAStringMatrix& liborConv,
								const LAStringMatrix& swapRates, 
								const LAStringMatrix& swapConv,
								const LAStringMatrix& fra3mRates,
								const LAStringMatrix& fra6mRates,
								const LAStringMatrix& fraConv,
								const LAStringMatrix& xccyBasisRates, 
								const LAStringMatrix& xccyBasisConv,
								const LAStringMatrix& threeSixRates,
								const LAStringMatrix& threeSixConv,
								const LAStringMatrix& futureRates, 
								const LAStringMatrix& futureConv,
								const LAStringMatrix& adjustData,
								const LAString& curveNames_3ML,
								const LAString& curveNames_6ML,
								const LAString& curveNames_DF,
								const LAString& curveName_DF2);
	//==============================================================================
	static void setUpOISCurve(LADataInstance* dataInstance,
								const LAString& curveID,
								const LAString& marketName,
								const LAStringMatrix& generateProp, 
								const LAStringMatrix& oisRates, 
								const LAStringMatrix& oisConv,
								const LAString& curveNames,
								const LAStringMatrix& histRates,
								const LAStringMatrix& lobasisRates = LAStringMatrix(), 
								const LAStringMatrix& lobasisConv = LAStringMatrix(), 
								const LAStringMatrix& swapRates = LAStringMatrix(), 
								const LAStringMatrix& swapConv = LAStringMatrix());
	//==============================================================================
	static void setUpFloater(LADataInstance* dataInstance,
							 const LAString& curveID,
							 const LAString& discountCurveName,
							 const LAString& forecastCurveName,
							 const LAStringMatrix& generateProp, 
							 const LAStringMatrix& basisMkt, 
							 const LAStringMatrix& basisConv,
							 const LAStringMatrix& swapConv,
							 const LAStringMatrix& adjustData);
	//==============================================================================
	static void setUpForIRServer(void);
	//==============================================================================
	static void setUpDefaultIRStaticData(LADataInstance& dataInstance);
	static void setUpDefaultIRStaticData(LADataInstance& dataInstance, LAString filePath);
	static void setUpDefaultCalibStaticData(LADataInstance& dataInstance);
	static void setUpDefaultCalibStaticData(LADataInstance& dataInstance, LAString filePath);
	static LAString getIRStaticData(const LAString& key);
	//==============================================================================
	static void resetService();
	//==============================================================================
	static void SetUpFundingSpread(LADataInstance* dataInstance, 
								   const LAStringMatrix &fundingSpread);

    static double compound(LADataInstance* dataInstance,
                           const LAString& curveID,
                           const LAString& forecastCurveName,
                           const LADate& start_date,
                           const LADate& end_date,
                           const double spread,
                           const LAString& frequency_,
                           const bool is_start_roll,
                           const LAString& roll_convention_,
                           const LAString& calendar_,
                           const LAString& sliding_rule_,
                           const LAString& day_count_,
                           const LAString& interpolation_,
                           const LAString& compound_type_,
                           const LADate* first_odd,
                           const LADate* last_odd);
	//==============================================================================
	static void CalcMeanAndCovariance(const DoubleMatrix& data, DoubleArray& mean, DoubleMatrix& covar);

	static void SetUpPCA(LADataInstance* dataInstance, const DoubleMatrix& corr, const size_t no_factors, const LAString& id);

	static DoubleMatrix GetPCAResult(LADataInstance* dataInstance, const LAString& type, const LAString& id);
	//==============================================================================
//#if defined(VISUAL_STUDIO_2010_ANALYTICS) || defined(ETRADINGADDIN)
	static LAStringMatrix LoadStaticDataObject( const LAString& prefix, const LAString& suffix );
//#endif
	//==============================================================================

//#if defined(VISUAL_STUDIO_2010_ANALYTICS) || defined(ETRADINGADDIN)
	static int setUpHazardRate(LADataInstance* dataInstance,
                               const LAString& cdsRefEntity, 
                               const LAStringMatrix& cdsRates, 
                               const LAStringMatrix& cdsConv,
							   const LAStringMatrix& generateProp,
							   const LAStringMatrix& riskScenarioProp);

	static int registerHazardRate(LADataInstance* dataInstance,
                                  const LAString& cdsRefEntity, 
                                  const DoubleArray& hazardTerms, 
                                  const DoubleArray& hazardRates,
								  const LAStringMatrix& generateProp);

	static void outPutHazardRate(LADataInstance* dataInstance,
                                 const LAString& cdsRefEntity,
								 DoubleArray& hazardRate);

	static double calcSurvivalProbability(LADataInstance* dataInstance,
                                          const LAString& cdsRefEntity,
								          const double t);

	static double calcHazardRate(LADataInstance* dataInstance,
                                 const LAString& cdsRefEntity,
								 const double t);

	static int calcCDSSingleTrade(LADataInstance* dataInstance,
								  const LADate& baseDate,
								  const std::map<LAString, LAString>& infoMap);

	static void getCDSSingleTradeResult(LADataInstance* dataInstance, 
										const LAString& cds_name,
										const LAStringVector& resultAttrNames,
										DoubleVector& resultValues);

#endif

protected:
    static std::istringstream* createFutureStream(const LAStringMatrix& future_rates, LAString& usegrid_future);
	static void setStaticDataValue(LAStaticData &staticData, const LAString &key, const LAString &val, const bool is_override = false);
};
//#endif