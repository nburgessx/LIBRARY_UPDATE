/*! @file
    @brief Yield Risk Scenario create class
*/
//  2008, AlgoQuantHub.
#ifndef LAScenarioConfigurationYieldCurveMultiPara_h
#define LAScenarioConfigurationYieldCurveMultiPara_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAScenarioConfigurationYieldCurveMultiParallel.h
//
//  DESCRIPTION :        Yield Risk Scenario create class
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma interface
#endif

#include "LAScenarioConfiguration.h"

//===================== Class Declare LAScenarioConfigurationYieldCurveMultiParallel==================================
/*! 
    @brief Yield Risk Scenario create class
	

*/
class LAScenarioConfigurationYieldCurveMultiParallel : public LAScenarioConfiguration
{
public:
	// constructor
	explicit LAScenarioConfigurationYieldCurveMultiParallel(void);
	// destructor
	virtual ~LAScenarioConfigurationYieldCurveMultiParallel(void);
	// copy constructor
	LAScenarioConfigurationYieldCurveMultiParallel(const LAScenarioConfigurationYieldCurveMultiParallel &rhs);
	LAScenarioConfigurationYieldCurveMultiParallel &operator=(const LAScenarioConfigurationYieldCurveMultiParallel &rhs);

	//==============================================
	// create risk scenario
	virtual std::vector<LAObject *> createScenario(LADataInstance &dataInstance, const MAScenarioParam &param) const;
protected:
};


#endif
