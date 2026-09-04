/*! @file
    @brief AQLRiskConfigurationManager factory class
*/
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif


#include "AQLRiskConfigurationManager.h"
#include "AQLDefinitions.h"
#include "AQLDefinitionsRisk.h"
#include "AQLRiskConfigurationYieldDelta.h"
#include "AQLRiskConfigurationYieldBasis.h"
#include "AQLRiskConfigurationVolCapVega.h"
#include "AQLRiskConfigurationVolSwaptionVega.h"
#include "AQLRiskConfigurationFXDelta.h"
#include "AQLRiskConfigurationVolFXATMParallelVega.h"
#include "AQLRiskConfigurationTheta.h"

#include "AQLRiskConfigurationYieldIRDelta.h"
#include "AQLRiskConfigurationYieldIRShiftDelta.h"
#include "AQLRiskConfigurationYieldBasisDelta.h"
#include "AQLRiskConfigurationVolIRVega.h"
#include "AQLRiskConfigurationFXDeltaEx1.h"
#include "AQLRiskConfigurationFXDeltaGamma.h"
#include "AQLRiskConfigurationVolFXVega.h"
#include "AQLRiskConfigurationFXShiftDelta.h"
#include "AQLRiskConfigurationFXShiftVolFXVega.h"
#include "AQLRiskConfigurationFXShiftYieldIRDelta.h"
#include "AQLRiskConfigurationFXShiftYieldBasisDelta.h"
#include "AQLRiskConfigurationVolFXShiftVolFXVega.h"
#include "AQLRiskConfigurationYieldIRShiftFXDelta.h"
#include "AQLRiskConfigurationYieldIRShiftVolFXVega.h"
#include "AQLRiskConfigurationCreditSpreadDelta.h"
#include "AQLRiskConfigurationFXDeltaOptionAnalytic.h"
#include "AQLRiskConfigurationVolFXVegaOptionAnalytic.h"
#include "AQLRiskConfigurationYieldIRDeltaOptionAnalytic.h"
#include "AQLRiskConfigurationYieldBasisDeltaOptionAnalytic.h"
#include "AQLRiskConfigurationFXShiftDeltaOptionAnalytic.h"
#include "AQLRiskConfigurationThetaOptionAnalytic.h"
#include "AQLRiskConfigurationYieldIRDeltaAnalytic.h"
#include "AQLRiskConfigurationYieldBasisDeltaAnalytic.h"
#include "AQLStaticData.h"
#include "AQLStaticDataManager.h"
#include "AQLRiskConfigurationYieldIRShiftVolIRVega.h"
#include "AQLRiskConfigurationYieldDeltaAnalytic.h"
#include "AQLRiskConfigurationYieldBasisAnalytic.h"
#include "AQLRiskConfigurationYieldIRDeltaSimple.h"
#include "AQLRiskConfigurationYieldIRDeltaGamma.h"
#include "AQLRiskConfigurationYieldIRDeltaMultiParallel.h"
#include "AQLRiskConfigurationCorDelta.h"

AQLRiskConfigurationManager *AQLRiskConfigurationManager::mpInstance = 0;

#define ANALYTIC "ANALYTIC"
#define SEMIANALYTIC "SEMIANALYTIC"

//================ AQLRiskConfigurationManager ===================================
// constructor
/*!

*/
AQLRiskConfigurationManager::AQLRiskConfigurationManager(void)
{
}

// destructor
/*!

*/
AQLRiskConfigurationManager::~AQLRiskConfigurationManager(void)
{ 
}

// 
/*!
    @brief get unique instance

	@return  AQLRiskConfigurationManager *
*/
AQLRiskConfigurationManager *
AQLRiskConfigurationManager::getInstance()
{
	if (!mpInstance)
	{
		mpInstance = new AQLRiskConfigurationManager();
	}
	return mpInstance;
}



