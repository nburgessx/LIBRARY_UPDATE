/*! @file
    @brief Yield Risk Scenario create class
*/
//  2008, AlgoQuantHub.
#ifndef AQLScenarioConfigurationYieldCurve_h
#define AQLScenarioConfigurationYieldCurve_h
#ifdef __GNUG__
#pragma interface
#endif

#include "AQLScenarioConfiguration.h"

//===================== Class Declare AQLScenarioConfigurationYieldCurve==================================
/*! 
    @brief Yield Risk Scenario create class
	

*/
class AQLScenarioConfigurationYieldCurve : public AQLScenarioConfiguration
{
public:
	// constructor
	explicit AQLScenarioConfigurationYieldCurve(void);
	// destructor
	virtual ~AQLScenarioConfigurationYieldCurve(void);
	// copy constructor
	AQLScenarioConfigurationYieldCurve(const AQLScenarioConfigurationYieldCurve &rhs);
	AQLScenarioConfigurationYieldCurve &operator=(const AQLScenarioConfigurationYieldCurve &rhs);

	//==============================================
	// create risk scenario
	virtual std::vector<AQLObject *> createScenario(AQLDataInstance &dataInstance, const AQLScenarioParam &param) const;
protected:
};


#endif
