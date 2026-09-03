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

class LACoreDataService;
class AQLString;
class LAStaticData;

#ifdef __HAS_MIC__
namespace common_lib {
	struct StaticMutex;
}
#endif
//===================== Class Declare LAStaticDataManager==================================
class LAStaticDataManager
{
public:
	
	// constructor
	LAStaticDataManager(void);
	
	// destructor
	~LAStaticDataManager(void);

	// copy constructor
	LAStaticDataManager(const LAStaticDataManager &rhs);
		
	LAStaticDataManager &operator=(const LAStaticDataManager &rhs);
	
	// return propertyAccessor
	static LAStaticData     &getStaticData();
	
	// return grid propertyAccessor
	static LAStaticData     &getIRGStaticData();
	
	// return risk propertyAccessor
	static LAStaticData     &getRiskStaticData();
	
	// return calib propertyAccessor
	static LAStaticData     &getCalibStaticData();

	// return xva propertyAccessor
	static LAStaticData     &getXVAStaticData();

	// return credit propertyAccessor
	static LAStaticData     &getCreditStaticData();

	// return market collection propertyAccessor
	static LAStaticData     &getMktCollectionStaticData();

private:
friend class LACoreDataService;
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

	typedef std::map<AQLString, LAStaticData *> pMap;
    typedef std::map<AQLString, LAStaticData *>::iterator pIter;

	//==============================================
	// return propertyAccessor
	static LAStaticData     &getStaticData(const AQLString& propertyName, pMap& propertyMap);

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

