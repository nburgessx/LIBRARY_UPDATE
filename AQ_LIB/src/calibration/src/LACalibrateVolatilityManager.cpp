/*! @file
    @brief LACalibrateVolatilityManager manager class
*/
//  2008, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LACalibrateVolatilityManager.cpp
//
//  DESCRIPTION :       LACalibrateVolatilityManager 
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


#include "LACalibrateVolatilityManager.h"
#include "LACalibrateVolatilityLMM.h"
#include "LACalibrateVolatilityHW.h"
#include "LACalibrateVolatilityPtberg.h"
#include "LACalibrateVolatilitySZ.h"
#include "LACalibrateVolatilityFXStrangleSolver.h"
#include "LACalibrateVolatilityIRSABR.h"
#include "LADefinitions.h"
#include "LADefinitionsRisk.h"
//hishida vannavolga
#include "LACalibrateVolatilityFXVannaVolga.h"


LACalibrateVolatilityManager *LACalibrateVolatilityManager::mpInstance = 0;

//================ LACalibrateVolatilityManager ===================================
// constructor
/*!

*/
LACalibrateVolatilityManager::LACalibrateVolatilityManager(void)
{
}

// destructor
/*!

*/
LACalibrateVolatilityManager::~LACalibrateVolatilityManager(void)
{ 
}

// 
/*!
    @brief get unique instance

	@return  LACalibrateVolatilityManager *
*/
LACalibrateVolatilityManager *
LACalibrateVolatilityManager::getInstance()
{
	if (!mpInstance)
	{
		mpInstance = new LACalibrateVolatilityManager();
	}
	return mpInstance;
}



// 
/*!
    @brief create Volatility creator

	@param[in] model
	@return  LACalibrateVolatility *
*/
LACalibrateVolatility *
LACalibrateVolatilityManager::createVolCreator(const AQLString &model) const
{
	AQLString tmpModel = model;
	tmpModel.toUpper();
	if (tmpModel == MODEL_LMM)
	{
		return new LACalibrateVolatilityLMM();
	}
	else if (tmpModel == MODEL_HW)
	{
		return new LACalibrateVolatilityHW();
	}
	else if (tmpModel == MODEL_PTBERG)
	{
		return new LACalibrateVolatilityPtberg();
	}
	else if (tmpModel == MODEL_SZ)
	{
		return new LACalibrateVolatilitySZ();
	}
	else if (tmpModel == MODEL_FXSTRGLSLV)
	{
		return new LACalibrateVolatilityFXStrangleSolver();
	}
	//hishida vannavolga
	else if (tmpModel == MODEL_FXVANNAVOLGA)
	{
		return new LACalibrateVolatilityFXVannaVolga();
	}
	else if (tmpModel == MODEL_IRSABR)
	{
		return new LACalibrateVolatilityIRSABR();
	}

	return 0;
}

