//
// LAUpdateObjectPoolForCurves.h
// This file used to be called CalibratorIR.h and before that AQLCalibrateModelIR.h
//
#pragma once



#include "AQLString.h"
#include "LAObjectPoolBase.h"


class AQLDataInstance;
#ifndef VISUAL_STUDIO_2010_ANALYTICS 
class AQLRatesCurveLogLinearInterpolation;
class AQLMathCorrelation;
#endif 

class BasisCurveCalibration;
class AQLStaticData;
class AQLMathYieldCurve;
class AQLObject;
class AQLObjectPool;


//===================== Class Declare LAUpdateObjectPoolForCurves ==================================
/*! 
    @brief IR SDE generator 
	
	this class is abstract

*/
class LAUpdateObjectPoolForCurves : public LAObjectPoolBase
{
public:
	//! constructor
	explicit LAUpdateObjectPoolForCurves(const AQLString &baseCurrency);
	//! destructor
	virtual ~LAUpdateObjectPoolForCurves(void);
	//! copy constructor
	LAUpdateObjectPoolForCurves(const LAUpdateObjectPoolForCurves &rhs);
	LAUpdateObjectPoolForCurves &operator=(const LAUpdateObjectPoolForCurves &rhs);
	//==============================================
	//! generate SDE market data
	virtual void generateSDEMarketData(const AQLString &currency, AQLDataInstance &dataInstance, const bool isCurve = true, const bool isOthers = true) const;
	//==============================================
	//! generate sde initial value
	virtual void generateInitialValue(const AQLString &currency, AQLDataInstance &dataInstance) const;
	//==============================================
	virtual void generateInitialValueDualBootstrap(const AQLString &currency, AQLDataInstance &dataInstance) const;
	//==============================================
	virtual void generateInitialValueGlobalEngineCurves(const AQLString &currency, AQLDataInstance &dataInstance) const;
	//==============================================
	//! generate sde initial value for fwdfx constant curve
	virtual void generateInitialValueFwdFXConst(const AQLString &currency, AQLDataInstance &dataInstance) const;
	//==============================================
	//! generate sde initial value for cheapest-to-deliver curve
	virtual void generateInitialValueCheapestToDeliver(const AQLString &currency, AQLDataInstance &dataInstance) const;
	//==============================================
#ifndef VISUAL_STUDIO_2010_ANALYTICS 
	//! generate sde correlation
	virtual void generateCorrelation(const AQLString &currency, AQLDataInstance &dataInstance) const;
#endif 

protected:
	//==============================================
	//! generate sde initial value when arbfree curve is generated
	virtual void generateInitialValueArbfree(const AQLString &currency, AQLDataInstance &dataInstance) const;

#ifndef VISUAL_STUDIO_2010_ANALYTICS 
	//==============================================
	//! set interpolation method 
	virtual  void setInterpolationMethod(const AQLString &currency, AQLRatesSDEBase &sde) const;
	//==============================================
	//! create sde instance 
//	virtual AQLRatesSDEBase *createSDEInstance(const AQLString &currency) const;
	//==============================================
	//! setup correlation input type
	virtual AQLString getCorTye(const AQLString &currency) const = 0;
	//==============================================
	//! setup correlation factor
	virtual void setUpCorFactor(const AQLString &currency, AQLMathCorrelation &cor, AQLDataInstance &dataInstance) const = 0;
	//==============================================
	//! setup correlation data
	virtual void setUpCorData(const AQLString &currency, AQLMathCorrelation &cor, AQLDataInstance &dataInstance) const = 0;
	//==============================================
	//! setup correlation method
	virtual void setUpCorFunc(const AQLString &currency, AQLMathCorrelation &cor, AQLDataInstance &dataInstance) const = 0;
#endif

