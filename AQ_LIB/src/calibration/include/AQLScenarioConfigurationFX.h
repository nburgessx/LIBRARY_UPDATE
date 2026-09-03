/*! @file
    @brief FX Risk Scenario create class
*/
//  2008, AlgoQuantHub.
#ifndef AQLScenarioConfigurationFX_h
#define AQLScenarioConfigurationFX_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLScenarioConfigurationFX.h
//
//  DESCRIPTION :        FX Risk Scenario create class
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
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
