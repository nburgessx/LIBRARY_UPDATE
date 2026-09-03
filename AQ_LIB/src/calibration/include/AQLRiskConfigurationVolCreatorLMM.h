/*! @file
    @brief LMM Volatility create class for calc risk
*/
//  2007, AlgoQuantHub.
#ifndef AQLRiskConfigurationVolCreatorLMM_h
#define AQLRiskConfigurationVolCreatorLMM_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLRiskConfigurationVolCreatorLMM.h
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

#include "AQLString.h"
#include "AQLDataInstance.h"
#include "AQLMathVolatility.h"
#include "AQLRiskConfigurationVolCreator.h"
#include "AQLMarketData.h"
#include "AQLMarketDataLMM.h"
#include "AQLDefinitionsRisk.h"
#include "AQLDefinitionsLMM.h"


//===================== Class Declare AQLRiskConfigurationVolCreatorLMM==================================
/*! 
    @brief LMM Volatility create class for calc risk
	

*/
class AQLRiskConfigurationVolCreatorLMM : public AQLRiskConfigurationVolCreator
{
public:
	// constructor
	explicit AQLRiskConfigurationVolCreatorLMM(bool funcFlg = true);
	// destructor
	virtual ~AQLRiskConfigurationVolCreatorLMM(void);
	// copy constructor
	AQLRiskConfigurationVolCreatorLMM(const AQLRiskConfigurationVolCreatorLMM &rhs);
	AQLRiskConfigurationVolCreatorLMM &operator=(const AQLRiskConfigurationVolCreatorLMM &rhs);


protected :
	//==============================================
	// setup volatility method 
	virtual void setUpVolatilityFuncForParallel(const AQLString &riskName, const AQLString &ccy, AQLDataInstance &dataInstance,
											AQLMathVolatility &vol, SCENARIONUM scenarioNum = SCENARIO_1) const;
	//==============================================
	// setup volatility data 
	virtual void setUpVolatilityDataForParallel(const AQLString &riskName, const AQLString &ccy, AQLDataInstance &dataInstance,
											AQLMathVolatility &vol, SCENARIONUM scenarioNum = SCENARIO_1) const;
	//==============================================
	// create volatility object for grid
	virtual std::vector<AQLMathVolatility *> createVolatilityEntityFuncForGrid
					(const AQLString &riskName, const AQLString &ccy, 
					AQLDataInstance &dataInstance, SCENARIONUM scenarioNum = SCENARIO_1) const;

	//==============================================
	// create volatility object for grid
	virtual std::vector<AQLMathVolatility *> createVolatilityEntityDataForGrid
					(const AQLString &riskName, const AQLString &ccy, 
					AQLDataInstance &dataInstance, SCENARIONUM scenarioNum = SCENARIO_1) const;

	//==============================================
	// get calibfile path 
	virtual AQLString getCalibFilePath(const AQLString &riskName, const AQLString &ccy, SCENARIONUM scenarioNum = SCENARIO_1) const;
	//==============================================
	// get adjfile path
	virtual AQLString getAdjFilePath(const AQLString &riskName, const AQLString &ccy, SCENARIONUM scenarioNum = SCENARIO_1) const;
private :
	//==============================================
	// get grid info
	void getGridInfo(DoubleArray &tenor_30_360, DoubleArray &tenor, DoubleArray &deltatenor) const;

};


#endif
