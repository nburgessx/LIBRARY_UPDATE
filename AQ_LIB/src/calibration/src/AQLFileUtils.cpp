// $Id: AQLMarketData.cpp,v 1.6 2008/07/14 10:42:25 hirayake Exp $
/*! @file
    @brief M-Library Market data util class
*/
//  2007, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLMarketData.cpp
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


#include "AQLFileUtils.h"
#include "AQLDefinitions.h"
#include "AQLCoreDataService.h"
#include "AQLString.h"
// constructor
/*!

*/
AQLFileUtils::AQLFileUtils(void)
{
}

// destructor
/*!

*/
AQLFileUtils::~AQLFileUtils(void)
{
}

// 
/*!
    @brief number attached fileName

	@param fileName
	@return AQLString
*/
AQLString
AQLFileUtils::getNumFileName(const AQLString &fileName, FileID fileid)
{
	AQLString num = AQ_NO_DATA;
	if (fileid == MARKETID)
	{
		num = AQLCoreDataService::getContext(ARG_KEY_MARKETID);
	}
	else if (fileid == MLIBID)
	{
		num = AQLCoreDataService::getContext(ARG_KEY_MLIBID);
	}
	else if (fileid == PROPERTIESID)
	{
		num = AQLCoreDataService::getContext(ARG_KEY_PROPERTIESID);
	}

	if (num == AQ_NO_DATA)
	{
		 num = AQLCoreDataService::getContext(ARG_KEY_FILENUM);
	}

	if (num != AQ_NO_DATA)
	{
		std::string sFileName(fileName.getCString());
		unsigned int ex_pos = sFileName.find_last_of(".");
		if (ex_pos != std::string::npos)
		{
			AQLString extension = fileName.subString(ex_pos, fileName.size() - 1);
			AQLString fileName_no_ex = fileName.subString(0, ex_pos - 1);
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
