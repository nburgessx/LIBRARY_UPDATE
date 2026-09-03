/*! @file
    @brief LMM Volatility create class for calc risk
*/
//  2007, Mizuho International London.
#ifndef LARiskConfigurationVolCreatorLMM_h
#define LARiskConfigurationVolCreatorLMM_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfigurationVolCreatorLMM.h
//
//  DESCRIPTION :       LMM Volatility create class for calc risk
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
#include "LARiskConfigurationVolCreator.h"
#include "LAMarketData.h"
#include "LAMarketDataLMM.h"
#include "LADefinitionsRisk.h"
#include "LADefinitionsLMM.h"


//===================== Class Declare LARiskConfigurationVolCreatorLMM==================================
/*! 
    @brief LMM Volatility create class for calc risk
	

*/
class LARiskConfigurationVolCreatorLMM : public LARiskConfigurationVolCreator
{
public:
	// constructor
	explicit LARiskConfigurationVolCreatorLMM(bool funcFlg = true);
	// destructor
	virtual ~LARiskConfigurationVolCreatorLMM(void);
	// copy constructor
	LARiskConfigurationVolCreatorLMM(const LARiskConfigurationVolCreatorLMM &rhs);
	LARiskConfigurationVolCreatorLMM &operator=(const LARiskConfigurationVolCreatorLMM &rhs);


protected :
	//==============================================
	// setup volatility method 
	virtual void setUpVolatilityFuncForParallel(const LAString &riskName, const LAString &ccy, LADataInstance &dataInstance,
											LAMathVolatility &vol, SCENARIONUM scenarioNum = SCENARIO_1) const;
	//==============================================
	// setup volatility data 
	virtual void setUpVolatilityDataForParallel(const LAString &riskName, const LAString &ccy, LADataInstance &dataInstance,
											LAMathVolatility &vol, SCENARIONUM scenarioNum = SCENARIO_1) const;
	//==============================================
	// create volatility object for grid
	virtual std::vector<LAMathVolatility *> createVolatilityEntityFuncForGrid
					(const LAString &riskName, const LAString &ccy, 
					LADataInstance &dataInstance, SCENARIONUM scenarioNum = SCENARIO_1) const;

	//==============================================
	// create volatility object for grid
	virtual std::vector<LAMathVolatility *> createVolatilityEntityDataForGrid
					(const LAString &riskName, const LAString &ccy, 
					LADataInstance &dataInstance, SCENARIONUM scenarioNum = SCENARIO_1) const;

	//==============================================
	// get calibfile path 
	virtual LAString getCalibFilePath(const LAString &riskName, const LAString &ccy, SCENARIONUM scenarioNum = SCENARIO_1) const;
	//==============================================
	// get adjfile path
	virtual LAString getAdjFilePath(const LAString &riskName, const LAString &ccy, SCENARIONUM scenarioNum = SCENARIO_1) const;
private :
	//==============================================
	// get grid info
	void getGridInfo(DoubleArray &tenor_30_360, DoubleArray &tenor, DoubleArray &deltatenor) const;

};


#endif
