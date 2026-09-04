
#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif
#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <stdio.h>

#include "AQLMathBaseFuncUtility.h"
#include "AQLObject.h"
#include "AQLDataBasics.h"
#include "AQLDataVector.h"
#include "AQLCoreTemplateType.h"
#include "AQLMathDefine.h"
#include "AQLPriceDataCalendar.h"
#include "AQLPriceDataSlidingRule.h"
#include "AQLPriceDataInterpolation.h"
#include "AQLBasic.h"
#include "AQLAlgorithm.h"
#include "AQLObjectHolder.h"
#include "AQLDataInstance.h"
#include "AQLObjectMaster.h"
#include "AQLPriceDataManager.h"
#include "AQLLinearInterpolation.h"
#include "AQLSplineInterpolation.h"
#include "AQLStepInterpolation.h"
#include "AQLConstrainedSplineInterpolation.h"
#include "AQLMonotoneConvexInterpolation.h"
#include "AQLParabolicInterpolation.h"
#include "AQLLinearSplineInterpolation.h"
#include "AQLLinearMonotoneSplineInterpolation.h"
#include "AQLMonotoneSplineInterpolation.h"
#include "AQLMathYieldCurvePro.h"

#include "AQLCoreComponentManager.h"
#include <cmath>
#include <float.h>

using namespace std;
//static-mem
std::map<AQLString, AQLString> AQLCoreComponentManager::interpolationMap;
std::map<AQLString, AQLString> AQLCoreComponentManager::dayCountMap;
std::map<AQLString, AQLString> AQLCoreComponentManager::basisTypeMap;
std::map<AQLString, AQLString> AQLCoreComponentManager::basisIndexMap;
std::map<AQLString, AQLBlackScholesBase* > AQLCoreComponentManager::blackComponentMap ;
std::map<AQLString, AnalyticParam* > AQLCoreComponentManager::blackParamComponentMap;

//static-method////////

std::map<AQLString, AQLString>& 
AQLCoreComponentManager::getInterpolationMap()
{
	if(interpolationMap.empty())
	{
		// Interpolation Names
		interpolationMap.insert(std::make_pair(LINEAR,					"fn_linearinterpolation"));
		interpolationMap.insert(std::make_pair(SPLINE,					"fn_splineinterpolation"));
		interpolationMap.insert(std::make_pair(MONOTONESPLINE,			"fn_monotonesplineinterpolation"));
        interpolationMap.insert(std::make_pair(NATURALSPLINE,			"fn_naturalsplineinterpolation"));
        interpolationMap.insert(std::make_pair(CLAMPEDSPLINE,			"fn_clampedsplineinterpolation"));
		interpolationMap.insert(std::make_pair(MONOTONEPARABOLIC,		"fn_monotoneparabolicinterpolation"));
        interpolationMap.insert(std::make_pair(PARABOLIC,				"fn_parabolicinterpolation"));
        interpolationMap.insert(std::make_pair(LINEARSPLINE,			"fn_linearsplineinterpolation"));
		interpolationMap.insert(std::make_pair(LINEARMONOTONESPLINE,	"fn_linearmonotonesplineinterpolation"));
		interpolationMap.insert(std::make_pair(LINEARMONOTONEPARABOLIC,	"fn_linearmonotoneparabolicinterpolation"));
		interpolationMap.insert(std::make_pair(STEP,					"fn_stepinterpolation"));
		interpolationMap.insert(std::make_pair(RIGHTCONTINUOUS,			"fn_rightcontinuousinterpolation"));
		interpolationMap.insert(std::make_pair(LEFTCONTINUOUS,			"fn_leftcontinuousinterpolation"));
		interpolationMap.insert(std::make_pair(CONSTRAINEDSPLINE,		"fn_constrainedsplineinterpolation"));
        interpolationMap.insert(std::make_pair(MONOTONECONVEX,			"fn_monotoneconvexinterpolation"));
		
		// basis Set
		interpolationMap.insert(std::make_pair(BASISSTEP,				"fn_basisfunc1"  ));
		interpolationMap.insert(std::make_pair(BASISLINEAR,				"fn_basisfunc2"  ));

	}
	return interpolationMap;
}

std::map<AQLString, AQLString>&
AQLCoreComponentManager::getDayCountMap()
{
	if(dayCountMap.empty())
	{
		/*AC_365I is ACT/365ISDA but, ACT/365ISDA means usual ACTACT daycount,
		on the other hand, ACT/ACT means the irregular ACTACT daycount which is developed by CREDIT team.
		So, we will use excel screen AC_AC = "ACT/ACT" regarding as  ACT/365_ISDA*/
		dayCountMap.insert(std::make_pair(AC_AC,		AC_365I));
		dayCountMap.insert(std::make_pair(AC_365,		AC_365));
		dayCountMap.insert(std::make_pair(AC_360,		AC_360));
		dayCountMap.insert(std::make_pair(N3_360,		N3_360));
		dayCountMap.insert(std::make_pair(E3_360,		E3_360));
		dayCountMap.insert(std::make_pair(AC_365I,		AC_365I));
		dayCountMap.insert(std::make_pair(AC_365F,		AC_365F));
		dayCountMap.insert(std::make_pair(AC_AC_ICMA,		AC_AC_ICMA));
	}
	return dayCountMap;
}

std::map<AQLString, AQLString>&
AQLCoreComponentManager::getBasisTypeMap()
{
	if(basisTypeMap.empty())
	{
		basisTypeMap.insert(std::make_pair(XCCYBASIS,	XCCYBASIS));
	}
	return basisTypeMap;
}

std::map<AQLString, AQLString>&
AQLCoreComponentManager::getBasisIndexMap()
{
	if(basisIndexMap.empty())
	{
		basisIndexMap.insert(std::make_pair(LIBOR,	LIBOR));
	}
	return basisIndexMap;
}

