/*! @file
    @brief AQLModelConfiguration manager class
*/
#if defined(__GNUG__) && !defined(__INTEL_COMPILER)
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLModelConfiguration.h"
#include "AQLDataBasics.h"
#include "AQLCoreDataService.h"
#include "AQLDefinitions.h"
#include "AQLDefinitionsRisk.h"
#include "AQLDefinitionsLMM.h"
#include "AQLDefinitionsHW.h"
#include "AQLDefinitionsPtberg.h"
#include "AQLCalibrateModel.h"
#include "AQLCalibrateModelIR.h"
#include "AQLCalibrateModelLMM.h"
#include "AQLCalibrateModelLMMDmy.h"
#include "AQLCalibrateModelHW.h"
#include "AQLCalibrateModelHW3F.h"
#include "AQLCalibrateModelFX.h"
#include "AQLCalibrateModelPtberg.h"
#include "AQLCalibrateModelPtbergSkew.h"
#include "AQLCalibrateModelPtberg3F.h"
#include "AQLDealUtils.h"
#include "AQLStaticDataManager.h"
#include "AQLStaticData.h"
#include "AQLCalibrateModelIRVanilla.h"
#include "AQLCalibrateModelFXVanilla.h"
#include "AQLCalibrateModelFXStrangleSolver.h"
#include "AQLCalibrateModelIRSABR.h"
//hishida vannavolga
#include "AQLCalibrateModelFXVannaVolga.h"
#include "AQLCalibrateModelSZ.h"

AQLModelConfiguration *AQLModelConfiguration::mpInstance = 0;

//================ AQLModelConfiguration ===================================
// constructor
/*!

*/
AQLModelConfiguration::AQLModelConfiguration(void)
{
}

// destructor
/*!

*/
AQLModelConfiguration::~AQLModelConfiguration(void)
{ 
}

// 
/*!
    @brief get unique instance

	@return  AQLModelConfiguration *
*/
AQLModelConfiguration *
AQLModelConfiguration::getInstance()
{
	if (!mpInstance)
	{
		mpInstance = new AQLModelConfiguration();
	}
	return mpInstance;
}



// 
/*!
    @brief create SDE generator

	@param[in] model
	@return  AQLCalibrateModel *
*/
AQLCalibrateModel *
AQLModelConfiguration::createSDEGenerator(const AQLString &model) const
{
	AQLString baseCurrency = AQLDealUtils::getSDECurrencys()[0];
	AQLStringVector simCurs = AQLDealUtils::getSimulationSDECurrencys();
	if (simCurs.size() != 0)
	{
		baseCurrency = simCurs[0];
	}

	AQLStaticData &staticData = AQLCoreDataService::getStaticDataManager().getStaticData();
	AQLString tmpModel = model;
	tmpModel.toUpper();
	if (tmpModel == MODEL_IRVANILLA)
	{
		return new AQLCalibrateModelIRVanilla(baseCurrency);
	}
	if (tmpModel == MODEL_IRSABR)
	{
		return new AQLCalibrateModelIRSABR(baseCurrency);
	}
	else if (tmpModel == MODEL_FXVANILLA)
	{
		return new AQLCalibrateModelFXVanilla();
	}
	else if (tmpModel == MODEL_FXSTRGLSLV)
	{
		return new AQLCalibrateModelFXStrangleSolver();
	}
	//hishida vannavolga
	else if (tmpModel == MODEL_FXVANNAVOLGA)
	{
		return new AQLCalibrateModelFXVannaVolga();
	}
	else if (tmpModel == MODEL_LMM)
	{
		// check dmy sde use
		if (AQLDealUtils::getSDECurrencys().size() > 1)
		{
			AQLDataBool isDmyUse;
			isDmyUse.convertFromString(staticData.getStaticData(KEY_LMM_CROSS_ISDMY));
			if (isDmyUse.get())
			{
				return new AQLCalibrateModelLMMDmy(baseCurrency);
			}

		}
		return new AQLCalibrateModelLMM(baseCurrency);
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
			return new AQLCalibrateModelHW3F(baseCurrency);
		}
		else
		{
			return new AQLCalibrateModelHW(baseCurrency);
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
			return new AQLCalibrateModelPtberg3F();
		}
		else
		{
			// check skew
			AQLDataBool isSkewInclude;
			isSkewInclude.convertFromString(staticData.getStaticData(KEY_PTBERG_SKEW_ISINCLUDE));
			if (isSkewInclude.get())
			{
				return new AQLCalibrateModelPtbergSkew();
			}
			else
			{
				return new AQLCalibrateModelPtberg();
			}
		}
	}
	else if (tmpModel == MODEL_SZ)
	{
		return new AQLCalibrateModelSZ();
	}
	else if (tmpModel == AQLString(MODEL_SZ) + AQLString(POSTFIX_VOL))
	{
		return new AQLCalibrateModelSZ::VF();
	}

	return 0;
}

