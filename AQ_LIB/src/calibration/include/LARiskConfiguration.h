/*! @file
    @brief Risk setup class
*/
//  2007, AlgoQuantHub.
#ifndef LARiskConfiguration_h
#define LARiskConfiguration_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfiguration.h
//
//  DESCRIPTION :        Risk setup class 
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#endif

#include "LADataInstance.h"
#include "LAString.h"
#include "LACoreTemplateType.h"
#include "LADefinitions.h"
#include "LADefinitionsRisk.h"
#include "LACoreDataService.h"

enum SCENARIONUM
{
	SCENARIO_BASE,
	SCENARIO_1,
	SCENARIO_2,
};


class LAObjectPool;
class LAObject;
class LARatesSDEBase;
class LARatesCurveLogLinearInterpolation;
class LAMathCorrelation;
class LAStaticData;
struct MAScenarioParam;

//===================== Class Declare LARiskConfiguration==================================
/*! 
    @brief Risk setup class
	
	this class is abstract

*/
class LARiskConfiguration
{
public:
	// constructor
	explicit LARiskConfiguration(bool fxFlg);
	// destructor
	virtual ~LARiskConfiguration(void);
	// copy constructor
	LARiskConfiguration(const LARiskConfiguration &rhs);
	LARiskConfiguration &operator=(const LARiskConfiguration &rhs);

	//==============================================
	// setup risk object
	virtual void setUpRiskEntity(LADataInstance &dataInstance, bool isReflesh = true) const;
	// set curveType
	void setCurveType(const LAString &ccy, const LAString &curveType);
	// set basescenario target
	void setBaseScenarioTarget(const LAString &ccy, const LAString &target);
	// get curveType
	LAString getCurveSuffix(const LAString &ccy) const;
	// get base shift curveType
	virtual LAString getBaseShiftCurveSuffix(const LAString &ccy) const;

protected:
	//==============================================
	// setup target names 
	virtual  void setUpTargetNames(const LAString &ccy, LAObject &e, LADataInstance &dataInstance) const;	
	//==============================================
	// setup basescenario 
	virtual  void setUpBaseScenario(const LAString &ccy, LAObject &e, LADataInstance &dataInstance, int index) const;	
	//==============================================
	// setup scenario
	virtual  void setUpScenario(const LAString &ccy, LAObject &e, LADataInstance &dataInstance, int index) const;
	//==============================================
	// setup operator
	virtual  void setUpOperator(const LAString &ccy, LAObject &e) const;	
	//==============================================
	// setup coefficient
	virtual  void setUpCoefficient(const LAString &ccy, LAObject &e, LADataInstance& dataInstance) const;
	//==============================================
	// setup output name
	virtual  void setUpOutputName(const LAString &ccy, LAObject &e, int index) const;
	//==============================================
	// setup isgridsensitivity
	virtual  void setUpIsGridSensitivity(const LAString &ccy, LAObject &e) const;
		//==============================================
	// setup iswave
	virtual  void setUpIsWave(const LAString &ccy, LAObject &e) const;
	//==============================================
	// create risk object
	virtual  std::vector<std::pair<LAString, std::vector<LAObject *> > > createRiskEntity(LAObjectPool &objPool) const;
	//==============================================
	// setup valuable entitty info
	virtual  void setUpValuableEntityInfo(const LAString &ccy, LAObjectPool &objPool,LAObject &e, int num) const;
	// setup 
	virtual  void setUpRiskOutputCurrency(const LAString &ccy, LAObject &e) const;
	// isAnalytic
	virtual	void setUpIsAnalytic (const LAString &ccy, LAObject &e) const;
	//==============================================
	// setup deltatype
	virtual	void setUpDeltaType (const LAString &ccy, LAObject &e) const;
	//==============================================
	// setup vegatype
	virtual	void setUpVegaType(const LAString &ccy, LAObject &e) const;
	//==============================================
	// check target currency or fx
	virtual  bool isTarget(const LAString &ccy) const;
	//==============================================
	// check calibrarion target currency or fx
	virtual  bool isCalibTarget(const LAString &ccy) const;
	//==============================================
	// check target currencies
	virtual  LAString getTargetCurrencies() const {  return "ALL"; }
	//==============================================
	// check calibration target currencies
	virtual  LAString getCalibTargetCurrencies() const {  return "NONE"; }
	//==============================================
	// get target names 
	virtual  LAString getTargetNames(const LAString &ccy, LADataInstance &dataInstance)  const = 0;	
	//==============================================
	// create scenario1 object 
	virtual  std::vector<LAObject *> createScenario1Entity(const LAString &ccy, LADataInstance &dataInstance, int index) const = 0;
	//==============================================
	// create scenario2 object
	virtual  std::vector<LAObject *> createScenario2Entity(const LAString &ccy, LADataInstance &dataInstance, int index) const = 0;
	//==============================================
	// get operator1
	virtual  LAString  getOperator1(void) const;	
	//==============================================
	// get operator2
	virtual  LAString  getOperator2(void) const { return AQ_NO_DATA; };
	//==============================================
	// get coefficient1
	virtual  LAString  getCoefficient1(const LAString &ccy) const;
	//==============================================
	// get coefficient2
	virtual  LAString  getCoefficient2(const LAString &ccy) const { (void)ccy; return AQ_NO_DATA; };	
	//==============================================
	// get outputname1
	virtual  LAString  getOutPutName1(const LAString &ccy, int index) const { (void)index; return getOutPutName1(ccy);}
	//==============================================
	// get outputname2
	virtual  LAString  getOutPutName2(const LAString &ccy, int index) const { (void)index; return getOutPutName2(ccy);}
	//==============================================
	// get outputname1
	virtual  LAString  getOutPutName1(const LAString &ccy) const = 0;
	//==============================================
	// get outputname2
	virtual  LAString  getOutPutName2(const LAString &ccy) const { (void)ccy; return AQ_NO_DATA; };
	//==============================================
	// isgridsensitivity
	virtual  bool isGridSensitivity(const LAString &ccy) const = 0;
	//==============================================
	// isparallelshift
	virtual  bool isParallelShift(const LAString &ccy) const = 0;

