/*! @file
    @brief Yield Risk Scenario create class(zero rate shift)
*/
//  2008, AlgoQuantHub.
#ifndef LAScenarioConfigurationYieldCurveZero_h
#define LAScenarioConfigurationYieldCurveZero_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAScenarioConfigurationYieldCurveZero.h
//
//  DESCRIPTION :        Yield Risk Scenario create class(zero rate shift)
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#endif

#include "LAScenarioConfiguration.h"

//===================== Class Declare LAScenarioConfigurationYieldCurveZero==================================
/*! 
    @brief Yield Risk Scenario create class(zero rate shift)
	

*/
class LAScenarioConfigurationYieldCurveZero : public LAScenarioConfiguration
{
public:
	// constructor
	explicit LAScenarioConfigurationYieldCurveZero(bool isBasis = false);
	// destructor
	virtual ~LAScenarioConfigurationYieldCurveZero(void);
	// copy constructor
	LAScenarioConfigurationYieldCurveZero(const LAScenarioConfigurationYieldCurveZero &rhs);
	LAScenarioConfigurationYieldCurveZero &operator=(const LAScenarioConfigurationYieldCurveZero &rhs);

	//==============================================
	// create risk scenario
	virtual std::vector<LAObject *> createScenario(LADataInstance &dataInstance, const MAScenarioParam &param) const;
private:
	bool mIsBasis; // basis flag
};


#endif
