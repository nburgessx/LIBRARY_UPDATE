/*! @file
    @brief LARiskConfigurationManager factory class
*/
//  2007, Mizuho International London.
////X///////////////////X///////////////////////////////X///////////////////
//  NAME        :       LARiskConfigurationManager.cpp
//
//  DESCRIPTION :       LARiskConfigurationManager 
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


#include "LARiskConfigurationManager.h"
#include "LADefinitions.h"
#include "LADefinitionsRisk.h"
#include "LARiskConfigurationYieldDelta.h"
#include "LARiskConfigurationYieldBasis.h"
#include "LARiskConfigurationVolCapVega.h"
#include "LARiskConfigurationVolSwaptionVega.h"
#include "LARiskConfigurationFXDelta.h"
#include "LARiskConfigurationVolFXATMParallelVega.h"
#include "LARiskConfigurationTheta.h"

#include "LARiskConfigurationYieldIRDelta.h"
#include "LARiskConfigurationYieldIRShiftDelta.h"
#include "LARiskConfigurationYieldBasisDelta.h"
#include "LARiskConfigurationVolIRVega.h"
#include "LARiskConfigurationFXDeltaEx1.h"
#include "LARiskConfigurationFXDeltaGamma.h"
#include "LARiskConfigurationVolFXVega.h"
#include "LARiskConfigurationFXShiftDelta.h"
#include "LARiskConfigurationFXShiftVolFXVega.h"
#include "LARiskConfigurationFXShiftYieldIRDelta.h"
#include "LARiskConfigurationFXShiftYieldBasisDelta.h"
#include "LARiskConfigurationVolFXShiftVolFXVega.h"
#include "LARiskConfigurationYieldIRShiftFXDelta.h"
#include "LARiskConfigurationYieldIRShiftVolFXVega.h"
#include "LARiskConfigurationCreditSpreadDelta.h"
#include "LARiskConfigurationFXDeltaOptionAnalytic.h"
#include "LARiskConfigurationVolFXVegaOptionAnalytic.h"
#include "LARiskConfigurationYieldIRDeltaOptionAnalytic.h"
#include "LARiskConfigurationYieldBasisDeltaOptionAnalytic.h"
#include "LARiskConfigurationFXShiftDeltaOptionAnalytic.h"
#include "LARiskConfigurationThetaOptionAnalytic.h"
#include "LARiskConfigurationYieldIRDeltaAnalytic.h"
#include "LARiskConfigurationYieldBasisDeltaAnalytic.h"
#include "LAStaticData.h"
#include "LAStaticDataManager.h"
#include "LARiskConfigurationYieldIRShiftVolIRVega.h"
#include "LARiskConfigurationYieldDeltaAnalytic.h"
#include "LARiskConfigurationYieldBasisAnalytic.h"
#include "LARiskConfigurationYieldIRDeltaSimple.h"
#include "LARiskConfigurationYieldIRDeltaGamma.h"
#include "LARiskConfigurationYieldIRDeltaMultiParallel.h"
#include "LARiskConfigurationCorDelta.h"

LARiskConfigurationManager *LARiskConfigurationManager::mpInstance = 0;

#define ANALYTIC "ANALYTIC"
#define SEMIANALYTIC "SEMIANALYTIC"

//================ LARiskConfigurationManager ===================================
// constructor
/*!

*/
LARiskConfigurationManager::LARiskConfigurationManager(void)
{
}

// destructor
/*!

*/
LARiskConfigurationManager::~LARiskConfigurationManager(void)
{ 
}

// 
/*!
    @brief get unique instance

	@return  LARiskConfigurationManager *
*/
LARiskConfigurationManager *
LARiskConfigurationManager::getInstance()
{
	if (!mpInstance)
	{
		mpInstance = new LARiskConfigurationManager();
	}
	return mpInstance;
}



