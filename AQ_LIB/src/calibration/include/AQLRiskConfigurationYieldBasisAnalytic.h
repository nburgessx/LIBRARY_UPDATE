/*! @file
    @brief IR Delta setup class
*/
#ifndef AQLRiskConfigurationYieldBasisAnalytic_h
#define AQLRiskConfigurationYieldBasisAnalytic_h
#ifdef __GNUG__
#pragma interface
#endif


#include "AQLRiskConfigurationYieldBasis.h"

//===================== Class Declare AQLRiskConfigurationYieldBasisAnalytic==================================
/*! 
    @brief IR Delta setup class
	

*/
class AQLRiskConfigurationYieldBasisAnalytic : public AQLRiskConfigurationYieldBasis
{
public:
	// constructor
	explicit AQLRiskConfigurationYieldBasisAnalytic(void);
	// destructor
	virtual ~AQLRiskConfigurationYieldBasisAnalytic(void);
	// copy constructor
	AQLRiskConfigurationYieldBasisAnalytic(const AQLRiskConfigurationYieldBasisAnalytic &rhs);
	AQLRiskConfigurationYieldBasisAnalytic &operator=(const AQLRiskConfigurationYieldBasisAnalytic &rhs);

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
