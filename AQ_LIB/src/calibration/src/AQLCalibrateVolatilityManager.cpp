/*! @file
    @brief AQLCalibrateVolatilityManager manager class
*/
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLCalibrateVolatilityManager.h"
#include "AQLCalibrateVolatilityLMM.h"
#include "AQLCalibrateVolatilityHW.h"
#include "AQLCalibrateVolatilityPtberg.h"
#include "AQLCalibrateVolatilitySZ.h"
#include "AQLCalibrateVolatilityFXStrangleSolver.h"
#include "AQLCalibrateVolatilityIRSABR.h"
#include "AQLDefinitions.h"
#include "AQLDefinitionsRisk.h"
// vanna-volga
#include "AQLCalibrateVolatilityFXVannaVolga.h"


AQLCalibrateVolatilityManager *AQLCalibrateVolatilityManager::mpInstance = 0;

//================ AQLCalibrateVolatilityManager ===================================
// constructor
/*!

*/
AQLCalibrateVolatilityManager::AQLCalibrateVolatilityManager(void)
{
}

// destructor
/*!

*/
AQLCalibrateVolatilityManager::~AQLCalibrateVolatilityManager(void)
{ 
}

// 
/*!
    @brief get unique instance

	@return  AQLCalibrateVolatilityManager *
*/
AQLCalibrateVolatilityManager *
AQLCalibrateVolatilityManager::getInstance()
{
	if (!mpInstance)
	{
		mpInstance = new AQLCalibrateVolatilityManager();
	}
	return mpInstance;
}



// 
/*!
    @brief create Volatility creator

	@param[in] model
	@return  AQLCalibrateVolatility *
*/
AQLCalibrateVolatility *
AQLCalibrateVolatilityManager::createVolCreator(const AQLString &model) const
{
	AQLString tmpModel = model;
	tmpModel.toUpper();
	if (tmpModel == MODEL_LMM)
	{
		return new AQLCalibrateVolatilityLMM();
	}
	else if (tmpModel == MODEL_HW)
	{
		return new AQLCalibrateVolatilityHW();
	}
	else if (tmpModel == MODEL_PTBERG)
	{
		return new AQLCalibrateVolatilityPtberg();
	}
	else if (tmpModel == MODEL_SZ)
	{
		return new AQLCalibrateVolatilitySZ();
	}
	else if (tmpModel == MODEL_FXSTRGLSLV)
	{
		return new AQLCalibrateVolatilityFXStrangleSolver();
	}
	// vanna-volga
	else if (tmpModel == MODEL_FXVANNAVOLGA)
	{
		return new AQLCalibrateVolatilityFXVannaVolga();
	}
	else if (tmpModel == MODEL_IRSABR)
	{
		return new AQLCalibrateVolatilityIRSABR();
	}

	return 0;
}

