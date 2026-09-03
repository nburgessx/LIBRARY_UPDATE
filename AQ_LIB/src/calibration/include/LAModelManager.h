/*! @file
    @brief LAModelSetup manager class
*/
//  2007, AlgoQuantHub.
#ifndef MAMasterRegistManager_h
#define MAMasterRegistManager_h
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       MAMasterRegistManager.h
//
//  DESCRIPTION :       LAModelSetup manger
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma interface
#endif

#include "LAModelSetupBase.h"


class LACoreDataService;
class LAString;

//===================== Class Declare MAMasterRegistManager==================================
/*! 
    @brief LAModelSetup manager class

    This class is singleton
*/
class MAMasterRegistManager
{
public:
	//==============================================
	// return instance
	static MAMasterRegistManager *getInstance(void);
	//==============================================
	// create LAModelSetup
	LAModelSetupBase *createRegister(const LAString &model);

private:
friend class LACoreDataService;
	// constructor
	MAMasterRegistManager(void);
	// destructor
	~MAMasterRegistManager(void);
	// copy constructor
	MAMasterRegistManager(const MAMasterRegistManager &rhs);
	MAMasterRegistManager &operator=(const MAMasterRegistManager &rhs);

	static MAMasterRegistManager *mpInstance; // single instance
};

#endif