// 
/*!
    @brief create risk setupper

	@param[in] risk
	@return LARiskConfiguration *
*/
LARiskConfiguration *
LARiskConfigurationManager::createRiskSetUpper(const LAString &risk, bool isOfficial) const
{
	LAString tmpRisk = risk;
	tmpRisk.toUpper();

	if (tmpRisk.findString(RISK_FRONT_OPTIONANALYTIC) >=0)
	{
		LAStringVector tmpvec = tmpRisk.toToken('_');
		if (tmpvec.size() != 2)
			throw LACoreInvalidData("risk variable is wrong",__FILE__,__LINE__);

		tmpRisk = tmpvec[0];

		LAString calc = LACoreDataService::getContext(ARG_KEY_CALC).toUpper();
		if (calc == "VANILLA")
		{
			if (tmpRisk == RISK_FX_DELTA)
			{
				return new LARiskConfigurationFXDeltaOptionAnalytic(LAString("DELTA"));
			}
			else if (tmpRisk == RISK_FRONT_VOL_FXVEGA)
			{
				return new LARiskConfigurationVolFXVegaOptionAnalytic(LAString("VEGA"));
			}
			else if (tmpRisk == RISK_FRONT_YIELD_IRDELTA)
			{
				return new LARiskConfigurationYieldIRDeltaOptionAnalytic(LAString("RHO"));
			}
			else if (tmpRisk == RISK_FRONT_YIELD_BASISDELTA)
			{
				return new LARiskConfigurationYieldBasisDeltaOptionAnalytic(LAString("RHO"));
			}
			else if (tmpRisk == RISK_FRONT_FX_SHIFTDELTA)
			{
				return new LARiskConfigurationFXShiftDeltaOptionAnalytic(LAString("GAMMA"));
			}
			else if (tmpRisk == RISK_OFFICIAL_THETA)
			{
				return new LARiskConfigurationThetaOptionAnalytic(LAString("THETA"));
			}
			else
				return 0;	
		}
	}

	
	LAStaticData &rprop = LACoreDataService::getStaticDataManager().getRiskStaticData();
	LAString analyticstr = rprop.getStaticData(RISK_KEY_ANALYTIC_CALCTYPE).toUpper();
	bool isanalytic = false;
	if (ANALYTIC == analyticstr)
	{
		isanalytic = true;
		throw LACoreInvalidData("Analytic risk is not supported now!",__FILE__,__LINE__);
	}
	
	if (isOfficial)
	{
		if (tmpRisk == RISK_OFFICIAL_YIELD_DELTA)
		{
			if (!isanalytic)
				return new LARiskConfigurationYieldDelta();
			else
				return new LARiskConfigurationYieldDeltaAnalytic();
		}
		else if (tmpRisk == RISK_OFFICIAL_YIELD_BASIS)
		{
			if (!isanalytic)
				return new LARiskConfigurationYieldBasis();
			else
				return new LARiskConfigurationYieldBasisAnalytic();
		}
		else if (tmpRisk == RISK_OFFICIAL_VOL_CAPVEGA)
		{
			return new LARiskConfigurationVolCapVega();
		}
		else if (tmpRisk == RISK_OFFICIAL_VOL_SWAPTIONVEGA)
		{
			return new LARiskConfigurationVolSwaptionVega();
		}
		else if (tmpRisk == RISK_FX_DELTA)
		{
			return new LARiskConfigurationFXDelta();
		}
		else if (tmpRisk == RISK_OFFICIAL_VOL_FXATMPARALLELVEGA)
		{
			return new LARiskConfigurationVolFXATMParallelVega();
		}
		else if (tmpRisk == RISK_OFFICIAL_YIELD_PARADELTA)
		{
			return new LARiskConfigurationYieldIRDeltaMultiParallel();
		}
		else if (tmpRisk == RISK_OFFICIAL_YIELD_CREDITSPREADDELTA)
		{
			return new LARiskConfigurationCreditSpreadDelta();
		}
		else if (tmpRisk == RISK_OFFICIAL_THETA)
		{
			return new LARiskConfigurationTheta();
		}
		else if (tmpRisk == RISK_OFFICIAL_YIELD_IRDELTA)
		{
			return new LARiskConfigurationYieldIRDeltaGamma();
		}
	}
	else
	{
		if (tmpRisk == RISK_FRONT_YIELD_IRDELTA)
		{
			if (LACoreDataService::getStaticDataManager().getRiskStaticData().getStaticData(RISK_FRONT_YIELD_IRDELTA_ISSIMPLECALC).toLower() == "true")
                return new LARiskConfigurationYieldIRDeltaSimple();
			else if ((!isanalytic)&&(LACoreDataService::getStaticDataManager().getRiskStaticData().getStaticData(RISK_FRONT_YIELD_IRDELTA_ISCALCGAMMA).toLower() == "true"))
				return new LARiskConfigurationYieldIRDeltaGamma();
			else if (!isanalytic)
				return new LARiskConfigurationYieldIRDelta();
			else
				return new LARiskConfigurationYieldIRDeltaAnalytic();
		}
		else if (tmpRisk == RISK_FRONT_YIELD_IRSHIFTDELTA)
		{
			return new LARiskConfigurationYieldIRShiftDelta();
		}
		else if (tmpRisk == RISK_FRONT_YIELD_BASISDELTA)
		{
			if (!isanalytic)
				return new LARiskConfigurationYieldBasisDelta();
			else
				return new LARiskConfigurationYieldBasisDeltaAnalytic();
		}
		else if (tmpRisk == RISK_FRONT_VOL_IRVEGA)
		{
			return new LARiskConfigurationVolIRVega();
		}
		else if (tmpRisk == RISK_FX_DELTA)
		{
			if (LACoreDataService::getStaticDataManager().getRiskStaticData().getStaticData(RISK_FRONT_FX_DELTA_ISCALCGAMMA).toLower() == "true")
				return new LARiskConfigurationFXDeltaGamma();
			else
				return new LARiskConfigurationFXDeltaEx1();
		}
		else if (tmpRisk == RISK_FRONT_VOL_FXVEGA)
		{
			return new LARiskConfigurationVolFXVega();
		}
		else if (tmpRisk == RISK_FRONT_FX_SHIFTDELTA)
		{
			return new LARiskConfigurationFXShiftDelta();
		}
		else if (tmpRisk == RISK_FRONT_FXSHIFT_VOL_FXVEGA)
		{
			return new LARiskConfigurationFXShiftVolFXVega();
		}
		else if (tmpRisk == RISK_FRONT_FXSHIFT_YIELD_IRDELTA)
		{
			return new LARiskConfigurationFXShiftYieldIRDelta();
		}
		else if (tmpRisk == RISK_FRONT_FXSHIFT_YIELD_BASISDELTA)
		{
			return new LARiskConfigurationFXShiftYieldBasisDelta();
		}
		else if (tmpRisk == RISK_FRONT_FXVOLSHIFT_VOL_FXVEGA)
		{
			return new LARiskConfigurationVolFXShiftVolFXVega();
		}
		else if (tmpRisk == RISK_FRONT_IRSHIFT_FX_DELTA)
		{
			return new LARiskConfigurationYieldIRShiftFXDelta();
		}
		else if (tmpRisk == RISK_FRONT_IRSHIFT_VOL_FXVEGA)
		{
			return new LARiskConfigurationYieldIRShiftVolFXVega();
		}
		else if (tmpRisk == RISK_OFFICIAL_YIELD_CREDITSPREADDELTA)
		{
			return new LARiskConfigurationCreditSpreadDelta();
		}
		else if (tmpRisk == RISK_OFFICIAL_THETA)
		{
			return new LARiskConfigurationTheta();
		}
		else if (tmpRisk == RISK_FRONT_VOL_IRSHIFTIRVEGA)
		{
			return new LARiskConfigurationYieldIRShiftVolIRVega();
		}
		else if (tmpRisk == RISK_FRONT_COR_CORDELTA)
		{
			return new LARiskConfigurationCorDelta();
		}


	}

	return 0;
}


