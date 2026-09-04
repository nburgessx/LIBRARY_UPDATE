/*! @file
    @brief IR Delta setup class
*/
//  2008, AlgoQuantHub.
#ifndef AQLRiskConfigurationYieldBasisDeltaAnalytic_h
#define AQLRiskConfigurationYieldBasisDeltaAnalytic_h
#ifdef __GNUG__
#pragma interface
#endif


#include "AQLRiskConfigurationYieldBasisDelta.h"

//===================== Class Declare AQLRiskConfigurationYieldBasisDeltaAnalytic==================================
/*! 
    @brief IR Delta setup class
	

*/
class AQLRiskConfigurationYieldBasisDeltaAnalytic : public AQLRiskConfigurationYieldBasisDelta
{
public:
	// constructor
	explicit AQLRiskConfigurationYieldBasisDeltaAnalytic(void);
	// destructor
	virtual ~AQLRiskConfigurationYieldBasisDeltaAnalytic(void);
	// copy constructor
	AQLRiskConfigurationYieldBasisDeltaAnalytic(const AQLRiskConfigurationYieldBasisDeltaAnalytic &rhs);
	AQLRiskConfigurationYieldBasisDeltaAnalytic &operator=(const AQLRiskConfigurationYieldBasisDeltaAnalytic &rhs);

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
