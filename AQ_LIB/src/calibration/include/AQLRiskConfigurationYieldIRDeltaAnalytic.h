/*! @file
    @brief IR Delta setup class
*/
//  2008, AlgoQuantHub.
#ifndef AQLRiskConfigurationYieldIRDeltaAnalytic_h
#define AQLRiskConfigurationYieldIRDeltaAnalytic_h
#ifdef __GNUG__
#pragma interface
#endif


#include "AQLRiskConfigurationYieldIRDelta.h"

//===================== Class Declare AQLRiskConfigurationYieldIRDeltaAnalytic==================================
/*! 
    @brief IR Delta setup class
	

*/
class AQLRiskConfigurationYieldIRDeltaAnalytic : public AQLRiskConfigurationYieldIRDelta
{
public:
	// constructor
	explicit AQLRiskConfigurationYieldIRDeltaAnalytic(void);
	// destructor
	virtual ~AQLRiskConfigurationYieldIRDeltaAnalytic(void);
	// copy constructor
	AQLRiskConfigurationYieldIRDeltaAnalytic(const AQLRiskConfigurationYieldIRDeltaAnalytic &rhs);
	AQLRiskConfigurationYieldIRDeltaAnalytic &operator=(const AQLRiskConfigurationYieldIRDeltaAnalytic &rhs);

protected:
	//==============================================
	// setup target names 
	virtual  void setUpTargetNames(const AQLString &ccy, AQLObject &e, AQLDataInstance &dataInstance) const {return;};
	// setup scenario
	virtual  void setUpScenario(const AQLString &ccy, AQLObject &e, AQLDataInstance &dataInstance, int index) const {return;};
	//==============================================
	// create scenario yield object
	virtual std::vector<AQLObject *> createYieldEntity(const AQLString &ccy, AQLDataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const {return std::vector<AQLObject *>(0);};
	//==============================================
	// isgridsensitivity
	//virtual  bool isGridSensitivity(const AQLString &ccy) const{(void)ccy; return true;};
	//==============================================
	// isparallelshift
	virtual  bool isParallelShift(const AQLString &ccy) const {(void)ccy; return true;};
	//==============================================
	// get bump direction
	virtual  AQLString getBumpDirection(const AQLString &ccy) const {(void)ccy; return RISK_BUMPDIRECTION_UPSHIFT; };
	//==============================================
	// get is wave
	virtual  bool  isWave(const AQLString &ccy) const {(void)ccy; return false;};
	//==============================================
	// isAnalyticMode
	virtual bool isAnalyticMode(const AQLString& ccy) const { (void)ccy; return true; };
};
#endif
