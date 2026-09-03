/*! @file
    @brief FX vega setup class
*/
//  2008, AlgoQuantHub.
#ifndef AQLRiskConfigurationVolFXVega_h
#define AQLRiskConfigurationVolFXVega_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLRiskConfigurationVolFXVega.h
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

#include "AQLRiskConfigurationVolatility.h"



class AQLString;
class AQLDataInstance;
class AQLRiskConfigurationVolCreatorLMM;
//===================== Class Declare AQLRiskConfigurationVolFXVega==================================
/*! 
    @brief FX Vega setup class
	

*/
class AQLRiskConfigurationVolFXVega : public AQLRiskConfigurationVolatility
{
public:
	// constructor
	explicit AQLRiskConfigurationVolFXVega(void);
	// destructor
	virtual ~AQLRiskConfigurationVolFXVega(void);
	// copy constructor
	AQLRiskConfigurationVolFXVega(const AQLRiskConfigurationVolFXVega &rhs);
	AQLRiskConfigurationVolFXVega &operator=(const AQLRiskConfigurationVolFXVega &rhs);

protected:
friend class AQLRiskConfigurationVolCreatorLMM;
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
	virtual void storeFXAdditionalInfo(AQLObjectPool &objPool, const AQLScenarioParam& param) const;
};


#endif
