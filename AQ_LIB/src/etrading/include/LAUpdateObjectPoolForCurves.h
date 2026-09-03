//
// LAUpdateObjectPoolForCurves.h
// This file used to be called CalibratorIR.h and before that LACalibrateModelIR.h
//
#pragma once



#include "LAString.h"
#include "LAObjectPoolBase.h"


class LADataInstance;
#ifndef VISUAL_STUDIO_2010_ANALYTICS 
class LARatesCurveLogLinearInterpolation;
class LAMathCorrelation;
#endif 

class BasisCurveCalibration;
class LAStaticData;
class LAMathYieldCurve;
class LAObject;
class LAObjectPool;


//===================== Class Declare LAUpdateObjectPoolForCurves ==================================
/*! 
    @brief IR SDE generator 
	
	this class is abstract

*/
class LAUpdateObjectPoolForCurves : public LAObjectPoolBase
{
public:
	//! constructor
	explicit LAUpdateObjectPoolForCurves(const LAString &baseCurrency);
	//! destructor
	virtual ~LAUpdateObjectPoolForCurves(void);
	//! copy constructor
	LAUpdateObjectPoolForCurves(const LAUpdateObjectPoolForCurves &rhs);
	LAUpdateObjectPoolForCurves &operator=(const LAUpdateObjectPoolForCurves &rhs);
	//==============================================
	//! generate SDE market data
	virtual void generateSDEMarketData(const LAString &currency, LADataInstance &dataInstance, const bool isCurve = true, const bool isOthers = true) const;
	//==============================================
	//! generate sde initial value
	virtual void generateInitialValue(const LAString &currency, LADataInstance &dataInstance) const;
	//==============================================
	virtual void generateInitialValueDualBootstrap(const LAString &currency, LADataInstance &dataInstance) const;
	//==============================================
	virtual void generateInitialValueGlobalEngineCurves(const LAString &currency, LADataInstance &dataInstance) const;
	//==============================================
	//! generate sde initial value for fwdfx constant curve
	virtual void generateInitialValueFwdFXConst(const LAString &currency, LADataInstance &dataInstance) const;
	//==============================================
	//! generate sde initial value for cheapest-to-deliver curve
	virtual void generateInitialValueCheapestToDeliver(const LAString &currency, LADataInstance &dataInstance) const;
	//==============================================
#ifndef VISUAL_STUDIO_2010_ANALYTICS 
	//! generate sde correlation
	virtual void generateCorrelation(const LAString &currency, LADataInstance &dataInstance) const;
#endif 

protected:
	//==============================================
	//! generate sde initial value when arbfree curve is generated
	virtual void generateInitialValueArbfree(const LAString &currency, LADataInstance &dataInstance) const;

#ifndef VISUAL_STUDIO_2010_ANALYTICS 
	//==============================================
	//! set interpolation method 
	virtual  void setInterpolationMethod(const LAString &currency, LARatesSDEBase &sde) const;
	//==============================================
	//! create sde instance 
//	virtual LARatesSDEBase *createSDEInstance(const LAString &currency) const;
	//==============================================
	//! setup correlation input type
	virtual LAString getCorTye(const LAString &currency) const = 0;
	//==============================================
	//! setup correlation factor
	virtual void setUpCorFactor(const LAString &currency, LAMathCorrelation &cor, LADataInstance &dataInstance) const = 0;
	//==============================================
	//! setup correlation data
	virtual void setUpCorData(const LAString &currency, LAMathCorrelation &cor, LADataInstance &dataInstance) const = 0;
	//==============================================
	//! setup correlation method
	virtual void setUpCorFunc(const LAString &currency, LAMathCorrelation &cor, LADataInstance &dataInstance) const = 0;
#endif