// 
/*!
    @brief create risk setupper

	@param[in] risk
	@return AQLRiskConfiguration *
*/
AQLRiskConfiguration *
AQLRiskConfigurationManager::createRiskSetUpper(const AQLString &risk, bool isOfficial) const
{
	AQLString tmpRisk = risk;
	tmpRisk.toUpper();

	if (tmpRisk.findString(RISK_FRONT_OPTIONANALYTIC) >=0)
	{
		AQLStringVector tmpvec = tmpRisk.toToken('_');
		if (tmpvec.size() != 2)
			throw AQLCoreInvalidData("risk variable is wrong",__FILE__,__LINE__);

		tmpRisk = tmpvec[0];

		AQLString calc = AQLCoreDataService::getContext(ARG_KEY_CALC).toUpper();
		if (calc == "VANILLA")
		{
			if (tmpRisk == RISK_FX_DELTA)
			{
				return new AQLRiskConfigurationFXDeltaOptionAnalytic(AQLString("DELTA"));
			}
			else if (tmpRisk == RISK_FRONT_VOL_FXVEGA)
			{
				return new AQLRiskConfigurationVolFXVegaOptionAnalytic(AQLString("VEGA"));
			}
			else if (tmpRisk == RISK_FRONT_YIELD_IRDELTA)
			{
				return new AQLRiskConfigurationYieldIRDeltaOptionAnalytic(AQLString("RHO"));
			}
			else if (tmpRisk == RISK_FRONT_YIELD_BASISDELTA)
			{
				return new AQLRiskConfigurationYieldBasisDeltaOptionAnalytic(AQLString("RHO"));
			}
			else if (tmpRisk == RISK_FRONT_FX_SHIFTDELTA)
			{
				return new AQLRiskConfigurationFXShiftDeltaOptionAnalytic(AQLString("GAMMA"));
			}
			else if (tmpRisk == RISK_OFFICIAL_THETA)
			{
				return new AQLRiskConfigurationThetaOptionAnalytic(AQLString("THETA"));
			}
			else
				return 0;	
		}
	}

	
	AQLStaticData &rprop = AQLCoreDataService::getStaticDataManager().getRiskStaticData();
	AQLString analyticstr = rprop.getStaticData(RISK_KEY_ANALYTIC_CALCTYPE).toUpper();
	bool isanalytic = false;
	if (ANALYTIC == analyticstr)
	{
		isanalytic = true;
		throw AQLCoreInvalidData("Analytic risk is not supported now!",__FILE__,__LINE__);
	}
	
	if (isOfficial)
	{
		if (tmpRisk == RISK_OFFICIAL_YIELD_DELTA)
		{
			if (!isanalytic)
				return new AQLRiskConfigurationYieldDelta();
			else
				return new AQLRiskConfigurationYieldDeltaAnalytic();
		}
		else if (tmpRisk == RISK_OFFICIAL_YIELD_BASIS)
		{
			if (!isanalytic)
				return new AQLRiskConfigurationYieldBasis();
			else
				return new AQLRiskConfigurationYieldBasisAnalytic();
		}
		else if (tmpRisk == RISK_OFFICIAL_VOL_CAPVEGA)
		{
			return new AQLRiskConfigurationVolCapVega();
		}
		else if (tmpRisk == RISK_OFFICIAL_VOL_SWAPTIONVEGA)
		{
			return new AQLRiskConfigurationVolSwaptionVega();
		}
		else if (tmpRisk == RISK_FX_DELTA)
		{
			return new AQLRiskConfigurationFXDelta();
		}
		else if (tmpRisk == RISK_OFFICIAL_VOL_FXATMPARALLELVEGA)
		{
			return new AQLRiskConfigurationVolFXATMParallelVega();
		}
		else if (tmpRisk == RISK_OFFICIAL_YIELD_PARADELTA)
		{
			return new AQLRiskConfigurationYieldIRDeltaMultiParallel();
		}
		else if (tmpRisk == RISK_OFFICIAL_YIELD_CREDITSPREADDELTA)
		{
			return new AQLRiskConfigurationCreditSpreadDelta();
		}
		else if (tmpRisk == RISK_OFFICIAL_THETA)
		{
			return new AQLRiskConfigurationTheta();
		}
		else if (tmpRisk == RISK_OFFICIAL_YIELD_IRDELTA)
		{
			return new AQLRiskConfigurationYieldIRDeltaGamma();
		}
	}
	else
	{
		if (tmpRisk == RISK_FRONT_YIELD_IRDELTA)
		{
			if (AQLCoreDataService::getStaticDataManager().getRiskStaticData().getStaticData(RISK_FRONT_YIELD_IRDELTA_ISSIMPLECALC).toLower() == "true")
                return new AQLRiskConfigurationYieldIRDeltaSimple();
			else if ((!isanalytic)&&(AQLCoreDataService::getStaticDataManager().getRiskStaticData().getStaticData(RISK_FRONT_YIELD_IRDELTA_ISCALCGAMMA).toLower() == "true"))
				return new AQLRiskConfigurationYieldIRDeltaGamma();
			else if (!isanalytic)
				return new AQLRiskConfigurationYieldIRDelta();
			else
				return new AQLRiskConfigurationYieldIRDeltaAnalytic();
		}
		else if (tmpRisk == RISK_FRONT_YIELD_IRSHIFTDELTA)
		{
			return new AQLRiskConfigurationYieldIRShiftDelta();
		}
		else if (tmpRisk == RISK_FRONT_YIELD_BASISDELTA)
		{
			if (!isanalytic)
				return new AQLRiskConfigurationYieldBasisDelta();
			else
				return new AQLRiskConfigurationYieldBasisDeltaAnalytic();
		}
		else if (tmpRisk == RISK_FRONT_VOL_IRVEGA)
		{
			return new AQLRiskConfigurationVolIRVega();
		}
		else if (tmpRisk == RISK_FX_DELTA)
		{
			if (AQLCoreDataService::getStaticDataManager().getRiskStaticData().getStaticData(RISK_FRONT_FX_DELTA_ISCALCGAMMA).toLower() == "true")
				return new AQLRiskConfigurationFXDeltaGamma();
			else
				return new AQLRiskConfigurationFXDeltaEx1();
		}
		else if (tmpRisk == RISK_FRONT_VOL_FXVEGA)
		{
			return new AQLRiskConfigurationVolFXVega();
		}
		else if (tmpRisk == RISK_FRONT_FX_SHIFTDELTA)
		{
			return new AQLRiskConfigurationFXShiftDelta();
		}
		else if (tmpRisk == RISK_FRONT_FXSHIFT_VOL_FXVEGA)
		{
			return new AQLRiskConfigurationFXShiftVolFXVega();
		}
		else if (tmpRisk == RISK_FRONT_FXSHIFT_YIELD_IRDELTA)
		{
			return new AQLRiskConfigurationFXShiftYieldIRDelta();
		}
		else if (tmpRisk == RISK_FRONT_FXSHIFT_YIELD_BASISDELTA)
		{
			return new AQLRiskConfigurationFXShiftYieldBasisDelta();
		}
		else if (tmpRisk == RISK_FRONT_FXVOLSHIFT_VOL_FXVEGA)
		{
			return new AQLRiskConfigurationVolFXShiftVolFXVega();
		}
		else if (tmpRisk == RISK_FRONT_IRSHIFT_FX_DELTA)
		{
			return new AQLRiskConfigurationYieldIRShiftFXDelta();
		}
		else if (tmpRisk == RISK_FRONT_IRSHIFT_VOL_FXVEGA)
		{
			return new AQLRiskConfigurationYieldIRShiftVolFXVega();
		}
		else if (tmpRisk == RISK_OFFICIAL_YIELD_CREDITSPREADDELTA)
		{
			return new AQLRiskConfigurationCreditSpreadDelta();
		}
		else if (tmpRisk == RISK_OFFICIAL_THETA)
		{
			return new AQLRiskConfigurationTheta();
		}
		else if (tmpRisk == RISK_FRONT_VOL_IRSHIFTIRVEGA)
		{
			return new AQLRiskConfigurationYieldIRShiftVolIRVega();
		}
		else if (tmpRisk == RISK_FRONT_COR_CORDELTA)
		{
			return new AQLRiskConfigurationCorDelta();
		}


	}

	return 0;
}