std::map<AQLString, AQLBlackScholesBase*>&
AQLCoreComponentManager::getBlackComponentMap()
{
	if(blackComponentMap.empty())
	{	
		//AQLZeroForError
		blackComponentMap.insert(std::make_pair(AQLString("ERROR"),	new AQLZeroForError()));
		//AQLBlack
		blackComponentMap.insert(std::make_pair(AQLString(BK)	+ AQLString(PREM) + AQLString(CALL),	new AQLBlackpremCall()	)	);
		blackComponentMap.insert(std::make_pair(AQLString(BK)	+ AQLString(PREM) + AQLString(PUT),	new AQLBlackpremPut()	)	);
		blackComponentMap.insert(std::make_pair(AQLString(BK)	+ AQLString(DELTA) + AQLString(CALL), new AQLBlackdeltaCall()	)	);
		blackComponentMap.insert(std::make_pair(AQLString(BK)	+ AQLString(GAMMA) + AQLString(CALL), new AQLBlackgammaCall()	)	);
		blackComponentMap.insert(std::make_pair(AQLString(BK)	+ AQLString(VEGA)  + AQLString(CALL), new AQLBlackvegaCall()	)	);
		blackComponentMap.insert(std::make_pair(AQLString(BK)	+ AQLString(THETA) + AQLString(CALL), new AQLBlackthetaCall()	)	);
		blackComponentMap.insert(std::make_pair(AQLString(BK)	+ AQLString(DELTA) + AQLString(PUT),  new AQLBlackdeltaPut()	)	);
		blackComponentMap.insert(std::make_pair(AQLString(BK)	+ AQLString(GAMMA) + AQLString(PUT),  new AQLBlackgammaPut()	)	);
		blackComponentMap.insert(std::make_pair(AQLString(BK)	+ AQLString(VEGA)  + AQLString(PUT),  new AQLBlackvegaPut()	)	);
		blackComponentMap.insert(std::make_pair(AQLString(BK)	+ AQLString(THETA) + AQLString(PUT),  new AQLBlackthetaPut()	)	);
		//AQLBlackPayOff
		blackComponentMap.insert(std::make_pair(AQLString(BKPAYOFF) + AQLString(PREM) + AQLString(CALL),	new AQLBlackPayOffpremCall()	)	);
		blackComponentMap.insert(std::make_pair(AQLString(BKPAYOFF) + AQLString(PREM) + AQLString(PUT),	new AQLBlackPayOffpremPut()		)	);
		//AQLGreek
		blackComponentMap.insert(std::make_pair(AQLString(GK) + AQLString(PREM)	+ AQLString(CALL), new AQLGreekpremCall()	)	);
		blackComponentMap.insert(std::make_pair(AQLString(GK) + AQLString(DELTA)	+ AQLString(CALL), new AQLGreekdeltaCall() )	);
		blackComponentMap.insert(std::make_pair(AQLString(GK) + AQLString(GAMMA)	+ AQLString(CALL), new AQLGreekgammaCall() )	);
		blackComponentMap.insert(std::make_pair(AQLString(GK) + AQLString(VEGA)	+ AQLString(CALL), new AQLGreekvegaCall()	)	);
		blackComponentMap.insert(std::make_pair(AQLString(GK) + AQLString(THETA)	+ AQLString(CALL), new AQLGreekthetaCall() )	);
		blackComponentMap.insert(std::make_pair(AQLString(GK) + AQLString(RHO)	+ AQLString(CALL), new AQLGreekrhoCall()	)	);
		blackComponentMap.insert(std::make_pair(AQLString(GK) + AQLString(PHI)	+ AQLString(CALL), new AQLGreekphiCall()	)	);
		blackComponentMap.insert(std::make_pair(AQLString(GK) + AQLString(VANNA)	+ AQLString(CALL), new AQLGreekvannaCall()	)	);
		blackComponentMap.insert(std::make_pair(AQLString(GK) + AQLString(VOLGA)	+ AQLString(CALL), new AQLGreekvolgaCall()	)	);
		blackComponentMap.insert(std::make_pair(AQLString(GK) + AQLString(PREM)	+ AQLString(PUT),	new AQLGreekpremPut()	)	);
		blackComponentMap.insert(std::make_pair(AQLString(GK) + AQLString(DELTA)	+ AQLString(PUT),	new AQLGreekdeltaPut()	)	);
		blackComponentMap.insert(std::make_pair(AQLString(GK) + AQLString(GAMMA)	+ AQLString(PUT),	new AQLGreekgammaPut()	)	);
		blackComponentMap.insert(std::make_pair(AQLString(GK) + AQLString(VEGA)	+ AQLString(PUT),	new AQLGreekvegaPut()	)	);
		blackComponentMap.insert(std::make_pair(AQLString(GK) + AQLString(THETA)	+ AQLString(PUT),	new AQLGreekthetaPut()	)	);
		blackComponentMap.insert(std::make_pair(AQLString(GK) + AQLString(RHO)	+ AQLString(PUT),	new AQLGreekrhoPut()	)	);
		blackComponentMap.insert(std::make_pair(AQLString(GK) + AQLString(PHI)	+ AQLString(PUT),	new AQLGreekphiPut()	)	);
		blackComponentMap.insert(std::make_pair(AQLString(GK) + AQLString(VANNA)	+ AQLString(PUT), new AQLGreekvannaPut()	)	);
		blackComponentMap.insert(std::make_pair(AQLString(GK) + AQLString(VOLGA)	+ AQLString(PUT), new AQLGreekvolgaPut()	)	);
        //AQLCapFloor
		blackComponentMap.insert(std::make_pair(AQLString(CF) + AQLString(PREM)	+ AQLString(CALL), new AQLCFpremCall()	)	);
		blackComponentMap.insert(std::make_pair(AQLString(CF) + AQLString(VEGA)	+ AQLString(CALL), new AQLCFvegaCall() )	);
		blackComponentMap.insert(std::make_pair(AQLString(CF) + AQLString(PREM)	+ AQLString(PUT), new AQLCFpremPut()	)	);
		blackComponentMap.insert(std::make_pair(AQLString(CF) + AQLString(VEGA)	+ AQLString(PUT), new AQLCFvegaPut() )	);
		//MMDG
		blackComponentMap.insert(std::make_pair(AQLString(DG) + AQLString(PREM)	+ AQLString(CALL), new AQLDGpremCall()	)	);
		blackComponentMap.insert(std::make_pair(AQLString(DG) + AQLString(DELTA)	+ AQLString(CALL), new AQLDGdeltaCall() )	);
		blackComponentMap.insert(std::make_pair(AQLString(DG) + AQLString(GAMMA)	+ AQLString(CALL), new AQLDGgammaCall() )	);
		blackComponentMap.insert(std::make_pair(AQLString(DG) + AQLString(VEGA)	+ AQLString(CALL), new AQLDGvegaCall()	)	);
		blackComponentMap.insert(std::make_pair(AQLString(DG) + AQLString(THETA)	+ AQLString(CALL), new AQLDGthetaCall() )	);
		blackComponentMap.insert(std::make_pair(AQLString(DG) + AQLString(RHO)	+ AQLString(CALL), new AQLDGrhoCall()	)	);
		blackComponentMap.insert(std::make_pair(AQLString(DG) + AQLString(PHI)	+ AQLString(CALL), new AQLDGphiCall()	)	);
		blackComponentMap.insert(std::make_pair(AQLString(DG) + AQLString(PREM)	+ AQLString(PUT),	new AQLDGpremPut()	)	);
		blackComponentMap.insert(std::make_pair(AQLString(DG) + AQLString(DELTA)	+ AQLString(PUT),	new AQLDGdeltaPut()	)	);
		blackComponentMap.insert(std::make_pair(AQLString(DG) + AQLString(GAMMA)	+ AQLString(PUT),	new AQLDGgammaPut()	)	);
		blackComponentMap.insert(std::make_pair(AQLString(DG) + AQLString(VEGA)	+ AQLString(PUT),	new AQLDGvegaPut()	)	);
		blackComponentMap.insert(std::make_pair(AQLString(DG) + AQLString(THETA)	+ AQLString(PUT),	new AQLDGthetaPut()	)	);
		blackComponentMap.insert(std::make_pair(AQLString(DG) + AQLString(RHO)	+ AQLString(PUT),	new AQLDGrhoPut()	)	);
		blackComponentMap.insert(std::make_pair(AQLString(DG) + AQLString(PHI)	+ AQLString(PUT),	new AQLDGphiPut()	)	);
		//MMSB
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(PREM)	+ AQLString(CALL) + AQLString(SBDOWN)	+ AQLString(SBIN)  , new AQLSBpremDIC() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(PREM)	+ AQLString(PUT)  + AQLString(SBDOWN)	+ AQLString(SBIN)  , new AQLSBpremDIP() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(PREM)	+ AQLString(CALL) + AQLString(SBDOWN)	+ AQLString(SBOUT) , new AQLSBpremDOC() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(PREM)	+ AQLString(PUT)  + AQLString(SBDOWN)	+ AQLString(SBOUT) , new AQLSBpremDOP() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(PREM)	+ AQLString(CALL) + AQLString(SBUP)	+ AQLString(SBIN)  , new AQLSBpremUIC() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(PREM)	+ AQLString(PUT)  + AQLString(SBUP)	+ AQLString(SBIN)  , new AQLSBpremUIP() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(PREM)	+ AQLString(CALL) + AQLString(SBUP)	+ AQLString(SBOUT) , new AQLSBpremUOC() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(PREM)	+ AQLString(PUT)  + AQLString(SBUP)	+ AQLString(SBOUT) , new AQLSBpremUOP() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(DELTA)	+ AQLString(CALL) + AQLString(SBDOWN)	+ AQLString(SBIN)  , new AQLSBdeltaDIC() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(DELTA)	+ AQLString(PUT)  + AQLString(SBDOWN)	+ AQLString(SBIN)  , new AQLSBdeltaDIP() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(DELTA)	+ AQLString(CALL) + AQLString(SBDOWN)	+ AQLString(SBOUT) , new AQLSBdeltaDOC() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(DELTA)	+ AQLString(PUT)  + AQLString(SBDOWN)	+ AQLString(SBOUT) , new AQLSBdeltaDOP() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(DELTA)	+ AQLString(CALL) + AQLString(SBUP)	+ AQLString(SBIN)  , new AQLSBdeltaUIC() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(DELTA)	+ AQLString(PUT)  + AQLString(SBUP)	+ AQLString(SBIN)  , new AQLSBdeltaUIP() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(DELTA)	+ AQLString(CALL) + AQLString(SBUP)	+ AQLString(SBOUT) , new AQLSBdeltaUOC() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(DELTA)	+ AQLString(PUT)  + AQLString(SBUP)	+ AQLString(SBOUT) , new AQLSBdeltaUOP() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(GAMMA)	+ AQLString(CALL) + AQLString(SBDOWN)	+ AQLString(SBIN)  , new AQLSBgammaDIC() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(GAMMA)	+ AQLString(PUT)  + AQLString(SBDOWN)	+ AQLString(SBIN)  , new AQLSBgammaDIP() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(GAMMA)	+ AQLString(CALL) + AQLString(SBDOWN)	+ AQLString(SBOUT) , new AQLSBgammaDOC() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(GAMMA)	+ AQLString(PUT)  + AQLString(SBDOWN)	+ AQLString(SBOUT) , new AQLSBgammaDOP() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(GAMMA)	+ AQLString(CALL) + AQLString(SBUP)	+ AQLString(SBIN)  , new AQLSBgammaUIC() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(GAMMA)	+ AQLString(PUT)  + AQLString(SBUP)	+ AQLString(SBIN)  , new AQLSBgammaUIP() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(GAMMA)	+ AQLString(CALL) + AQLString(SBUP)	+ AQLString(SBOUT) , new AQLSBgammaUOC() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(GAMMA)	+ AQLString(PUT)  + AQLString(SBUP)	+ AQLString(SBOUT) , new AQLSBgammaUOP() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(VEGA)	+ AQLString(CALL) + AQLString(SBDOWN)	+ AQLString(SBIN)  , new AQLSBvegaDIC() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(VEGA)	+ AQLString(PUT)  + AQLString(SBDOWN)	+ AQLString(SBIN)  , new AQLSBvegaDIP() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(VEGA)	+ AQLString(CALL) + AQLString(SBDOWN)	+ AQLString(SBOUT) , new AQLSBvegaDOC() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(VEGA)	+ AQLString(PUT)  + AQLString(SBDOWN)	+ AQLString(SBOUT) , new AQLSBvegaDOP() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(VEGA)	+ AQLString(CALL) + AQLString(SBUP)	+ AQLString(SBIN)  , new AQLSBvegaUIC() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(VEGA)	+ AQLString(PUT)  + AQLString(SBUP)	+ AQLString(SBIN)  , new AQLSBvegaUIP() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(VEGA)	+ AQLString(CALL) + AQLString(SBUP)	+ AQLString(SBOUT) , new AQLSBvegaUOC() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(VEGA)	+ AQLString(PUT)  + AQLString(SBUP)	+ AQLString(SBOUT) , new AQLSBvegaUOP() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(THETA)	+ AQLString(CALL) + AQLString(SBDOWN)	+ AQLString(SBIN)  , new AQLSBthetaDIC() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(THETA)	+ AQLString(PUT)  + AQLString(SBDOWN)	+ AQLString(SBIN)  , new AQLSBthetaDIP() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(THETA)	+ AQLString(CALL) + AQLString(SBDOWN)	+ AQLString(SBOUT) , new AQLSBthetaDOC() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(THETA)	+ AQLString(PUT)  + AQLString(SBDOWN)	+ AQLString(SBOUT) , new AQLSBthetaDOP() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(THETA)	+ AQLString(CALL) + AQLString(SBUP)	+ AQLString(SBIN)  , new AQLSBthetaUIC() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(THETA)	+ AQLString(PUT)  + AQLString(SBUP)	+ AQLString(SBIN)  , new AQLSBthetaUIP() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(THETA)	+ AQLString(CALL) + AQLString(SBUP)	+ AQLString(SBOUT) , new AQLSBthetaUOC() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(THETA)	+ AQLString(PUT)  + AQLString(SBUP)	+ AQLString(SBOUT) , new AQLSBthetaUOP() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(RHO)	+ AQLString(CALL) + AQLString(SBDOWN)	+ AQLString(SBIN)  , new AQLSBrhoDIC() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(RHO)	+ AQLString(PUT)  + AQLString(SBDOWN)	+ AQLString(SBIN)  , new AQLSBrhoDIP() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(RHO)	+ AQLString(CALL) + AQLString(SBDOWN)	+ AQLString(SBOUT) , new AQLSBrhoDOC() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(RHO)	+ AQLString(PUT)  + AQLString(SBDOWN)	+ AQLString(SBOUT) , new AQLSBrhoDOP() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(RHO)	+ AQLString(CALL) + AQLString(SBUP)	+ AQLString(SBIN)  , new AQLSBrhoUIC() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(RHO)	+ AQLString(PUT)  + AQLString(SBUP)	+ AQLString(SBIN)  , new AQLSBrhoUIP() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(RHO)	+ AQLString(CALL) + AQLString(SBUP)	+ AQLString(SBOUT) , new AQLSBrhoUOC() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(RHO)	+ AQLString(PUT)  + AQLString(SBUP)	+ AQLString(SBOUT) , new AQLSBrhoUOP() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(PHI)	+ AQLString(CALL) + AQLString(SBDOWN)	+ AQLString(SBIN)  , new AQLSBphiDIC() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(PHI)	+ AQLString(PUT)  + AQLString(SBDOWN)	+ AQLString(SBIN)  , new AQLSBphiDIP() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(PHI)	+ AQLString(CALL) + AQLString(SBDOWN)	+ AQLString(SBOUT) , new AQLSBphiDOC() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(PHI)	+ AQLString(PUT)  + AQLString(SBDOWN)	+ AQLString(SBOUT) , new AQLSBphiDOP() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(PHI)	+ AQLString(CALL) + AQLString(SBUP)	+ AQLString(SBIN)  , new AQLSBphiUIC() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(PHI)	+ AQLString(PUT)  + AQLString(SBUP)	+ AQLString(SBIN)  , new AQLSBphiUIP() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(PHI)	+ AQLString(CALL) + AQLString(SBUP)	+ AQLString(SBOUT) , new AQLSBphiUOC() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(PHI)	+ AQLString(PUT)  + AQLString(SBUP)	+ AQLString(SBOUT) , new AQLSBphiUOP() ));

		//MMDB
		blackComponentMap.insert(std::make_pair(AQLString(DB) + AQLString(PREM)	+ AQLString(CALL) + AQLString(DBIN)	+ AQLString(DBIN)  , new AQLDBpremKIC() ));
		blackComponentMap.insert(std::make_pair(AQLString(DB) + AQLString(PREM)	+ AQLString(CALL) + AQLString(DBOUT)	+ AQLString(DBOUT) , new AQLDBpremKOC()	));
		blackComponentMap.insert(std::make_pair(AQLString(DB) + AQLString(PREM)	+ AQLString(PUT)  + AQLString(DBIN)	+ AQLString(DBIN)  , new AQLDBpremKIP() ));
		blackComponentMap.insert(std::make_pair(AQLString(DB) + AQLString(PREM)	+ AQLString(PUT)  + AQLString(DBOUT)	+ AQLString(DBOUT) , new AQLDBpremKOP()	));
		blackComponentMap.insert(std::make_pair(AQLString(DB) + AQLString(REBATE)	+ AQLString(RHIGH),	new AQLDBpremRH()				));
		blackComponentMap.insert(std::make_pair(AQLString(DB) + AQLString(REBATE)	+ AQLString(RLOW),		new AQLDBpremRL()				));
		blackComponentMap.insert(std::make_pair(AQLString(DB) + AQLString(REBATE)	+ AQLString(RIN),		new AQLDBpremRI()				));
		
		blackComponentMap.insert(std::make_pair(AQLString(FD) + AQLString(DELTA)	+ AQLString(CALL) + AQLString(SPOT)  , new AQLFDdeltaCallSpot() ));
		blackComponentMap.insert(std::make_pair(AQLString(FD) + AQLString(DELTA)	+ AQLString(PUT) + AQLString(SPOT)  , new AQLFDdeltaPutSpot() ));
		blackComponentMap.insert(std::make_pair(AQLString(FD) + AQLString(FIRSTDIFF)	+ AQLString(CALL) + AQLString(SPOT)  , new AQLFD1stDiffCallSpot() ));
		blackComponentMap.insert(std::make_pair(AQLString(FD) + AQLString(FIRSTDIFF)	+ AQLString(PUT) + AQLString(SPOT)  , new AQLFD1stDiffPutSpot() ));
		blackComponentMap.insert(std::make_pair(AQLString(FD) + AQLString(DELTA)	+ AQLString(CALL) + AQLString(FORWARD)  , new AQLFDdeltaCallFwd() ));
		blackComponentMap.insert(std::make_pair(AQLString(FD) + AQLString(DELTA)	+ AQLString(PUT) + AQLString(FORWARD)  , new AQLFDdeltaPutFwd() ));
		blackComponentMap.insert(std::make_pair(AQLString(FD) + AQLString(FIRSTDIFF)	+ AQLString(CALL) + AQLString(FORWARD)  , new AQLFD1stDiffCallFwd() ));
		blackComponentMap.insert(std::make_pair(AQLString(FD) + AQLString(FIRSTDIFF)	+ AQLString(PUT) + AQLString(FORWARD)  , new AQLFD1stDiffPutFwd() ));
		blackComponentMap.insert(std::make_pair(AQLString(FD) + AQLString(SECONDDIFF)	+ AQLString(CALL) + AQLString(SPOT)  , new AQLFD2ndDiffCallSpot() ));
		blackComponentMap.insert(std::make_pair(AQLString(FD) + AQLString(SECONDDIFF)	+ AQLString(CALL) + AQLString(FORWARD)  , new AQLFD2ndDiffCallFwd() ));		
		//MMAFF
		blackComponentMap.insert(std::make_pair(AQLString(AFF)	+ AQLString(PREM) + AQLString(CALL),	new AQLAFFpremCall()	)	);
		blackComponentMap.insert(std::make_pair(AQLString(AFF)	+ AQLString(PREM) + AQLString(PUT),	new AQLAFFpremPut()	)	);
	}
	return blackComponentMap;
}
std::map<AQLString, AnalyticParam*>& 
AQLCoreComponentManager::getBlackParamComponentMap()
{
	if(blackParamComponentMap.empty())
	{
		blackParamComponentMap.insert(std::make_pair(AQLString(BK), new AnalyticBKParam() ));
		blackParamComponentMap.insert(std::make_pair(AQLString(GK), new AnalyticGKParam() ));
		blackParamComponentMap.insert(std::make_pair(AQLString(DG), new AnalyticDGParam() ));
		blackParamComponentMap.insert(std::make_pair(AQLString(SB), new AnalyticSBParam() ));
		blackParamComponentMap.insert(std::make_pair(AQLString(DB), new AnalyticDBParam() ));
	}
	return blackParamComponentMap;
}

