/*! @file
    @brief Ptberg(fx) Volatility create class for calc risk 
*/
#ifndef AQLRiskConfigurationVolCreatorPtberg_h
#define AQLRiskConfigurationVolCreatorPtberg_h
#ifdef __GNUG__
#pragma interface
#endif

#include "AQLString.h"
#include "AQLDataInstance.h"
#include "AQLMathVolatility.h"
#include "AQLRiskConfigurationVolCreator.h"
#include "AQLMarketData.h"
#include "AQLPtbergUtils.h"
#include "AQLDefinitionsRisk.h"
#include "AQLDefinitionsPtberg.h"


//===================== Class Declare AQLRiskConfigurationVolCreatorPtberg==================================
/*! 
    @brief Ptberg(fx) Volatility create class for calc risk 
	

*/
class AQLRiskConfigurationVolCreatorPtberg : public AQLRiskConfigurationVolCreator
{
public:
	// constructor
	explicit AQLRiskConfigurationVolCreatorPtberg(bool funcFlg = true);
	// destructor
	virtual ~AQLRiskConfigurationVolCreatorPtberg(void);
	// copy constructor
	AQLRiskConfigurationVolCreatorPtberg(const AQLRiskConfigurationVolCreatorPtberg &rhs);
	AQLRiskConfigurationVolCreatorPtberg &operator=(const AQLRiskConfigurationVolCreatorPtberg &rhs);

protected :
	//==============================================
	// setup volatility method for parallel
	virtual void setUpVolatilityFuncForParallel(const AQLString &riskName, const AQLString &ccy, AQLDataInstance &dataInstance,
											AQLMathVolatility &vol, SCENARIONUM scenarioNum = SCENARIO_1) const;
	//==============================================
	// setup volatility data for parallel
	virtual void setUpVolatilityDataForParallel(const AQLString &riskName, const AQLString &ccy, AQLDataInstance &dataInstance,
											AQLMathVolatility &vol, SCENARIONUM scenarioNum = SCENARIO_1) const;
	//==============================================
	// create volatility object for grid
	virtual std::vector<AQLMathVolatility *> createVolatilityEntityFuncForGrid
					(const AQLString &riskName, const AQLString &key, 
					AQLDataInstance &dataInstance, SCENARIONUM scenarioNum = SCENARIO_1) const { riskName, key, dataInstance, scenarioNum; return std::vector<AQLMathVolatility *>(0);}

	//==============================================
	// create volatility object for grid
	virtual std::vector<AQLMathVolatility *> createVolatilityEntityDataForGrid
					(const AQLString &riskName, const AQLString &key, 
					AQLDataInstance &dataInstance, SCENARIONUM scenarioNum = SCENARIO_1) const { riskName, key, dataInstance, scenarioNum; return std::vector<AQLMathVolatility *>(0);}
	//==============================================
	// get calibfile path 
	virtual AQLString getCalibFilePath(const AQLString &riskName, const AQLString &fx, SCENARIONUM scenarioNum = SCENARIO_1) const;

};


#endif
