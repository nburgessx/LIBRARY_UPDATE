/*! @file
    @brief AQLModelSetup manager class
*/
//  2007, AlgoQuantHub.
#ifndef AQLMasterRegistManager_h
#define AQLMasterRegistManager_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLMasterRegistManager.h
//
//  DESCRIPTION :       AQLModelSetup manger
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#endif

#include "AQLModelSetupBase.h"


class AQLCoreDataService;
class AQLString;

//===================== Class Declare AQLMasterRegistManager==================================
/*! 
    @brief AQLModelSetup manager class

    This class is singleton
*/
class AQLMasterRegistManager
{
public:
	//==============================================
	// return instance
	static AQLMasterRegistManager *getInstance(void);
	//==============================================
	// create AQLModelSetup
	AQLModelSetupBase *createRegister(const AQLString &model);

private:
friend class AQLCoreDataService;
	// constructor
	AQLMasterRegistManager(void);
	// destructor
	~AQLMasterRegistManager(void);
	// copy constructor
	AQLMasterRegistManager(const AQLMasterRegistManager &rhs);
	AQLMasterRegistManager &operator=(const AQLMasterRegistManager &rhs);

	static AQLMasterRegistManager *mpInstance; // single instance
};

#endif
