/*! @file
    @brief IR Delta setup class
*/
#ifndef AQLRiskConfigurationYieldDeltaAnalytic_h
#define AQLRiskConfigurationYieldDeltaAnalytic_h
#ifdef __GNUG__
#pragma interface
#endif


#include "AQLRiskConfigurationYieldDelta.h"

//===================== Class Declare AQLRiskConfigurationYieldDeltaAnalytic==================================
/*! 
    @brief IR Delta setup class
	

*/
class AQLRiskConfigurationYieldDeltaAnalytic : public AQLRiskConfigurationYieldDelta
{
public:
	// constructor
	explicit AQLRiskConfigurationYieldDeltaAnalytic(void);
	// destructor
	virtual ~AQLRiskConfigurationYieldDeltaAnalytic(void);
	// copy constructor
	AQLRiskConfigurationYieldDeltaAnalytic(const AQLRiskConfigurationYieldDeltaAnalytic &rhs);
	AQLRiskConfigurationYieldDeltaAnalytic &operator=(const AQLRiskConfigurationYieldDeltaAnalytic &rhs);

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
	//==============================================
	// not calculate gamma in analytical case
	virtual  AQLString  getOutPutName2(const AQLString &ccy) const { (void)ccy; return AQ_NO_DATA; };
};
#endif
