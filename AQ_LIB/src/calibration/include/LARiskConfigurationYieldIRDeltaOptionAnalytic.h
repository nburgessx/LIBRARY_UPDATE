/*! @file
    @brief FX vega setup class
*/
//  2008, AlgoQuantHub.
#ifndef LARiskConfigurationYieldIRDeltaOptionAnalytic_h
#define LARiskConfigurationYieldIRDeltaOptionAnalytic_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfigurationYieldIRDeltaOptionAnalytic.h
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

#include "LARiskConfigurationYieldIRDelta.h"



class AQLString;
class AQLDataInstance;
//===================== Class Declare LARiskConfigurationYieldIRDeltaOptionAnalytic==================================
/*! 
    @brief FX Vega setup class
	

*/
class LARiskConfigurationYieldIRDeltaOptionAnalytic : public LARiskConfigurationYieldIRDelta
{
public:
	// constructor
	explicit LARiskConfigurationYieldIRDeltaOptionAnalytic(const AQLString& risktype);
	// destructor
	virtual ~LARiskConfigurationYieldIRDeltaOptionAnalytic(void);
	// copy constructor
	LARiskConfigurationYieldIRDeltaOptionAnalytic(const LARiskConfigurationYieldIRDeltaOptionAnalytic &rhs);
	LARiskConfigurationYieldIRDeltaOptionAnalytic &operator=(const LARiskConfigurationYieldIRDeltaOptionAnalytic &rhs);

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
	// create extraScenario object 
	virtual  std::vector<std::vector<AQLObject *> > createExtraScenarioEntity(const AQLString &ccy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const;

	
};


#endif
