#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "LAStaticDataManager.h"
#include "LAString.h"
#include "LAStaticData.h"
#include "LAMarketData.h"
#ifdef __HAS_MIC__

#endif

std::map<StandardString, StandardString *> staticDataMapByCurve_;
std::map<LAString, LAStaticData *> LAStaticDataManager::mProperties;
std::map<LAString, LAStaticData *> LAStaticDataManager::mRiskProperties;
std::map<LAString, LAStaticData *> LAStaticDataManager::mCalibProperties;
std::map<LAString, LAStaticData *> LAStaticDataManager::mGridStaticData;
std::map<LAString, LAStaticData *> LAStaticDataManager::mXVAProperties;
std::map<LAString, LAStaticData *> LAStaticDataManager::mCreditProperties;
std::map<LAString, LAStaticData *> LAStaticDataManager::mMktCollectionProperties;

#ifdef __HAS_MIC__
common_lib::StaticMutex LAStaticDataManager::mMutex;
#endif
//================ LAStaticDataManager ===================================
// constructor
/*!

*/
LAStaticDataManager::LAStaticDataManager(void)
{
}

// destructor
/*!

*/
LAStaticDataManager::~LAStaticDataManager(void)
{
}

// 
/*!
	@brief finalize method

*/
void
LAStaticDataManager::finalize(void)
{

#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif

	for (pIter itr=mProperties.begin(); itr!= mProperties.end(); itr++)
    {
		if (itr->second)
		{
			delete itr->second;
			itr->second = 0;
		} 
    }
	mProperties.clear();
	for (pIter itr=mRiskProperties.begin(); itr!= mRiskProperties.end(); itr++)
    {
  		if (itr->second)
		{
			delete itr->second;
			itr->second = 0;
		}
    }
	mRiskProperties.clear();

	for (pIter itr=mCalibProperties.begin(); itr!= mCalibProperties.end(); itr++)
    {
  		if (itr->second)
		{
			delete itr->second;
			itr->second = 0;
		}
    }
	mCalibProperties.clear();

	for (pIter itr=mGridStaticData.begin(); itr!= mGridStaticData.end(); itr++)
    {
  		if (itr->second)
		{
			delete itr->second;
			itr->second = 0;
		}
    }
	mGridStaticData.clear();

	for (pIter itr = mXVAProperties.begin(); itr != mXVAProperties.end(); itr++)
	{
		if (itr->second)
		{
			delete itr->second;
			itr->second = 0;
		}
	}
	mXVAProperties.clear();

	for (pIter itr = mCreditProperties.begin(); itr != mCreditProperties.end(); itr++)
	{
		if (itr->second)
		{
			delete itr->second;
			itr->second = 0;
		}
	}
	mCreditProperties.clear();

	for (pIter itr = mMktCollectionProperties.begin(); itr != mMktCollectionProperties.end(); itr++)
	{
		if (itr->second)
		{
			delete itr->second;
			itr->second = 0;
		}
	}
	mMktCollectionProperties.clear();
}


// 
/*!
	@brief get property accessor

	@param [in] propertyName	property name (ir.properties, calib.properties, .......)
	@param [in] propertyMap		property map (mProperties, mCalibProperties, .......)
	@return LAStaticData

*/
LAStaticData &
LAStaticDataManager::getStaticData(const LAString& propertyName, pMap& propertyMap)
{
#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
	LAString id = getPropertiesID();
	pIter itr = propertyMap.find(id);
	if (itr == propertyMap.end())
	{
		try
		{
			LAString filePath = LAMarketData::getNumFileName(LAString(PROPERTY_DIR) + propertyName, PROPERTIESID);
			if (!LACoreDataService::isFileExist(filePath))
			{
				filePath = LAMarketData::getNumFileName(propertyName, PROPERTIESID);
			}
			propertyMap.insert(std::make_pair(id, new LAStaticData(filePath)));
			itr = propertyMap.find(id);
		}
		catch (LACoreError &e)
		{
			throw e;
		}
		catch (...)
		{
			throw LACoreSystemError("Error has occurred in creating property accessor.", __FILE__, __LINE__);
		}
	}

	return *itr->second;
}

LAStaticData & LAStaticDataManager::getStaticData()
{
	return LAStaticDataManager::getStaticData(PROPERTY_FILE, mProperties);
}

