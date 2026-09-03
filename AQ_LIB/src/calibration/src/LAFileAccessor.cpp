/*! @file
    @brief Class to load excel data
*/
//  2007, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       MAFileAccessor.h
//
//  DESCRIPTION :       Class to load txt file data
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

#include "LADataBasics.h"
#include "LAFileAccessor.h"
#include "LACoreDataService.h"
#include "LAString.h"
#include "LADefinitions.h"
#ifdef __HAS_MIC__

#endif

using namespace std;

bool MAFileAccessor::mIsIStringStream = false;
bool MAFileAccessor::mIsSFlgInitial = false;
std::map<LAString, LAStringVector> MAFileAccessor::mDataMap;
#ifdef __HAS_MIC__
common_lib::StaticMutex MAFileAccessor::mMutex;
#endif
// constructor
/*!
	file open in the constructor

	@param[in] file
	@param[in] mode

*/
MAFileAccessor::MAFileAccessor(const LAString &name, ios_base::openmode mode)
: mName(name), /*mIstream(new ifstream(file.getCString(),mode))*/mIstream(0), mFilemode(mode)
{
	if (!mIsSFlgInitial)
	{
		mIsIStringStream = isIStringStream();
		mIsSFlgInitial = true;
	}

	if (mIsIStringStream)
	{
		mIstream = LACoreDataService::getIStringStream(name);
	}
}

// destructor
/*!

*/
MAFileAccessor::~MAFileAccessor(void)
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
MAFileAccessor::MAFileAccessor(const MAFileAccessor &rhs)
: mName(rhs.mName), mIstream(0), mFilemode(rhs.mFilemode)
{
	if (mIsIStringStream)
	{
		mIstream = LACoreDataService::getIStringStream(rhs.mName);
	}	
}

// 
/*!
    @brief return is stringstream
	get all file data as LAStringVector

	@param[out] vec
*/
bool
MAFileAccessor::isIStringStream()
{
	LAString str = LACoreDataService::getContext(CONTEXT_KEY_ISPRICER);
	LADataBool tmp;
	tmp.convertFromString(str);
	return tmp.get();
}

// 
/*!
    @brief get all file data

	get all file data as LAStringVector

	@param[out] vec
*/
void
MAFileAccessor::readAllData(LAStringVector &vec)
{
	vec.clear();
#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
	try
	{
		//LAStringVector &sData = mDataMap[mName];
		//if (!sData.empty())
		//{
		//	vec = sData;

		//	return;
		//}
		map<LAString, LAStringVector>::const_iterator itr = mDataMap.find(mName);
		if (itr!=mDataMap.end())
		{
			vec = itr->second;

			return;
		}

		open();
		string str;
		LAStringVector &sData = mDataMap[mName];
		while (getline(*mIstream, str))
		{
			const LAString tmp(str.c_str());
			if (tmp.findString(STATIC_DATA_COMMENT_OUT_CHAR) == 0)
			{
				continue;
			}
			sData.push_back(LAString(str.c_str()));
		}

		vec = sData;
	}
	catch(LACoreError &e)
	{
		throw e;
	}
	catch(...)
	{
		LAString msg = "Error has occurred in reading file . file = " + mName;
		throw LACoreSystemError(msg.getCString(), __FILE__, __LINE__);
	}
}

// 
/*!
    @brief get all file data

	get all file data as LAStringMatrix.
	data is tokenized as specified param.
	
	@param[in] demi
	@param[out] mat
*/
void
MAFileAccessor::readAllData(const char demi, LAStringMatrix &mat)
{
	LAStringVector rowVec;
	readAllData(rowVec);

	mat.clear();
	mat.resize(rowVec.size());

	int row = 0;
	StrItr it = rowVec.begin();
	while (it != rowVec.end())
	{
		const LAString &line = *it;
		mat[row] = line.toToken(demi);
		++row;
		++it;
	}
}

// 
/*!
    @brief get all file data

	get all file data as LAStringMatrix.
	data is tokenized as specified param.
	
	@param[in] demi
	@param[in] trimStr
	@param[out] mat
	@param[in] isLock
*/
void
MAFileAccessor::readAllData(const char demi, const LAString trimStr, LAStringMatrix &mat)
{
	LAStringVector rowVec;
	readAllData(rowVec);

	mat.clear();
	mat.resize(rowVec.size());

	int row = 0;
	StrItr it = rowVec.begin();
	while (it != rowVec.end())
	{
		LAString &line = *it;
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
MAFileAccessor::open(void)
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
		LAString msg = LAString(e.what()) + " Cannot open file.. File : " + mName;
		throw LACoreSystemError(msg.getCString(), __FILE__, __LINE__);
	}
	// check
	if (!mIstream || !*mIstream)
	{
		LAString msg = mName + " cannot be open ";
		throw LACoreSystemError(msg.getCString(), __FILE__, __LINE__);
	}

}

// 
/*!
	@brief close file

*/
void
MAFileAccessor::close(void)
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
		LAString msg = LAString(e.what()) + " Cannnot close file.. File : " + mName;
		throw LACoreSystemError(msg.getCString(), __FILE__, __LINE__);
	}
}

// 
/*!
    @brief initialize method

*/
void
MAFileAccessor::initialize()
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
MAFileAccessor::clearFileCache(const LAString &fileNum)
{
#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
	// clear file cache
	LAStringVector keyVec;
	map<LAString, LAStringVector>::iterator fIt = MAFileAccessor::mDataMap.begin();
	while (fIt != MAFileAccessor::mDataMap.end())
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
		MAFileAccessor::mDataMap.erase(keyVec[i]);
	}
}

// clear all filecache
/*!
	@brief clear all filecache

*/
void
MAFileAccessor::clearAllFileCache()
{
#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
	// clear file cache
	map<LAString, LAStringVector>::iterator fIt = MAFileAccessor::mDataMap.begin();
	while (fIt != MAFileAccessor::mDataMap.end())
	{
		fIt->second.clear();
		++fIt;
	}
	MAFileAccessor::mDataMap.clear();
}

// clear file member
/*!
	@brief clear file member
	@param[in] key file key
*/
void
MAFileAccessor::clearFileMember(const LAString &key)
{
#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
	// clear file member
	map<LAString, LAStringVector>::iterator fIt = MAFileAccessor::mDataMap.find(key);
	if (fIt != MAFileAccessor::mDataMap.end())
	{
		fIt->second.clear();
		MAFileAccessor::mDataMap.erase(key);
	}
}


