/*! @file
    @brief Volatility create class for calc risk
*/
#ifndef AQLRiskConfigurationVolCreator_h
#define AQLRiskConfigurationVolCreator_h
#ifdef __GNUG__
#pragma interface
#endif

#include "AQLString.h"
#include "AQLDataInstance.h"
#include "AQLMathVolatility.h"
#include "AQLCoreDataService.h"
#include "AQLStaticData.h"
#include "AQLMarketData.h"
#include "AQLRiskConfiguration.h"


//===================== Class Declare AQLRiskConfigurationVolCreator==================================
/*! 
    @brief Volatility create class for calc risk
	
	this class is abstract

*/
class AQLRiskConfigurationVolCreator
{
public:
	// constructor
	explicit AQLRiskConfigurationVolCreator(bool funcFlg = true);

	// destructor
	virtual ~AQLRiskConfigurationVolCreator(void);
	// copy constructor
	AQLRiskConfigurationVolCreator(const AQLRiskConfigurationVolCreator &rhs);
	AQLRiskConfigurationVolCreator &operator=(const AQLRiskConfigurationVolCreator &rhs);

	//==============================================
	// create volatility object for parallel
	virtual AQLMathVolatility *createVolatilityEntityForParallel(const AQLString &riskName, const AQLString &key, 
											AQLDataInstance &dataInstance, SCENARIONUM scenarioNum = SCENARIO_1) const;
	//==============================================
	// create volatility object for grid
	virtual std::vector<AQLMathVolatility *> createVolatilityEntityForGrid(const AQLString &riskName, const AQLString &key, 
											AQLDataInstance &dataInstance, SCENARIONUM scenarioNum = SCENARIO_1) const;

protected:
	//==============================================
	// setup volatility method for parallel
	virtual void setUpVolatilityFuncForParallel(const AQLString &riskName, const AQLString &key, AQLDataInstance &dataInstance,
											AQLMathVolatility &vol, SCENARIONUM scenarioNum = SCENARIO_1) const = 0;
	//==============================================
	// setup volatility data for grid
	virtual void setUpVolatilityDataForParallel(const AQLString &riskName, const AQLString &key, AQLDataInstance &dataInstance,
											AQLMathVolatility &vol, SCENARIONUM scenarioNum = SCENARIO_1) const = 0;
	//==============================================
	// create volatility object for grid
	virtual std::vector<AQLMathVolatility *> createVolatilityEntityFuncForGrid(const AQLString &riskName, const AQLString &key, 
											AQLDataInstance &dataInstance, SCENARIONUM scenarioNum = SCENARIO_1) const = 0;

	//==============================================
	// create volatility object for grid
	virtual std::vector<AQLMathVolatility *> createVolatilityEntityDataForGrid(const AQLString &riskName, const AQLString &key, 
											AQLDataInstance &dataInstance, SCENARIONUM scenarioNum = SCENARIO_1) const = 0;
	AQLStaticData *mpStaticData; // property accessor
	AQLStaticData *mpRiskStaticData; // risk property accessor
	bool mFuncFlg; // vol method flg 

};


#endif
