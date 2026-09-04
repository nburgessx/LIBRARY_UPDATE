/*! @file
    @brief Yield Risk Scenario create class(zero rate shift)
*/
//  2008, AlgoQuantHub.
#ifndef AQLScenarioConfigurationYieldCurveZero_h
#define AQLScenarioConfigurationYieldCurveZero_h
#ifdef __GNUG__
#pragma interface
#endif

#include "AQLScenarioConfiguration.h"

//===================== Class Declare AQLScenarioConfigurationYieldCurveZero==================================
/*! 
    @brief Yield Risk Scenario create class(zero rate shift)
	

*/
class AQLScenarioConfigurationYieldCurveZero : public AQLScenarioConfiguration
{
public:
	// constructor
	explicit AQLScenarioConfigurationYieldCurveZero(bool isBasis = false);
	// destructor
	virtual ~AQLScenarioConfigurationYieldCurveZero(void);
	// copy constructor
	AQLScenarioConfigurationYieldCurveZero(const AQLScenarioConfigurationYieldCurveZero &rhs);
	AQLScenarioConfigurationYieldCurveZero &operator=(const AQLScenarioConfigurationYieldCurveZero &rhs);

	//==============================================
	// create risk scenario
	virtual std::vector<AQLObject *> createScenario(AQLDataInstance &dataInstance, const AQLScenarioParam &param) const;
private:
	bool mIsBasis; // basis flag
};


#endif
