
#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include <map>
#include "AQLCoreDataService.h"
#include "AQLFileAccessor.h"
#include "AQLStaticDataManager.h"
#include "AQLStaticData.h"
#include "AQLLogManager.h"
#include "AQLMarketData.h"

#ifndef VISUAL_STUDIO_2010_ANALYTICS
#include "AQLModelManager.h"
#include "AQLModelConfiguration.h"
#include "AQLObjectConfigurationManager.h"
#include "AQLRiskConfigurationManager.h"
#include "AQLScenarioConfigurationManager.h"
#include "AQLCalibrateVolatilityManager.h"
#include "AQLCalibrationParametersManager.h"
#include "AQLCalibratePool.h"
#include "AQLCalibrate.h"
#endif

#ifdef __HAS_MIC__

#endif

using namespace std;

map<AQLString, istringstream *> AQLCoreDataService::mIStringStreamMap;
map<AQLString, AQLString> AQLCoreDataService::mSettingFiles;
AQLStaticDataManager *AQLCoreDataService::mpPropertyManager = 0;
AQLLogManager *AQLCoreDataService::mpLogManager = 0;
bool AQLCoreDataService::mInitializeFlg = false;

#ifdef __HAS_MIC__
common_lib::StaticMutex AQLCoreDataService::mMutex;
#endif

#if defined (WIN32) || defined (WIN64)
map<DWORD, map<AQLString, AQLString> > AQLCoreDataService::mContextMap;
#else
map<pthread_t, map<AQLString, AQLString> > AQLCoreDataService::mContextMap;
#endif
 
//AQLString AQLCoreDataService::mPropertyFile;

//================ AQLCoreDataService ===================================

// constructor
/*!

*/
AQLCoreDataService::AQLCoreDataService(void)
{
}


// destructor
/*!

*/
AQLCoreDataService::~AQLCoreDataService(void)
{
}

// initialize
/*!
	@brief initialize service

*/
void
AQLCoreDataService::initialize(void)
{
	if(!mInitializeFlg)
	{
		// call initialize
		AQLFileAccessor::initialize();
		mpPropertyManager = new AQLStaticDataManager();
		mpLogManager = new AQLLogManager();
#ifdef __HAS_MIC__
		// prepare using instances for multi-thread
		AQLMasterRegistManager::getInstance();
		AQLModelConfiguration::getInstance();
#endif
		mInitializeFlg = true;
	}
}

// clear context
/*!
	@brief clear context
	@param[in] isAll all clear flag

*/
void
AQLCoreDataService::clearContext(bool isAll)
{
#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
	if (isAll)
	{

#if defined (WIN32) || defined (WIN64)
		map<DWORD, map<AQLString, AQLString> >::iterator it = mContextMap.begin();
#else
		map<pthread_t, map<AQLString, AQLString> >::iterator it = mContextMap.begin();
#endif

        while (it != mContextMap.end())
		{
			it->second.clear();
			++it;
		}
		mContextMap.clear();
	}
	else
	{

#if defined (WIN32) || defined (WIN64)
		DWORD thread_id = GetCurrentThreadId();
#else
		pthread_t thread_id = pthread_self();
#endif

		mContextMap[thread_id].clear();
	}
}

// clear stringstream
/*!
	@brief clear stringstream
	@param[in] fileNum
*/
void
AQLCoreDataService::clearStringStream(const AQLString &fileNum)
{
#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
	// clear string stream
	AQLStringVector keyVec;
	map<AQLString, istringstream *>::iterator strIt = mIStringStreamMap.begin();
	while (strIt != mIStringStreamMap.end())
	{
		if (strIt->first.findString(fileNum) >= 0)
		{
			delete strIt->second;
			keyVec.push_back(strIt->first);
		}
		++strIt;
	}
	const unsigned int delSize = keyVec.size();
	for (unsigned int i = 0; i < delSize; ++i)
	{
		mIStringStreamMap.erase(keyVec[i]);
	}
}

