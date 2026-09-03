#pragma once

#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma interface
#endif

#include <map>
#include "LACoreAppError.h"

#if defined(WIN32) || defined(WIN64)
#include <windows.h>
#else
#include <pthread.h>
#endif

class LACoreDataService;
class LAString;
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
	static void         clearStaticDataObject(const LAString &fileNum);
	//==============================================
	// clear risk properties                 
	static void         clearRiskStaticDataObject(const LAString &fileNum);
	//==============================================
    // clear calib properties 
    static void         clearCalibStaticDataObject(const LAString &fileNum);
	//==============================================
    // clear grid properties 
    static void         clearGridStaticData(const LAString &fileNum);
	//==============================================
	// clear xva properties 
	static void         clearXVAStaticDataObject(const LAString &fileNum);
	//==============================================
	// clear credit properties 
	static void         clearCreditStaticDataObject(const LAString &fileNum);

private:
	//==============================================
	// return property ID
	static LAString     getPropertiesID();
	//==============================================

	typedef std::map<LAString, LAStaticData *> pMap;
    typedef std::map<LAString, LAStaticData *>::iterator pIter;

	//==============================================
	// return propertyAccessor
	static LAStaticData     &getStaticData(const LAString& propertyName, pMap& propertyMap);

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

