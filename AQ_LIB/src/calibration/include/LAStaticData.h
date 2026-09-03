#pragma once

#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma interface
#endif

#include <vector>
#include <map>
#include <unordered_map>
#include <functional>		// for std::hash

#include "AQLString.h"
#include "AQLCoreTemplateType.h"

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
	AQLString                    getStaticData(const AQLString &key) const;
	AQLString                    getStaticData(const AQLString &key, const AQLString &alias) const;
	void						setStaticData(const AQLString &key, const AQLString &value);
	void						removeStaticData(const AQLString &key);

#ifdef VISUAL_STUDIO_2010_ANALYTICS
	AQLStringMatrix           loadStaticDataObject(const AQLString &prefix, const AQLString &suffix) const;
#endif

private:

	friend class LAStaticDataManager;
	friend class LACoreDataService;
	friend class LACurveSetup;
	friend class StaticDataName;

	typedef std::map<AQLString, AQLString>::iterator Itr;
	typedef std::map<AQLString, AQLString>::const_iterator CItr;
	
	// constructor
	LAStaticData(const AQLString &file, std::ios_base::openmode mode = std::ios_base::out);
	
	// destructor
	~LAStaticData();
    
	// copy constructor
	LAStaticData(const LAStaticData &rhs);
	
	// Assignment
	LAStaticData &operator=(const LAStaticData &rhs);

	// Members
	MAFileAccessor *mpFile;         
	std::map<AQLString, AQLString> mProp; 

#ifdef __HAS_MIC__
	mutable common_lib::Mutex mMutex;
#endif

};
