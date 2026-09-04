/*! @file
    @brief FX Risk Scenario create class
*/
#ifndef AQLScenarioConfigurationFX_h
#define AQLScenarioConfigurationFX_h
#ifdef __GNUG__
#pragma interface
#endif

#include "AQLScenarioConfiguration.h"

//===================== Class Declare AQLScenarioConfigurationFX==================================
/*! 
    @brief FX Risk Scenario create class
	

*/
class AQLScenarioConfigurationFX : public AQLScenarioConfiguration
{
public:
	// constructor
	explicit AQLScenarioConfigurationFX(void);
	// destructor
	virtual ~AQLScenarioConfigurationFX(void);
	// copy constructor
	AQLScenarioConfigurationFX(const AQLScenarioConfigurationFX &rhs);
	AQLScenarioConfigurationFX &operator=(const AQLScenarioConfigurationFX &rhs);

	//==============================================
	// create risk scenario
	virtual std::vector<AQLObject *> createScenario(AQLDataInstance &dataInstance, const AQLScenarioParam &param) const;
};


#endif
