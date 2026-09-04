/*! @file
    @brief Yield Risk Scenario create class
*/
//  2008, AlgoQuantHub.
#ifndef AQLScenarioConfigurationYieldCurveMultiPara_h
#define AQLScenarioConfigurationYieldCurveMultiPara_h
#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma interface
#endif

#include "AQLScenarioConfiguration.h"

//===================== Class Declare AQLScenarioConfigurationYieldCurveMultiParallel==================================
/*! 
    @brief Yield Risk Scenario create class
	

*/
class AQLScenarioConfigurationYieldCurveMultiParallel : public AQLScenarioConfiguration
{
public:
	// constructor
	explicit AQLScenarioConfigurationYieldCurveMultiParallel(void);
	// destructor
	virtual ~AQLScenarioConfigurationYieldCurveMultiParallel(void);
	// copy constructor
	AQLScenarioConfigurationYieldCurveMultiParallel(const AQLScenarioConfigurationYieldCurveMultiParallel &rhs);
	AQLScenarioConfigurationYieldCurveMultiParallel &operator=(const AQLScenarioConfigurationYieldCurveMultiParallel &rhs);

	//==============================================
	// create risk scenario
	virtual std::vector<AQLObject *> createScenario(AQLDataInstance &dataInstance, const AQLScenarioParam &param) const;
protected:
};


#endif
