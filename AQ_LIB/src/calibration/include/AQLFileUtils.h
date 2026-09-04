// $Id: AQLMarketData.h,v 1.5 2008/07/03 12:03:18 hirayake Exp $
/*! @file
    @brief  M-Library Market util class
*/
//  2007, AlgoQuantHub.
#ifndef AQLFileUtils_h
#define AQLFileUtils_h

#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma interface
#endif
#if defined(WIN32) || defined(WIN64)
#include <windows.h>
#else
#include <pthread.h>
#endif


class AQLString;
enum FileID
{
	FILENUM,		   
    MARKETID,		   
    AQLID,		    
    PROPERTIESID
};

//===================== Class Declare AQLMarketData==================================
/*! 
    @brief  Market data util class

   
*/
class AQLFileUtils
{
public:
	//=================================================
	// get number attached file name
	static AQLString getNumFileName(const AQLString &fileName, FileID fileid = MARKETID);

private:
friend class AQLCoreDataService;
	// constructor
	AQLFileUtils(void);
	// destructor
	~AQLFileUtils(void);
	// copy constructor
	AQLFileUtils(const AQLFileUtils &rhs);
	AQLFileUtils &operator=(const AQLFileUtils &rhs);

};


#endif
