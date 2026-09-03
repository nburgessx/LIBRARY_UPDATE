/*! @file
    @brief   AQLPtbergUtils util class
*/
//  2007, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLPtbergUtils.cpp
//
//  DESCRIPTION :       LMM(Libor market model) util class
//                      
//  SEE ALSO    :       
//  VIRSION		:
//  STATUS      :       
////X///////////////////X///////////////////////////////X///////////////////
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif
//#define __DATAOUT__

#include "AQLPtbergUtils.h"
#include "AQLDefinitionsPtberg.h"
#include "AQLStaticDataManager.h"
#include "AQLStaticData.h"

using namespace std;

// constructor
/*!

*/
AQLPtbergUtils::AQLPtbergUtils(void)
{
}

// destructor
/*!

*/
AQLPtbergUtils::~AQLPtbergUtils(void)
{
}



/*!
    @brief check is 3f calib
	
*/
bool
AQLPtbergUtils::is3FCalib()
{
	AQLString is3FStr = AQLCoreDataService::getStaticDataManager().getStaticData().getStaticData(KEY_PTBERG_IS3F);
	AQLDataBool is3FCalib;
	if (is3FStr == AQ_NO_DATA)
	{
		is3FCalib.set(false);
	}
	else
	{
		is3FCalib.convertFromString(is3FStr);
	}
	if (is3FCalib.get())
	{
		AQLString is3FCalibStr = AQLCoreDataService::getStaticDataManager().getCalibStaticData().getStaticData(KEY_CALIB_PTBERG_IS3F);
		if (is3FCalibStr != AQ_NO_DATA)
		{
			is3FCalib.convertFromString(is3FCalibStr);
		}
	}
	return is3FCalib.get();
}
