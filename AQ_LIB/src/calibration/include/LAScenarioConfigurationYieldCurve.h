/*! @file
    @brief Yield Risk Scenario create class
*/
//  2008, AlgoQuantHub.
#ifndef LAScenarioConfigurationYieldCurve_h
#define LAScenarioConfigurationYieldCurve_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAScenarioConfigurationYieldCurve.h
//
//  DESCRIPTION :        Yield Risk Scenario create class
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#endif

#include "LAScenarioConfiguration.h"

//===================== Class Declare LAScenarioConfigurationYieldCurve==================================
/*! 
    @brief Yield Risk Scenario create class
	

*/
class LAScenarioConfigurationYieldCurve : public LAScenarioConfiguration
{
public:
	// constructor
	explicit LAScenarioConfigurationYieldCurve(void);
	// destructor
	virtual ~LAScenarioConfigurationYieldCurve(void);
	// copy constructor
	LAScenarioConfigurationYieldCurve(const LAScenarioConfigurationYieldCurve &rhs);
	LAScenarioConfigurationYieldCurve &operator=(const LAScenarioConfigurationYieldCurve &rhs);

	//==============================================
	// create risk scenario
	virtual std::vector<AQLObject *> createScenario(AQLDataInstance &dataInstance, const MAScenarioParam &param) const;
protected:
};


#endif
