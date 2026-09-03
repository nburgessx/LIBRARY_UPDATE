/*! @file
    @brief  YieldShift FX shift delta setup class
*/
//  2008, AlgoQuantHub.
#ifndef LARiskConfigurationYieldIRShiftFXDelta_h
#define LARiskConfigurationYieldIRShiftFXDelta_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfigurationYieldIRShiftFXDelta.h
//
//  DESCRIPTION :         FX delta setup class
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#endif

#include "LARiskConfigurationFXDeltaEx1.h"


//===================== Class Declare LARiskConfigurationYieldIRShiftFXDelta==================================
/*! 
    @brief  FX delta setup class

*/
class LARiskConfigurationYieldIRShiftFXDelta : public LARiskConfigurationFXDeltaEx1
{
public:
	// constructor
	explicit LARiskConfigurationYieldIRShiftFXDelta();
	// destructor
	virtual ~LARiskConfigurationYieldIRShiftFXDelta(void);
	// copy constructor
	LARiskConfigurationYieldIRShiftFXDelta(const LARiskConfigurationYieldIRShiftFXDelta &rhs);
	LARiskConfigurationYieldIRShiftFXDelta &operator=(const LARiskConfigurationYieldIRShiftFXDelta &rhs);

protected:
	//==============================================
	// get base outputname
	virtual LAString getBaseOutPutName(const LAString &ccy , int index) const;
	//==============================================
	// get base operator
	virtual  LAString getBaseOperator() const;
	//==============================================
	// create basecoefficient
	virtual  LAString getBaseCoefficient(const LAString &ccy) const;
	//==============================================
	// get get FX shift vals
	virtual  DoubleArray  getBaseShiftVals(const LAString &ccy) const;
	//==============================================
	// create basescenario object
	virtual std::vector<LAObject *> createBaseScenarioEntity(const LAString &ccy, LADataInstance &dataInstance, int index) const; 
	//==============================================
	// get target names 
	virtual  LAString getTargetNames(const LAString &fx, LADataInstance &dataInstance)  const;
	////==============================================
	// get outputname1
	virtual  LAString  getOutPutName1(const LAString &fx) const;
	////==============================================
	// get outputname1
	virtual  LAString  getOutPutName1(const LAString &ccy, int index) const;
	////==============================================
	// get risk name
	virtual  LAString getRiskName(void) const;
	//==============================================
	// get bump direction
	virtual  LAString getBumpDirection(const LAString &fx) const;
	//==============================================
	// get shift type
	virtual  LAString getShiftType(const LAString &fx) const;
	//==============================================
	// get target currencies
	virtual  LAString getTargetCurrencies() const;
	////==============================================
	// get calibration target currencies
	virtual  LAString getCalibTargetCurrencies() const;
	//==============================================
	// get divid one unit
	virtual  double getDivUnit(const LAString &fx) const;
	//==============================================
	// get shift value
	virtual double getShiftVal(const LAString &fx, SCENARIONUM scenarioNum) const;
	//==============================================
	// get shift vals
	virtual double getBaseYieldVal(const LAString &ccy, int index) const;
	//==============================================
	// get shift grid term
	virtual std::vector<LAString> getShiftGridTerm(const LAString &ccy) const;
	//  get cross base currency
	virtual  LAString  getCrossBaseCurrency(const LAString &ccy) const;
	//==============================================
	// create scenario1 object 
	virtual  std::vector<LAObject *> createScenario1Entity(const LAString &key, LADataInstance &dataInstance, int index) const;
	//==============================================
	// create scenario2 object
	virtual  std::vector<LAObject *> createScenario2Entity(const LAString &key, LADataInstance &dataInstance, int index) const;
	//==============================================
	// create scenario volatility object
	virtual std::vector<LAObject *> createVolatilityEntity(const LAString &ccy, LADataInstance &dataInstance, SCENARIONUM scenarioNum = SCENARIO_1, int index = 0) const;
	//==============================================
	// create scenario fx object
	virtual std::vector<LAObject *> createFXEntity(const LAString &fx, LADataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const;
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
	// create base extra scenario
	virtual std::vector<LAObject *> createBaseExtraScenarioEntity(const LAString &ccy, LADataInstance &dataInstance, int index) const;
	//==============================================
	// get base target names 
	virtual  LAString getBaseTargetNames(const LAString &ccy, LADataInstance &dataInstance)  const;
	//==============================================
	// get base extra target
	virtual LAStringVector getBaseExtraTargetNames(const LAString &ccy, LADataInstance &dataInstance) const;
	//==============================================
	// get extra target names1
	virtual  LAStringVector getExtraTargetNames1(const LAString &ccy, LADataInstance &dataInstance) const;
	//==============================================
	// get extra target names2
	virtual  LAStringVector getExtraTargetNames2(const LAString &ccy, LADataInstance &dataInstance) const;
	//==============================================
	// create extra scenario1 object 
	virtual std::vector<std::vector<LAObject *> > createExtraScenario1Entity(const LAString &ccy, LADataInstance &dataInstance, int index) const;
	//==============================================
	// create extra scenario2 object 
	virtual std::vector<std::vector<LAObject *> > createExtraScenario2Entity(const LAString &key, LADataInstance &dataInstance, int index) const;
};


#endif
