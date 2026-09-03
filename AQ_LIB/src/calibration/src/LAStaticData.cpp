// LAStaticData.cpp

#include "LAStaticData.h"
#include "LAFileAccessor.h"
#include "LADefinitions.h"
#include "ExceptionMacros.h"


// constructor
LAStaticData::LAStaticData(const AQLString &file, std::ios_base::openmode mode)
: mpFile(new MAFileAccessor(file, mode))
{
	AQLStringMatrix mat;
	mpFile->readAllData(STATIC_DATA_DELIMITER, mat);
	AQLStringMatrix::iterator it = mat.begin();
	while (it != mat.end())
	{
		AQLString key(it->at(0));

		int coPos = key.findString(STATIC_DATA_COMMENT_OUT_CHAR);
		int keySize = key.size();
		// it'comment out or blank
		if ( coPos  == 0 || keySize < 1)
		{
			++it;
			continue;
		}

		if (it->size() != 2)
		{
			AQLString msg = "Invalid Property File Data: Data must consist of a key and value";
			throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
		}

		AQLString value(it->at(1));

		mProp.insert(std::make_pair(key, value));
		++it;
	}
	mpFile->close();
}


// destructor
LAStaticData::~LAStaticData(void)
{
	mpFile->close();
	delete mpFile;
}


AQLString LAStaticData::getStaticData(const AQLString &key) const
{

#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::Mutex> lock(mMutex);
#endif

	CItr it = mProp.find(key);

	if (it != mProp.end())
	{
		return it->second;
	}
	else
	{
		return AQ_NO_DATA;
	}
}


AQLString LAStaticData::getStaticData(const AQLString &key, const AQLString &alias) const
{

#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::Mutex> lock(mMutex);
#endif

	// Search for Primary Key
	CItr it = mProp.find(key);
	
	// Search for Alias Key
	CItr it1 = mProp.find(alias);
	
	// Prevent Key and Alias Keys being used at the same time
	if( it != mProp.end() && it1 != mProp.end() )
	{
		AQ_THROW("Duplicate Static Data Key: Cannot use key '" + key + "' and alias '" + alias + "' at the same time")
	}

	// First Return Primary Key if found
	if (it != mProp.end())
	{
		return it->second;
	}

	// Second Return Alias Key if Primary Key not Used
	if (it1 != mProp.end())
	{
		return it1->second;
	}
	
	return AQ_NO_DATA;
}


void LAStaticData::setStaticData(const AQLString &key, const AQLString &value)
{

#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::Mutex> lock(mMutex);
#endif

	Itr it = mProp.find(key);

	if (it != mProp.end())
	{
		mProp.erase(key);
	}
	mProp.insert(std::make_pair(key, value));
}


void LAStaticData::removeStaticData(const AQLString &key)
{

#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::Mutex> lock(mMutex);
#endif

	Itr it = mProp.find(key);

	if (it != mProp.end())
	{
		mProp.erase(key);
	}
}
