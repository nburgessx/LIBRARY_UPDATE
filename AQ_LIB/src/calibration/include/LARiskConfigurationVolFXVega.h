/*! @file
    @brief FX vega setup class
*/
//  2008, AlgoQuantHub.
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



class AQLString;
class AQLDataInstance;
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
	virtual  AQLString getTargetNames(const AQLString &fx, AQLDataInstance &dataInstance)  const;	
	//==============================================
	// create scenario volatility object
	virtual std::vector<AQLObject *> createVolatilityEntity(const AQLString &fx, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum = SCENARIO_1, int index = 0) const;
		//==============================================
	// create scenario volatility object
	virtual std::vector<AQLObject *> createVolatilityEntityOld(const AQLString &fx, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum = SCENARIO_1) const;
	//==============================================
	// get outputname1
	virtual  AQLString  getOutPutName1(const AQLString &fx) const;
	//==============================================
	// get shift type
	virtual  AQLString  getShiftType(const AQLString &fx) const;
	//==============================================
	// get bump direction
	virtual  AQLString  getBumpDirection(const AQLString &fx) const;
	//==============================================
	// get risk name
	virtual  AQLString getRiskName(void) const;
	//==============================================
	// check target currencies
	virtual  AQLString getTargetCurrencies() const;
	//==============================================
	// isgridsensitivity
	virtual  bool  isGridSensitivity(const AQLString &fx) const;
	//==============================================
	// isparallelshift
	virtual  bool isParallelShift(const AQLString &fx) const;
	//==============================================
	// get deltatype
	virtual AQLString getDeltaType(const AQLString &ccy) const;
	//==============================================
	// get grid term
	virtual std::vector<AQLString> getGridTerm(const AQLString &fx) const;
	//==============================================
	// get bucket grid term
	virtual std::vector<AQLString> getBucketGridTerm(const AQLString &fx) const;
	//==============================================
	// get property bucket grid term
	virtual AQLString getPropertyBucketGridTerm(const AQLString &fx) const;
	//==============================================
	// get divunit
	virtual double getDivUnit(const AQLString &fx) const;
	//==============================================
	// get is wave
	virtual  bool  isWave(const AQLString &fx) const;
	//==============================================
	// get scenario1 shift
	virtual double getScenario1ShiftValue(const AQLString &fx) const;
	//==============================================
	// get scenario2 shift
	virtual double getScenario2ShiftValue(const AQLString &fx) const;
	//==============================================
	// isRiskCurrencyMode
	virtual bool isRiskCurrencyMode(const AQLString& fx) const;
	//==============================================
	// store forward FX and vol for risk print value
	virtual void storeFXAdditionalInfo(AQLObjectPool &objPool, const MAScenarioParam& param) const;
};


#endif
