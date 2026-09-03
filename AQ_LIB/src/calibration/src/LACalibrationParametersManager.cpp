/*! @file
    @brief LACalibrationParametersManager manager class
*/
//  2008, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LACalibrationParametersManager.cpp
//
//  DESCRIPTION :       LACalibrationParametersManager 
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


#include "LADataBasics.h"
#include "LACalibrationParametersManager.h"
#include "LACalibrationParametersHW.h"
#include "LACalibrationParametersPtberg.h"
#include "LACalibrationParametersPtberg3F.h"
#include "LACalibrationParametersSZ.h"
#include "LACalibrationParametersLMM.h"
#include "LACalibrationParametersFXStrangleSolver.h"
#include "LACalibrationParametersIRSABR.h"
//hishida vannavolga
#include "LACalibrationParametersFXVannaVolga.h"
#include "LADefinitions.h"
#include "LADefinitionsPtberg.h"
#include "LAStaticData.h"
#include "LAPtbergUtils.h"


LACalibrationParametersManager *LACalibrationParametersManager::mpInstance = 0;

//================ LACalibrationParametersManager ===================================
// constructor
/*!

*/
LACalibrationParametersManager::LACalibrationParametersManager(void)
{
}

// destructor
/*!

*/
LACalibrationParametersManager::~LACalibrationParametersManager(void)
{ 
}

// 
/*!
    @brief get unique instance

	@return  LACalibrationParametersManager *
*/
LACalibrationParametersManager *
LACalibrationParametersManager::getInstance()
{
	if (!mpInstance)
	{
		mpInstance = new LACalibrationParametersManager();
	}
	return mpInstance;
}



// 
/*!
    @brief create calibrationinfo creator

	@param[in] model
	@return  LACalibrationParameters *
*/
LACalibrationParameters *
LACalibrationParametersManager::createCalibInfoCreator(const LAString &model) const
{
	LAString tmpModel = model;
	tmpModel.toUpper();
	LAStaticData &staticData = LACoreDataService::getStaticDataManager().getStaticData();
	if (tmpModel == MODEL_HW)
	{
		return new LACalibrationParametersHW();
	}
	else if (tmpModel == MODEL_PTBERG)
	{
		// check 3F
		const bool is3F = MAPtbergUtils::is3FCalib();
		if (is3F)
		{
			return new LACalibrationParametersPtberg3F;
		}
		else
		{
			return new LACalibrationParametersPtberg;
		}
	}
	else if (tmpModel == MODEL_SZ)
	{
		return new LACalibrationParametersSZ;
	}
	else if (tmpModel == MODEL_LMM)
	{
		return new LACalibrationParametersLMM();
	}
	else if (tmpModel == MODEL_FXSTRGLSLV)
	{
		return new LACalibrationParametersFXStrangleSolver();
	}
	else if (tmpModel == MODEL_IRSABR)
	{
		return new LACalibrationParametersIRSABR();
	}
	//hishida vannavolga
	else if (tmpModel == MODEL_FXVANNAVOLGA)
	{
		return new LACalibrationParametersFXVannaVolga();
	}
	return 0;
}

