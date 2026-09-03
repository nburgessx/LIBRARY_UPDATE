/*! @file
    @brief IR Delta setup class
*/
//  2008, Mizuho International London.
#ifndef LARiskConfigurationYieldIRDeltaAnalytic_h
#define LARiskConfigurationYieldIRDeltaAnalytic_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfigurationYieldIRDeltaAnalytic.h
//
//  DESCRIPTION :       IR Delta setup class
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#endif


#include "LARiskConfigurationYieldIRDelta.h"

//===================== Class Declare LARiskConfigurationYieldIRDeltaAnalytic==================================
/*! 
    @brief IR Delta setup class
	

*/
class LARiskConfigurationYieldIRDeltaAnalytic : public LARiskConfigurationYieldIRDelta
{
public:
	// constructor
	explicit LARiskConfigurationYieldIRDeltaAnalytic(void);
	// destructor
	virtual ~LARiskConfigurationYieldIRDeltaAnalytic(void);
	// copy constructor
	LARiskConfigurationYieldIRDeltaAnalytic(const LARiskConfigurationYieldIRDeltaAnalytic &rhs);
	LARiskConfigurationYieldIRDeltaAnalytic &operator=(const LARiskConfigurationYieldIRDeltaAnalytic &rhs);

protected:
	//==============================================
	// setup target names 
	virtual  void setUpTargetNames(const LAString &ccy, LAObject &e, LADataInstance &dataInstance) const {return;};
	// setup scenario
	virtual  void setUpScenario(const LAString &ccy, LAObject &e, LADataInstance &dataInstance, int index) const {return;};
	//==============================================
	// create scenario yield object
	virtual std::vector<LAObject *> createYieldEntity(const LAString &ccy, LADataInstance &dataInstance, SCENARIONUM scenarioNum, int index) const {return std::vector<LAObject *>(0);};
	//==============================================
	// isgridsensitivity
	//virtual  bool isGridSensitivity(const LAString &ccy) const{(void)ccy; return true;};
	//==============================================
	// isparallelshift
	virtual  bool isParallelShift(const LAString &ccy) const {(void)ccy; return true;};
	//==============================================
	// get bump direction
	virtual  LAString getBumpDirection(const LAString &ccy) const {(void)ccy; return RISK_BUMPDIRECTION_UPSHIFT; };
	//==============================================
	// get is wave
	virtual  bool  isWave(const LAString &ccy) const {(void)ccy; return false;};
	//==============================================
	// isAnalyticMode
	virtual bool isAnalyticMode(const LAString& ccy) const { (void)ccy; return true; };
};
#endif
