/*! @file
    @brief Log manager class
*/
//  2007, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       MALogManager.cpp
//
//  DESCRIPTION :       Log manager class
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "LALogManager.h"
#include "LADefinitions.h"
#include "LAStaticData.h"
#ifdef __HAS_MIC__

#endif
using namespace std;

MALogger *MALogManager::mpLogger = 0;
MALogger *MALogManager::mpXVALogger = 0;
#ifdef __HAS_MIC__
common_lib::StaticMutex MALogManager::mMutex;
#endif

//================ MALogManager ===================================
// constructor
/*!

*/
MALogManager::MALogManager(void)
{
}

// destructor
/*!

*/
MALogManager::~MALogManager(void)
{
}


// 
/*!
    @brief return logger

	return logger status is always open.

	@return  MALogger &
*/
MALogger &
MALogManager::getLogger(void)
{
#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
	if (mpLogger)
	{
		// open check
		if(!mpLogger->is_open())
		{
			mpLogger->open();

		}
	}
	else
	{
		LAStaticDataManager manager;
		LAStaticData &accessor = manager.getStaticData();
		// log file name
		LAString logFile  = accessor.getStaticData(KEY_LOG_FILE);
		// log level
		MALoggerImpl::logLevel = accessor.getStaticData(KEY_LOG_LEVEL).getIntValue();
		mpLogger = new MALoggerImpl(logFile);
	}
	return *mpLogger;

}


// 
/*!
    @brief return logger

	return logger status is always open.

	@return  MALogger &
*/
MALogger &
MALogManager::getXVALogger(void)
{
#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
	if (mpXVALogger)
	{
		// open check
		if(!mpXVALogger->is_open())
		{
			mpXVALogger->open();

		}
	}
	else
	{
		LAStaticDataManager manager;
		LAStaticData &accessor = manager.getXVAStaticData();
		// log file name
		LAString logFile  = accessor.getStaticData(KEY_LOG_FILE);
		// log level
		MALoggerImpl::logLevel = accessor.getStaticData(KEY_LOG_LEVEL).getIntValue();
		mpXVALogger = new MALoggerImpl(logFile);
	}
	return *mpXVALogger;

}

// 
/*!
    @brief finalize method

*/
void
MALogManager::finalize()
{
#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
	if (mpLogger)
	{
		delete mpLogger;
		mpLogger = 0;
	}
	if (mpXVALogger)
	{
		delete mpXVALogger;
		mpXVALogger = 0;
	}
}