//
// LAObjectPoolBase.h
// This file used to be called Calibrator.h and before that AQLCalibrateModel.h
//
#pragma once

#ifdef __GNUG__
#pragma interface
#endif

//#include "AQLString.h"
#include "AQLRatesSDEBase.h"


class AQLString;
class AQLRatesSDEBase;
class AQLDataInstance;
class AQLRatesCurveLogLinearInterpolation;
class AQLMathCorrelation;
class AQLMathVolatility;
class AQLStaticData;

//===================== Class Declare LAObjectPoolBase ==================================
/*! 
    @brief SDE generator 
	
	this class is abstract

*/
class LAObjectPoolBase
{
public:
	// constructor
	explicit LAObjectPoolBase(void);
	// destructor
	virtual ~LAObjectPoolBase(void);
	// copy constructor
	LAObjectPoolBase(const LAObjectPoolBase &rhs);
	LAObjectPoolBase &operator=(const LAObjectPoolBase &rhs);

	//==============================================
	// generate SDE
	virtual void generateSDE(const AQLString &key, AQLDataInstance &dataInstance,const bool isMarketCreate = true, const bool isFirst = true) const;

	//==============================================
	// generate SDE market data
	virtual void loadModelDataAndCalibrate(const AQLString &key, AQLDataInstance &dataInstance, const bool isCurve = true, const bool isModel = true, const AQLString & curveID = "", const AQLString & marketName = "") const = 0;

	//==============================================
	// generate sde volatility
	virtual void loadVolatilityDataAndCalibrate(const AQLString &key, AQLDataInstance &dataInstance) const;

	//==============================================
	// generate sde initial value for fwdfx constant curve
	virtual void loadFwdFXConstCurveDataAndCalibrate(const AQLString &currency, AQLDataInstance &dataInstance, const AQLString & curveID = "", const AQLString & marketName = "") const = 0;

	//==============================================
	virtual void loadDualBootstrapCurveDataAndCalibrate(const AQLString &currency, AQLDataInstance &dataInstance) const = 0;

	//==============================================
	virtual void loadGlobalCurveDataAndCalibrate(const AQLString &currency, AQLDataInstance &dataInstance) const = 0;

	//==============================================
	// is fwdfx const curve
	bool isFwdFXConst(const AQLString& ccy) const;
	
	//==============================================
	// is collateral ccy
	bool isCollateral(const AQLString& ccy) const;

protected:
	//==============================================
	// get sde Type
	virtual  SDE_TYPE getSDEType(const AQLString &key) const = 0;
	//==============================================
	// check LJ
	virtual bool isLJ(const AQLString &key) const = 0;
	//==============================================
	// set volatility 
	virtual  void setVolatility(const AQLString &key, AQLRatesSDEBase &sde)  const = 0;	
	//==============================================
	// set drift 
	virtual  void setDrift(const AQLString &key, AQLRatesSDEBase &sde)  const = 0;
	//==============================================
	// set numeraire 
	virtual  void setNumeraire(const AQLString &key, AQLRatesSDEBase &sde)  const = 0;
	//==============================================
	// set path 
	virtual  void setOutputTemplate(const AQLString &key, AQLRatesSDEBase &sde)  const  = 0;
	//==============================================
	// set integralfunction 
	virtual  void setIntegralFunction(const AQLString &key, AQLRatesSDEBase &sde)  const = 0;
	//==============================================
	// set interpolation method 
	virtual  void setInterpolationMethod(const AQLString &key, AQLRatesSDEBase &sde) const = 0;
	//==============================================
	// get function master regist name 
	virtual AQLString getFunctionMasterResistName(const AQLString &key) const = 0;
	//==============================================
	// get create sde instance 
	virtual AQLRatesSDEBase *createSDEInstance(const AQLString &key, AQLDataInstance &dataInstance) const = 0;
	//==============================================
	// setup vol type
	virtual AQLString getVolType(const AQLString &key) const = 0;
	//==============================================
	// setup vol data
	virtual void setUpVolData(const AQLString &key, AQLMathVolatility &vol, AQLDataInstance &dataInstance) const = 0;
	//==============================================
	// setup vol method
	virtual void setUpVolFunc(const AQLString &key,AQLMathVolatility &vol, AQLDataInstance &dataInstance) const = 0;
	//==============================================
	// get sde function name
	virtual AQLString getSDEAttrName(const AQLString &key) const = 0;

	//==============================================
	// check cancel target for funding change 
	virtual  bool isCancelForFunding(const AQLString &ccy) const;
	//==============================================
	// check calibrarion target currency or fx
	virtual  bool isCalibTarget(const AQLString &ccy) const;

	AQLStaticData *mpStaticData; // property accessor
};