	//==============================================
	//! get SDE data name
	virtual LAString getSDEAttrName(const LAString &currency) const;
	//==============================================
	//! get grid propval
	LAString getGridStaticData(const LAString &key, const LAString &curve, const LAString &grid) const;
	//==============================================
	//! set up generate configuration
	void setUpGenerateConfig(LADataInstance &dataInstance, const LADate &asOfDate, const LAString &currency, LAMathYieldCurve &yc, BasisCurveCalibration &basisCurveEngine,
							 LAObject &ycData, bool &isAudExtra, bool &isSwapTenorAdjust, bool &isSpotUse, bool isArbFree, const LAString& suffix = "", const LAString& epSuffix = "") const;
	//==============================================
	//! set up basis curve data
	void setUpBasisCurveData(LADataInstance &dataInstance, LAString &refData, const LADate &asOfDate, const LAString &curveCurrency, 
							 const LAString &marketName, const LAString &yieldDataName, bool isSpotUse, BasisCurveCalibration &basisCurveEngine, const LAString& currentCurveName = LAString(), const LAString* pMktCurrency = 0) const;
	//==============================================
	//! set up base curve data
	void setUpGenCurveData(LADataInstance &dataInstance, LAString &refData, const LADate &asOfDate, const LAString &currency, 
						   const LAString &marketName, const LAString &yieldDataName, bool isSpotUse,
						   bool isAudExtra, BasisCurveCalibration &basisCurveEngine, std::map<LAString, std::map<LAString, double> > &aud_origSwapRate, const LAString& curveName = "") const;
	//==============================================
	//! set up ois curve data
	void setUpGenCurveDataOIS(LADataInstance &dataInstance, LAString &refData, const LADate &asOfDate, const LAString &currency, 
							  const LAString &marketName, const LAString &yieldDataName, bool isSpotUse, bool isAudExtra, BasisCurveCalibration &basisCurveEngine, 
							  std::map<LAString, std::map<LAString, double> >& aud_origSwapRate) const;
	//==============================================
	//! set up 36basis curve dummy data
	void setUp36BasisDummyData(LADataInstance &dataInstance, LAString &refData, const LADate &asOfDate, const LAString &currency, 
							   const LAString &marketName, const LAString &yieldDataName, bool isSpotUse, BasisCurveCalibration &basisCurveEngine) const;
	//==============================================
	//! set up floater
	void setUpFloater(const LAString &currency, BasisCurveCalibration &basisCurveEngine, const LAString &genFloaterName) const;
	//==============================================
	//! set up curve data by reading file
	void setUpCurveDataByReadFile(LADataInstance &dataInstance, const LADate &asOfDate, const LAString &currency, const LAString &marketName, const LAString &yieldDataName, BasisCurveCalibration &basisCurveEngine ) const;
	//==============================================
	//! set up daycount
	void setUpCurveTypeDayCount(BasisCurveCalibration &basisCurveEngine, LAMathYieldCurve &yc) const;

	void dataoutCurve(const LAStringVector &dataoutCurves, LAObject &eData, const LAString &yieldDataName) const;

///// update for XLL Plus //////////////////////////
	//==============================================
	//! generate sde initial value when use curve that generated by excel pricer.
	void generateInitialValueForPricer(const LAString &currency, LADataInstance &dataInstance) const;
	//==============================================
	//! set up curve data by context for excel pricer
	void setUpCurveDataByContext(BasisCurveCalibration &basisCurveEngine, LAObject *eData, const LAString& currency, const LAString& marketName ) const;
////////////////////////////////////////////////////

	const LAString mBaseCurrency; //!< baseCurrency

	mutable std::map<LAString, bool> mCurveGenCcyMap; //!< Generate Curve Currency Map
	//==============================================
	//! check frequency
	virtual bool checkFrequency(const LAString& freq, const LAString& mktRateTerm) const;
	
	//! Configure curve by preparing data used for calibration
	void configureCurve(LAStaticData *mpStaticData,
							BasisCurveCalibration *basisCurveEngine,
							LAMathYieldCurve *yc,
							LADataInstance &dataInstance,
							const LADate& asOfDate,
							const LAString& currency,
							const LAString& curveName,
							const LAString& yieldDataName,
							const LAString& suffix,
							const LAString& ep_suffix,
							bool enableCalculation,
							bool isSpotUse,
							bool isAudExtra,
							bool isSwapTenorAdjust) const;
private:
	//==============================================
	//! convertcurve name from properties
	void convertCurveName(const LAString &propCurve, const LAString &ccy, LAString &curveCcy, LAString &curveName, const bool isPricer, LAObjectPool& objPool) const;
	//==============================================
	//! get Accessory from Frequency
	const LAString getAccFromFreq(const LAString &freq) const;

};

