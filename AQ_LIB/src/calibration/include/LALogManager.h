#pragma once

#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma interface
#endif

#include "LALogger.h"
#include "LAStaticDataManager.h"

#if defined (WIN32) || defined (WIN64)
#include <windows.h>
#else
#include <pthread.h>
#endif


class LACoreDataService;

//===================== Class Declare MALogManager==================================
/*! 
    @brief Log manager class

	This class is singleton

   
*/
class MALogManager
{
public:
	// constructor
	MALogManager(void);
	// destructor
	~MALogManager(void);
	// copy constructor
	MALogManager(const MALogManager &rhs);
	MALogManager &operator=(const MALogManager &rhs);

	//==============================================
	// return instance
	//static MALogManager           *getInstance();
	//==============================================
	// return logger
	MALogger                       &getLogger();
	//==============================================
	// return logger
	MALogger                       &getXVALogger();

private:
friend class LACoreDataService;
	//==============================================
	// finalize method
	static void finalize();
	static MALogger *mpLogger;   // logger instance
	static MALogger *mpXVALogger;   // logger instance
#ifdef __HAS_MIC__
	static common_lib::StaticMutex mMutex;
#endif
};