// clear all stringstream
/*!
	@brief clear all stringstream
*/
void
AQLCoreDataService::clearAllStringStream()
{
#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
	map<AQLString, istringstream *>::iterator strIt = mIStringStreamMap.begin();
	while (strIt != mIStringStreamMap.end())
	{
		delete strIt->second;
		++strIt;
	}
	mIStringStreamMap.clear();
}
// clear fileNum
/*!
	@brief clear context and property
	@param[in] fileNum
*/
void
AQLCoreDataService::clear(const AQLString &fileNum)
{
	// clear properties
	AQLStaticDataManager::clearStaticDataObject(fileNum);
	// clear risk properties
	AQLStaticDataManager::clearRiskStaticDataObject(fileNum);
	// clear calib properties 
	AQLStaticDataManager::clearCalibStaticDataObject(fileNum); 
	// clear grid properties 
	AQLStaticDataManager::clearGridStaticData(fileNum); 
	// clear file cache
	AQLFileAccessor::clearFileCache(fileNum);
	// clear string stream
	clearStringStream(fileNum);
	// clear context
	clearContext(false);
#ifndef VISUAL_STUDIO_2010_ANALYTICS
	// clear calibration data
	AQLCalibrate::clear(fileNum);
#endif
}

// clear instance
/*!
	@brief delete sigleton instance

*/
void
AQLCoreDataService::clearInstance(void)
{
#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
	
	if (mpPropertyManager)
	{
		delete mpPropertyManager;
		mpPropertyManager = 0;
	}

	if (mpLogManager)
	{
		delete mpLogManager;
		mpLogManager =0;
	}

#ifndef VISUAL_STUDIO_2010_ANALYTICS

	if (AQLMasterRegistManager::mpInstance)
	{
		delete AQLMasterRegistManager::mpInstance;
		AQLMasterRegistManager::mpInstance = 0;
	}
	if (AQLModelConfiguration::mpInstance)
	{
		delete AQLModelConfiguration::mpInstance;
		AQLModelConfiguration::mpInstance = 0;
	}
	if (AQLObjectConfigurationManager::mpInstance)
	{
		delete AQLObjectConfigurationManager::mpInstance;
		AQLObjectConfigurationManager::mpInstance = 0;
	}
	if (AQLRiskConfigurationManager::mpInstance)
	{
		delete AQLRiskConfigurationManager::mpInstance;
		AQLRiskConfigurationManager::mpInstance = 0;
	}
	if (AQLScenarioConfigurationManager::mpInstance)
	{
		delete AQLScenarioConfigurationManager::mpInstance;
		AQLScenarioConfigurationManager::mpInstance = 0;
	}
	if (AQLCalibrateVolatilityManager::mpInstance)
	{
		delete AQLCalibrateVolatilityManager::mpInstance;
		AQLCalibrateVolatilityManager::mpInstance = 0;
	}
	if (AQLCalibrationParametersManager::mpInstance)
	{
		delete AQLCalibrationParametersManager::mpInstance;
		AQLCalibrationParametersManager::mpInstance = 0;
	}

	if (AQLCalibratePool::mpInstance)
	{
		delete AQLCalibratePool::mpInstance;
		AQLCalibratePool::mpInstance = 0;
	}

#endif
	
}

// finalize
/*!
	@brief delete sigleton instance

*/
void
AQLCoreDataService::finalize(void)
{
	try
	{
		// stream clear
		clearAllStringStream();

		// clear file cache
		AQLFileAccessor::clearAllFileCache();

		// clear all instance
		clearInstance();

		AQLStaticDataManager::finalize();

		AQLLogManager::finalize();

		clearContext(true);

		mInitializeFlg = false;
		//delete AQLCoreDataService::mpInstance;
	}
	catch(AQLCoreError& e)
	{
        AQLCoreError ex("Error at AQLCoreDataService::finalize", __FILE__, __LINE__);
        ex += e;
		throw ex;
	}
	catch (...)
	{
        throw AQLCoreSystemError(__FILE__, __LINE__);
	}
}

