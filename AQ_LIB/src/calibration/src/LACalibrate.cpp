
#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "LACalibrate.h"
#include "LACoreDataService.h"
#include "LACoreAppError.h"
#include <sstream>
#ifdef __HAS_MIC__

#endif
using namespace std;

map<LAString, LAString> LACalibrate::mSerializeMap;
map<LAString, map<LAString, LAString> > LACalibrate::mDeserializedEMap;
map<LAString, bool> LACalibrate::mIsDeserializedMap;
#ifdef __HAS_MIC__
common_lib::StaticMutex LACalibrate::mMutex;
#endif
// constructor
/*!

*/

LACalibrate::LACalibrate()
:mGridPos(0), mpFunc(0), mpCaibEngine(0), mpDataInstance(0)
{
}

// destructor
/*!

*/
LACalibrate::~LACalibrate()
{
}

/*!
    @brief clear 
	clear static member

	@param[in] fileNum

*/
void
LACalibrate::clear(const LAString &fileNum)
{
#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
	// clear serialized map
	map<LAString, LAString>::iterator it = mSerializeMap.begin();
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
	map<LAString, std::map<LAString, LAString> >::iterator it_ = mDeserializedEMap.begin();
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
	map<LAString, bool>::iterator it__ = mIsDeserializedMap.begin();
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
LACalibrate::clear()
{
#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
	// clear serialized map
	mSerializeMap.clear();
	// clear deserialized object map
	map<LAString, std::map<LAString, LAString> >::iterator it = mDeserializedEMap.begin();
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
	Deserialize stream from LACoreDataService.
	And create mDeserializedEMap

	@param[in] key

*/
void
LACalibrate::deserializeStream(const LAString &key)
{
#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
	//deserialize
	istringstream *dataStream = LACoreDataService::getIStringStream(key);
	if (!dataStream)
	{
		LAString msg = "Serialize stream is not set in LACoreDataService. Key = " + key;
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}

	map<LAString, LAString> &dataMap = mDeserializedEMap[key];
	dataMap.clear();

	// read data
	string str;
	while (getline(*dataStream, str))
	{
		LAString line = LAString(str.c_str());
		LAString tempKey = line.toToken(',')[0];
		line.exchange(tempKey + ",", "");
		dataMap[tempKey] = line;
	}
}
/*!
    @brief set mSerializeMap
	@param[in] fileNum
*/
void 
LACalibrate::setmSerializeMap(const LAString &fileNum)
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
LACalibrate::setmIsDeserializedMap(bool isDeserializedMap)
{
#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
	mIsDeserializedMap[mSerializeFile] = isDeserializedMap;
}
