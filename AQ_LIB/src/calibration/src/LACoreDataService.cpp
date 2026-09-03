
#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include <map>
#include "LACoreDataService.h"
#include "LAFileAccessor.h"
#include "LAStaticDataManager.h"
#include "LAStaticData.h"
#include "LALogManager.h"
#include "LAMarketData.h"

#ifndef VISUAL_STUDIO_2010_ANALYTICS
#include "LAModelManager.h"
#include "LAModelConfiguration.h"
#include "LAObjectConfigurationManager.h"
#include "LARiskConfigurationManager.h"
#include "LAScenarioConfigurationManager.h"
#include "LACalibrateVolatilityManager.h"
#include "LACalibrationParametersManager.h"
#include "LACalibratePool.h"
#include "LACalibrate.h"
#endif

#ifdef __HAS_MIC__

#endif

using namespace std;

map<LAString, istringstream *> LACoreDataService::mIStringStreamMap;
map<LAString, LAString> LACoreDataService::mSettingFiles;
LAStaticDataManager *LACoreDataService::mpPropertyManager = 0;
MALogManager *LACoreDataService::mpLogManager = 0;
bool LACoreDataService::mInitializeFlg = false;

#ifdef __HAS_MIC__
common_lib::StaticMutex LACoreDataService::mMutex;
#endif

#if defined (WIN32) || defined (WIN64)
map<DWORD, map<LAString, LAString> > LACoreDataService::mContextMap;
#else
map<pthread_t, map<LAString, LAString> > LACoreDataService::mContextMap;
#endif
 
//LAString LACoreDataService::mPropertyFile;

//================ LACoreDataService ===================================

// constructor
/*!

*/
LACoreDataService::LACoreDataService(void)
{
}


// destructor
/*!

*/
LACoreDataService::~LACoreDataService(void)
{
}

