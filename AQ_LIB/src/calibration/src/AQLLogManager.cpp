/*! @file
    @brief Log manager class
*/
#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLLogManager.h"
#include "AQLDefinitions.h"
#include "AQLStaticData.h"
using namespace std;

AQLLogger *AQLLogManager::mpLogger = 0;
AQLLogger *AQLLogManager::mpXVALogger = 0;

//================ AQLLogManager ===================================
// constructor
/*!

*/
AQLLogManager::AQLLogManager(void)
{
}

// destructor
/*!

*/
AQLLogManager::~AQLLogManager(void)
{
}


// 
/*!
    @brief return logger

	return logger status is always open.

	@return  AQLLogger &
*/
AQLLogger &
AQLLogManager::getLogger(void)
{
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
		AQLStaticDataManager manager;
		AQLStaticData &accessor = manager.getStaticData();
		// log file name
		AQLString logFile  = accessor.getStaticData(KEY_LOG_FILE);
		// log level
		AQLLoggerImpl::logLevel = accessor.getStaticData(KEY_LOG_LEVEL).getIntValue();
		mpLogger = new AQLLoggerImpl(logFile);
	}
	return *mpLogger;

}


// 
/*!
    @brief return logger

	return logger status is always open.

	@return  AQLLogger &
*/
AQLLogger &
AQLLogManager::getXVALogger(void)
{
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
		AQLStaticDataManager manager;
		AQLStaticData &accessor = manager.getXVAStaticData();
		// log file name
		AQLString logFile  = accessor.getStaticData(KEY_LOG_FILE);
		// log level
		AQLLoggerImpl::logLevel = accessor.getStaticData(KEY_LOG_LEVEL).getIntValue();
		mpXVALogger = new AQLLoggerImpl(logFile);
	}
	return *mpXVALogger;

}

// 
/*!
    @brief finalize method

*/
void
AQLLogManager::finalize()
{
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
