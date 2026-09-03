#pragma once

#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma interface
#endif

#include <map>
#include "AQLCoreAppError.h"

#if defined(WIN32) || defined(WIN64)
#include <windows.h>
#else
#include <pthread.h>
#endif

class AQLCoreDataService;
class AQLString;
class AQLStaticData;

#ifdef __HAS_MIC__
namespace common_lib {
	struct StaticMutex;
}
#endif
//===================== Class Declare AQLStaticDataManager==================================
class AQLStaticDataManager
{
public:
	
	// constructor
	AQLStaticDataManager(void);
	
	// destructor
	~AQLStaticDataManager(void);

	// copy constructor
	AQLStaticDataManager(const AQLStaticDataManager &rhs);
		
	AQLStaticDataManager &operator=(const AQLStaticDataManager &rhs);
	
	// return propertyAccessor
	static AQLStaticData     &getStaticData();
	
	// return grid propertyAccessor
	static AQLStaticData     &getIRGStaticData();
	
	// return risk propertyAccessor
	static AQLStaticData     &getRiskStaticData();
	
	// return calib propertyAccessor
	static AQLStaticData     &getCalibStaticData();

	// return xva propertyAccessor
	static AQLStaticData     &getXVAStaticData();

	// return credit propertyAccessor
	static AQLStaticData     &getCreditStaticData();

	// return market collection propertyAccessor
	static AQLStaticData     &getMktCollectionStaticData();

private:
friend class AQLCoreDataService;
	//==============================================
	// finalize method                 
	static void  finalize();

	//==============================================
	// file cache all clear method                 
	static void         clearStaticDataObject(const AQLString &fileNum);
	//==============================================
	// clear risk properties                 
	static void         clearRiskStaticDataObject(const AQLString &fileNum);
	//==============================================
    // clear calib properties 
    static void         clearCalibStaticDataObject(const AQLString &fileNum);
	//==============================================
    // clear grid properties 
    static void         clearGridStaticData(const AQLString &fileNum);
	//==============================================
	// clear xva properties 
	static void         clearXVAStaticDataObject(const AQLString &fileNum);
	//==============================================
	// clear credit properties 
	static void         clearCreditStaticDataObject(const AQLString &fileNum);

private:
	//==============================================
	// return property ID
	static AQLString     getPropertiesID();
	//==============================================

	typedef std::map<AQLString, AQLStaticData *> pMap;
    typedef std::map<AQLString, AQLStaticData *>::iterator pIter;

	//==============================================
	// return propertyAccessor
	static AQLStaticData     &getStaticData(const AQLString& propertyName, pMap& propertyMap);

	static pMap mProperties;
	static pMap mRiskProperties;
	static pMap mCalibProperties;
	static pMap mGridStaticData;
	static pMap mXVAProperties;
	static pMap mCreditProperties;
	static pMap mMktCollectionProperties;

#ifdef __HAS_MIC__
	static common_lib::StaticMutex mMutex;
#endif

};

