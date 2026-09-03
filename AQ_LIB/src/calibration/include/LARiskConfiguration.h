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

#include "AQLDataInstance.h"
#include "AQLString.h"
#include "AQLCoreTemplateType.h"
#include "LADefinitions.h"
#include "LADefinitionsRisk.h"
#include "LACoreDataService.h"

enum SCENARIONUM
{
	SCENARIO_BASE,
	SCENARIO_1,
	SCENARIO_2,
};


class AQLObjectPool;
class AQLObject;
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
	virtual void setUpRiskEntity(AQLDataInstance &dataInstance, bool isReflesh = true) const;
	// set curveType
	void setCurveType(const AQLString &ccy, const AQLString &curveType);
	// set basescenario target
	void setBaseScenarioTarget(const AQLString &ccy, const AQLString &target);
	// get curveType
	AQLString getCurveSuffix(const AQLString &ccy) const;
	// get base shift curveType
	virtual AQLString getBaseShiftCurveSuffix(const AQLString &ccy) const;

protected:
	//==============================================
	// setup target names 
	virtual  void setUpTargetNames(const AQLString &ccy, AQLObject &e, AQLDataInstance &dataInstance) const;	
	//==============================================
	// setup basescenario 
	virtual  void setUpBaseScenario(const AQLString &ccy, AQLObject &e, AQLDataInstance &dataInstance, int index) const;	
	//==============================================
	// setup scenario
	virtual  void setUpScenario(const AQLString &ccy, AQLObject &e, AQLDataInstance &dataInstance, int index) const;
	//==============================================
	// setup operator
	virtual  void setUpOperator(const AQLString &ccy, AQLObject &e) const;	
	//==============================================
	// setup coefficient
	virtual  void setUpCoefficient(const AQLString &ccy, AQLObject &e, AQLDataInstance& dataInstance) const;
	//==============================================
	// setup output name
	virtual  void setUpOutputName(const AQLString &ccy, AQLObject &e, int index) const;
	//==============================================
	// setup isgridsensitivity
	virtual  void setUpIsGridSensitivity(const AQLString &ccy, AQLObject &e) const;
		//==============================================
	// setup iswave
	virtual  void setUpIsWave(const AQLString &ccy, AQLObject &e) const;
	//==============================================
	// create risk object
	virtual  std::vector<std::pair<AQLString, std::vector<AQLObject *> > > createRiskEntity(AQLObjectPool &objPool) const;
	//==============================================
	// setup valuable entitty info
	virtual  void setUpValuableEntityInfo(const AQLString &ccy, AQLObjectPool &objPool,AQLObject &e, int num) const;
	// setup 
	virtual  void setUpRiskOutputCurrency(const AQLString &ccy, AQLObject &e) const;
	// isAnalytic
	virtual	void setUpIsAnalytic (const AQLString &ccy, AQLObject &e) const;
	//==============================================
	// setup deltatype
	virtual	void setUpDeltaType (const AQLString &ccy, AQLObject &e) const;
	//==============================================
	// setup vegatype
	virtual	void setUpVegaType(const AQLString &ccy, AQLObject &e) const;
	//==============================================
	// check target currency or fx
	virtual  bool isTarget(const AQLString &ccy) const;
	//==============================================
	// check calibrarion target currency or fx
	virtual  bool isCalibTarget(const AQLString &ccy) const;
	//==============================================
	// check target currencies
	virtual  AQLString getTargetCurrencies() const {  return "ALL"; }
	//==============================================
	// check calibration target currencies
	virtual  AQLString getCalibTargetCurrencies() const {  return "NONE"; }
	//==============================================
	// get target names 
	virtual  AQLString getTargetNames(const AQLString &ccy, AQLDataInstance &dataInstance)  const = 0;	
	//==============================================
	// create scenario1 object 
	virtual  std::vector<AQLObject *> createScenario1Entity(const AQLString &ccy, AQLDataInstance &dataInstance, int index) const = 0;
	//==============================================
	// create scenario2 object
	virtual  std::vector<AQLObject *> createScenario2Entity(const AQLString &ccy, AQLDataInstance &dataInstance, int index) const = 0;
	//==============================================
	// get operator1
	virtual  AQLString  getOperator1(void) const;	
	//==============================================
	// get operator2
	virtual  AQLString  getOperator2(void) const { return AQ_NO_DATA; };
	//==============================================
	// get coefficient1
	virtual  AQLString  getCoefficient1(const AQLString &ccy) const;
	//==============================================
	// get coefficient2
	virtual  AQLString  getCoefficient2(const AQLString &ccy) const { (void)ccy; return AQ_NO_DATA; };	
	//==============================================
	// get outputname1
	virtual  AQLString  getOutPutName1(const AQLString &ccy, int index) const { (void)index; return getOutPutName1(ccy);}
	//==============================================
	// get outputname2
	virtual  AQLString  getOutPutName2(const AQLString &ccy, int index) const { (void)index; return getOutPutName2(ccy);}
	//==============================================
	// get outputname1
	virtual  AQLString  getOutPutName1(const AQLString &ccy) const = 0;
	//==============================================
	// get outputname2
	virtual  AQLString  getOutPutName2(const AQLString &ccy) const { (void)ccy; return AQ_NO_DATA; };
	//==============================================
	// isgridsensitivity
	virtual  bool isGridSensitivity(const AQLString &ccy) const = 0;
	//==============================================
	// isparallelshift
	virtual  bool isParallelShift(const AQLString &ccy) const = 0;

	//==============================================
	// @Description: Tells IMM forward risk mode
	//  @param ccy [in] The currency
	//  @return 0 if no IMM-forward-risk-like care is applied,
	//          1 if market rate bump risk -> IMM forward risk conversion is applied,
	//          2 if market zero rate bump risk -> IMM forward risk conversion is applied;
	//          This class simply returns 0
	virtual int getIMMFwdRiskMode(const AQLString &ccy) const;

	//==============================================
	// get imm term
	virtual std::vector<int> getIMMTerm(const AQLString &ccy) const { return std::vector<int>(); }

	//==============================================
	// @Description: Returns (possibly term-wise) risk yield curve name(s) for IMM forward risks
	//  @param ccy [in] The currency
	//  @return An empty vector if all IMM forward risks shall be zero,
	//          a vector with one risk curve name if all risk curves are the same,
	//          a vector with risk curve names whose number is the same as the risk grids,
	//          where a risk curve name is the curve type which is suitable to be passed to
	//          LAMathYieldCurvePro::setCurveType (for example, "JPYOIS") to deduce the forward rate changes
	//          against the market rate bumps;
	//          This class simply returns a vector which contains one empty AQLString object
	virtual AQLStringVector getIMMRiskYieldCurveName(const AQLString &ccy) const;

	//==============================================
	// @Description: Returns the risk floor term, the date from which the first market rate bump risk is attributed to,
	//                that is, if this is "3Y" and the first market bump term is "4Y", then the first risk is attributed to
	//                the period [3Y, 4Y) instead of [baseDate, 4Y)
	//  @param ccy [in] The currency
	//  @arapm objPool [in] the object pool
	//  @return An empty AQLString if there is no such risk floor term, a appropriate term string otherwise;
	//         This class simply returns an empty AQLString object
	virtual AQLString getIMMRiskFloorTerm(const AQLString &ccy, AQLObjectPool &objPool) const;

	//==============================================
	// return risk name
	virtual  AQLString  getRiskName(void) const = 0;
	//==============================================
	// get deltatype
	virtual AQLString getDeltaType(const AQLString &ccy) const { (void)ccy; return AQ_NO_DATA; };
	//==============================================
	// get vegatype
	virtual AQLString getVegaType(const AQLString &ccy) const { (void)ccy; return AQ_NO_DATA; };
	//==============================================
	// get grid term
	virtual std::vector<AQLString> getGridTerm(const AQLString &ccy) const = 0;
	//==============================================
	// get grid term
	virtual std::vector<AQLString> getGridTerm(const AQLString &ccy, int index) const { (void)ccy, (void)index; return getGridTerm(ccy); }
	//==============================================
	// get bucket grid term
	virtual std::vector<AQLString> getBucketGridTerm(const AQLString &ccy) const = 0;
	//==============================================
	// get bucket grid term
	virtual std::vector<AQLString> getBucketGridTerm(const AQLString &ccy, int index) const { (void)ccy, (void)index; return getBucketGridTerm(ccy); }
	//==============================================
	// get gridRange term
	virtual std::vector<AQLString> getGridRangeTerm(const AQLString &ccy) const;
	//==============================================
	// get property bucket grid term
	virtual AQLString getPropertyBucketGridTerm(const AQLString &ccy) const { (void) ccy; return AQ_NO_DATA;};
	//==============================================
	// get waveoperator
	virtual  AQLString  getWaveOperator(void) const;	
	//==============================================
	// get wavecoefficient
	virtual  AQLString  getWaveCoefficient(const AQLString &ccy) const;
	//==============================================
	// get basewavecoefficient
	virtual  AQLString  getBaseWaveCoefficient(const AQLString &ccy) const;
	//==============================================
	// get is wave
	virtual  bool  isWave(const AQLString &ccy) const { (void)ccy; return false; }
	//==============================================
	// create basescenario object
	virtual std::vector<AQLObject *> createBaseScenarioEntity(const AQLString &ccy, AQLDataInstance &dataInstance, int index) const { (void)ccy, (void)dataInstance, (void)index; return std::vector<AQLObject *>(0); }
	//==============================================
	// get baseoperator
	virtual  AQLString  getBaseOperator(void) const;	
	//==============================================
	// get basecoefficient
	virtual  AQLString  getBaseCoefficient(const AQLString &ccy) const;
	//==============================================
	// get baseshift string
	virtual  AQLString  getBaseShiftStr(const AQLString &ccy, int index) const  { (void)ccy, (void)index; return AQ_NO_DATA; }
	//==============================================
	// get extra target names1
	virtual  AQLStringVector getExtraTargetNames1(const AQLString &ccy, AQLDataInstance &dataInstance) const { (void)ccy; (void)dataInstance; return AQLStringVector(0); }
	//==============================================
	// get extra target names2 
	virtual  AQLStringVector getExtraTargetNames2(const AQLString &ccy, AQLDataInstance &dataInstance) const  { (void)ccy; (void)dataInstance; return AQLStringVector(0); }
	//==============================================
	// get base extra target names 
	virtual  AQLStringVector getBaseExtraTargetNames(const AQLString &ccy, AQLDataInstance &dataInstance) const  { (void)ccy; (void)dataInstance; return AQLStringVector(0); }
	//==============================================
	// create extraScenario1 object 
	virtual  std::vector<std::vector<AQLObject *> > createExtraScenario1Entity(const AQLString &ccy, AQLDataInstance &dataInstance, int index) const { (void)ccy, (void)dataInstance, (void)index; return std::vector<std::vector<AQLObject *> >(0); }
	//==============================================
	// create extraScenario2 object
	virtual  std::vector<std::vector<AQLObject *> > createExtraScenario2Entity(const AQLString &ccy, AQLDataInstance &dataInstance, int index) const { (void)ccy, (void)dataInstance, (void)index; return std::vector<std::vector<AQLObject *> >(0); }
	//==============================================
	// create base extraScenario object 
	virtual  std::vector<AQLObject *> createBaseExtraScenarioEntity(const AQLString &ccy, AQLDataInstance &dataInstance, int index) const { (void)ccy, (void)dataInstance, (void)index; return std::vector<AQLObject *>(0); }
	//==============================================
	// get baseoutputname
	virtual  AQLString  getBaseOutPutName(const AQLString &ccy, int index) const  { (void)ccy, (void)index; return AQ_NO_DATA; }
	//==============================================
	// convert str to bool
	bool convertBoolFromStr(const AQLString &str) const;
	//==============================================
	// check calibration target
	virtual  AQLStringVector getCalibTargetFX(const AQLString &ccy, AQLDataInstance &dataInstance) const;
	//==============================================
	// get bump direction
	virtual  AQLString getBumpDirection(const AQLString &ccy) const {(void)ccy; return RISK_BUMPDIRECTION_UPSHIFT; }
	//==============================================
	// get divid one unit
	virtual  double getDivUnit(const AQLString &ccy) const { (void)ccy; return 1.0; }
	//==============================================
	// get max  grid index 
	virtual  int getMaxGridIndex(const AQLString &ccy)  const { return static_cast<int>(getGridTerm(ccy).size() - 1);}
	//==============================================
	// check is real calib 
	bool isRealCalib()  const;
	//==============================================
	// get cross base currency
	virtual  AQLString  getCrossBaseCurrency(const AQLString &ccy) const{(void)ccy; return AQ_NO_DATA;}
	//============================================== 
	// get get FX shift vals
	virtual  DoubleArray  getBaseShiftVals(const AQLString &ccy) const {(void)ccy; return DoubleArray(1, 0.0);};
	//==============================================
	// get base shift val
	virtual  double getBaseSpotVal(const AQLString &ccy, int index) const { (void)ccy;(void)index; return 0.0;}
	//==============================================
	// get base shift val
	virtual  double getBaseVolVal(const AQLString &ccy, int index) const { (void)ccy;(void)index; return 0.0;}
	////==============================================
	// get shift yield vals
	virtual double getBaseYieldVal(const AQLString &ccy, int index) const { (void)ccy;(void)index; return 0.0;}
	//==============================================
	// get shift grid term
	virtual std::vector<AQLString> getShiftGridTerm(const AQLString &ccy) const {(void)ccy; return std::vector<AQLString>();};
	////==============================================
	// get is adjst df
	virtual bool isAdjustDf(const AQLString &ccy) const;
	////==============================================
	// convert string vector to double rate vector 
	DoubleArray convertToRateValues(const AQLStringVector &vec) const;
	//==============================================
	// create option analytic object
	std::vector<AQLObject *> createOptionAnalyticEntity(const AQLString &ccy, AQLDataInstance &dataInstance, int index)  const;
	//==============================================
	// create option analytic risk object
	std::vector<std::pair<AQLString, std::vector<AQLObject *> > > createOptionAnalyticRiskEntity(AQLObjectPool &objPool) const;
	//==============================================
	// create optin analytic target names
	void setUpOptionAnalyticTargetNames(const AQLString &ccy, AQLObject &e) const;
	//==============================================
	// get shift type
	virtual  AQLString  getShiftType(const AQLString &fx) const {(void)fx; return AQ_NO_DATA;};
	//==============================================
	// getShiftValForOptionAnalytic
	virtual double getShiftValForOptionAnalytic(const AQLString &fx) const {(void)fx;return 0.0;};
	//==============================================
	// getAnalyticalRiskType
	virtual AQLString getAnalyticalRiskType(const AQLString& fx, AQLObject& e) const;
	// IsSucceedAnalyticalRiskType
	virtual bool IsSucceedAnalyticalRiskType(const AQLString& ccy, AQLObject& e) const;
	// get coefficient ratio1
	virtual double getCoefficientRatio1(const AQLString& ccy, AQLObject& e, AQLDataInstance& dataInstance) const {(void)ccy;(void)e;(void)dataInstance; return 1.0;};
	// get coefficient ratio2
	virtual double getCoefficientRatio2(const AQLString& ccy, AQLObject& e, AQLDataInstance& dataInstance) const {(void)ccy;(void)e;(void)dataInstance; return 1.0;};
	// get curveType
	AQLString getCurveType(const AQLString &ccy) const;
	// get base chift curveType
	virtual AQLString getBaseShiftCurveType(const AQLString &ccy) const;
	// isSavePastFixing
	virtual bool isSavePastFixing() const { return true; };
	// getRiksCurrencys 
	virtual AQLStringVector getRiskCurrencys(AQLObjectPool& objPool) const;
	// isRiskCurrencyMode
	virtual bool isRiskCurrencyMode(const AQLString& ccy) const { (void)ccy; return false; };
	// isAnalyticMode
	virtual bool isAnalyticMode(const AQLString& ccy) const { (void)ccy; return false; };
	// getScenarioBaseYieldName
	virtual AQLString getScenarioBaseYieldName(const AQLString& ccy) const;
	// get scenario1 grid shift
	virtual DoubleArray getScenario1GridShift(const AQLString &ccy) const {(void) ccy; return DoubleArray(0);};
	// get base target names 
	virtual  AQLString getBaseTargetNames(const AQLString &ccy, AQLDataInstance &dataInstance)  const { return getTargetNames(ccy, dataInstance); };	
	// create ir base scenario object
	virtual std::vector<AQLObject *> createIRBaseScenarioEntity(const AQLString &ccy, AQLDataInstance &dataInstance, int index) const;
	// create ir base extra scenario object
	virtual std::vector<AQLObject *> createIRBaseExtraScenarioEntity(const AQLString &ccy, AQLDataInstance &dataInstance, int index) const;
	// create foreign yield object in base scenario 
	virtual std::vector<AQLObject *> createBaseForeignYieldEntity(const AQLString &ccy, const AQLString &fCcy, AQLDataInstance &dataInstance, int index, const bool isFirst = true)  const;
	// create collateral yield object in base scenario 
	virtual std::vector<AQLObject *> createBaseCollateralYieldEntity(const AQLString &ccy, const AQLString &fCcy, AQLDataInstance &dataInstance, int index)  const;
	// create foreign vol object in base scenario 
	virtual std::vector<AQLObject *> createBaseForeignIRVolEntity(const AQLString& ccy, const AQLString& fCcy, AQLDataInstance &dataInstance, int index) const;
	// create ir base scenario object
	virtual std::vector<AQLObject *> createBasisBaseScenarioEntity(const AQLString &ccy, AQLDataInstance &dataInstance, int index) const;
	// get ir shift extra target names 
	virtual AQLStringVector getIRBaseExtraTargetNames(const AQLString &ccy, AQLDataInstance &dataInstance) const;
	// get shift vals(bp)
	virtual DoubleArray getBaseShifts(const AQLString &ccy, int index) const;
	// get ir shift vals
	virtual DoubleArray getIRBaseShifts(const AQLString &ccy, int index) const;
	// get ir shift vals string
	virtual AQLString getIRBaseShiftStr(const AQLString &ccy, int index) const;
	// get ir shift vals 
	virtual DoubleArray getIRShiftVals(const AQLString &ccy) const {(void)ccy; return DoubleArray(0);};
	// get ir shift base currency
	virtual AQLString getIRBaseCurrency(const AQLString &ccy) const { return ccy;};
	// get base calc type
	virtual AQLString getBaseCalcType(const AQLString &ccy, int index) const;
	// get base extra calc type
	virtual AQLString getBaseExtraCalcType(const AQLString &ccy, int index) const;
	// check isBasescenario Target
	bool isBaseScenarioTarget(const AQLString &ccy, int index) const;
	// set scenario param references 
	void setScenarioParamReferences(const AQLString& targetCcy, const AQLStringVector& ccys, const AQLStringVector& fCurveCcys, AQLObjectPool &objPool, AQLStringVector& refName, 
		const AQLString& sceName, const AQLString& calcType, /*const AQLString* targetCurveType, */const AQLString& suffix, const int idx) const;
	//==============================================
	// create scenario fx object
	virtual std::vector<AQLObject *> createBaseFXEntity(const AQLString &fx, AQLDataInstance &dataInstance, int index) const;
	//==============================================
	// create scenario fx object
	virtual std::vector<AQLObject *> createBaseFXVolEntity(const AQLString &fx, AQLDataInstance &dataInstance, int index) const;
	//==============================================
	// create ir base scenario object
	virtual std::vector<AQLObject *> createFXBaseScenarioEntity(const AQLString &ccy, AQLDataInstance &dataInstance, int index) const;
	//==============================================
	// create ir base extra scenario object
	virtual std::vector<AQLObject *> createFXBaseExtraScenarioEntity(const AQLString &ccy, AQLDataInstance &dataInstance, int index) const;
	//==============================================
	// get base target names 
	virtual  AQLString getFXBaseTargetNames(const AQLString &ccy, AQLDataInstance &dataInstance)  const;	
	//==============================================
	// get base extra target names 
	virtual  AQLStringVector getFXBaseExtraTargetNames(const AQLString &ccy, AQLDataInstance &dataInstance) const;
	//==============================================
	// get basis type
	virtual  AQLString getBasisType(const AQLString &ccy) const{ (void)ccy; return AQ_NO_DATA; };
	//==============================================
	// get calc type
	virtual AQLString getCalcType(const AQLString &ccy, SCENARIONUM scenarioNum, int index) const;
	//==============================================
	// get extra calc type
	virtual AQLString getExtraCalcType(const AQLString &ccy, SCENARIONUM scenarioNum, int index) const;
	//==============================================
	// get shift value
	virtual double getShiftVal(const AQLString &fx, SCENARIONUM scenarioNum) const { (void)fx, (void)scenarioNum; return 0.0; };
	//==============================================
	// set fx object params
	virtual void setFXEntityParams(const AQLString &ccy, const AQLString &fx, AQLDataInstance &dataInstance, const SCENARIONUM scenarioNum, int index, MAScenarioParam &param) const;
	//==============================================
	// set fx volatility object params
	virtual void setFXVolEntityParams(const AQLString &ccy, const AQLString &fx, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index, MAScenarioParam &param, bool isYield) const;

	virtual bool omitNotionalExposure(const AQLString &ccy) const { return false; };

	mutable AQLStringVector mBaseSceNames;  // base scenario object names
	mutable AQLStringVector mSce1Names;   // scenario1 object names
	mutable AQLStringVector mSce2Names;   // scenario2 object names
	mutable AQLStringVector mExtraBaseSceNames;   // extra base scenario object names
	mutable std::vector<AQLObject* > mBaseEntity; // < object name
	LAStaticData *mpStaticData; // property accessor
	LAStaticData *mpCalibStaticData; // calib property accessor
	LAStaticData *mpRiskStaticData; // risk property accessor
	bool mFXFlg;    // fx flg
	mutable AQLString mAnalyticRiskType;
	mutable std::map<AQLString, AQLString> mCurveTypeMap;  // curvetype target map
	mutable std::map<AQLString, AQLString> mBaseTargetMap; // base scenario target map
	AQLString mAnalyticCalcType; //ANALYTIC or SEMIANALYTIC
};


#endif
