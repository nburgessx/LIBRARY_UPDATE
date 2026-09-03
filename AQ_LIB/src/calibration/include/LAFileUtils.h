// $Id: LAMarketData.h,v 1.5 2008/07/03 12:03:18 hirayake Exp $
/*! @file
    @brief  M-Library Market util class
*/
//  2007, Mizuho International London.
#ifndef MAFileUtils_h
#define MAFileUtils_h

///X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAMarketData.h
//
//  DESCRIPTION :       M-Library Market Data util class 
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma interface
#endif
#if defined(WIN32) || defined(WIN64)
#include <windows.h>
#else
#include <pthread.h>
#endif


class LAString;
enum FileID
{
	FILENUM,		   
    MARKETID,		   
    MLIBID,		    
    PROPERTIESID
};

//===================== Class Declare LAMarketData==================================
/*! 
    @brief  Market data util class

   
*/
class MAFileUtils
{
public:
	//=================================================
	// get number attached file name
	static LAString getNumFileName(const LAString &fileName, FileID fileid = MARKETID);

private:
friend class LACoreDataService;
	// constructor
	MAFileUtils(void);
	// destructor
	~MAFileUtils(void);
	// copy constructor
	MAFileUtils(const MAFileUtils &rhs);
	MAFileUtils &operator=(const MAFileUtils &rhs);

};


#endif