// 
/*!
    @brief set context
	note: if already data is set, override value

	@param key
	@param data

*/
void
AQLCoreDataService::setContext(const AQLString &key, const AQLString &data)
{
#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
	// get current thread id

#if defined (WIN32) || defined (WIN64)
	DWORD thread_id = GetCurrentThreadId();
#else
	pthread_t thread_id = pthread_self();
#endif

    map<AQLString, AQLString> &context = mContextMap[thread_id];
	map<AQLString, AQLString>::iterator it = context.find(key);
	if (it != context.end())
	{
		context.erase(key);
	}
	context.insert(make_pair(key, data));
}

// 
/*!
    @brief get context

	@return AQLString

*/
AQLString
AQLCoreDataService::getContext(const AQLString &key)
{
#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
    // get current thread id

#if defined (WIN32) || defined (WIN64)
	DWORD thread_id = GetCurrentThreadId();
#else
	pthread_t thread_id = pthread_self();
#endif
	
	map<AQLString, AQLString> &context = mContextMap[thread_id];
	map<AQLString, AQLString>::const_iterator it = context.find(key);
	if (it != context.end())
	{
		return it->second;
	}
	else
	{
		return AQ_NO_DATA;
		//AQLString msg = AQLString("Key is not set in context key = ") + key;
		//throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
}

// 
/*!
    @brief set istringstream

	@param key
	@param istringstream
*/
void 
AQLCoreDataService::setIStringStream(const AQLString &key, std::istringstream *pstream)
{
#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
	map<AQLString, istringstream *>::iterator it = mIStringStreamMap.find(key);
	if (it != mIStringStreamMap.end())
	{
		delete it->second;
		mIStringStreamMap.erase(key);

		AQLFileAccessor::clearFileMember(key);
	}
	mIStringStreamMap.insert(make_pair(key, pstream));
}

// 
/*!
    @brief get istringstream

	@return istringstream pointer

*/
std::istringstream *
AQLCoreDataService::getIStringStream(const AQLString &key)
{
#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
	map<AQLString, istringstream *>::iterator it = mIStringStreamMap.find(key);

	if (it != mIStringStreamMap.end())
	{
		return it->second;
	}
	else
	{
		return 0;
	}

}


// 
/*!
    @brief set setting filestream
	note: if already data is set, override value

	@param[in] key
	@param[in] data

*/
void
AQLCoreDataService::setSettingFileStream(const AQLString key, const AQLString data)
{
#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
	map<AQLString, AQLString>::iterator it = mSettingFiles.find(key);
	if (it != mSettingFiles.end())
	{
		mSettingFiles.erase(key);
	}
	mSettingFiles.insert(make_pair(key, data));
}

// 
/*!
    @brief get context

	@return AQLString

*/
AQLString
AQLCoreDataService::getSettingFileStream(const AQLString key)
{
#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
	map<AQLString, AQLString>::iterator it = mSettingFiles.find(key);

	if (it != mSettingFiles.end())
	{
		AQLString tmp(it->second);
		return tmp;
		//return it->second;
	}
	else
	{
		AQLString msg = AQLString("Key is not set in settting file key = ") + key;
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
}

AQLString
AQLCoreDataService::getOutputDirectory()
{
	AQLString dirName = AQLCoreDataService::getStaticDataManager().
							getStaticData().getStaticData( KEY_DEAL_PV_FILE );	
	
	std::string sDirName( dirName.getCString() );
	unsigned int ex_pos = sDirName.find_last_of("/");
	AQLString dirName_no_ex;
	if ( ex_pos != std::string::npos )
	{
		dirName_no_ex = dirName.subString(0, ex_pos );
	}
	else
	{
		dirName_no_ex = "";
	}

	return dirName_no_ex;
}

// 
/*!
	@brief file exists or not

*/
bool
AQLCoreDataService::isFileExist(const AQLString &key)
{
	bool ret;
	if (AQLFileAccessor::isIStringStream())
	{
#ifdef __HAS_MIC__
		common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
		map<AQLString, istringstream *>::iterator it = mIStringStreamMap.find(key);

		if (it != mIStringStreamMap.end())
		{
			ret = true;
		}
		else
		{
			ret = false;
		}
	}
	else
	{
		ifstream fin;
		fin.open(key.getCString());
		if (fin)
		{
			ret = true;
		}
		else
		{
			ret = false;
		}
		fin.close();
	}

	return ret;
}
