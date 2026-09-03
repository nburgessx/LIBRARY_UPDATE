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



class LAString;
class LADataInstance;
//===================== Class Declare LARiskConfigurationYieldBasisDeltaOptionAnalytic==================================
/*! 
    @brief FX Vega setup class
	

*/
class LARiskConfigurationYieldBasisDeltaOptionAnalytic : public LARiskConfigurationYieldBasisDelta
{
public:
	// constructor
	explicit LARiskConfigurationYieldBasisDeltaOptionAnalytic(const LAString& risktype);
	// destructor
	virtual ~LARiskConfigurationYieldBasisDeltaOptionAnalytic(void);
	// copy constructor
	LARiskConfigurationYieldBasisDeltaOptionAnalytic(const LARiskConfigurationYieldBasisDeltaOptionAnalytic &rhs);
	LARiskConfigurationYieldBasisDeltaOptionAnalytic &operator=(const LARiskConfigurationYieldBasisDeltaOptionAnalytic &rhs);

protected:

	//==============================================
	// get target names 
	virtual  LAString getTargetNames(const LAString &ccy, LADataInstance &dataInstance)  const {ccy; return AQ_NO_DATA;};
	//==============================================
	// create risk object
	virtual std::vector<std::pair<LAString, std::vector<LAObject *> > > createRiskEntity(LAObjectPool &objPool) const;
	//==============================================
	// setup target names 
	virtual  void setUpTargetNames(const LAString &ccy, LAObject &e, LADataInstance &dataInstance) const;	
	//==============================================
	// create scenario1 object 
	virtual  std::vector<LAObject *> createScenario1Entity(const LAString &ccy, LADataInstance &dataInstance, int index) const;
	//==============================================
	// create scenario2 object
	virtual  std::vector<LAObject *> createScenario2Entity(const LAString &ccy, LADataInstance &dataInstance, int index) const;
	//==============================================
	// get coefficient1
	virtual  LAString  getCoefficient1(const LAString &ccy) const;
	//==============================================
	// get risk name
	virtual  LAString getRiskName(void) const;
	//==============================================
	// get is wave
	virtual  bool  isWave(const LAString &ccy) const;
	//==============================================
	// get is gridsensitivity
	virtual  bool isGridSensitivity(const LAString &ccy) const;
	//==============================================
	// get ShiftValForOptionAnalytic
	virtual double getShiftValForOptionAnalytic(const LAString &ccy) const;
	//==============================================
	// get AnalyticalRiskType when domesticcur RHO otherwise PHI
	virtual LAString getAnalyticalRiskType(const LAString& fx, LAObject& e) const;
	//==============================================
	// IsSucceedAnalyticalRiskType
	virtual bool IsSucceedAnalyticalRiskType(const LAString& ccy, LAObject& e) const;
	
	//==============================================
	// create extraScenario object 
	virtual  std::vector<std::vector<LAObject *> > createExtraScenarioEntity(const LAString &ccy, LADataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const;

	
};


#endif