/*!
	@brief getIntetpolation

	@param[in] key
	@return AQLString

*/
AQLString
AQLCoreComponentManager::getInterpolation(const AQLString &key)
{
	if (key == LINEAR)
	{
		return FN_LINEARINTERPOLATION_STR;
	}
	else if (key == SPLINE)
	{
		return FN_SPLINEINTERPOLATION_STR;
	}
	else if (key == MONOTONESPLINE)
	{
		return FN_MONOTONESPLINEINTERPOLATION_STR;
	}
	else if (key == MONOTONEPARABOLIC)
	{
		return FN_MONOTONEPARABOLICINTERPOLATION_STR;
	}
    else if (key == NATURALSPLINE)
	{
		return FN_NATURALSPLINEINTERPOLATION_STR;
	}
    else if (key == CLAMPEDSPLINE)
	{
		return FN_CLAMPEDSPLINEINTERPOLATION_STR;
	}
    else if (key == PARABOLIC)
	{
		return FN_PARABOLICINTERPOLATION_STR;
	}
    else if (key == LINEARSPLINE)
	{
		return FN_LINEARSPLINEINTERPOLATION_STR;
	}
	else if (key == LINEARMONOTONESPLINE)
	{
		return FN_LINEARMONOTONESPLINEINTERPOLATION_STR;
	}
	else if (key == LINEARMONOTONEPARABOLIC)
	{
		return FN_LINEARMONOTONEPARABOLICINTERPOLATION_STR;
	}
	else if (key == STEP)
	{
		return FN_STEPINTERPOLATION_STR;
	}
	else if (key == RIGHTCONTINUOUS)
	{
		return FN_RIGHTCONTINUOUS_INTERPOLATION_STR;
	}
	else if (key == LEFTCONTINUOUS)
	{
		return FN_LEFTCONTINUOUS_INTERPOLATION_STR;
	}
    else if (key == CONSTRAINEDSPLINE)
	{
		return FN_CONSTRAINEDSPLINEINTERPOLATION_STR;
	}
    else if (key == MONOTONECONVEX)
	{
		return FN_MONOTONECONVEXINTERPOLATION_STR;
	}
	else if (key == BASISSTEP)
	{
		return FN_BASISFUNC1_STR;
	}
	else if (key == BASISLINEAR)
	{
		return FN_BASISFUNC2_STR;
	}
	else
	{
		AQLString msg = key + " is not registered in interpolation methods";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__,__LINE__);
	}

}

