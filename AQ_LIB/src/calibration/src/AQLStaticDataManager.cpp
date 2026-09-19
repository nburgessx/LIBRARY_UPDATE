#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLStaticDataManager.h"
#include "AQLString.h"
#include "AQLStaticData.h"
#include "AQLMarketData.h"

std::map<StandardString, StandardString *> staticDataMapByCurve_;
std::map<AQLString, AQLStaticData *> AQLStaticDataManager::mProperties;
std::map<AQLString, AQLStaticData *> AQLStaticDataManager::mRiskProperties;
std::map<AQLString, AQLStaticData *> AQLStaticDataManager::mCalibProperties;
std::map<AQLString, AQLStaticData *> AQLStaticDataManager::mGridStaticData;
std::map<AQLString, AQLStaticData *> AQLStaticDataManager::mXVAProperties;
std::map<AQLString, AQLStaticData *> AQLStaticDataManager::mCreditProperties;
std::map<AQLString, AQLStaticData *> AQLStaticDataManager::mMktCollectionProperties;

//================ AQLStaticDataManager ===================================
// constructor
/*!

*/
AQLStaticDataManager::AQLStaticDataManager(void)
{
}

// destructor
/*!

*/
AQLStaticDataManager::~AQLStaticDataManager(void)
{
}

// 
/*!
	@brief finalize method

*/
void
AQLStaticDataManager::finalize(void)
{


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
	@return AQLStaticData

*/
AQLStaticData &
AQLStaticDataManager::getStaticData(const AQLString& propertyName, pMap& propertyMap)
{
	AQLString id = getPropertiesID();
	pIter itr = propertyMap.find(id);
	if (itr == propertyMap.end())
	{
		try
		{
			AQLString filePath = AQLMarketData::getNumFileName(AQLString(PROPERTY_DIR) + propertyName, PROPERTIESID);
			if (!AQLCoreDataService::isFileExist(filePath))
			{
				filePath = AQLMarketData::getNumFileName(propertyName, PROPERTIESID);
			}
			propertyMap.insert(std::make_pair(id, new AQLStaticData(filePath)));
			itr = propertyMap.find(id);
		}
		catch (AQLCoreError &e)
		{
			throw e;
		}
		catch (...)
		{
			throw AQLCoreSystemError("Error has occurred in creating property accessor.", __FILE__, __LINE__);
		}
	}

	return *itr->second;
}

AQLStaticData & AQLStaticDataManager::getStaticData()
{
	return AQLStaticDataManager::getStaticData(PROPERTY_FILE, mProperties);
}

AQLStaticData & AQLStaticDataManager::getRiskStaticData()
{
	return AQLStaticDataManager::getStaticData(RISK_PROPERTY_FILE, mRiskProperties);
}

AQLStaticData & AQLStaticDataManager::getCalibStaticData()
{
	return AQLStaticDataManager::getStaticData(CALIB_PROPERTY_FILE, mCalibProperties);
}

AQLStaticData & AQLStaticDataManager::getIRGStaticData()
{
	return AQLStaticDataManager::getStaticData(GRID_PROPERTY_FILE, mGridStaticData);
}

AQLStaticData & AQLStaticDataManager::getXVAStaticData()
{
	return AQLStaticDataManager::getStaticData(XVA_PROPERTY_FILE, mXVAProperties);
}

AQLStaticData & AQLStaticDataManager::getCreditStaticData()
{
	return AQLStaticDataManager::getStaticData(CREDIT_PROPERTY_FILE, mCreditProperties);
}

AQLStaticData & AQLStaticDataManager::getMktCollectionStaticData()
{
	return AQLStaticDataManager::getStaticData(MKTCOLLECTION_PROPERTY_FILE, mMktCollectionProperties);
}

void AQLStaticDataManager::clearStaticDataObject(const AQLString &fileNum)
{
	// clear properties
	std::map<AQLString, AQLStaticData *>::iterator proIt = mProperties.find(fileNum);
	if (proIt != mProperties.end())
	{
		delete proIt->second;
		mProperties.erase(fileNum);
	}
}

void AQLStaticDataManager::clearRiskStaticDataObject(const AQLString &fileNum)
{
	// clear risk properties
	std::map<AQLString, AQLStaticData *>::iterator proIt = mRiskProperties.find(fileNum);
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
AQLStaticDataManager::clearCalibStaticDataObject(const AQLString &fileNum) 
{
	// clear calib properties 
	std::map<AQLString, AQLStaticData *>::iterator proIt = mCalibProperties.find(fileNum); 
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
AQLStaticDataManager::clearGridStaticData(const AQLString &fileNum) 
{ 
	// clear calib properties 
	std::map<AQLString, AQLStaticData *>::iterator proIt = mGridStaticData.find(fileNum); 
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
AQLStaticDataManager::clearXVAStaticDataObject(const AQLString &fileNum)
{
	// clear calib properties 
	std::map<AQLString, AQLStaticData *>::iterator proIt = mXVAProperties.find(fileNum);
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
AQLStaticDataManager::clearCreditStaticDataObject(const AQLString &fileNum)
{
	// clear credit properties 
	std::map<AQLString, AQLStaticData *>::iterator proIt = mCreditProperties.find(fileNum);
	if (proIt != mCreditProperties.end())
	{
		delete proIt->second;
		mCreditProperties.erase(fileNum);
	}
}


// 
/*!
    @brief get properties id 

	@return  AQLString
*/
AQLString
AQLStaticDataManager::getPropertiesID()
{
	AQLString id = AQLCoreDataService::getContext(ARG_KEY_PROPERTIESID);
	if (id == AQ_NO_DATA)
	{
		id = AQLCoreDataService::getContext(ARG_KEY_FILENUM);
	}
	return id;
}
