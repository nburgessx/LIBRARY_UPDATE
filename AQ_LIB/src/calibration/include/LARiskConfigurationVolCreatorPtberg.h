/*! @file
    @brief Ptberg(fx) Volatility create class for calc risk 
*/
//  2007, AlgoQuantHub.
#ifndef LARiskConfigurationVolCreatorPtberg_h
#define LARiskConfigurationVolCreatorPtberg_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfigurationVolCreatorPtberg.h
//
//  DESCRIPTION :        Ptberg(fx) Volatility create class for calc risk 
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
#include "LARiskConfigurationVolCreator.h"
#include "LAMarketData.h"
#include "LAPtbergUtils.h"
#include "LADefinitionsRisk.h"
#include "LADefinitionsPtberg.h"


//===================== Class Declare LARiskConfigurationVolCreatorPtberg==================================
/*! 
    @brief Ptberg(fx) Volatility create class for calc risk 
	

*/
class LARiskConfigurationVolCreatorPtberg : public LARiskConfigurationVolCreator
{
public:
	// constructor
	explicit LARiskConfigurationVolCreatorPtberg(bool funcFlg = true);
	// destructor
	virtual ~LARiskConfigurationVolCreatorPtberg(void);
	// copy constructor
	LARiskConfigurationVolCreatorPtberg(const LARiskConfigurationVolCreatorPtberg &rhs);
	LARiskConfigurationVolCreatorPtberg &operator=(const LARiskConfigurationVolCreatorPtberg &rhs);

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
