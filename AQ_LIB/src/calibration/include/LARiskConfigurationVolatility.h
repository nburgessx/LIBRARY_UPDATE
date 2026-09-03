/*! @file
    @brief   Volatility setup class for calc risk
*/
//  2007, AlgoQuantHub.
#ifndef LARiskConfigurationVolatility_h
#define LARiskConfigurationVolatility_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfigurationVolatility.h
//
//  DESCRIPTION :         Volatility setup class for calc risk
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#endif

#include "LARiskConfiguration.h"

//===================== Class Declare LARiskConfigurationVolatility==================================
/*! 
    @brief Volatility setup class for calc risk
	
	this class is abstract

*/
class LARiskConfigurationVolatility : public LARiskConfiguration
{
public:
	// constructor
	explicit LARiskConfigurationVolatility(bool fxFlg);
	// destructor
	virtual ~LARiskConfigurationVolatility(void);
	// copy constructor
	LARiskConfigurationVolatility(const LARiskConfigurationVolatility &rhs);
	LARiskConfigurationVolatility &operator=(const LARiskConfigurationVolatility &rhs);

protected:
	//==============================================
	// create scenario1 object 
	virtual  std::vector<LAObject *> createScenario1Entity(const LAString &key, LADataInstance &dataInstance, int index) const;
	//==============================================
	// create scenario2 object
	virtual  std::vector<LAObject *> createScenario2Entity(const LAString &key, LADataInstance &dataInstance, int index) const;
	//==============================================
	// create scenario volatility object
	virtual std::vector<LAObject *> createVolatilityEntity(const LAString &key, LADataInstance &dataInstance, SCENARIONUM scenarioNum = SCENARIO_1, int index = 0) const = 0;
	//==============================================
	// create scenario volatility object old
	virtual std::vector<LAObject *> createVolatilityEntityOld(const LAString &key, LADataInstance &dataInstance, SCENARIONUM scenarioNum = SCENARIO_1) const { return createVolatilityEntity(key, dataInstance, scenarioNum); }
	//==============================================
	// check target currency or fx
	virtual  bool isTarget(const LAString &ccy) const;
};
#endif
