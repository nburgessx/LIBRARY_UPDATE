/*! @file
    @brief LAModelConfiguration manager class
*/
//  2007, AlgoQuantHub.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LAModelConfiguration.cpp
//
//  DESCRIPTION :       LAModelConfiguration 
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


#include "LAModelConfiguration.h"
#include "AQLDataBasics.h"
#include "LACoreDataService.h"
#include "LADefinitions.h"
#include "LADefinitionsRisk.h"
#include "LADefinitionsLMM.h"
#include "LADefinitionsHW.h"
#include "LADefinitionsPtberg.h"
#include "LACalibrateModel.h"
#include "LACalibrateModelIR.h"
#include "LACalibrateModelLMM.h"
#include "LACalibrateModelLMMDmy.h"
#include "LACalibrateModelHW.h"
#include "LACalibrateModelHW3F.h"
#include "LACalibrateModelFX.h"
#include "LACalibrateModelPtberg.h"
#include "LACalibrateModelPtbergSkew.h"
#include "LACalibrateModelPtberg3F.h"
#include "LADealUtils.h"
#include "LAStaticDataManager.h"
#include "LAStaticData.h"
#include "LACalibrateModelIRVanilla.h"
#include "LACalibrateModelFXVanilla.h"
#include "LACalibrateModelFXStrangleSolver.h"
#include "LACalibrateModelIRSABR.h"
//hishida vannavolga
#include "LACalibrateModelFXVannaVolga.h"
#include "LACalibrateModelSZ.h"

LAModelConfiguration *LAModelConfiguration::mpInstance = 0;

//================ LAModelConfiguration ===================================
// constructor
/*!

*/
LAModelConfiguration::LAModelConfiguration(void)
{
}

// destructor
/*!

*/
LAModelConfiguration::~LAModelConfiguration(void)
{ 
}

// 
/*!
    @brief get unique instance

	@return  LAModelConfiguration *
*/
LAModelConfiguration *
LAModelConfiguration::getInstance()
{
	if (!mpInstance)
	{
		mpInstance = new LAModelConfiguration();
	}
	return mpInstance;
}



// 
/*!
    @brief create SDE generator

	@param[in] model
	@return  LACalibrateModel *
*/
LACalibrateModel *
LAModelConfiguration::createSDEGenerator(const AQLString &model) const
{
	AQLString baseCurrency = MADealUtils::getSDECurrencys()[0];
	AQLStringVector simCurs = MADealUtils::getSimulationSDECurrencys();
	if (simCurs.size() != 0)
	{
		baseCurrency = simCurs[0];
	}

	LAStaticData &staticData = LACoreDataService::getStaticDataManager().getStaticData();
	AQLString tmpModel = model;
	tmpModel.toUpper();
	if (tmpModel == MODEL_IRVANILLA)
	{
		return new LACalibrateModelIRVanilla(baseCurrency);
	}
	if (tmpModel == MODEL_IRSABR)
	{
		return new LACalibrateModelIRSABR(baseCurrency);
	}
	else if (tmpModel == MODEL_FXVANILLA)
	{
		return new LACalibrateModelFXVanilla();
	}
	else if (tmpModel == MODEL_FXSTRGLSLV)
	{
		return new LACalibrateModelFXStrangleSolver();
	}
	//hishida vannavolga
	else if (tmpModel == MODEL_FXVANNAVOLGA)
	{
		return new LACalibrateModelFXVannaVolga();
	}
	else if (tmpModel == MODEL_LMM)
	{
		// check dmy sde use
		if (MADealUtils::getSDECurrencys().size() > 1)
		{
			AQLDataBool isDmyUse;
			isDmyUse.convertFromString(staticData.getStaticData(KEY_LMM_CROSS_ISDMY));
			if (isDmyUse.get())
			{
				return new LACalibrateModelLMMDmy(baseCurrency);
			}

		}
		return new LACalibrateModelLMM(baseCurrency);
	}
	else if (tmpModel == MODEL_HW)
	{
		AQLString is3FStr = staticData.getStaticData(KEY_HW_IS3F);
		AQLDataBool is3F;
		if (is3FStr == AQ_NO_DATA)
		{
			is3F.set(false);
		}
		else
		{
			is3F.convertFromString(is3FStr);
		}
		if (is3F.get())
		{
			return new LACalibrateModelHW3F(baseCurrency);
		}
		else
		{
			return new LACalibrateModelHW(baseCurrency);
		}
	}
	else if  (tmpModel == MODEL_PTBERG)
	{
		// check 3F
		AQLString is3FStr = staticData.getStaticData(KEY_PTBERG_IS3F);
		AQLDataBool is3F;
		if (is3FStr == AQ_NO_DATA)
		{
			is3F.set(false);
		}
		else
		{
			is3F.convertFromString(is3FStr);
		}

		if (is3F.get())
		{
			return new LACalibrateModelPtberg3F();
		}
		else
		{
			// check skew
			AQLDataBool isSkewInclude;
			isSkewInclude.convertFromString(staticData.getStaticData(KEY_PTBERG_SKEW_ISINCLUDE));
			if (isSkewInclude.get())
			{
				return new LACalibrateModelPtbergSkew();
			}
			else
			{
				return new LACalibrateModelPtberg();
			}
		}
	}
	else if (tmpModel == MODEL_SZ)
	{
		return new LACalibrateModelSZ();
	}
	else if (tmpModel == AQLString(MODEL_SZ) + AQLString(POSTFIX_VOL))
	{
		return new LACalibrateModelSZ::VF();
	}

	return 0;
}