/*!
	@brief getDayCount

	@param[in] key
	@return AQLString

*/
AQLString
AQLCoreComponentManager::getDayCount(const AQLString &key)
{
	if (key == AC_365I || key == AC_AC)
	{
		return AC_365I;
	}
	else if (key == AC_365)
	{
		return AC_365;
	}
	else if (key == AC_360)
	{
		return AC_360;
	}
	else if (key == N3_360)
	{
		return N3_360;
	}
	else if (key == E3_360)
	{
		return E3_360;
	}
	else if (key == AC_365F)
	{
		return AC_365F;
	}
	else if (key == AC_AC_ICMA)
	{
		return AC_AC_ICMA;
	}
	else if (key == ONE)
	{
		return ONE;
	}
	else if (key == HALF)
	{
		return HALF;
	}
	else if (key == QUARTER)
	{
		return QUARTER;
	}
	else
	{
		AQLString msg = key + " is not registered in daycount conventions";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__,__LINE__);
	}

}

/*!
	@brief getBasisType

	@param[in] key
	@return AQLString

*/
AQLString
AQLCoreComponentManager::getBasisType(const AQLString &key)
{
	if (key == XCCYBASIS)
	{
		return XCCYBASIS;
	}
	else
	{
		throw AQLCoreInvalidData("BasisType is not registered ", __FILE__,__LINE__);
	}
}

