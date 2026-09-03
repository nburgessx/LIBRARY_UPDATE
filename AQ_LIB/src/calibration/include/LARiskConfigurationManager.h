/*! @file
    @brief LARiskConfiguration manager class 
*/
//  2007, Mizuho International London.
#ifndef LARiskConfigurationManager_h
#define LARiskConfigurationManager_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfigurationManager.h
//
//  DESCRIPTION :       LARiskConfigurationManager 
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#endif

#include "LARiskConfiguration.h"


class LACoreDataService;
//===================== Class Declare LARiskConfigurationManager==================================
/*! 
    @brief LARiskConfigurationManager Factory class

    This class is singleton
*/
class LARiskConfigurationManager
{
public:
	//==============================================
	// return singleton
	static LARiskConfigurationManager     *getInstance();
	//==============================================
	// create LARiskConfiguration
	LARiskConfiguration *createRiskSetUpper(const LAString &risk, bool isOfficial = true) const;

private:

friend class LACoreDataService;
	// constructor
	LARiskConfigurationManager(void);
	// destructor
	~LARiskConfigurationManager(void);
	// copy constructor
	LARiskConfigurationManager(const LARiskConfigurationManager &rhs);
	LARiskConfigurationManager &operator=(const LARiskConfigurationManager &rhs);

	static LARiskConfigurationManager *mpInstance; // single instance

};

#endif
