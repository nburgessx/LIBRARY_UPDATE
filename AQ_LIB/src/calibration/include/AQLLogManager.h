#pragma once

#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma interface
#endif

#include "AQLLogger.h"
#include "AQLStaticDataManager.h"

#if defined (WIN32) || defined (WIN64)
#include <windows.h>
#else
#include <pthread.h>
#endif


class AQLCoreDataService;

//===================== Class Declare AQLLogManager==================================
/*! 
    @brief Log manager class

	This class is singleton

   
*/
class AQLLogManager
{
public:
	// constructor
	AQLLogManager(void);
	// destructor
	~AQLLogManager(void);
	// copy constructor
	AQLLogManager(const AQLLogManager &rhs);
	AQLLogManager &operator=(const AQLLogManager &rhs);

	//==============================================
	// return instance
	//static AQLLogManager           *getInstance();
	//==============================================
	// return logger
	AQLLogger                       &getLogger();
	//==============================================
	// return logger
	AQLLogger                       &getXVALogger();

private:
friend class AQLCoreDataService;
	//==============================================
	// finalize method
	static void finalize();
	static AQLLogger *mpLogger;   // logger instance
	static AQLLogger *mpXVALogger;   // logger instance
};
