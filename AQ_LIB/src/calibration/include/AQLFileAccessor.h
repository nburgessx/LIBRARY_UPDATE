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
#include "AQLDefinitions.h"

class AQLCoreDataService;
class AQLString;
//===================== Class Declare AQLFileAccessor==================================
/*! 
    @brief txt file accessor class

    
*/
class AQLFileAccessor
{
public:
	// constructor
	explicit AQLFileAccessor(const AQLString &file, 
					std::ios_base::openmode mode = std::ios_base::in);
	// destructor
	virtual ~AQLFileAccessor(void);
	// copy constructor
	AQLFileAccessor(const AQLFileAccessor &rhs);
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

friend class AQLCoreDataService;
	AQLFileAccessor &operator=(const AQLFileAccessor &rhs);
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
};

