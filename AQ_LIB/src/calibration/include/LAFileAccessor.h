#pragma once

#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma interface
#endif

#include <fstream>
#include <string>
#include <sstream>

#if defined (WIN32) || defined (WIN64)
#include <windows.h>
#else
#include <pthread.h>
#endif

#include "AQLCoreTemplateType.h"
#include "AQLCoreSystemError.h"
#include "LADefinitions.h"

class LACoreDataService;
class AQLString;
//===================== Class Declare MAFileAccessor==================================
/*! 
    @brief txt file accessor class

    
*/
class MAFileAccessor
{
public:
	// constructor
	explicit MAFileAccessor(const AQLString &file, 
					std::ios_base::openmode mode = std::ios_base::in);
	// destructor
	virtual ~MAFileAccessor(void);
	// copy constructor
	MAFileAccessor(const MAFileAccessor &rhs);
	//==============================================================================
	// read all data as vector<AQLString>
	virtual void				readAllData(AQLStringVector &vec);
	//==============================================================================
	// read all data as vector<vector<AQLString> >  
	virtual void			readAllData(const char demi, AQLStringMatrix &mat);
//==============================================================================
	// read all data as vector<vector<AQLString> >  
	virtual void			readAllData(const char demi, const AQLString trimStr, AQLStringMatrix &mat);

	//==============================================================================
	// ! file open method
	virtual void						open();
	//==============================================================================
	// ! file close method
	virtual void						close();

protected:

friend class LACoreDataService;
	MAFileAccessor &operator=(const MAFileAccessor &rhs);
	//==============================================
	// initialize method                 
	static void initialize();
	// check string stream
	static bool isIStringStream();

	//==============================================
	// file cache clear method
	static void         clearFileCache(const AQLString &fileNum);
	//==============================================
	// file cache all clear method
	static void         clearAllFileCache();
	//==============================================
	// file member clear
	static void         clearFileMember(const AQLString &key);

private:
	const AQLString mName;     // file or stream name
	//AQLStringVector mData;       // file data 
	//std::ifstream *mFilestream;   // file stream
	std::istream *mIstream;   // stream
	std::ios_base::openmode mFilemode; // open mode 

	static std::map<AQLString, AQLStringVector> mDataMap; // staic data map
	static bool mIsIStringStream; // string stream flg
	static bool mIsSFlgInitial; // string stream flg
#ifdef __HAS_MIC__
	static common_lib::StaticMutex mMutex;
#endif
};

