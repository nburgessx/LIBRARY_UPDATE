/*! @file
    @brief FX vega setup class
*/
//  2008, AlgoQuantHub.
#ifndef LARiskConfigurationYieldIRShiftVolFXVega_h
#define LARiskConfigurationYieldIRShiftVolFXVega_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfigurationYieldIRShiftVolFXVega.h
//
//  DESCRIPTION :        FX Vega setup class 
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#endif

#include "LARiskConfigurationVolFXVega.h"



class LAString;
class LADataInstance;
class LARiskConfigurationVolCreatorLMM;
//===================== Class Declare LARiskConfigurationYieldIRShiftVolFXVega==================================
/*! 
    @brief FX Vega setup class
	

*/
class LARiskConfigurationYieldIRShiftVolFXVega : public LARiskConfigurationVolFXVega
{
public:
	// constructor
	explicit LARiskConfigurationYieldIRShiftVolFXVega(void);
	// destructor
	virtual ~LARiskConfigurationYieldIRShiftVolFXVega(void);
	// copy constructor
	LARiskConfigurationYieldIRShiftVolFXVega(const LARiskConfigurationYieldIRShiftVolFXVega &rhs);
	LARiskConfigurationYieldIRShiftVolFXVega &operator=(const LARiskConfigurationYieldIRShiftVolFXVega &rhs);

protected:
friend class LARiskConfigurationVolCreatorLMM;
	//==============================================
	// create scenario volatility object
	virtual std::vector<LAObject *> createVolatilityEntity(const LAString &fx, LADataInstance &dataInstance, SCENARIONUM scenarioNum = SCENARIO_1, int index = 0) const;
	//==============================================
	// get outputname1
	virtual  LAString  getOutPutName1(const LAString &fx) const;
	////==============================================
	// get outputname1
	virtual  LAString  getOutPutName1(const LAString &ccy, int index) const;
	//==============================================
	// get shift vals
	virtual double getBaseYieldVal(const LAString &ccy, int index) const;
	////==============================================
	// get get FX shift vals
	virtual  DoubleArray  getBaseShiftVals(const LAString &ccy) const;
	//==============================================
	// get shift type
	virtual  LAString  getShiftType(const LAString &fx) const;
	//==============================================
	// get bump direction
	virtual  LAString  getBumpDirection(const LAString &fx) const;
	//==============================================
	// get risk name
	virtual  LAString getRiskName(void) const;
	//==============================================
	// check target currencies
	virtual  LAString getTargetCurrencies() const;
	//==============================================
	// check calib target Currencies
	virtual  LAString getCalibTargetCurrencies() const;
	//==============================================
	// isgridsensitivity
	virtual  bool  isGridSensitivity(const LAString &fx) const;
	//==============================================
	// isparallelshift
	virtual  bool isParallelShift(const LAString &fx) const;
	//==============================================
	// get deltatype
	virtual LAString getDeltaType(const LAString &ccy) const;
	//==============================================
	// get grid term
	virtual std::vector<LAString> getGridTerm(const LAString &fx) const;
	//==============================================
	// get bucket grid term
	virtual std::vector<LAString> getBucketGridTerm(const LAString &fx) const;
	//==============================================
	// get property bucket grid term
	virtual LAString getPropertyBucketGridTerm(const LAString &fx) const;
	//==============================================
	// get divunit
	virtual double getDivUnit(const LAString &fx) const;
	//==============================================
	// get is wave
	virtual  bool  isWave(const LAString &fx) const;
	//==============================================
	// get scenario1 shift
	virtual double getScenario1ShiftValue(const LAString &fx) const;
	//==============================================
	//  get cross base currency
	virtual  LAString  getCrossBaseCurrency(const LAString &ccy) const;
	//==============================================
	// create basescenario object
	virtual std::vector<LAObject *> createBaseScenarioEntity(const LAString &ccy, LADataInstance &dataInstance, int index) const;
	//==============================================
	// get baseoperator
	virtual  LAString  getBaseOperator(void) const;	
	//==============================================
	// get basecoefficient
	virtual  LAString  getBaseCoefficient(const LAString &ccy) const;
	//==============================================
	// get baseoutputname
	virtual  LAString  getBaseOutPutName(const LAString &ccy, int index) const;
	//==============================================
	// get shift grid term
	virtual std::vector<LAString> getShiftGridTerm(const LAString &ccy) const;
	//==============================================
	// isRiskCurrencyMode
	virtual bool isRiskCurrencyMode(const LAString& fx) const;
	//==============================================
	// get shift vals(bp)
	virtual DoubleArray getBaseShifts(const LAString &ccy, int index) const;
	//==============================================
	// get ir shift base currency
	virtual LAString getIRBaseCurrency(const LAString &ccy) const;
	//==============================================
	// get IR shift vals
	virtual  DoubleArray  getIRShiftVals(const LAString &ccy) const;	
	//==============================================
	// get base target names 
	virtual  LAString getBaseTargetNames(const LAString &ccy, LADataInstance &dataInstance)  const;
	//==============================================
	// create base extraScenario object 
	virtual  std::vector<LAObject *> createBaseExtraScenarioEntity(const LAString &ccy, LADataInstance &dataInstance, int index) const;
	//==============================================
	// get base extra target
	virtual LAStringVector getBaseExtraTargetNames(const LAString &ccy, LADataInstance &dataInstance) const;
	//==============================================
	// create scenario1 object 
	virtual  std::vector<LAObject *> createScenario1Entity(const LAString &key, LADataInstance &dataInstance, int index) const;
	//==============================================
	// create scenario2 object 
	virtual  std::vector<LAObject *> createScenario2Entity(const LAString &key, LADataInstance &dataInstance, int index) const;
};


#endif
