/*! @file
    @brief AQLRiskConfigurationVolCreator manager class 
*/
//  2007, AlgoQuantHub.
#ifndef AQLRiskConfigurationVolCreatorManager_h
#define AQLRiskConfigurationVolCreatorManager_h
#ifdef __GNUG__
#pragma interface
#endif

#include "AQLDefinitions.h"
#include "AQLDefinitionsRisk.h"
#include "AQLRiskConfigurationVolCreator.h"
#include "AQLRiskConfigurationVolCreatorLMM.h"
#include "AQLRiskConfigurationVolCreatorPtberg.h"


class AQLCoreDataService;
//===================== Class Declare AQLRiskConfigurationVolCreatorManager==================================
/*! 
    @brief AQLRiskConfigurationVolCreatorManager Factory class

    This class is singleton
*/
class AQLRiskConfigurationVolCreatorManager
{
public:
	//==============================================
	// return singleton
	static AQLRiskConfigurationVolCreatorManager     *getInstance();
	//==============================================
	// create AQLObjectConfiguration
	AQLRiskConfigurationVolCreator *createVolCreator(const AQLString &model, bool funcFlg = true) const;

private:

friend class AQLCoreDataService;
	// constructor
	AQLRiskConfigurationVolCreatorManager(void);
	// destructor
	~AQLRiskConfigurationVolCreatorManager(void);
	// copy constructor
	AQLRiskConfigurationVolCreatorManager(const AQLRiskConfigurationVolCreatorManager &rhs);
	AQLRiskConfigurationVolCreatorManager &operator=(const AQLRiskConfigurationVolCreatorManager &rhs);

	static AQLRiskConfigurationVolCreatorManager *mpInstance; // single instance

};

#endif
