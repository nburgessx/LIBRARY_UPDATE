/*! @file
    @brief AQLCalibrationParametersManager manager class
*/
//  2008, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       AQLCalibrationParametersManager.cpp
//
//  DESCRIPTION :       AQLCalibrationParametersManager 
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


#include "AQLDataBasics.h"
#include "AQLCalibrationParametersManager.h"
#include "AQLCalibrationParametersHW.h"
#include "AQLCalibrationParametersPtberg.h"
#include "AQLCalibrationParametersPtberg3F.h"
#include "AQLCalibrationParametersSZ.h"
#include "AQLCalibrationParametersLMM.h"
#include "AQLCalibrationParametersFXStrangleSolver.h"
#include "AQLCalibrationParametersIRSABR.h"
//hishida vannavolga
#include "AQLCalibrationParametersFXVannaVolga.h"
#include "AQLDefinitions.h"
#include "AQLDefinitionsPtberg.h"
#include "AQLStaticData.h"
#include "AQLPtbergUtils.h"


AQLCalibrationParametersManager *AQLCalibrationParametersManager::mpInstance = 0;

//================ AQLCalibrationParametersManager ===================================
// constructor
/*!

*/
AQLCalibrationParametersManager::AQLCalibrationParametersManager(void)
{
}

// destructor
/*!

*/
AQLCalibrationParametersManager::~AQLCalibrationParametersManager(void)
{ 
}

// 
/*!
    @brief get unique instance

	@return  AQLCalibrationParametersManager *
*/
AQLCalibrationParametersManager *
AQLCalibrationParametersManager::getInstance()
{
	if (!mpInstance)
	{
		mpInstance = new AQLCalibrationParametersManager();
	}
	return mpInstance;
}



// 
/*!
    @brief create calibrationinfo creator

	@param[in] model
	@return  AQLCalibrationParameters *
*/
AQLCalibrationParameters *
AQLCalibrationParametersManager::createCalibInfoCreator(const AQLString &model) const
{
	AQLString tmpModel = model;
	tmpModel.toUpper();
	AQLStaticData &staticData = AQLCoreDataService::getStaticDataManager().getStaticData();
	if (tmpModel == MODEL_HW)
	{
		return new AQLCalibrationParametersHW();
	}
	else if (tmpModel == MODEL_PTBERG)
	{
		// check 3F
		const bool is3F = AQLPtbergUtils::is3FCalib();
		if (is3F)
		{
			return new AQLCalibrationParametersPtberg3F;
		}
		else
		{
			return new AQLCalibrationParametersPtberg;
		}
	}
	else if (tmpModel == MODEL_SZ)
	{
		return new AQLCalibrationParametersSZ;
	}
	else if (tmpModel == MODEL_LMM)
	{
		return new AQLCalibrationParametersLMM();
	}
	else if (tmpModel == MODEL_FXSTRGLSLV)
	{
		return new AQLCalibrationParametersFXStrangleSolver();
	}
	else if (tmpModel == MODEL_IRSABR)
	{
		return new AQLCalibrationParametersIRSABR();
	}
	//hishida vannavolga
	else if (tmpModel == MODEL_FXVANNAVOLGA)
	{
		return new AQLCalibrationParametersFXVannaVolga();
	}
	return 0;
}

