/*! @file
    @brief FX vega setup class
*/
//  2008, Mizuho International London.
#ifndef LARiskConfigurationVolFXVega_h
#define LARiskConfigurationVolFXVega_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfigurationVolFXVega.h
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

#include "LARiskConfigurationVolatility.h"



class LAString;
class LADataInstance;
class LARiskConfigurationVolCreatorLMM;
//===================== Class Declare LARiskConfigurationVolFXVega==================================
/*! 
    @brief FX Vega setup class
	

*/
class LARiskConfigurationVolFXVega : public LARiskConfigurationVolatility
{
public:
	// constructor
	explicit LARiskConfigurationVolFXVega(void);
	// destructor
	virtual ~LARiskConfigurationVolFXVega(void);
	// copy constructor
	LARiskConfigurationVolFXVega(const LARiskConfigurationVolFXVega &rhs);
	LARiskConfigurationVolFXVega &operator=(const LARiskConfigurationVolFXVega &rhs);

protected:
friend class LARiskConfigurationVolCreatorLMM;
	//==============================================
	// get  target names 
	virtual  LAString getTargetNames(const LAString &fx, LADataInstance &dataInstance)  const;	
	//==============================================
	// create scenario volatility object
	virtual std::vector<LAObject *> createVolatilityEntity(const LAString &fx, LADataInstance &dataInstance, SCENARIONUM scenarioNum = SCENARIO_1, int index = 0) const;
		//==============================================
	// create scenario volatility object
	virtual std::vector<LAObject *> createVolatilityEntityOld(const LAString &fx, LADataInstance &dataInstance, SCENARIONUM scenarioNum = SCENARIO_1) const;
	//==============================================
	// get outputname1
	virtual  LAString  getOutPutName1(const LAString &fx) const;
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
	// get scenario2 shift
	virtual double getScenario2ShiftValue(const LAString &fx) const;
	//==============================================
	// isRiskCurrencyMode
	virtual bool isRiskCurrencyMode(const LAString& fx) const;
	//==============================================
	// store forward FX and vol for risk print value
	virtual void storeFXAdditionalInfo(LAObjectPool &objPool, const MAScenarioParam& param) const;
};


#endif