	//==============================================
	//! get SDE data name
	virtual AQLString getSDEAttrName(const AQLString &currency) const;
	//==============================================
	//! get grid propval
	AQLString getGridStaticData(const AQLString &key, const AQLString &curve, const AQLString &grid) const;
	//==============================================
	//! set up generate configuration
	void setUpGenerateConfig(AQLDataInstance &dataInstance, const AQLDate &asOfDate, const AQLString &currency, AQLMathYieldCurve &yc, BasisCurveCalibration &basisCurveEngine,
							 AQLObject &ycData, bool &isAudExtra, bool &isSwapTenorAdjust, bool &isSpotUse, bool isArbFree, const AQLString& suffix = "", const AQLString& epSuffix = "") const;
	//==============================================
	//! set up basis curve data
	void setUpBasisCurveData(AQLDataInstance &dataInstance, AQLString &refData, const AQLDate &asOfDate, const AQLString &curveCurrency, 
							 const AQLString &marketName, const AQLString &yieldDataName, bool isSpotUse, BasisCurveCalibration &basisCurveEngine, const AQLString& currentCurveName = AQLString(), const AQLString* pMktCurrency = 0) const;
	//==============================================
	//! set up base curve data
	void setUpGenCurveData(AQLDataInstance &dataInstance, AQLString &refData, const AQLDate &asOfDate, const AQLString &currency, 
						   const AQLString &marketName, const AQLString &yieldDataName, bool isSpotUse,
						   bool isAudExtra, BasisCurveCalibration &basisCurveEngine, std::map<AQLString, std::map<AQLString, double> > &aud_origSwapRate, const AQLString& curveName = "") const;
	//==============================================
	//! set up ois curve data
	void setUpGenCurveDataOIS(AQLDataInstance &dataInstance, AQLString &refData, const AQLDate &asOfDate, const AQLString &currency, 
							  const AQLString &marketName, const AQLString &yieldDataName, bool isSpotUse, bool isAudExtra, BasisCurveCalibration &basisCurveEngine, 
							  std::map<AQLString, std::map<AQLString, double> >& aud_origSwapRate) const;
	//==============================================
	//! set up 36basis curve dummy data
	void setUp36BasisDummyData(AQLDataInstance &dataInstance, AQLString &refData, const AQLDate &asOfDate, const AQLString &currency, 
							   const AQLString &marketName, const AQLString &yieldDataName, bool isSpotUse, BasisCurveCalibration &basisCurveEngine) const;
	//==============================================
	//! set up floater
	void setUpFloater(const AQLString &currency, BasisCurveCalibration &basisCurveEngine, const AQLString &genFloaterName) const;
	//==============================================
	//! set up curve data by reading file
	void setUpCurveDataByReadFile(AQLDataInstance &dataInstance, const AQLDate &asOfDate, const AQLString &currency, const AQLString &marketName, const AQLString &yieldDataName, BasisCurveCalibration &basisCurveEngine ) const;
	//==============================================
	//! set up daycount
	void setUpCurveTypeDayCount(BasisCurveCalibration &basisCurveEngine, AQLMathYieldCurve &yc) const;

	void dataoutCurve(const AQLStringVector &dataoutCurves, AQLObject &eData, const AQLString &yieldDataName) const;

///// update for XLL Plus //////////////////////////
	//==============================================
	//! generate sde initial value when use curve that generated by excel pricer.
	void generateInitialValueForPricer(const AQLString &currency, AQLDataInstance &dataInstance) const;
	//==============================================
	//! set up curve data by context for excel pricer
	void setUpCurveDataByContext(BasisCurveCalibration &basisCurveEngine, AQLObject *eData, const AQLString& currency, const AQLString& marketName ) const;
////////////////////////////////////////////////////

	const AQLString mBaseCurrency; //!< baseCurrency

	mutable std::map<AQLString, bool> mCurveGenCcyMap; //!< Generate Curve Currency Map
	//==============================================
	//! check frequency
	virtual bool checkFrequency(const AQLString& freq, const AQLString& mktRateTerm) const;
	
	//! Configure curve by preparing data used for calibration
	void configureCurve(AQLStaticData *mpStaticData,
							BasisCurveCalibration *basisCurveEngine,
							AQLMathYieldCurve *yc,
							AQLDataInstance &dataInstance,
							const AQLDate& asOfDate,
							const AQLString& currency,
							const AQLString& curveName,
							const AQLString& yieldDataName,
							const AQLString& suffix,
							const AQLString& ep_suffix,
							bool enableCalculation,
							bool isSpotUse,
							bool isAudExtra,
							bool isSwapTenorAdjust) const;
private:
	//==============================================
	//! convertcurve name from properties
	void convertCurveName(const AQLString &propCurve, const AQLString &ccy, AQLString &curveCcy, AQLString &curveName, const bool isPricer, AQLObjectPool& objPool) const;
	//==============================================
	//! get Accessory from Frequency
	const AQLString getAccFromFreq(const AQLString &freq) const;

};

