/*! @file
    @brief  sde generator class
*/
//  2007, Mizuho International London.
#ifndef LACalibrateModel_h
#define LACalibrateModel_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LACalibrateModel.h
//
//  DESCRIPTION :        SDE generator 
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#endif

//#include "LAString.h"
#include "LARatesSDEBase.h"


class LAString;
class LARatesSDEBase;
class LADataInstance;
class LARatesCurveLogLinearInterpolation;
class LAMathCorrelation;
class LAMathVolatility;
class LAStaticData;

//===================== Class Declare LACalibrateModel==================================
/*! 
    @brief SDE generator 
	
	this class is abstract

*/
class LACalibrateModel
{
public:
	// constructor
	explicit LACalibrateModel(void);
	// destructor
	virtual ~LACalibrateModel(void);
	// copy constructor
	LACalibrateModel(const LACalibrateModel &rhs);
	LACalibrateModel &operator=(const LACalibrateModel &rhs);

	//==============================================
	// generate SDE
	virtual void generateSDE(const LAString &key, LADataInstance &dataInstance,const bool isMarketCreate = true, const bool isFirst = true) const;
	//==============================================
	// generate SDE market data
	virtual void loadModelDataAndCalibrate(const LAString &key, LADataInstance &dataInstance, const bool isCurve = true, const bool isModel = true, const LAString & curveID = "", const LAString & marketName = "") const = 0;

#ifndef VISUAL_STUDIO_2010_ANALYTICS
	//==============================================
	// generate sde volatility
	virtual void loadVolatilityDataAndCalibrate(const LAString &key, LADataInstance &dataInstance) const;
#endif
	//==============================================
	// generate sde initial value for fwdfx constant curve
	virtual void loadFwdFXConstCurveDataAndCalibrate(const LAString &currency, LADataInstance &dataInstance, bool isCalcFwdBeforeFwdFXConsant = false) const = 0;
	//==============================================
	// is fwdfx const curve
	bool isFwdFXConst(const LAString& ccy) const;
	//==============================================
	// is collateral ccy
	bool isCollateral(const LAString& ccy) const;

protected:
	//==============================================
	// get sde Type
	virtual  SDE_TYPE getSDEType(const LAString &key) const = 0;
	//==============================================
	// check LJ
	virtual bool isLJ(const LAString &key) const = 0;
	//==============================================
	// set volatility 
	virtual  void setVolatility(const LAString &key, LARatesSDEBase &sde)  const = 0;	
	//==============================================
	// set drift 
	virtual  void setDrift(const LAString &key, LARatesSDEBase &sde)  const = 0;
	//==============================================
	// set numeraire 
	virtual  void setNumeraire(const LAString &key, LARatesSDEBase &sde)  const = 0;
	//==============================================
	// set path 
	virtual  void setOutputTemplate(const LAString &key, LARatesSDEBase &sde)  const  = 0;
	//==============================================
	// set integralfunction 
	virtual  void setIntegralFunction(const LAString &key, LARatesSDEBase &sde)  const = 0;
	//==============================================
	// set interpolation method 
	virtual  void setInterpolationMethod(const LAString &key, LARatesSDEBase &sde) const = 0;
	//==============================================
	// get function master regist name 
	virtual LAString getFunctionMasterResistName(const LAString &key) const = 0;
	//==============================================
	// get create sde instance 
	virtual LARatesSDEBase *createSDEInstance(const LAString &key, LADataInstance &dataInstance) const = 0;
	//==============================================
	// setup vol type
	virtual LAString getVolType(const LAString &key) const = 0;
	//==============================================
	// setup vol data
	virtual void setUpVolData(const LAString &key, LAMathVolatility &vol, LADataInstance &dataInstance) const = 0;
	//==============================================
	// setup vol method
	virtual void setUpVolFunc(const LAString &key,LAMathVolatility &vol, LADataInstance &dataInstance) const = 0;
	//==============================================
	// get sde function name
	virtual LAString getSDEAttrName(const LAString &key) const = 0;
#ifndef VISUAL_STUDIO_2010_ANALYTICS
	//==============================================
	// check cancel target for funding change 
	virtual  bool isCancelForFunding(const LAString &ccy) const;
	//==============================================
	// check calibrarion target currency or fx
	virtual  bool isCalibTarget(const LAString &ccy) const;
#endif

	

	LAStaticData *mpStaticData; // property accessor
};
#endif
