/*! @file
    @brief Class to load excel data
*/
#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "AQLDataBasics.h"
#include "AQLFileAccessor.h"
#include "AQLCoreDataService.h"
#include "AQLString.h"
#include "AQLDefinitions.h"
#ifdef __HAS_MIC__

#endif

using namespace std;

bool AQLFileAccessor::mIsIStringStream = false;
bool AQLFileAccessor::mIsSFlgInitial = false;
std::map<AQLString, AQLStringVector> AQLFileAccessor::mDataMap;
#ifdef __HAS_MIC__
common_lib::StaticMutex AQLFileAccessor::mMutex;
#endif
// constructor
/*!
	file open in the constructor

	@param[in] file
	@param[in] mode

*/
AQLFileAccessor::AQLFileAccessor(const AQLString &name, ios_base::openmode mode)
: mName(name), /*mIstream(new ifstream(file.getCString(),mode))*/mIstream(0), mFilemode(mode)
{
	if (!mIsSFlgInitial)
	{
		mIsIStringStream = isIStringStream();
		mIsSFlgInitial = true;
	}

	if (mIsIStringStream)
	{
		mIstream = AQLCoreDataService::getIStringStream(name);
	}
}

// destructor
/*!

*/
AQLFileAccessor::~AQLFileAccessor(void)
{
	if (!mIsIStringStream)
	{
		close();
		delete mIstream;
	}
}

// copy constructor
/*!

*/
AQLFileAccessor::AQLFileAccessor(const AQLFileAccessor &rhs)
: mName(rhs.mName), mIstream(0), mFilemode(rhs.mFilemode)
{
	if (mIsIStringStream)
	{
		mIstream = AQLCoreDataService::getIStringStream(rhs.mName);
	}	
}

// 
/*!
    @brief return is stringstream
	get all file data as AQLStringVector

	@param[out] vec
*/
bool
AQLFileAccessor::isIStringStream()
{
	AQLString str = AQLCoreDataService::getContext(CONTEXT_KEY_ISPRICER);
	AQLDataBool tmp;
	tmp.convertFromString(str);
	return tmp.get();
}

// 
/*!
    @brief get all file data

	get all file data as AQLStringVector

	@param[out] vec
*/
void
AQLFileAccessor::readAllData(AQLStringVector &vec)
{
	vec.clear();
#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
	try
	{
		//AQLStringVector &sData = mDataMap[mName];
		//if (!sData.empty())
		//{
		//	vec = sData;

		//	return;
		//}
		map<AQLString, AQLStringVector>::const_iterator itr = mDataMap.find(mName);
		if (itr!=mDataMap.end())
		{
			vec = itr->second;

			return;
		}

		open();
		string str;
		AQLStringVector &sData = mDataMap[mName];
		while (getline(*mIstream, str))
		{
			const AQLString tmp(str.c_str());
			if (tmp.findString(STATIC_DATA_COMMENT_OUT_CHAR) == 0)
			{
				continue;
			}
			sData.push_back(AQLString(str.c_str()));
		}

		vec = sData;
	}
	catch(AQLCoreError &e)
	{
		throw e;
	}
	catch(...)
	{
		AQLString msg = "Error has occurred in reading file . file = " + mName;
		throw AQLCoreSystemError(msg.getCString(), __FILE__, __LINE__);
	}
}

// 
/*!
    @brief get all file data

	get all file data as AQLStringMatrix.
	data is tokenized as specified param.
	
	@param[in] demi
	@param[out] mat
*/
void
AQLFileAccessor::readAllData(const char demi, AQLStringMatrix &mat)
{
	AQLStringVector rowVec;
	readAllData(rowVec);

	mat.clear();
	mat.resize(rowVec.size());

	int row = 0;
	StrItr it = rowVec.begin();
	while (it != rowVec.end())
	{
		const AQLString &line = *it;
		mat[row] = line.toToken(demi);
		++row;
		++it;
	}
}

