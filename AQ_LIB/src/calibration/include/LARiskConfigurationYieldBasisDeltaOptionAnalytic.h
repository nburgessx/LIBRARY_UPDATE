/*! @file
    @brief FX vega setup class
*/
//  2008, AlgoQuantHub.
#ifndef LARiskConfigurationYieldBasisDeltaOptionAnalytic_h
#define LARiskConfigurationYieldBasisDeltaOptionAnalytic_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfigurationYieldBasisDeltaOptionAnalytic.h
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

#include "LARiskConfigurationYieldBasisDelta.h"



class AQLString;
class AQLDataInstance;
//===================== Class Declare LARiskConfigurationYieldBasisDeltaOptionAnalytic==================================
/*! 
    @brief FX Vega setup class
	

*/
class LARiskConfigurationYieldBasisDeltaOptionAnalytic : public LARiskConfigurationYieldBasisDelta
{
public:
	// constructor
	explicit LARiskConfigurationYieldBasisDeltaOptionAnalytic(const AQLString& risktype);
	// destructor
	virtual ~LARiskConfigurationYieldBasisDeltaOptionAnalytic(void);
	// copy constructor
	LARiskConfigurationYieldBasisDeltaOptionAnalytic(const LARiskConfigurationYieldBasisDeltaOptionAnalytic &rhs);
	LARiskConfigurationYieldBasisDeltaOptionAnalytic &operator=(const LARiskConfigurationYieldBasisDeltaOptionAnalytic &rhs);

protected:

	//==============================================
	// get target names 
	virtual  AQLString getTargetNames(const AQLString &ccy, AQLDataInstance &dataInstance)  const {ccy; return AQ_NO_DATA;};
	//==============================================
	// create risk object
	virtual std::vector<std::pair<AQLString, std::vector<AQLObject *> > > createRiskEntity(AQLObjectPool &objPool) const;
	//==============================================
	// setup target names 
	virtual  void setUpTargetNames(const AQLString &ccy, AQLObject &e, AQLDataInstance &dataInstance) const;	
	//==============================================
	// create scenario1 object 
	virtual  std::vector<AQLObject *> createScenario1Entity(const AQLString &ccy, AQLDataInstance &dataInstance, int index) const;
	//==============================================
	// create scenario2 object
	virtual  std::vector<AQLObject *> createScenario2Entity(const AQLString &ccy, AQLDataInstance &dataInstance, int index) const;
	//==============================================
	// get coefficient1
	virtual  AQLString  getCoefficient1(const AQLString &ccy) const;
	//==============================================
	// get risk name
	virtual  AQLString getRiskName(void) const;
	//==============================================
	// get is wave
	virtual  bool  isWave(const AQLString &ccy) const;
	//==============================================
	// get is gridsensitivity
	virtual  bool isGridSensitivity(const AQLString &ccy) const;
	//==============================================
	// get ShiftValForOptionAnalytic
	virtual double getShiftValForOptionAnalytic(const AQLString &ccy) const;
	//==============================================
	// get AnalyticalRiskType when domesticcur RHO otherwise PHI
	virtual AQLString getAnalyticalRiskType(const AQLString& fx, AQLObject& e) const;
	//==============================================
	// IsSucceedAnalyticalRiskType
	virtual bool IsSucceedAnalyticalRiskType(const AQLString& ccy, AQLObject& e) const;
	
	//==============================================
	// create extraScenario object 
	virtual  std::vector<std::vector<AQLObject *> > createExtraScenarioEntity(const AQLString &ccy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const;

	
};


#endif
