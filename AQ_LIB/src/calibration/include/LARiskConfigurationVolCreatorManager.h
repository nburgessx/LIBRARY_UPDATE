/*! @file
    @brief LARiskConfigurationVolCreator manager class 
*/
//  2007, Mizuho International London.
#ifndef LARiskConfigurationVolCreatorManager_h
#define LARiskConfigurationVolCreatorManager_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfigurationVolCreatorManager.h
//
//  DESCRIPTION :       LARiskConfigurationVolCreatorManager 
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#endif

#include "LADefinitions.h"
#include "LADefinitionsRisk.h"
#include "LARiskConfigurationVolCreator.h"
#include "LARiskConfigurationVolCreatorLMM.h"
#include "LARiskConfigurationVolCreatorPtberg.h"


class LACoreDataService;
//===================== Class Declare LARiskConfigurationVolCreatorManager==================================
/*! 
    @brief LARiskConfigurationVolCreatorManagerÅ@Factory class

    This class is singleton
*/
class LARiskConfigurationVolCreatorManager
{
public:
	//==============================================
	// return singleton
	static LARiskConfigurationVolCreatorManager     *getInstance();
	//==============================================
	// create LAObjectConfiguration
	LARiskConfigurationVolCreator *createVolCreator(const LAString &model, bool funcFlg = true) const;

private:

friend class LACoreDataService;
	// constructor
	LARiskConfigurationVolCreatorManager(void);
	// destructor
	~LARiskConfigurationVolCreatorManager(void);
	// copy constructor
	LARiskConfigurationVolCreatorManager(const LARiskConfigurationVolCreatorManager &rhs);
	LARiskConfigurationVolCreatorManager &operator=(const LARiskConfigurationVolCreatorManager &rhs);

	static LARiskConfigurationVolCreatorManager *mpInstance; // single instance

};

#endif
