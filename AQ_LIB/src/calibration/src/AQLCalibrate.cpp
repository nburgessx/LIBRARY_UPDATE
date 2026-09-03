
#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "AQLCalibrate.h"
#include "AQLCoreDataService.h"
#include "AQLCoreAppError.h"
#include <sstream>
#ifdef __HAS_MIC__

#endif
using namespace std;

map<AQLString, AQLString> AQLCalibrate::mSerializeMap;
map<AQLString, map<AQLString, AQLString> > AQLCalibrate::mDeserializedEMap;
map<AQLString, bool> AQLCalibrate::mIsDeserializedMap;
#ifdef __HAS_MIC__
common_lib::StaticMutex AQLCalibrate::mMutex;
#endif
// constructor
/*!

*/

AQLCalibrate::AQLCalibrate()
:mGridPos(0), mpFunc(0), mpCaibEngine(0), mpDataInstance(0)
{
}

// destructor
/*!

*/
AQLCalibrate::~AQLCalibrate()
{
}

/*!
    @brief clear 
	clear static member

	@param[in] fileNum

*/
void
AQLCalibrate::clear(const AQLString &fileNum)
{
#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
	// clear serialized map
	map<AQLString, AQLString>::iterator it = mSerializeMap.begin();
	while (it != mSerializeMap.end())
	{
		if (it->first.findString(fileNum) >= 0)
		{
			mSerializeMap.erase(it++);
		}
		else
		{
			++it;
		}
	}
	// clear deserialized object map
	map<AQLString, std::map<AQLString, AQLString> >::iterator it_ = mDeserializedEMap.begin();
	while (it_ != mDeserializedEMap.end())
	{
		if (it_->first.findString(fileNum) >= 0)
		{
			it_->second.clear();
			mDeserializedEMap.erase(it_++);
		}
		else
		{
			++it_;
		}
	}
	// clear deserialized flag
	map<AQLString, bool>::iterator it__ = mIsDeserializedMap.begin();
	while (it__ != mIsDeserializedMap.end())
	{
		if (it__->first.findString(fileNum) >= 0)
		{
			mIsDeserializedMap.erase(it__++);
		}
		else
		{
			++it__;
		}
	}
}

/*!
    @brief clear
	clear static member

*/
void
AQLCalibrate::clear()
{
#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
	// clear serialized map
	mSerializeMap.clear();
	// clear deserialized object map
	map<AQLString, std::map<AQLString, AQLString> >::iterator it = mDeserializedEMap.begin();
	while (it != mDeserializedEMap.end())
	{
		it->second.clear();
		++it;
	}
	mDeserializedEMap.clear();
	// clear deserialized flag
	mIsDeserializedMap.clear();
}


/*!
    @brief Deserialize stream
	Deserialize stream from AQLCoreDataService.
	And create mDeserializedEMap

	@param[in] key

*/
void
AQLCalibrate::deserializeStream(const AQLString &key)
{
#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
	//deserialize
	istringstream *dataStream = AQLCoreDataService::getIStringStream(key);
	if (!dataStream)
	{
		AQLString msg = "Serialize stream is not set in AQLCoreDataService. Key = " + key;
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}

	map<AQLString, AQLString> &dataMap = mDeserializedEMap[key];
	dataMap.clear();

	// read data
	string str;
	while (getline(*dataStream, str))
	{
		AQLString line = AQLString(str.c_str());
		AQLString tempKey = line.toToken(',')[0];
		line.exchange(tempKey + ",", "");
		dataMap[tempKey] = line;
	}
}
/*!
    @brief set mSerializeMap
	@param[in] fileNum
*/
void 
AQLCalibrate::setmSerializeMap(const AQLString &fileNum)
{
#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
	mSerializeMap[mSerializeFile] += fileNum + "\n";
}

/*!
    @brief set mIsDeserializedMap
	@param[in] isDeserializedMap
*/
void 
AQLCalibrate::setmIsDeserializedMap(bool isDeserializedMap)
{
#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
	mIsDeserializedMap[mSerializeFile] = isDeserializedMap;
}
