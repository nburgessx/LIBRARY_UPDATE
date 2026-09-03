/*! @file
    @brief FX Risk Scenario create class
*/
//  2008, AlgoQuantHub.
#ifndef LAScenarioConfigurationFX_h
#define LAScenarioConfigurationFX_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAScenarioConfigurationFX.h
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

#include "LAScenarioConfiguration.h"

//===================== Class Declare LAScenarioConfigurationFX==================================
/*! 
    @brief FX Risk Scenario create class
	

*/
class LAScenarioConfigurationFX : public LAScenarioConfiguration
{
public:
	// constructor
	explicit LAScenarioConfigurationFX(void);
	// destructor
	virtual ~LAScenarioConfigurationFX(void);
	// copy constructor
	LAScenarioConfigurationFX(const LAScenarioConfigurationFX &rhs);
	LAScenarioConfigurationFX &operator=(const LAScenarioConfigurationFX &rhs);

	//==============================================
	// create risk scenario
	virtual std::vector<AQLObject *> createScenario(AQLDataInstance &dataInstance, const MAScenarioParam &param) const;
};


#endif