	//==============================================
	// @Description: Tells IMM forward risk mode
	//  @param ccy [in] The currency
	//  @return 0 if no IMM-forward-risk-like care is applied,
	//          1 if market rate bump risk -> IMM forward risk conversion is applied,
	//          2 if market zero rate bump risk -> IMM forward risk conversion is applied;
	//          This class simply returns 0
	virtual int getIMMFwdRiskMode(const LAString &ccy) const;

	//==============================================
	// get imm term
	virtual std::vector<int> getIMMTerm(const LAString &ccy) const { return std::vector<int>(); }

	//==============================================
	// @Description: Returns (possibly term-wise) risk yield curve name(s) for IMM forward risks
	//  @param ccy [in] The currency
	//  @return An empty vector if all IMM forward risks shall be zero,
	//          a vector with one risk curve name if all risk curves are the same,
	//          a vector with risk curve names whose number is the same as the risk grids,
	//          where a risk curve name is the curve type which is suitable to be passed to
	//          LAMathYieldCurvePro::setCurveType (for example, "JPYOIS") to deduce the forward rate changes
	//          against the market rate bumps;
	//          This class simply returns a vector which contains one empty LAString object
	virtual LAStringVector getIMMRiskYieldCurveName(const LAString &ccy) const;

	//==============================================
	// @Description: Returns the risk floor term, the date from which the first market rate bump risk is attributed to,
	//                that is, if this is "3Y" and the first market bump term is "4Y", then the first risk is attributed to
	//                the period [3Y, 4Y) instead of [baseDate, 4Y)
	//  @param ccy [in] The currency
	//  @arapm objPool [in] the object pool
	//  @return An empty LAString if there is no such risk floor term, a appropriate term string otherwise;
	//         This class simply returns an empty LAString object
	virtual LAString getIMMRiskFloorTerm(const LAString &ccy, LAObjectPool &objPool) const;

