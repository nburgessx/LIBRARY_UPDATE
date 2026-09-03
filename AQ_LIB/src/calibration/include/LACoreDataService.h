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

class LAString;
class LAStaticDataManager;
class MALogManager;

#ifdef __HAS_MIC__
namespace common_lib {
	struct StaticMutex;
}
#endif

//===================== Class Declare LACoreDataService==================================
/*! 
    @brief  Service provide class
*/
class LACoreDataService
{
public:
	// constructor
	LACoreDataService(void);
	// destructor
	~LACoreDataService(void);
	// copy constructor
	LACoreDataService(const LACoreDataService &rhs);
	LACoreDataService &operator=(const LACoreDataService &rhs);

	//==============================================
	// return property manager
	static LAStaticDataManager        &getStaticDataManager()
	{
		return *mpPropertyManager;
	}
	//==============================================
	// return log manager
	static MALogManager               &getLogManager()
	{
		return *mpLogManager;
	}
	//==============================================
	// set context
	static void setContext(const LAString &key, const LAString &data);
	//==============================================
	// get context
	static LAString getContext(const LAString &key);
	//==============================================
	// set istring stream
	static void setIStringStream(const LAString &key, std::istringstream *pstream);
	//==============================================
	// get istring stream
	static std::istringstream *getIStringStream(const LAString &key);
	//==============================================
	// set setting file stream
	static void setSettingFileStream(LAString key, LAString data);
	//==============================================
	// get setting file stream
	static LAString getSettingFileStream(LAString key);
	//==============================================
	// initialize method                 
	static void         initialize();
	//==============================================
	// clear method                 
	static void         clear(const LAString &fileNum);
    //==============================================
	// finalize method
	static void finalize();

	static LAString		getOutputDirectory( void );
	//==============================================================================
	// ! file exists or not
	static bool			isFileExist(const LAString &key);

	//==============================================
	// context clear method                 
	static void         clearContext(bool isAll = true);
private:
	//==============================================
	// string stream clear method                 
	static void         clearStringStream(const LAString &fileNum);
	//==============================================
	// string stream all clear method                 
	static void         clearAllStringStream();
	//==============================================
	// clear singleton instance
	static void clearInstance(void);

	static bool mInitializeFlg; // initialize flag

#if defined (WIN32) || defined (WIN64)
	static std::map<DWORD, std::map<LAString, LAString> > mContextMap;// context
#else
	static std::map<pthread_t, std::map<LAString, LAString> > mContextMap;// context
#endif

	static std::map<LAString, std::istringstream *> mIStringStreamMap;// istringstream map

    static std::map<LAString, LAString> mSettingFiles;
	static LAStaticDataManager *mpPropertyManager; //! property manager
	static MALogManager  *mpLogManager;      //! log manager

#ifdef __HAS_MIC__
	static common_lib::StaticMutex mMutex;
#endif

};