// initialize
/*!
	@brief initialize service

*/
void
LACoreDataService::initialize(void)
{
	if(!mInitializeFlg)
	{
		// call initialize
		MAFileAccessor::initialize();
		mpPropertyManager = new LAStaticDataManager();
		mpLogManager = new MALogManager();
#ifdef __HAS_MIC__
		// prepare using instances for multi-thread
		MAMasterRegistManager::getInstance();
		LAModelConfiguration::getInstance();
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
LACoreDataService::clearContext(bool isAll)
{
#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
	if (isAll)
	{

#if defined (WIN32) || defined (WIN64)
		map<DWORD, map<LAString, LAString> >::iterator it = mContextMap.begin();
#else
		map<pthread_t, map<LAString, LAString> >::iterator it = mContextMap.begin();
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
LACoreDataService::clearStringStream(const LAString &fileNum)
{
#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
	// clear string stream
	LAStringVector keyVec;
	map<LAString, istringstream *>::iterator strIt = mIStringStreamMap.begin();
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
LACoreDataService::clearAllStringStream()
{
#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
	map<LAString, istringstream *>::iterator strIt = mIStringStreamMap.begin();
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
LACoreDataService::clear(const LAString &fileNum)
{
	// clear properties
	LAStaticDataManager::clearStaticDataObject(fileNum);
	// clear risk properties
	LAStaticDataManager::clearRiskStaticDataObject(fileNum);
	// clear calib properties 
	LAStaticDataManager::clearCalibStaticDataObject(fileNum); 
	// clear grid properties 
	LAStaticDataManager::clearGridStaticData(fileNum); 
	// clear file cache
	MAFileAccessor::clearFileCache(fileNum);
	// clear string stream
	clearStringStream(fileNum);
	// clear context
	clearContext(false);
#ifndef VISUAL_STUDIO_2010_ANALYTICS
	// clear calibration data
	LACalibrate::clear(fileNum);
#endif
}

// clear instance
/*!
	@brief delete sigleton instance

*/
void
LACoreDataService::clearInstance(void)
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

	if (MAMasterRegistManager::mpInstance)
	{
		delete MAMasterRegistManager::mpInstance;
		MAMasterRegistManager::mpInstance = 0;
	}
	if (LAModelConfiguration::mpInstance)
	{
		delete LAModelConfiguration::mpInstance;
		LAModelConfiguration::mpInstance = 0;
	}
	if (LAObjectConfigurationManager::mpInstance)
	{
		delete LAObjectConfigurationManager::mpInstance;
		LAObjectConfigurationManager::mpInstance = 0;
	}
	if (LARiskConfigurationManager::mpInstance)
	{
		delete LARiskConfigurationManager::mpInstance;
		LARiskConfigurationManager::mpInstance = 0;
	}
	if (LAScenarioConfigurationManager::mpInstance)
	{
		delete LAScenarioConfigurationManager::mpInstance;
		LAScenarioConfigurationManager::mpInstance = 0;
	}
	if (LACalibrateVolatilityManager::mpInstance)
	{
		delete LACalibrateVolatilityManager::mpInstance;
		LACalibrateVolatilityManager::mpInstance = 0;
	}
	if (LACalibrationParametersManager::mpInstance)
	{
		delete LACalibrationParametersManager::mpInstance;
		LACalibrationParametersManager::mpInstance = 0;
	}

	if (LACalibratePool::mpInstance)
	{
		delete LACalibratePool::mpInstance;
		LACalibratePool::mpInstance = 0;
	}

#endif
	
}

// finalize
/*!
	@brief delete sigleton instance

*/
void
LACoreDataService::finalize(void)
{
	try
	{
		// stream clear
		clearAllStringStream();

		// clear file cache
		MAFileAccessor::clearAllFileCache();

		// clear all instance
		clearInstance();

		LAStaticDataManager::finalize();

		MALogManager::finalize();

		clearContext(true);

		mInitializeFlg = false;
		//delete LACoreDataService::mpInstance;
	}
	catch(LACoreError& e)
	{
        LACoreError ex("Error at LACoreDataService::finalize", __FILE__, __LINE__);
        ex += e;
		throw ex;
	}
	catch (...)
	{
        throw LACoreSystemError(__FILE__, __LINE__);
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
LACoreDataService::setContext(const LAString &key, const LAString &data)
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

    map<LAString, LAString> &context = mContextMap[thread_id];
	map<LAString, LAString>::iterator it = context.find(key);
	if (it != context.end())
	{
		context.erase(key);
	}
	context.insert(make_pair(key, data));
}

// 
/*!
    @brief get context

	@return LAString

*/
LAString
LACoreDataService::getContext(const LAString &key)
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
	
	map<LAString, LAString> &context = mContextMap[thread_id];
	map<LAString, LAString>::const_iterator it = context.find(key);
	if (it != context.end())
	{
		return it->second;
	}
	else
	{
		return AQ_NO_DATA;
		//LAString msg = LAString("Key is not set in context key = ") + key;
		//throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
}

// 
/*!
    @brief set istringstream

	@param key
	@param istringstream
*/
void 
LACoreDataService::setIStringStream(const LAString &key, std::istringstream *pstream)
{
#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
	map<LAString, istringstream *>::iterator it = mIStringStreamMap.find(key);
	if (it != mIStringStreamMap.end())
	{
		delete it->second;
		mIStringStreamMap.erase(key);

		MAFileAccessor::clearFileMember(key);
	}
	mIStringStreamMap.insert(make_pair(key, pstream));
}

// 
/*!
    @brief get istringstream

	@return istringstream pointer

*/
std::istringstream *
LACoreDataService::getIStringStream(const LAString &key)
{
#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
	map<LAString, istringstream *>::iterator it = mIStringStreamMap.find(key);

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
LACoreDataService::setSettingFileStream(const LAString key, const LAString data)
{
#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
	map<LAString, LAString>::iterator it = mSettingFiles.find(key);
	if (it != mSettingFiles.end())
	{
		mSettingFiles.erase(key);
	}
	mSettingFiles.insert(make_pair(key, data));
}

// 
/*!
    @brief get context

	@return LAString

*/
LAString
LACoreDataService::getSettingFileStream(const LAString key)
{
#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
	map<LAString, LAString>::iterator it = mSettingFiles.find(key);

	if (it != mSettingFiles.end())
	{
		LAString tmp(it->second);
		return tmp;
		//return it->second;
	}
	else
	{
		LAString msg = LAString("Key is not set in settting file key = ") + key;
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}
}

LAString
LACoreDataService::getOutputDirectory()
{
	LAString dirName = LACoreDataService::getStaticDataManager().
							getStaticData().getStaticData( KEY_DEAL_PV_FILE );	
	
	std::string sDirName( dirName.getCString() );
	unsigned int ex_pos = sDirName.find_last_of("/");
	LAString dirName_no_ex;
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
LACoreDataService::isFileExist(const LAString &key)
{
	bool ret;
	if (MAFileAccessor::isIStringStream())
	{
#ifdef __HAS_MIC__
		common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
		map<LAString, istringstream *>::iterator it = mIStringStreamMap.find(key);

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
