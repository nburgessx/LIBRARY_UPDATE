#pragma once

#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma interface
#endif

#include <sstream>
#include <map>

#if defined (WIN32) || defined (WIN64)
#include <windows.h>
#else
#include <pthread.h>
#endif

class AQLString;
class AQLStaticDataManager;
class AQLLogManager;

#ifdef __HAS_MIC__
namespace common_lib {
	struct StaticMutex;
}
#endif

//===================== Class Declare AQLCoreDataService==================================
/*! 
    @brief  Service provide class
*/
class AQLCoreDataService
{
public:
	// constructor
	AQLCoreDataService(void);
	// destructor
	~AQLCoreDataService(void);
	// copy constructor
	AQLCoreDataService(const AQLCoreDataService &rhs);
	AQLCoreDataService &operator=(const AQLCoreDataService &rhs);

	//==============================================
	// return property manager
	static AQLStaticDataManager        &getStaticDataManager()
	{
		return *mpPropertyManager;
	}
	//==============================================
	// return log manager
	static AQLLogManager               &getLogManager()
	{
		return *mpLogManager;
	}
	//==============================================
	// set context
	static void setContext(const AQLString &key, const AQLString &data);
	//==============================================
	// get context
	static AQLString getContext(const AQLString &key);
	//==============================================
	// set istring stream
	static void setIStringStream(const AQLString &key, std::istringstream *pstream);
	//==============================================
	// get istring stream
	static std::istringstream *getIStringStream(const AQLString &key);
	//==============================================
	// set setting file stream
	static void setSettingFileStream(AQLString key, AQLString data);
	//==============================================
	// get setting file stream
	static AQLString getSettingFileStream(AQLString key);
	//==============================================
	// initialize method                 
	static void         initialize();
	//==============================================
	// clear method                 
	static void         clear(const AQLString &fileNum);
    //==============================================
	// finalize method
	static void finalize();

	static AQLString		getOutputDirectory( void );
	//==============================================================================
	// ! file exists or not
	static bool			isFileExist(const AQLString &key);

	//==============================================
	// context clear method                 
	static void         clearContext(bool isAll = true);
private:
	//==============================================
	// string stream clear method                 
	static void         clearStringStream(const AQLString &fileNum);
	//==============================================
	// string stream all clear method                 
	static void         clearAllStringStream();
	//==============================================
	// clear singleton instance
	static void clearInstance(void);

	static bool mInitializeFlg; // initialize flag

#if defined (WIN32) || defined (WIN64)
	static std::map<DWORD, std::map<AQLString, AQLString> > mContextMap;// context
#else
	static std::map<pthread_t, std::map<AQLString, AQLString> > mContextMap;// context
#endif

	static std::map<AQLString, std::istringstream *> mIStringStreamMap;// istringstream map

    static std::map<AQLString, AQLString> mSettingFiles;
	static AQLStaticDataManager *mpPropertyManager; //! property manager
	static AQLLogManager  *mpLogManager;      //! log manager

#ifdef __HAS_MIC__
	static common_lib::StaticMutex mMutex;
#endif

};