/*!
	@brief initialize

*/
void
AQLCoreComponentManager::initialize()
{
	// interpolation map
	interpolationMap.insert(std::make_pair(LINEAR,					"fn_linearinterpolation"));
	interpolationMap.insert(std::make_pair(SPLINE,					"fn_splineinterpolation"));
	interpolationMap.insert(std::make_pair(MONOTONESPLINE,			"fn_monotonesplineinterpolation"));
    interpolationMap.insert(std::make_pair(NATURALSPLINE,			"fn_naturalsplineinterpolation"));
    interpolationMap.insert(std::make_pair(CLAMPEDSPLINE,			"fn_clampedsplineinterpolation"));
	interpolationMap.insert(std::make_pair(MONOTONEPARABOLIC,		"fn_monotoneparabolicinterpolation"));
    interpolationMap.insert(std::make_pair(PARABOLIC,				"fn_parabolicinterpolation"));
    interpolationMap.insert(std::make_pair(LINEARSPLINE,			"fn_linearsplineinterpolation"));
	interpolationMap.insert(std::make_pair(LINEARMONOTONESPLINE,	"fn_linearmonotonesplineinterpolation"));
	interpolationMap.insert(std::make_pair(LINEARMONOTONEPARABOLIC,	"fn_linearmonotoneparabolicinterpolation"));
	interpolationMap.insert(std::make_pair(STEP,					"fn_stepinterpolation"  ));
	interpolationMap.insert(std::make_pair(RIGHTCONTINUOUS,			"fn_rightcontinuousinterpolation"  ));
	interpolationMap.insert(std::make_pair(LEFTCONTINUOUS,			"fn_leftcontinuousinterpolation"  ));
    interpolationMap.insert(std::make_pair(CONSTRAINEDSPLINE,		"fn_constrainedsplineinterpolation"  ));
    interpolationMap.insert(std::make_pair(MONOTONECONVEX,			"fn_monotoneconvexinterpolation"  ));
	
	//basis Set
	interpolationMap.insert(std::make_pair(BASISSTEP,	"fn_basisfunc1"  ));
	interpolationMap.insert(std::make_pair(BASISLINEAR,	"fn_basisfunc2"  ));

	// daycount map
	dayCountMap.insert(std::make_pair(AC_AC,		AC_365I));
	dayCountMap.insert(std::make_pair(AC_365,		AC_365));
	dayCountMap.insert(std::make_pair(AC_360,		AC_360));
	dayCountMap.insert(std::make_pair(N3_360,		N3_360));
	dayCountMap.insert(std::make_pair(E3_360,		E3_360));
	dayCountMap.insert(std::make_pair(AC_365I,		AC_365I));
	dayCountMap.insert(std::make_pair(AC_365F,		AC_365F));
	dayCountMap.insert(std::make_pair(AC_AC_ICMA,		AC_AC_ICMA));

	// basis type map
	basisTypeMap.insert(std::make_pair(XCCYBASIS,	XCCYBASIS));

	// basis index map
	basisIndexMap.insert(std::make_pair(LIBOR,	LIBOR));

	// black component map
	
	//AQLZeroForError
	blackComponentMap.insert(std::make_pair(AQLString("ERROR"),	new AQLZeroForError()));
	//AQLBlackPayOff
	blackComponentMap.insert(std::make_pair(AQLString(BKPAYOFF)	+ AQLString(PREM) + AQLString(CALL),	new AQLBlackPayOffpremCall())	);
	blackComponentMap.insert(std::make_pair(AQLString(BKPAYOFF)	+ AQLString(PREM) + AQLString(PUT),	new AQLBlackPayOffpremPut()	)	);
	
	
	
	
	//AQLBlack
	blackComponentMap.insert(std::make_pair(AQLString(BK)	+ AQLString(PREM) + AQLString(CALL),	new AQLBlackpremCall()	)	);
	blackComponentMap.insert(std::make_pair(AQLString(BK)	+ AQLString(PREM) + AQLString(PUT),	new AQLBlackpremPut()	)	);
	blackComponentMap.insert(std::make_pair(AQLString(BK)	+ AQLString(DELTA) + AQLString(CALL), new AQLBlackdeltaCall()	)	);
	blackComponentMap.insert(std::make_pair(AQLString(BK)	+ AQLString(GAMMA) + AQLString(CALL), new AQLBlackgammaCall()	)	);
	blackComponentMap.insert(std::make_pair(AQLString(BK)	+ AQLString(VEGA)  + AQLString(CALL), new AQLBlackvegaCall()	)	);
	blackComponentMap.insert(std::make_pair(AQLString(BK)	+ AQLString(THETA) + AQLString(CALL), new AQLBlackthetaCall()	)	);
	blackComponentMap.insert(std::make_pair(AQLString(BK)	+ AQLString(DELTA) + AQLString(PUT),  new AQLBlackdeltaPut()	)	);
	blackComponentMap.insert(std::make_pair(AQLString(BK)	+ AQLString(GAMMA) + AQLString(PUT),  new AQLBlackgammaPut()	)	);
	blackComponentMap.insert(std::make_pair(AQLString(BK)	+ AQLString(VEGA)  + AQLString(PUT),  new AQLBlackvegaPut()	)	);
	blackComponentMap.insert(std::make_pair(AQLString(BK)	+ AQLString(THETA) + AQLString(PUT),  new AQLBlackthetaPut()	)	);
	//AQLGreek
	blackComponentMap.insert(std::make_pair(AQLString(GK) + AQLString(PREM)	+ AQLString(CALL), new AQLGreekpremCall()	)	);
	blackComponentMap.insert(std::make_pair(AQLString(GK) + AQLString(DELTA)	+ AQLString(CALL), new AQLGreekdeltaCall() )	);
	blackComponentMap.insert(std::make_pair(AQLString(GK) + AQLString(GAMMA)	+ AQLString(CALL), new AQLGreekgammaCall() )	);
	blackComponentMap.insert(std::make_pair(AQLString(GK) + AQLString(VEGA)	+ AQLString(CALL), new AQLGreekvegaCall()	)	);
	blackComponentMap.insert(std::make_pair(AQLString(GK) + AQLString(THETA)	+ AQLString(CALL), new AQLGreekthetaCall() )	);
	blackComponentMap.insert(std::make_pair(AQLString(GK) + AQLString(RHO)	+ AQLString(CALL), new AQLGreekrhoCall()	)	);
	blackComponentMap.insert(std::make_pair(AQLString(GK) + AQLString(PHI)	+ AQLString(CALL), new AQLGreekphiCall()	)	);
	blackComponentMap.insert(std::make_pair(AQLString(GK) + AQLString(VANNA)	+ AQLString(CALL), new AQLGreekvannaCall()	)	);
	blackComponentMap.insert(std::make_pair(AQLString(GK) + AQLString(VOLGA)	+ AQLString(CALL), new AQLGreekvolgaCall()	)	);
	blackComponentMap.insert(std::make_pair(AQLString(GK) + AQLString(PREM)	+ AQLString(PUT),	new AQLGreekpremPut()	)	);
	blackComponentMap.insert(std::make_pair(AQLString(GK) + AQLString(DELTA)	+ AQLString(PUT),	new AQLGreekdeltaPut()	)	);
	blackComponentMap.insert(std::make_pair(AQLString(GK) + AQLString(GAMMA)	+ AQLString(PUT),	new AQLGreekgammaPut()	)	);
	blackComponentMap.insert(std::make_pair(AQLString(GK) + AQLString(VEGA)	+ AQLString(PUT),	new AQLGreekvegaPut()	)	);
	blackComponentMap.insert(std::make_pair(AQLString(GK) + AQLString(THETA)	+ AQLString(PUT),	new AQLGreekthetaPut()	)	);
	blackComponentMap.insert(std::make_pair(AQLString(GK) + AQLString(RHO)	+ AQLString(PUT),	new AQLGreekrhoPut()	)	);
	blackComponentMap.insert(std::make_pair(AQLString(GK) + AQLString(PHI)	+ AQLString(PUT),	new AQLGreekphiPut()	)	);
	blackComponentMap.insert(std::make_pair(AQLString(GK) + AQLString(VANNA)	+ AQLString(PUT), new AQLGreekvannaPut()	)	);
	blackComponentMap.insert(std::make_pair(AQLString(GK) + AQLString(VOLGA)	+ AQLString(PUT), new AQLGreekvolgaPut()	)	);
	//AQLCapFloor
	blackComponentMap.insert(std::make_pair(AQLString(CF) + AQLString(PREM) + AQLString(CALL), new AQLCFpremCall()));
	blackComponentMap.insert(std::make_pair(AQLString(CF) + AQLString(VEGA) + AQLString(CALL), new AQLCFvegaCall()));
	blackComponentMap.insert(std::make_pair(AQLString(CF) + AQLString(PREM) + AQLString(PUT), new AQLCFpremPut()));
	blackComponentMap.insert(std::make_pair(AQLString(CF) + AQLString(VEGA) + AQLString(PUT), new AQLCFvegaPut()));
	//MMDG
	blackComponentMap.insert(std::make_pair(AQLString(DG) + AQLString(PREM)	+ AQLString(CALL), new AQLDGpremCall()	)	);
	blackComponentMap.insert(std::make_pair(AQLString(DG) + AQLString(DELTA)	+ AQLString(CALL), new AQLDGdeltaCall() )	);
	blackComponentMap.insert(std::make_pair(AQLString(DG) + AQLString(GAMMA)	+ AQLString(CALL), new AQLDGgammaCall() )	);
	blackComponentMap.insert(std::make_pair(AQLString(DG) + AQLString(VEGA)	+ AQLString(CALL), new AQLDGvegaCall()	)	);
	blackComponentMap.insert(std::make_pair(AQLString(DG) + AQLString(THETA)	+ AQLString(CALL), new AQLDGthetaCall() )	);
	blackComponentMap.insert(std::make_pair(AQLString(DG) + AQLString(RHO)	+ AQLString(CALL), new AQLDGrhoCall()	)	);
	blackComponentMap.insert(std::make_pair(AQLString(DG) + AQLString(PHI)	+ AQLString(CALL), new AQLDGphiCall()	)	);
	blackComponentMap.insert(std::make_pair(AQLString(DG) + AQLString(PREM)	+ AQLString(PUT),	new AQLDGpremPut()	)	);
	blackComponentMap.insert(std::make_pair(AQLString(DG) + AQLString(DELTA)	+ AQLString(PUT),	new AQLDGdeltaPut()	)	);
	blackComponentMap.insert(std::make_pair(AQLString(DG) + AQLString(GAMMA)	+ AQLString(PUT),	new AQLDGgammaPut()	)	);
	blackComponentMap.insert(std::make_pair(AQLString(DG) + AQLString(VEGA)	+ AQLString(PUT),	new AQLDGvegaPut()	)	);
	blackComponentMap.insert(std::make_pair(AQLString(DG) + AQLString(THETA)	+ AQLString(PUT),	new AQLDGthetaPut()	)	);
	blackComponentMap.insert(std::make_pair(AQLString(DG) + AQLString(RHO)	+ AQLString(PUT),	new AQLDGrhoPut()	)	);
	blackComponentMap.insert(std::make_pair(AQLString(DG) + AQLString(PHI)	+ AQLString(PUT),	new AQLDGphiPut()	)	);
	//hishida vannavolga
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(PROB)	+ AQLString(SBUP) + AQLString(NOTOUCH), new AQLSBprobUNT() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(PROB)	+ AQLString(SBDOWN) + AQLString(NOTOUCH), new AQLSBprobDNT() ));
	//hishida vannavolga
	//MMSB
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(PREM)	+ AQLString(CALL) + AQLString(SBDOWN)	+ AQLString(SBIN)  , new AQLSBpremDIC() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(PREM)	+ AQLString(PUT)  + AQLString(SBDOWN)	+ AQLString(SBIN)  , new AQLSBpremDIP() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(PREM)	+ AQLString(CALL) + AQLString(SBDOWN)	+ AQLString(SBOUT) , new AQLSBpremDOC() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(PREM)	+ AQLString(PUT)  + AQLString(SBDOWN)	+ AQLString(SBOUT) , new AQLSBpremDOP() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(PREM)	+ AQLString(CALL) + AQLString(SBUP)	+ AQLString(SBIN)  , new AQLSBpremUIC() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(PREM)	+ AQLString(PUT)  + AQLString(SBUP)	+ AQLString(SBIN)  , new AQLSBpremUIP() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(PREM)	+ AQLString(CALL) + AQLString(SBUP)	+ AQLString(SBOUT) , new AQLSBpremUOC() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(PREM)	+ AQLString(PUT)  + AQLString(SBUP)	+ AQLString(SBOUT) , new AQLSBpremUOP() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(DELTA)	+ AQLString(CALL) + AQLString(SBDOWN)	+ AQLString(SBIN)  , new AQLSBdeltaDIC() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(DELTA)	+ AQLString(PUT)  + AQLString(SBDOWN)	+ AQLString(SBIN)  , new AQLSBdeltaDIP() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(DELTA)	+ AQLString(CALL) + AQLString(SBDOWN)	+ AQLString(SBOUT) , new AQLSBdeltaDOC() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(DELTA)	+ AQLString(PUT)  + AQLString(SBDOWN)	+ AQLString(SBOUT) , new AQLSBdeltaDOP() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(DELTA)	+ AQLString(CALL) + AQLString(SBUP)	+ AQLString(SBIN)  , new AQLSBdeltaUIC() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(DELTA)	+ AQLString(PUT)  + AQLString(SBUP)	+ AQLString(SBIN)  , new AQLSBdeltaUIP() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(DELTA)	+ AQLString(CALL) + AQLString(SBUP)	+ AQLString(SBOUT) , new AQLSBdeltaUOC() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(DELTA)	+ AQLString(PUT)  + AQLString(SBUP)	+ AQLString(SBOUT) , new AQLSBdeltaUOP() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(GAMMA)	+ AQLString(CALL) + AQLString(SBDOWN)	+ AQLString(SBIN)  , new AQLSBgammaDIC() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(GAMMA)	+ AQLString(PUT)  + AQLString(SBDOWN)	+ AQLString(SBIN)  , new AQLSBgammaDIP() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(GAMMA)	+ AQLString(CALL) + AQLString(SBDOWN)	+ AQLString(SBOUT) , new AQLSBgammaDOC() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(GAMMA)	+ AQLString(PUT)  + AQLString(SBDOWN)	+ AQLString(SBOUT) , new AQLSBgammaDOP() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(GAMMA)	+ AQLString(CALL) + AQLString(SBUP)	+ AQLString(SBIN)  , new AQLSBgammaUIC() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(GAMMA)	+ AQLString(PUT)  + AQLString(SBUP)	+ AQLString(SBIN)  , new AQLSBgammaUIP() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(GAMMA)	+ AQLString(CALL) + AQLString(SBUP)	+ AQLString(SBOUT) , new AQLSBgammaUOC() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(GAMMA)	+ AQLString(PUT)  + AQLString(SBUP)	+ AQLString(SBOUT) , new AQLSBgammaUOP() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(VEGA)	+ AQLString(CALL) + AQLString(SBDOWN)	+ AQLString(SBIN)  , new AQLSBvegaDIC() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(VEGA)	+ AQLString(PUT)  + AQLString(SBDOWN)	+ AQLString(SBIN)  , new AQLSBvegaDIP() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(VEGA)	+ AQLString(CALL) + AQLString(SBDOWN)	+ AQLString(SBOUT) , new AQLSBvegaDOC() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(VEGA)	+ AQLString(PUT)  + AQLString(SBDOWN)	+ AQLString(SBOUT) , new AQLSBvegaDOP() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(VEGA)	+ AQLString(CALL) + AQLString(SBUP)	+ AQLString(SBIN)  , new AQLSBvegaUIC() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(VEGA)	+ AQLString(PUT)  + AQLString(SBUP)	+ AQLString(SBIN)  , new AQLSBvegaUIP() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(VEGA)	+ AQLString(CALL) + AQLString(SBUP)	+ AQLString(SBOUT) , new AQLSBvegaUOC() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(VEGA)	+ AQLString(PUT)  + AQLString(SBUP)	+ AQLString(SBOUT) , new AQLSBvegaUOP() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(THETA)	+ AQLString(CALL) + AQLString(SBDOWN)	+ AQLString(SBIN)  , new AQLSBthetaDIC() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(THETA)	+ AQLString(PUT)  + AQLString(SBDOWN)	+ AQLString(SBIN)  , new AQLSBthetaDIP() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(THETA)	+ AQLString(CALL) + AQLString(SBDOWN)	+ AQLString(SBOUT) , new AQLSBthetaDOC() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(THETA)	+ AQLString(PUT)  + AQLString(SBDOWN)	+ AQLString(SBOUT) , new AQLSBthetaDOP() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(THETA)	+ AQLString(CALL) + AQLString(SBUP)	+ AQLString(SBIN)  , new AQLSBthetaUIC() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(THETA)	+ AQLString(PUT)  + AQLString(SBUP)	+ AQLString(SBIN)  , new AQLSBthetaUIP() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(THETA)	+ AQLString(CALL) + AQLString(SBUP)	+ AQLString(SBOUT) , new AQLSBthetaUOC() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(THETA)	+ AQLString(PUT)  + AQLString(SBUP)	+ AQLString(SBOUT) , new AQLSBthetaUOP() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(RHO)	+ AQLString(CALL) + AQLString(SBDOWN)	+ AQLString(SBIN)  , new AQLSBrhoDIC() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(RHO)	+ AQLString(PUT)  + AQLString(SBDOWN)	+ AQLString(SBIN)  , new AQLSBrhoDIP() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(RHO)	+ AQLString(CALL) + AQLString(SBDOWN)	+ AQLString(SBOUT) , new AQLSBrhoDOC() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(RHO)	+ AQLString(PUT)  + AQLString(SBDOWN)	+ AQLString(SBOUT) , new AQLSBrhoDOP() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(RHO)	+ AQLString(CALL) + AQLString(SBUP)	+ AQLString(SBIN)  , new AQLSBrhoUIC() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(RHO)	+ AQLString(PUT)  + AQLString(SBUP)	+ AQLString(SBIN)  , new AQLSBrhoUIP() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(RHO)	+ AQLString(CALL) + AQLString(SBUP)	+ AQLString(SBOUT) , new AQLSBrhoUOC() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(RHO)	+ AQLString(PUT)  + AQLString(SBUP)	+ AQLString(SBOUT) , new AQLSBrhoUOP() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(PHI)	+ AQLString(CALL) + AQLString(SBDOWN)	+ AQLString(SBIN)  , new AQLSBphiDIC() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(PHI)	+ AQLString(PUT)  + AQLString(SBDOWN)	+ AQLString(SBIN)  , new AQLSBphiDIP() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(PHI)	+ AQLString(CALL) + AQLString(SBDOWN)	+ AQLString(SBOUT) , new AQLSBphiDOC() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(PHI)	+ AQLString(PUT)  + AQLString(SBDOWN)	+ AQLString(SBOUT) , new AQLSBphiDOP() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(PHI)	+ AQLString(CALL) + AQLString(SBUP)	+ AQLString(SBIN)  , new AQLSBphiUIC() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(PHI)	+ AQLString(PUT)  + AQLString(SBUP)	+ AQLString(SBIN)  , new AQLSBphiUIP() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(PHI)	+ AQLString(CALL) + AQLString(SBUP)	+ AQLString(SBOUT) , new AQLSBphiUOC() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(PHI)	+ AQLString(PUT)  + AQLString(SBUP)	+ AQLString(SBOUT) , new AQLSBphiUOP() ));

	//MMDB
	blackComponentMap.insert(std::make_pair(AQLString(DB) + AQLString(PREM)	+ AQLString(CALL) + AQLString(DBIN)	+ AQLString(DBIN)  , new AQLDBpremKIC() ));
	blackComponentMap.insert(std::make_pair(AQLString(DB) + AQLString(PREM)	+ AQLString(CALL) + AQLString(DBOUT)	+ AQLString(DBOUT) , new AQLDBpremKOC()	));
	blackComponentMap.insert(std::make_pair(AQLString(DB) + AQLString(PREM)	+ AQLString(PUT)  + AQLString(DBIN)	+ AQLString(DBIN)  , new AQLDBpremKIP() ));
	blackComponentMap.insert(std::make_pair(AQLString(DB) + AQLString(PREM)	+ AQLString(PUT)  + AQLString(DBOUT)	+ AQLString(DBOUT) , new AQLDBpremKOP()	));
	blackComponentMap.insert(std::make_pair(AQLString(DB) + AQLString(REBATE)	+ AQLString(RHIGH),	new AQLDBpremRH()				));
	blackComponentMap.insert(std::make_pair(AQLString(DB) + AQLString(REBATE)	+ AQLString(RLOW),		new AQLDBpremRL()				));
	blackComponentMap.insert(std::make_pair(AQLString(DB) + AQLString(REBATE)	+ AQLString(RIN),		new AQLDBpremRI()				));
	
	blackComponentMap.insert(std::make_pair(AQLString(FD) + AQLString(DELTA)	+ AQLString(CALL) + AQLString(SPOT)  , new AQLFDdeltaCallSpot() ));
	blackComponentMap.insert(std::make_pair(AQLString(FD) + AQLString(DELTA)	+ AQLString(PUT) + AQLString(SPOT)  , new AQLFDdeltaPutSpot() ));
	blackComponentMap.insert(std::make_pair(AQLString(FD) + AQLString(FIRSTDIFF)	+ AQLString(CALL) + AQLString(SPOT)  , new AQLFD1stDiffCallSpot() ));
	blackComponentMap.insert(std::make_pair(AQLString(FD) + AQLString(FIRSTDIFF)	+ AQLString(PUT) + AQLString(SPOT)  , new AQLFD1stDiffPutSpot() ));
	blackComponentMap.insert(std::make_pair(AQLString(FD) + AQLString(DELTA)	+ AQLString(CALL) + AQLString(FORWARD)  , new AQLFDdeltaCallFwd() ));
	blackComponentMap.insert(std::make_pair(AQLString(FD) + AQLString(DELTA)	+ AQLString(PUT) + AQLString(FORWARD)  , new AQLFDdeltaPutFwd() ));
	blackComponentMap.insert(std::make_pair(AQLString(FD) + AQLString(FIRSTDIFF)	+ AQLString(CALL) + AQLString(FORWARD)  , new AQLFD1stDiffCallFwd() ));
	blackComponentMap.insert(std::make_pair(AQLString(FD) + AQLString(FIRSTDIFF)	+ AQLString(PUT) + AQLString(FORWARD)  , new AQLFD1stDiffPutFwd() ));
	blackComponentMap.insert(std::make_pair(AQLString(FD) + AQLString(SECONDDIFF)	+ AQLString(CALL) + AQLString(SPOT)  , new AQLFD2ndDiffCallSpot() ));
	blackComponentMap.insert(std::make_pair(AQLString(FD) + AQLString(SECONDDIFF)	+ AQLString(CALL) + AQLString(FORWARD)  , new AQLFD2ndDiffCallFwd() ));
	//MMAFF
	blackComponentMap.insert(std::make_pair(AQLString(AFF)	+ AQLString(PREM) + AQLString(CALL),	new AQLAFFpremCall()	)	);
	blackComponentMap.insert(std::make_pair(AQLString(AFF)	+ AQLString(PREM) + AQLString(PUT),	new AQLAFFpremPut()	)	);


	// black param component map
	blackParamComponentMap.insert(std::make_pair(AQLString(BK), new AnalyticBKParam() ));
	blackParamComponentMap.insert(std::make_pair(AQLString(GK), new AnalyticGKParam() ));
	blackParamComponentMap.insert(std::make_pair(AQLString(DG), new AnalyticDGParam() ));
	blackParamComponentMap.insert(std::make_pair(AQLString(SB), new AnalyticSBParam() ));
	blackParamComponentMap.insert(std::make_pair(AQLString(DB), new AnalyticDBParam() ));

}

/*!
	@brief finalize

*/
void
AQLCoreComponentManager::finalize()
{
	interpolationMap.clear();
	dayCountMap.clear();
	basisTypeMap.clear();
	basisIndexMap.clear();
	deleteBlackComponentMap();
	deleteBlackParamComponentMap();
}




void 
AQLCoreComponentManager::deleteBlackComponentMap()
{
	std::map<AQLString, AQLBlackScholesBase*> &var = AQLCoreComponentManager::getBlackComponentMap();
	
	std::map<AQLString, AQLBlackScholesBase*>::iterator it = var.begin();
	while (it != var.end())
	{
		delete it->second;
		++it;
	}
	var.clear();
}
void 
AQLCoreComponentManager::deleteBlackParamComponentMap()
{
	std::map<AQLString, AnalyticParam*> &var = AQLCoreComponentManager::getBlackParamComponentMap();
	
	std::map<AQLString, AnalyticParam*>::iterator it = var.begin();
	while (it != var.end())
	{
		delete it->second;
		++it;
	}
	var.clear();
}
