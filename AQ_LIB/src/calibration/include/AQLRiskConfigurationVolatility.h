/*! @file
    @brief   Volatility setup class for calc risk
*/
#ifndef AQLRiskConfigurationVolatility_h
#define AQLRiskConfigurationVolatility_h
#ifdef __GNUG__
#pragma interface
#endif

#include "AQLRiskConfiguration.h"

//===================== Class Declare AQLRiskConfigurationVolatility==================================
/*! 
    @brief Volatility setup class for calc risk
	
	this class is abstract

*/
class AQLRiskConfigurationVolatility : public AQLRiskConfiguration
{
public:
	// constructor
	explicit AQLRiskConfigurationVolatility(bool fxFlg);
	// destructor
	virtual ~AQLRiskConfigurationVolatility(void);
	// copy constructor
	AQLRiskConfigurationVolatility(const AQLRiskConfigurationVolatility &rhs);
	AQLRiskConfigurationVolatility &operator=(const AQLRiskConfigurationVolatility &rhs);

protected:
	//==============================================
	// create scenario1 object 
	virtual  std::vector<AQLObject *> createScenario1Entity(const AQLString &key, AQLDataInstance &dataInstance, int index) const;
	//==============================================
	// create scenario2 object
	virtual  std::vector<AQLObject *> createScenario2Entity(const AQLString &key, AQLDataInstance &dataInstance, int index) const;
	//==============================================
	// create scenario volatility object
	virtual std::vector<AQLObject *> createVolatilityEntity(const AQLString &key, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum = SCENARIO_1, int index = 0) const = 0;
	//==============================================
	// create scenario volatility object old
	virtual std::vector<AQLObject *> createVolatilityEntityOld(const AQLString &key, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum = SCENARIO_1) const { return createVolatilityEntity(key, dataInstance, scenarioNum); }
	//==============================================
	// check target currency or fx
	virtual  bool isTarget(const AQLString &ccy) const;
};
#endif
