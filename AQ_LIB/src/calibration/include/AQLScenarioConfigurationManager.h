/*! @file
    @brief AQLScenarioConfigurationManager manager class 
*/
//  2008, AlgoQuantHub.
#ifndef AQLScenarioConfigurationManager_h
#define AQLScenarioConfigurationManager_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLScenarioConfigurationManager.h
//
//  DESCRIPTION :       AQLScenarioConfigurationManager 
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#endif

#include "AQLString.h"


class AQLCoreDataService;
class AQLScenarioConfiguration;
//===================== Class Declare AQLScenarioConfigurationManager==================================
/*! 
    @brief Scenario manager class

    This class is singleton
*/
class AQLScenarioConfigurationManager
{
public:
	//==============================================
	// return singleton 
	static AQLScenarioConfigurationManager     *getInstance();
	//==============================================
	// create  AQLScenarioConfiguration
	 AQLScenarioConfiguration *createScenarioCreator(const AQLString &type) const;

private:

friend class AQLCoreDataService;
	// constructor
	AQLScenarioConfigurationManager(void);
	// destructor
	~AQLScenarioConfigurationManager(void);
	// copy constructor
	AQLScenarioConfigurationManager(const AQLScenarioConfigurationManager &rhs);
	AQLScenarioConfigurationManager &operator=(const AQLScenarioConfigurationManager &rhs);

	static AQLScenarioConfigurationManager *mpInstance; // single instance

};

#endif