LAStaticData & LAStaticDataManager::getRiskStaticData()
{
	return LAStaticDataManager::getStaticData(RISK_PROPERTY_FILE, mRiskProperties);
}

LAStaticData & LAStaticDataManager::getCalibStaticData()
{
	return LAStaticDataManager::getStaticData(CALIB_PROPERTY_FILE, mCalibProperties);
}

LAStaticData & LAStaticDataManager::getIRGStaticData()
{
	return LAStaticDataManager::getStaticData(GRID_PROPERTY_FILE, mGridStaticData);
}

LAStaticData & LAStaticDataManager::getXVAStaticData()
{
	return LAStaticDataManager::getStaticData(XVA_PROPERTY_FILE, mXVAProperties);
}

LAStaticData & LAStaticDataManager::getCreditStaticData()
{
	return LAStaticDataManager::getStaticData(CREDIT_PROPERTY_FILE, mCreditProperties);
}

LAStaticData & LAStaticDataManager::getMktCollectionStaticData()
{
	return LAStaticDataManager::getStaticData(MKTCOLLECTION_PROPERTY_FILE, mMktCollectionProperties);
}

void LAStaticDataManager::clearStaticDataObject(const LAString &fileNum)
{
#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
	// clear properties
	std::map<LAString, LAStaticData *>::iterator proIt = mProperties.find(fileNum);
	if (proIt != mProperties.end())
	{
		delete proIt->second;
		mProperties.erase(fileNum);
	}
}

void LAStaticDataManager::clearRiskStaticDataObject(const LAString &fileNum)
{
#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
	// clear risk properties
	std::map<LAString, LAStaticData *>::iterator proIt = mRiskProperties.find(fileNum);
	if (proIt != mRiskProperties.end())
	{
		delete proIt->second;
		mRiskProperties.erase(fileNum);
	}
}

// clear calib properties 
/*! 
       @brief clear calib properties 
       @param[in] fileNum file number 
 
*/ 
void 
LAStaticDataManager::clearCalibStaticDataObject(const LAString &fileNum) 
{
#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
	// clear calib properties 
	std::map<LAString, LAStaticData *>::iterator proIt = mCalibProperties.find(fileNum); 
	if (proIt != mCalibProperties.end()) 
	{ 
		delete proIt->second; 
		mCalibProperties.erase(fileNum); 
	} 
} 

// clear grid properties 
/*! 
       @brief clear grid properties 
       @param[in] fileNum file number 
 
*/ 
void 
LAStaticDataManager::clearGridStaticData(const LAString &fileNum) 
{ 
#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
	// clear calib properties 
	std::map<LAString, LAStaticData *>::iterator proIt = mGridStaticData.find(fileNum); 
	if (proIt != mGridStaticData.end()) 
	{ 
		delete proIt->second; 
		mGridStaticData.erase(fileNum); 
	} 
} 

// clear grid properties 
/*!
	@brief clear xva properties
	@param[in] fileNum file number

*/
void
LAStaticDataManager::clearXVAStaticDataObject(const LAString &fileNum)
{
#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
	// clear calib properties 
	std::map<LAString, LAStaticData *>::iterator proIt = mXVAProperties.find(fileNum);
	if (proIt != mXVAProperties.end())
	{
		delete proIt->second;
		mXVAProperties.erase(fileNum);
	}
}

// clear credit properties 
/*!
@brief clear credit properties
@param[in] fileNum file number

*/
void
LAStaticDataManager::clearCreditStaticDataObject(const LAString &fileNum)
{
#ifdef __HAS_MIC__
	common_lib::ScopedLock<common_lib::StaticMutex> lock(mMutex);
#endif
	// clear credit properties 
	std::map<LAString, LAStaticData *>::iterator proIt = mCreditProperties.find(fileNum);
	if (proIt != mCreditProperties.end())
	{
		delete proIt->second;
		mCreditProperties.erase(fileNum);
	}
}


// 
/*!
    @brief get properties id 

	@return  LAString
*/
LAString
LAStaticDataManager::getPropertiesID()
{
	LAString id = LACoreDataService::getContext(ARG_KEY_PROPERTIESID);
	if (id == AQ_NO_DATA)
	{
		id = LACoreDataService::getContext(ARG_KEY_FILENUM);
	}
	return id;
}
