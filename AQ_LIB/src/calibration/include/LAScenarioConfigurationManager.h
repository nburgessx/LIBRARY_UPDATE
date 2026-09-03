/*! @file
    @brief LAScenarioConfigurationManager manager class 
*/
//  2008, AlgoQuantHub.
#ifndef LAScenarioConfigurationManager_h
#define LAScenarioConfigurationManager_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAScenarioConfigurationManager.h
//
//  DESCRIPTION :       LAScenarioConfigurationManager 
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#endif

#include "AQLString.h"


class LACoreDataService;
class LAScenarioConfiguration;
//===================== Class Declare LAScenarioConfigurationManager==================================
/*! 
    @brief Scenario manager class

    This class is singleton
*/
class LAScenarioConfigurationManager
{
public:
	//==============================================
	// return singleton 
	static LAScenarioConfigurationManager     *getInstance();
	//==============================================
	// create  LAScenarioConfiguration
	 LAScenarioConfiguration *createScenarioCreator(const AQLString &type) const;

private:

friend class LACoreDataService;
	// constructor
	LAScenarioConfigurationManager(void);
	// destructor
	~LAScenarioConfigurationManager(void);
	// copy constructor
	LAScenarioConfigurationManager(const LAScenarioConfigurationManager &rhs);
	LAScenarioConfigurationManager &operator=(const LAScenarioConfigurationManager &rhs);

	static LAScenarioConfigurationManager *mpInstance; // single instance

};

#endif
