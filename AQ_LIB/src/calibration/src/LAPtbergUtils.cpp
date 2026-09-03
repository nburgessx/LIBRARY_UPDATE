/*! @file
    @brief   MAPtbergUtils util class
*/
//  2007, Mizuho International London.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       MAPtbergUtils.cpp
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

#include "LAPtbergUtils.h"
#include "LADefinitionsPtberg.h"
#include "LAStaticDataManager.h"
#include "LAStaticData.h"

using namespace std;

// constructor
/*!

*/
MAPtbergUtils::MAPtbergUtils(void)
{
}

// destructor
/*!

*/
MAPtbergUtils::~MAPtbergUtils(void)
{
}



/*!
    @brief check is 3f calib
	
*/
bool
MAPtbergUtils::is3FCalib()
{
	LAString is3FStr = LACoreDataService::getStaticDataManager().getStaticData().getStaticData(KEY_PTBERG_IS3F);
	LADataBool is3FCalib;
	if (is3FStr == MLIB_NO_DATA)
	{
		is3FCalib.set(false);
	}
	else
	{
		is3FCalib.convertFromString(is3FStr);
	}
	if (is3FCalib.get())
	{
		LAString is3FCalibStr = LACoreDataService::getStaticDataManager().getCalibStaticData().getStaticData(KEY_CALIB_PTBERG_IS3F);
		if (is3FCalibStr != MLIB_NO_DATA)
		{
			is3FCalib.convertFromString(is3FCalibStr);
		}
	}
	return is3FCalib.get();
}
