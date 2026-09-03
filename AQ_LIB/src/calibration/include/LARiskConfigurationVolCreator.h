/*! @file
    @brief Volatility create class for calc risk
*/
//  2007, AlgoQuantHub.
#ifndef LARiskConfigurationVolCreator_h
#define LARiskConfigurationVolCreator_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfigurationVolCreator.h
//
//  DESCRIPTION :         Volatility create class for calc risk
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#endif

#include "LAString.h"
#include "LADataInstance.h"
#include "LAMathVolatility.h"
#include "LACoreDataService.h"
#include "LAStaticData.h"
#include "LAMarketData.h"
#include "LARiskConfiguration.h"


//===================== Class Declare LARiskConfigurationVolCreator==================================
/*! 
    @brief Volatility create class for calc risk
	
	this class is abstract

*/
class LARiskConfigurationVolCreator
{
public:
	// constructor
	explicit LARiskConfigurationVolCreator(bool funcFlg = true);

	// destructor
	virtual ~LARiskConfigurationVolCreator(void);
	// copy constructor
	LARiskConfigurationVolCreator(const LARiskConfigurationVolCreator &rhs);
	LARiskConfigurationVolCreator &operator=(const LARiskConfigurationVolCreator &rhs);

	//==============================================
	// create volatility object for parallel
	virtual LAMathVolatility *createVolatilityEntityForParallel(const LAString &riskName, const LAString &key, 
											LADataInstance &dataInstance, SCENARIONUM scenarioNum = SCENARIO_1) const;
	//==============================================
	// create volatility object for grid
	virtual std::vector<LAMathVolatility *> createVolatilityEntityForGrid(const LAString &riskName, const LAString &key, 
											LADataInstance &dataInstance, SCENARIONUM scenarioNum = SCENARIO_1) const;

protected:
	//==============================================
	// setup volatility method for parallel
	virtual void setUpVolatilityFuncForParallel(const LAString &riskName, const LAString &key, LADataInstance &dataInstance,
											LAMathVolatility &vol, SCENARIONUM scenarioNum = SCENARIO_1) const = 0;
	//==============================================
	// setup volatility data for grid
	virtual void setUpVolatilityDataForParallel(const LAString &riskName, const LAString &key, LADataInstance &dataInstance,
											LAMathVolatility &vol, SCENARIONUM scenarioNum = SCENARIO_1) const = 0;
	//==============================================
	// create volatility object for grid
	virtual std::vector<LAMathVolatility *> createVolatilityEntityFuncForGrid(const LAString &riskName, const LAString &key, 
											LADataInstance &dataInstance, SCENARIONUM scenarioNum = SCENARIO_1) const = 0;

	//==============================================
	// create volatility object for grid
	virtual std::vector<LAMathVolatility *> createVolatilityEntityDataForGrid(const LAString &riskName, const LAString &key, 
											LADataInstance &dataInstance, SCENARIONUM scenarioNum = SCENARIO_1) const = 0;
	LAStaticData *mpStaticData; // property accessor
	LAStaticData *mpRiskStaticData; // risk property accessor
	bool mFuncFlg; // vol method flg 

};


#endif
