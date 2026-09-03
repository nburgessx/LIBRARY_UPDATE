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
	virtual AQLString getBaseOutPutName(const AQLString &ccy , int index) const;
	//==============================================
	// get base operator
	virtual  AQLString getBaseOperator() const;
	//==============================================
	// create basecoefficient
	virtual  AQLString getBaseCoefficient(const AQLString &ccy) const;
	//==============================================
	// get get FX shift vals
	virtual  DoubleArray  getBaseShiftVals(const AQLString &ccy) const;
	//==============================================
	// create basescenario object
	virtual std::vector<AQLObject *> createBaseScenarioEntity(const AQLString &ccy, AQLDataInstance &dataInstance, int index) const; 
	//==============================================
	// get target names 
	virtual  AQLString getTargetNames(const AQLString &fx, AQLDataInstance &dataInstance)  const;
	////==============================================
	// get outputname1
	virtual  AQLString  getOutPutName1(const AQLString &fx) const;
	////==============================================
	// get outputname1
	virtual  AQLString  getOutPutName1(const AQLString &ccy, int index) const;
	////==============================================
	// get risk name
	virtual  AQLString getRiskName(void) const;
	//==============================================
	// get bump direction
	virtual  AQLString getBumpDirection(const AQLString &fx) const;
	//==============================================
	// get shift type
	virtual  AQLString getShiftType(const AQLString &fx) const;
	//==============================================
	// get target currencies
	virtual  AQLString getTargetCurrencies() const;
	////==============================================
	// get calibration target currencies
	virtual  AQLString getCalibTargetCurrencies() const;
	//==============================================
	// get divid one unit
	virtual  double getDivUnit(const AQLString &fx) const;
	//==============================================
	// get shift value
	virtual double getShiftVal(const AQLString &fx, SCENARIONUM scenarioNum) const;
	//==============================================
	// get shift vals
	virtual double getBaseYieldVal(const AQLString &ccy, int index) const;
	//==============================================
	// get shift grid term
	virtual std::vector<AQLString> getShiftGridTerm(const AQLString &ccy) const;
	//  get cross base currency
	virtual  AQLString  getCrossBaseCurrency(const AQLString &ccy) const;
	//==============================================
	// create scenario1 object 
	virtual  std::vector<AQLObject *> createScenario1Entity(const AQLString &key, AQLDataInstance &dataInstance, int index) const;
	//==============================================
	// create scenario2 object
	virtual  std::vector<AQLObject *> createScenario2Entity(const AQLString &key, AQLDataInstance &dataInstance, int index) const;
	//==============================================
	// create scenario volatility object
	virtual std::vector<AQLObject *> createVolatilityEntity(const AQLString &ccy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum = SCENARIO_1, int index = 0) const;
	//==============================================
	// create scenario fx object
	virtual std::vector<AQLObject *> createFXEntity(const AQLString &fx, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const;
	//==============================================
	// isRiskCurrencyMode
	virtual bool isRiskCurrencyMode(const AQLString& fx) const;
	//==============================================
	// get shift vals(bp)
	virtual DoubleArray getBaseShifts(const AQLString &ccy, int index) const;
	//==============================================
	// get ir shift base currency
	virtual AQLString getIRBaseCurrency(const AQLString &ccy) const;
	//==============================================
	// get IR shift vals
	virtual  DoubleArray  getIRShiftVals(const AQLString &ccy) const;	
	//==============================================
	// create base extra scenario
	virtual std::vector<AQLObject *> createBaseExtraScenarioEntity(const AQLString &ccy, AQLDataInstance &dataInstance, int index) const;
	//==============================================
	// get base target names 
	virtual  AQLString getBaseTargetNames(const AQLString &ccy, AQLDataInstance &dataInstance)  const;
	//==============================================
	// get base extra target
	virtual AQLStringVector getBaseExtraTargetNames(const AQLString &ccy, AQLDataInstance &dataInstance) const;
	//==============================================
	// get extra target names1
	virtual  AQLStringVector getExtraTargetNames1(const AQLString &ccy, AQLDataInstance &dataInstance) const;
	//==============================================
	// get extra target names2
	virtual  AQLStringVector getExtraTargetNames2(const AQLString &ccy, AQLDataInstance &dataInstance) const;
	//==============================================
	// create extra scenario1 object 
	virtual std::vector<std::vector<AQLObject *> > createExtraScenario1Entity(const AQLString &ccy, AQLDataInstance &dataInstance, int index) const;
	//==============================================
	// create extra scenario2 object 
	virtual std::vector<std::vector<AQLObject *> > createExtraScenario2Entity(const AQLString &key, AQLDataInstance &dataInstance, int index) const;
};


#endif
