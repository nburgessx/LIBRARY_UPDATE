/*! @file
    @brief AQLRiskConfiguration manager class 
*/
#ifndef AQLRiskConfigurationManager_h
#define AQLRiskConfigurationManager_h
#ifdef __GNUG__
#pragma interface
#endif

#include "AQLRiskConfiguration.h"


class AQLCoreDataService;
//===================== Class Declare AQLRiskConfigurationManager==================================
/*! 
    @brief AQLRiskConfigurationManager Factory class

    This class is singleton
*/
class AQLRiskConfigurationManager
{
public:
	//==============================================
	// return singleton
	static AQLRiskConfigurationManager     *getInstance();
	//==============================================
	// create AQLRiskConfiguration
	AQLRiskConfiguration *createRiskSetUpper(const AQLString &risk, bool isOfficial = true) const;

private:

friend class AQLCoreDataService;
	// constructor
	AQLRiskConfigurationManager(void);
	// destructor
	~AQLRiskConfigurationManager(void);
	// copy constructor
	AQLRiskConfigurationManager(const AQLRiskConfigurationManager &rhs);
	AQLRiskConfigurationManager &operator=(const AQLRiskConfigurationManager &rhs);

	static AQLRiskConfigurationManager *mpInstance; // single instance

};

#endif