	//==============================================
	// return risk name
	virtual  LAString  getRiskName(void) const = 0;
	//==============================================
	// get deltatype
	virtual LAString getDeltaType(const LAString &ccy) const { (void)ccy; return AQ_NO_DATA; };
	//==============================================
	// get vegatype
	virtual LAString getVegaType(const LAString &ccy) const { (void)ccy; return AQ_NO_DATA; };
	//==============================================
	// get grid term
	virtual std::vector<LAString> getGridTerm(const LAString &ccy) const = 0;
	//==============================================
	// get grid term
	virtual std::vector<LAString> getGridTerm(const LAString &ccy, int index) const { (void)ccy, (void)index; return getGridTerm(ccy); }
	//==============================================
	// get bucket grid term
	virtual std::vector<LAString> getBucketGridTerm(const LAString &ccy) const = 0;
	//==============================================
	// get bucket grid term
	virtual std::vector<LAString> getBucketGridTerm(const LAString &ccy, int index) const { (void)ccy, (void)index; return getBucketGridTerm(ccy); }
	//==============================================
	// get gridRange term
	virtual std::vector<LAString> getGridRangeTerm(const LAString &ccy) const;
	//==============================================
	// get property bucket grid term
	virtual LAString getPropertyBucketGridTerm(const LAString &ccy) const { (void) ccy; return AQ_NO_DATA;};
	//==============================================
	// get waveoperator
	virtual  LAString  getWaveOperator(void) const;	
	//==============================================
	// get wavecoefficient
	virtual  LAString  getWaveCoefficient(const LAString &ccy) const;
	//==============================================
	// get basewavecoefficient
	virtual  LAString  getBaseWaveCoefficient(const LAString &ccy) const;
	//==============================================
	// get is wave
	virtual  bool  isWave(const LAString &ccy) const { (void)ccy; return false; }
	//==============================================
	// create basescenario object
	virtual std::vector<LAObject *> createBaseScenarioEntity(const LAString &ccy, LADataInstance &dataInstance, int index) const { (void)ccy, (void)dataInstance, (void)index; return std::vector<LAObject *>(0); }
	//==============================================
	// get baseoperator
	virtual  LAString  getBaseOperator(void) const;	
	//==============================================
	// get basecoefficient
	virtual  LAString  getBaseCoefficient(const LAString &ccy) const;
	//==============================================
	// get baseshift string
	virtual  LAString  getBaseShiftStr(const LAString &ccy, int index) const  { (void)ccy, (void)index; return AQ_NO_DATA; }
	//==============================================
	// get extra target names1
	virtual  LAStringVector getExtraTargetNames1(const LAString &ccy, LADataInstance &dataInstance) const { (void)ccy; (void)dataInstance; return LAStringVector(0); }
	//==============================================
	// get extra target names2 
	virtual  LAStringVector getExtraTargetNames2(const LAString &ccy, LADataInstance &dataInstance) const  { (void)ccy; (void)dataInstance; return LAStringVector(0); }
	//==============================================
	// get base extra target names 
	virtual  LAStringVector getBaseExtraTargetNames(const LAString &ccy, LADataInstance &dataInstance) const  { (void)ccy; (void)dataInstance; return LAStringVector(0); }
	//==============================================
	// create extraScenario1 object 
	virtual  std::vector<std::vector<LAObject *> > createExtraScenario1Entity(const LAString &ccy, LADataInstance &dataInstance, int index) const { (void)ccy, (void)dataInstance, (void)index; return std::vector<std::vector<LAObject *> >(0); }
	//==============================================
	// create extraScenario2 object
	virtual  std::vector<std::vector<LAObject *> > createExtraScenario2Entity(const LAString &ccy, LADataInstance &dataInstance, int index) const { (void)ccy, (void)dataInstance, (void)index; return std::vector<std::vector<LAObject *> >(0); }
	//==============================================
	// create base extraScenario object 
	virtual  std::vector<LAObject *> createBaseExtraScenarioEntity(const LAString &ccy, LADataInstance &dataInstance, int index) const { (void)ccy, (void)dataInstance, (void)index; return std::vector<LAObject *>(0); }
	//==============================================
	// get baseoutputname
	virtual  LAString  getBaseOutPutName(const LAString &ccy, int index) const  { (void)ccy, (void)index; return AQ_NO_DATA; }
	//==============================================
	// convert str to bool
	bool convertBoolFromStr(const LAString &str) const;
	//==============================================
	// check calibration target
	virtual  LAStringVector getCalibTargetFX(const LAString &ccy, LADataInstance &dataInstance) const;
	//==============================================
	// get bump direction
	virtual  LAString getBumpDirection(const LAString &ccy) const {(void)ccy; return RISK_BUMPDIRECTION_UPSHIFT; }
	//==============================================
	// get divid one unit
	virtual  double getDivUnit(const LAString &ccy) const { (void)ccy; return 1.0; }
	//==============================================
	// get max  grid index 
	virtual  int getMaxGridIndex(const LAString &ccy)  const { return static_cast<int>(getGridTerm(ccy).size() - 1);}
	//==============================================
	// check is real calib 
	bool isRealCalib()  const;
	//==============================================
	// get cross base currency
	virtual  LAString  getCrossBaseCurrency(const LAString &ccy) const{(void)ccy; return AQ_NO_DATA;}
	//============================================== 
	// get get FX shift vals
	virtual  DoubleArray  getBaseShiftVals(const LAString &ccy) const {(void)ccy; return DoubleArray(1, 0.0);};
	//==============================================
	// get base shift val
	virtual  double getBaseSpotVal(const LAString &ccy, int index) const { (void)ccy;(void)index; return 0.0;}
	//==============================================
	// get base shift val
	virtual  double getBaseVolVal(const LAString &ccy, int index) const { (void)ccy;(void)index; return 0.0;}
	////==============================================
	// get shift yield vals
	virtual double getBaseYieldVal(const LAString &ccy, int index) const { (void)ccy;(void)index; return 0.0;}
	//==============================================
	// get shift grid term
	virtual std::vector<LAString> getShiftGridTerm(const LAString &ccy) const {(void)ccy; return std::vector<LAString>();};
	////==============================================
	// get is adjst df
	virtual bool isAdjustDf(const LAString &ccy) const;
	////==============================================
	// convert string vector to double rate vector 
	DoubleArray convertToRateValues(const LAStringVector &vec) const;
	//==============================================
	// create option analytic object
	std::vector<LAObject *> createOptionAnalyticEntity(const LAString &ccy, LADataInstance &dataInstance, int index)  const;
	//==============================================
	// create option analytic risk object
	std::vector<std::pair<LAString, std::vector<LAObject *> > > createOptionAnalyticRiskEntity(LAObjectPool &objPool) const;
	//==============================================
	// create optin analytic target names
	void setUpOptionAnalyticTargetNames(const LAString &ccy, LAObject &e) const;
	//==============================================
	// get shift type
	virtual  LAString  getShiftType(const LAString &fx) const {(void)fx; return AQ_NO_DATA;};
	//==============================================
	// getShiftValForOptionAnalytic
	virtual double getShiftValForOptionAnalytic(const LAString &fx) const {(void)fx;return 0.0;};
	//==============================================
	// getAnalyticalRiskType
	virtual LAString getAnalyticalRiskType(const LAString& fx, LAObject& e) const;
	// IsSucceedAnalyticalRiskType
	virtual bool IsSucceedAnalyticalRiskType(const LAString& ccy, LAObject& e) const;
	// get coefficient ratio1
	virtual double getCoefficientRatio1(const LAString& ccy, LAObject& e, LADataInstance& dataInstance) const {(void)ccy;(void)e;(void)dataInstance; return 1.0;};
	// get coefficient ratio2
	virtual double getCoefficientRatio2(const LAString& ccy, LAObject& e, LADataInstance& dataInstance) const {(void)ccy;(void)e;(void)dataInstance; return 1.0;};
	// get curveType
	LAString getCurveType(const LAString &ccy) const;
	// get base chift curveType
	virtual LAString getBaseShiftCurveType(const LAString &ccy) const;
	// isSavePastFixing
	virtual bool isSavePastFixing() const { return true; };
	// getRiksCurrencys 
	virtual LAStringVector getRiskCurrencys(LAObjectPool& objPool) const;
	// isRiskCurrencyMode
	virtual bool isRiskCurrencyMode(const LAString& ccy) const { (void)ccy; return false; };
	// isAnalyticMode
	virtual bool isAnalyticMode(const LAString& ccy) const { (void)ccy; return false; };
	// getScenarioBaseYieldName
	virtual LAString getScenarioBaseYieldName(const LAString& ccy) const;
	// get scenario1 grid shift
	virtual DoubleArray getScenario1GridShift(const LAString &ccy) const {(void) ccy; return DoubleArray(0);};
	// get base target names 
	virtual  LAString getBaseTargetNames(const LAString &ccy, LADataInstance &dataInstance)  const { return getTargetNames(ccy, dataInstance); };	
	// create ir base scenario object
	virtual std::vector<LAObject *> createIRBaseScenarioEntity(const LAString &ccy, LADataInstance &dataInstance, int index) const;
	// create ir base extra scenario object
	virtual std::vector<LAObject *> createIRBaseExtraScenarioEntity(const LAString &ccy, LADataInstance &dataInstance, int index) const;
	// create foreign yield object in base scenario 
	virtual std::vector<LAObject *> createBaseForeignYieldEntity(const LAString &ccy, const LAString &fCcy, LADataInstance &dataInstance, int index, const bool isFirst = true)  const;
	// create collateral yield object in base scenario 
	virtual std::vector<LAObject *> createBaseCollateralYieldEntity(const LAString &ccy, const LAString &fCcy, LADataInstance &dataInstance, int index)  const;
	// create foreign vol object in base scenario 
	virtual std::vector<LAObject *> createBaseForeignIRVolEntity(const LAString& ccy, const LAString& fCcy, LADataInstance &dataInstance, int index) const;
	// create ir base scenario object
	virtual std::vector<LAObject *> createBasisBaseScenarioEntity(const LAString &ccy, LADataInstance &dataInstance, int index) const;
	// get ir shift extra target names 
	virtual LAStringVector getIRBaseExtraTargetNames(const LAString &ccy, LADataInstance &dataInstance) const;
	// get shift vals(bp)
	virtual DoubleArray getBaseShifts(const LAString &ccy, int index) const;
	// get ir shift vals
	virtual DoubleArray getIRBaseShifts(const LAString &ccy, int index) const;
	// get ir shift vals string
	virtual LAString getIRBaseShiftStr(const LAString &ccy, int index) const;
	// get ir shift vals 
	virtual DoubleArray getIRShiftVals(const LAString &ccy) const {(void)ccy; return DoubleArray(0);};
	// get ir shift base currency
	virtual LAString getIRBaseCurrency(const LAString &ccy) const { return ccy;};
	// get base calc type
	virtual LAString getBaseCalcType(const LAString &ccy, int index) const;
	// get base extra calc type
	virtual LAString getBaseExtraCalcType(const LAString &ccy, int index) const;
	// check isBasescenario Target
	bool isBaseScenarioTarget(const LAString &ccy, int index) const;
	// set scenario param references 
	void setScenarioParamReferences(const LAString& targetCcy, const LAStringVector& ccys, const LAStringVector& fCurveCcys, LAObjectPool &objPool, LAStringVector& refName, 
		const LAString& sceName, const LAString& calcType, /*const LAString* targetCurveType, */const LAString& suffix, const int idx) const;
	//==============================================
	// create scenario fx object
	virtual std::vector<LAObject *> createBaseFXEntity(const LAString &fx, LADataInstance &dataInstance, int index) const;
	//==============================================
	// create scenario fx object
	virtual std::vector<LAObject *> createBaseFXVolEntity(const LAString &fx, LADataInstance &dataInstance, int index) const;
	//==============================================
	// create ir base scenario object
	virtual std::vector<LAObject *> createFXBaseScenarioEntity(const LAString &ccy, LADataInstance &dataInstance, int index) const;
	//==============================================
	// create ir base extra scenario object
	virtual std::vector<LAObject *> createFXBaseExtraScenarioEntity(const LAString &ccy, LADataInstance &dataInstance, int index) const;
	//==============================================
	// get base target names 
	virtual  LAString getFXBaseTargetNames(const LAString &ccy, LADataInstance &dataInstance)  const;	
	//==============================================
	// get base extra target names 
	virtual  LAStringVector getFXBaseExtraTargetNames(const LAString &ccy, LADataInstance &dataInstance) const;
	//==============================================
	// get basis type
	virtual  LAString getBasisType(const LAString &ccy) const{ (void)ccy; return AQ_NO_DATA; };
	//==============================================
	// get calc type
	virtual LAString getCalcType(const LAString &ccy, SCENARIONUM scenarioNum, int index) const;
	//==============================================
	// get extra calc type
	virtual LAString getExtraCalcType(const LAString &ccy, SCENARIONUM scenarioNum, int index) const;
	//==============================================
	// get shift value
	virtual double getShiftVal(const LAString &fx, SCENARIONUM scenarioNum) const { (void)fx, (void)scenarioNum; return 0.0; };
	//==============================================
	// set fx object params
	virtual void setFXEntityParams(const LAString &ccy, const LAString &fx, LADataInstance &dataInstance, const SCENARIONUM scenarioNum, int index, MAScenarioParam &param) const;
	//==============================================
	// set fx volatility object params
	virtual void setFXVolEntityParams(const LAString &ccy, const LAString &fx, LADataInstance &dataInstance, SCENARIONUM scenarioNum, int index, MAScenarioParam &param, bool isYield) const;

	virtual bool omitNotionalExposure(const LAString &ccy) const { return false; };

	mutable LAStringVector mBaseSceNames;  // base scenario object names
	mutable LAStringVector mSce1Names;   // scenario1 object names
	mutable LAStringVector mSce2Names;   // scenario2 object names
	mutable LAStringVector mExtraBaseSceNames;   // extra base scenario object names
	mutable std::vector<LAObject* > mBaseEntity; // < object name
	LAStaticData *mpStaticData; // property accessor
	LAStaticData *mpCalibStaticData; // calib property accessor
	LAStaticData *mpRiskStaticData; // risk property accessor
	bool mFXFlg;    // fx flg
	mutable LAString mAnalyticRiskType;
	mutable std::map<LAString, LAString> mCurveTypeMap;  // curvetype target map
	mutable std::map<LAString, LAString> mBaseTargetMap; // base scenario target map
	LAString mAnalyticCalcType; //ANALYTIC or SEMIANALYTIC
};


#endif
