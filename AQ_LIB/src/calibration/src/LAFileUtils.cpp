// $Id: LAMarketData.cpp,v 1.6 2008/07/14 10:42:25 hirayake Exp $
/*! @file
    @brief M-Library Market data util class
*/
//  2007, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAMarketData.cpp
//
//  DESCRIPTION :       Market data util class
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "LAFileUtils.h"
#include "LADefinitions.h"
#include "LACoreDataService.h"
#include "LAString.h"
// constructor
/*!

*/
MAFileUtils::MAFileUtils(void)
{
}

// destructor
/*!

*/
MAFileUtils::~MAFileUtils(void)
{
}

// 
/*!
    @brief number attached fileName

	@param fileName
	@return LAString
*/
LAString
MAFileUtils::getNumFileName(const LAString &fileName, FileID fileid)
{
	LAString num = MLIB_NO_DATA;
	if (fileid == MARKETID)
	{
		num = LACoreDataService::getContext(ARG_KEY_MARKETID);
	}
	else if (fileid == MLIBID)
	{
		num = LACoreDataService::getContext(ARG_KEY_MLIBID);
	}
	else if (fileid == PROPERTIESID)
	{
		num = LACoreDataService::getContext(ARG_KEY_PROPERTIESID);
	}

	if (num == MLIB_NO_DATA)
	{
		 num = LACoreDataService::getContext(ARG_KEY_FILENUM);
	}

	if (num != MLIB_NO_DATA)
	{
		std::string sFileName(fileName.getCString());
		unsigned int ex_pos = sFileName.find_last_of(".");
		if (ex_pos != std::string::npos)
		{
			LAString extension = fileName.subString(ex_pos, fileName.size() - 1);
			LAString fileName_no_ex = fileName.subString(0, ex_pos - 1);
			return fileName_no_ex + num + extension;
		}
		else
		{
			// no extension case
			return fileName + num;
		}
	}
	else
	{
		return fileName;
	}
}