// 
/*!
    @brief get all file data

	get all file data as AQLStringMatrix.
	data is tokenized as specified param.
	
	@param[in] demi
	@param[in] trimStr
	@param[out] mat
	@param[in] isLock
*/
void
AQLFileAccessor::readAllData(const char demi, const AQLString trimStr, AQLStringMatrix &mat)
{
	AQLStringVector rowVec;
	readAllData(rowVec);

	mat.clear();
	mat.resize(rowVec.size());

	int row = 0;
	StrItr it = rowVec.begin();
	while (it != rowVec.end())
	{
		AQLString &line = *it;
		line.exchange(trimStr, "");
		mat[row] = line.toToken(demi);
		++row;
		++it;
	}
}

// 
/*!
    @brief open file

*/
void
AQLFileAccessor::open(void)
{
	if (mIstream && *mIstream)
	{
		if (!mIsIStringStream)
		{
			dynamic_cast<ifstream *>(mIstream)->close();
		}
	}
	
	try
	{
		if (!mIsIStringStream)
		{
			// open
			delete mIstream;
			mIstream = new ifstream(mName.getCString(), mFilemode);
		}
	}
	catch (bad_alloc &e)
	{
		AQLString msg = AQLString(e.what()) + " Cannot open file.. File : " + mName;
		throw AQLCoreSystemError(msg.getCString(), __FILE__, __LINE__);
	}
	// check
	if (!mIstream || !*mIstream)
	{
		AQLString msg = mName + " cannot be open ";
		throw AQLCoreSystemError(msg.getCString(), __FILE__, __LINE__);
	}

}

// 
/*!
	@brief close file

*/
void
AQLFileAccessor::close(void)
{
	try
	{
		if (!mIsIStringStream && mIstream)
		{
			dynamic_cast<ifstream *>(mIstream)->close();
		}
	}
	catch (exception &e)
	{
		AQLString msg = AQLString(e.what()) + " Cannnot close file.. File : " + mName;
		throw AQLCoreSystemError(msg.getCString(), __FILE__, __LINE__);
	}
}

// 
/*!
    @brief initialize method

*/
void
AQLFileAccessor::initialize()
{
	mIsIStringStream = false;
	mIsSFlgInitial = false;
}

// clear filecache
/*!
	@brief clear filecache
	@param[in] fileNum

*/
void
AQLFileAccessor::clearFileCache(const AQLString &fileNum)
{
#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
	// clear file cache
	AQLStringVector keyVec;
	map<AQLString, AQLStringVector>::iterator fIt = AQLFileAccessor::mDataMap.begin();
	while (fIt != AQLFileAccessor::mDataMap.end())
	{
		if (fIt->first.findString(fileNum) >= 0)
		{
			fIt->second.clear();
			keyVec.push_back(fIt->first);
		}
		++fIt;
	}
	const unsigned int delSize = keyVec.size();
	for (unsigned int i = 0; i < delSize; ++i)
	{
		AQLFileAccessor::mDataMap.erase(keyVec[i]);
	}
}

// clear all filecache
/*!
	@brief clear all filecache

*/
void
AQLFileAccessor::clearAllFileCache()
{
#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
	// clear file cache
	map<AQLString, AQLStringVector>::iterator fIt = AQLFileAccessor::mDataMap.begin();
	while (fIt != AQLFileAccessor::mDataMap.end())
	{
		fIt->second.clear();
		++fIt;
	}
	AQLFileAccessor::mDataMap.clear();
}

// clear file member
/*!
	@brief clear file member
	@param[in] key file key
*/
void
AQLFileAccessor::clearFileMember(const AQLString &key)
{
#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
	// clear file member
	map<AQLString, AQLStringVector>::iterator fIt = AQLFileAccessor::mDataMap.find(key);
	if (fIt != AQLFileAccessor::mDataMap.end())
	{
		fIt->second.clear();
		AQLFileAccessor::mDataMap.erase(key);
	}
}


