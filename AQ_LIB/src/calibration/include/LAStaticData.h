#pragma once

#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma interface
#endif

#include <vector>
#include <map>
#include <unordered_map>
#include <functional>		// for std::hash

#include "LAString.h"
#include "LACoreTemplateType.h"

#if defined (WIN32) || defined (WIN64)
#include <windows.h>
#else
#include <pthread.h>
#endif
 
// Forward Declarations
class MAFileAccessor;
class LAStaticDataManager;

//===================== Class Declare LAStaticData==================================
class LAStaticData
{
public:
	LAString                    getStaticData(const LAString &key) const;
	LAString                    getStaticData(const LAString &key, const LAString &alias) const;
	void						setStaticData(const LAString &key, const LAString &value);
	void						removeStaticData(const LAString &key);

#ifdef VISUAL_STUDIO_2010_ANALYTICS
	LAStringMatrix           loadStaticDataObject(const LAString &prefix, const LAString &suffix) const;
#endif

private:

	friend class LAStaticDataManager;
	friend class LACoreDataService;
	friend class LACurveSetup;
	friend class StaticDataName;

	typedef std::map<LAString, LAString>::iterator Itr;
	typedef std::map<LAString, LAString>::const_iterator CItr;
	
	// constructor
	LAStaticData(const LAString &file, std::ios_base::openmode mode = std::ios_base::out);
	
	// destructor
	~LAStaticData();
    
	// copy constructor
	LAStaticData(const LAStaticData &rhs);
	
	// Assignment
	LAStaticData &operator=(const LAStaticData &rhs);

	// Members
	MAFileAccessor *mpFile;         
	std::map<LAString, LAString> mProp; 

#ifdef __HAS_MIC__
	mutable common_lib::Mutex mMutex;
#endif

};
