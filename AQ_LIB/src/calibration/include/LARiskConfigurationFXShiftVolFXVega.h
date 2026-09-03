/*! @file
    @brief FX vega setup class
*/
//  2008, AlgoQuantHub.
#ifndef LARiskConfigurationFXShiftVolFXVega_h
#define LARiskConfigurationFXShiftVolFXVega_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfigurationFXShiftVolFXVega.h
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
//===================== Class Declare LARiskConfigurationFXShiftVolFXVega==================================
/*! 
    @brief FX Vega setup class
	

*/
class LARiskConfigurationFXShiftVolFXVega : public LARiskConfigurationVolFXVega
{
public:
	// constructor
	explicit LARiskConfigurationFXShiftVolFXVega(void);
	// destructor
	virtual ~LARiskConfigurationFXShiftVolFXVega(void);
	// copy constructor
	LARiskConfigurationFXShiftVolFXVega(const LARiskConfigurationFXShiftVolFXVega &rhs);
	LARiskConfigurationFXShiftVolFXVega &operator=(const LARiskConfigurationFXShiftVolFXVega &rhs);

protected:
	//==============================================
	// create scenario volatility object
	virtual std::vector<LAObject *> createVolatilityEntity(const LAString &fx, LADataInstance &dataInstance, SCENARIONUM scenarioNum = SCENARIO_1, int index = 0) const;
	//==============================================
	// get outputname1
	virtual  LAString  getOutPutName1(const LAString &fx) const;
	//==============================================
	// get outputname1
	virtual  LAString  getOutPutName1(const LAString &ccy, int index) const;
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
	// create basescenario object
	virtual std::vector<LAObject *> createBaseScenarioEntity(const LAString &ccy, LADataInstance &dataInstance, int index) const;
	//==============================================
	// get get FX shift vals
	virtual  DoubleArray  getBaseShiftVals(const LAString &ccy) const;
	//==============================================
	// get shift vals
	virtual double getBaseSpotVal(const LAString &ccy, int index) const;
	//==============================================
	// get base extra target names 
	virtual  LAStringVector getBaseExtraTargetNames(const LAString &ccy, LADataInstance &dataInstance) const;
	//==============================================
	// create base extraScenario object 
	virtual  std::vector<LAObject *> createBaseExtraScenarioEntity(const LAString &ccy, LADataInstance &dataInstance, int index) const;
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
	// isRiskCurrencyMode
	virtual bool isRiskCurrencyMode(const LAString& fx) const;
	
	
};


#endif
