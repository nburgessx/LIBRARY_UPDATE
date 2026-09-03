
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
		//MMZeroForError
		blackComponentMap.insert(std::make_pair(AQLString("ERROR"),	new MMZeroForError()));
		//LABlack
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
		//LABlackPayOff
		blackComponentMap.insert(std::make_pair(AQLString(BKPAYOFF) + AQLString(PREM) + AQLString(CALL),	new AQLBlackPayOffpremCall()	)	);
		blackComponentMap.insert(std::make_pair(AQLString(BKPAYOFF) + AQLString(PREM) + AQLString(PUT),	new AQLBlackPayOffpremPut()		)	);
		//LAGreek
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
        //MMCapFloor
		blackComponentMap.insert(std::make_pair(AQLString(CF) + AQLString(PREM)	+ AQLString(CALL), new MMCFpremCall()	)	);
		blackComponentMap.insert(std::make_pair(AQLString(CF) + AQLString(VEGA)	+ AQLString(CALL), new MMCFvegaCall() )	);
		blackComponentMap.insert(std::make_pair(AQLString(CF) + AQLString(PREM)	+ AQLString(PUT), new MMCFpremPut()	)	);
		blackComponentMap.insert(std::make_pair(AQLString(CF) + AQLString(VEGA)	+ AQLString(PUT), new MMCFvegaPut() )	);
		//MMDG
		blackComponentMap.insert(std::make_pair(AQLString(DG) + AQLString(PREM)	+ AQLString(CALL), new MMDGpremCall()	)	);
		blackComponentMap.insert(std::make_pair(AQLString(DG) + AQLString(DELTA)	+ AQLString(CALL), new MMDGdeltaCall() )	);
		blackComponentMap.insert(std::make_pair(AQLString(DG) + AQLString(GAMMA)	+ AQLString(CALL), new MMDGgammaCall() )	);
		blackComponentMap.insert(std::make_pair(AQLString(DG) + AQLString(VEGA)	+ AQLString(CALL), new MMDGvegaCall()	)	);
		blackComponentMap.insert(std::make_pair(AQLString(DG) + AQLString(THETA)	+ AQLString(CALL), new MMDGthetaCall() )	);
		blackComponentMap.insert(std::make_pair(AQLString(DG) + AQLString(RHO)	+ AQLString(CALL), new MMDGrhoCall()	)	);
		blackComponentMap.insert(std::make_pair(AQLString(DG) + AQLString(PHI)	+ AQLString(CALL), new MMDGphiCall()	)	);
		blackComponentMap.insert(std::make_pair(AQLString(DG) + AQLString(PREM)	+ AQLString(PUT),	new MMDGpremPut()	)	);
		blackComponentMap.insert(std::make_pair(AQLString(DG) + AQLString(DELTA)	+ AQLString(PUT),	new MMDGdeltaPut()	)	);
		blackComponentMap.insert(std::make_pair(AQLString(DG) + AQLString(GAMMA)	+ AQLString(PUT),	new MMDGgammaPut()	)	);
		blackComponentMap.insert(std::make_pair(AQLString(DG) + AQLString(VEGA)	+ AQLString(PUT),	new MMDGvegaPut()	)	);
		blackComponentMap.insert(std::make_pair(AQLString(DG) + AQLString(THETA)	+ AQLString(PUT),	new MMDGthetaPut()	)	);
		blackComponentMap.insert(std::make_pair(AQLString(DG) + AQLString(RHO)	+ AQLString(PUT),	new MMDGrhoPut()	)	);
		blackComponentMap.insert(std::make_pair(AQLString(DG) + AQLString(PHI)	+ AQLString(PUT),	new MMDGphiPut()	)	);
		//MMSB
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(PREM)	+ AQLString(CALL) + AQLString(SBDOWN)	+ AQLString(SBIN)  , new MMSBpremDIC() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(PREM)	+ AQLString(PUT)  + AQLString(SBDOWN)	+ AQLString(SBIN)  , new MMSBpremDIP() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(PREM)	+ AQLString(CALL) + AQLString(SBDOWN)	+ AQLString(SBOUT) , new MMSBpremDOC() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(PREM)	+ AQLString(PUT)  + AQLString(SBDOWN)	+ AQLString(SBOUT) , new MMSBpremDOP() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(PREM)	+ AQLString(CALL) + AQLString(SBUP)	+ AQLString(SBIN)  , new MMSBpremUIC() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(PREM)	+ AQLString(PUT)  + AQLString(SBUP)	+ AQLString(SBIN)  , new MMSBpremUIP() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(PREM)	+ AQLString(CALL) + AQLString(SBUP)	+ AQLString(SBOUT) , new MMSBpremUOC() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(PREM)	+ AQLString(PUT)  + AQLString(SBUP)	+ AQLString(SBOUT) , new MMSBpremUOP() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(DELTA)	+ AQLString(CALL) + AQLString(SBDOWN)	+ AQLString(SBIN)  , new MMSBdeltaDIC() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(DELTA)	+ AQLString(PUT)  + AQLString(SBDOWN)	+ AQLString(SBIN)  , new MMSBdeltaDIP() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(DELTA)	+ AQLString(CALL) + AQLString(SBDOWN)	+ AQLString(SBOUT) , new MMSBdeltaDOC() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(DELTA)	+ AQLString(PUT)  + AQLString(SBDOWN)	+ AQLString(SBOUT) , new MMSBdeltaDOP() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(DELTA)	+ AQLString(CALL) + AQLString(SBUP)	+ AQLString(SBIN)  , new MMSBdeltaUIC() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(DELTA)	+ AQLString(PUT)  + AQLString(SBUP)	+ AQLString(SBIN)  , new MMSBdeltaUIP() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(DELTA)	+ AQLString(CALL) + AQLString(SBUP)	+ AQLString(SBOUT) , new MMSBdeltaUOC() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(DELTA)	+ AQLString(PUT)  + AQLString(SBUP)	+ AQLString(SBOUT) , new MMSBdeltaUOP() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(GAMMA)	+ AQLString(CALL) + AQLString(SBDOWN)	+ AQLString(SBIN)  , new MMSBgammaDIC() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(GAMMA)	+ AQLString(PUT)  + AQLString(SBDOWN)	+ AQLString(SBIN)  , new MMSBgammaDIP() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(GAMMA)	+ AQLString(CALL) + AQLString(SBDOWN)	+ AQLString(SBOUT) , new MMSBgammaDOC() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(GAMMA)	+ AQLString(PUT)  + AQLString(SBDOWN)	+ AQLString(SBOUT) , new MMSBgammaDOP() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(GAMMA)	+ AQLString(CALL) + AQLString(SBUP)	+ AQLString(SBIN)  , new MMSBgammaUIC() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(GAMMA)	+ AQLString(PUT)  + AQLString(SBUP)	+ AQLString(SBIN)  , new MMSBgammaUIP() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(GAMMA)	+ AQLString(CALL) + AQLString(SBUP)	+ AQLString(SBOUT) , new MMSBgammaUOC() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(GAMMA)	+ AQLString(PUT)  + AQLString(SBUP)	+ AQLString(SBOUT) , new MMSBgammaUOP() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(VEGA)	+ AQLString(CALL) + AQLString(SBDOWN)	+ AQLString(SBIN)  , new MMSBvegaDIC() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(VEGA)	+ AQLString(PUT)  + AQLString(SBDOWN)	+ AQLString(SBIN)  , new MMSBvegaDIP() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(VEGA)	+ AQLString(CALL) + AQLString(SBDOWN)	+ AQLString(SBOUT) , new MMSBvegaDOC() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(VEGA)	+ AQLString(PUT)  + AQLString(SBDOWN)	+ AQLString(SBOUT) , new MMSBvegaDOP() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(VEGA)	+ AQLString(CALL) + AQLString(SBUP)	+ AQLString(SBIN)  , new MMSBvegaUIC() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(VEGA)	+ AQLString(PUT)  + AQLString(SBUP)	+ AQLString(SBIN)  , new MMSBvegaUIP() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(VEGA)	+ AQLString(CALL) + AQLString(SBUP)	+ AQLString(SBOUT) , new MMSBvegaUOC() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(VEGA)	+ AQLString(PUT)  + AQLString(SBUP)	+ AQLString(SBOUT) , new MMSBvegaUOP() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(THETA)	+ AQLString(CALL) + AQLString(SBDOWN)	+ AQLString(SBIN)  , new MMSBthetaDIC() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(THETA)	+ AQLString(PUT)  + AQLString(SBDOWN)	+ AQLString(SBIN)  , new MMSBthetaDIP() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(THETA)	+ AQLString(CALL) + AQLString(SBDOWN)	+ AQLString(SBOUT) , new MMSBthetaDOC() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(THETA)	+ AQLString(PUT)  + AQLString(SBDOWN)	+ AQLString(SBOUT) , new MMSBthetaDOP() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(THETA)	+ AQLString(CALL) + AQLString(SBUP)	+ AQLString(SBIN)  , new MMSBthetaUIC() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(THETA)	+ AQLString(PUT)  + AQLString(SBUP)	+ AQLString(SBIN)  , new MMSBthetaUIP() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(THETA)	+ AQLString(CALL) + AQLString(SBUP)	+ AQLString(SBOUT) , new MMSBthetaUOC() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(THETA)	+ AQLString(PUT)  + AQLString(SBUP)	+ AQLString(SBOUT) , new MMSBthetaUOP() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(RHO)	+ AQLString(CALL) + AQLString(SBDOWN)	+ AQLString(SBIN)  , new MMSBrhoDIC() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(RHO)	+ AQLString(PUT)  + AQLString(SBDOWN)	+ AQLString(SBIN)  , new MMSBrhoDIP() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(RHO)	+ AQLString(CALL) + AQLString(SBDOWN)	+ AQLString(SBOUT) , new MMSBrhoDOC() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(RHO)	+ AQLString(PUT)  + AQLString(SBDOWN)	+ AQLString(SBOUT) , new MMSBrhoDOP() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(RHO)	+ AQLString(CALL) + AQLString(SBUP)	+ AQLString(SBIN)  , new MMSBrhoUIC() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(RHO)	+ AQLString(PUT)  + AQLString(SBUP)	+ AQLString(SBIN)  , new MMSBrhoUIP() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(RHO)	+ AQLString(CALL) + AQLString(SBUP)	+ AQLString(SBOUT) , new MMSBrhoUOC() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(RHO)	+ AQLString(PUT)  + AQLString(SBUP)	+ AQLString(SBOUT) , new MMSBrhoUOP() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(PHI)	+ AQLString(CALL) + AQLString(SBDOWN)	+ AQLString(SBIN)  , new MMSBphiDIC() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(PHI)	+ AQLString(PUT)  + AQLString(SBDOWN)	+ AQLString(SBIN)  , new MMSBphiDIP() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(PHI)	+ AQLString(CALL) + AQLString(SBDOWN)	+ AQLString(SBOUT) , new MMSBphiDOC() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(PHI)	+ AQLString(PUT)  + AQLString(SBDOWN)	+ AQLString(SBOUT) , new MMSBphiDOP() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(PHI)	+ AQLString(CALL) + AQLString(SBUP)	+ AQLString(SBIN)  , new MMSBphiUIC() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(PHI)	+ AQLString(PUT)  + AQLString(SBUP)	+ AQLString(SBIN)  , new MMSBphiUIP() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(PHI)	+ AQLString(CALL) + AQLString(SBUP)	+ AQLString(SBOUT) , new MMSBphiUOC() ));
		blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(PHI)	+ AQLString(PUT)  + AQLString(SBUP)	+ AQLString(SBOUT) , new MMSBphiUOP() ));

		//MMDB
		blackComponentMap.insert(std::make_pair(AQLString(DB) + AQLString(PREM)	+ AQLString(CALL) + AQLString(DBIN)	+ AQLString(DBIN)  , new MMDBpremKIC() ));
		blackComponentMap.insert(std::make_pair(AQLString(DB) + AQLString(PREM)	+ AQLString(CALL) + AQLString(DBOUT)	+ AQLString(DBOUT) , new MMDBpremKOC()	));
		blackComponentMap.insert(std::make_pair(AQLString(DB) + AQLString(PREM)	+ AQLString(PUT)  + AQLString(DBIN)	+ AQLString(DBIN)  , new MMDBpremKIP() ));
		blackComponentMap.insert(std::make_pair(AQLString(DB) + AQLString(PREM)	+ AQLString(PUT)  + AQLString(DBOUT)	+ AQLString(DBOUT) , new MMDBpremKOP()	));
		blackComponentMap.insert(std::make_pair(AQLString(DB) + AQLString(REBATE)	+ AQLString(RHIGH),	new MMDBpremRH()				));
		blackComponentMap.insert(std::make_pair(AQLString(DB) + AQLString(REBATE)	+ AQLString(RLOW),		new MMDBpremRL()				));
		blackComponentMap.insert(std::make_pair(AQLString(DB) + AQLString(REBATE)	+ AQLString(RIN),		new MMDBpremRI()				));
		
		blackComponentMap.insert(std::make_pair(AQLString(FD) + AQLString(DELTA)	+ AQLString(CALL) + AQLString(SPOT)  , new MMFDdeltaCallSpot() ));
		blackComponentMap.insert(std::make_pair(AQLString(FD) + AQLString(DELTA)	+ AQLString(PUT) + AQLString(SPOT)  , new MMFDdeltaPutSpot() ));
		blackComponentMap.insert(std::make_pair(AQLString(FD) + AQLString(FIRSTDIFF)	+ AQLString(CALL) + AQLString(SPOT)  , new MMFD1stDiffCallSpot() ));
		blackComponentMap.insert(std::make_pair(AQLString(FD) + AQLString(FIRSTDIFF)	+ AQLString(PUT) + AQLString(SPOT)  , new MMFD1stDiffPutSpot() ));
		blackComponentMap.insert(std::make_pair(AQLString(FD) + AQLString(DELTA)	+ AQLString(CALL) + AQLString(FORWARD)  , new MMFDdeltaCallFwd() ));
		blackComponentMap.insert(std::make_pair(AQLString(FD) + AQLString(DELTA)	+ AQLString(PUT) + AQLString(FORWARD)  , new MMFDdeltaPutFwd() ));
		blackComponentMap.insert(std::make_pair(AQLString(FD) + AQLString(FIRSTDIFF)	+ AQLString(CALL) + AQLString(FORWARD)  , new MMFD1stDiffCallFwd() ));
		blackComponentMap.insert(std::make_pair(AQLString(FD) + AQLString(FIRSTDIFF)	+ AQLString(PUT) + AQLString(FORWARD)  , new MMFD1stDiffPutFwd() ));
		blackComponentMap.insert(std::make_pair(AQLString(FD) + AQLString(SECONDDIFF)	+ AQLString(CALL) + AQLString(SPOT)  , new MMFD2ndDiffCallSpot() ));
		blackComponentMap.insert(std::make_pair(AQLString(FD) + AQLString(SECONDDIFF)	+ AQLString(CALL) + AQLString(FORWARD)  , new MMFD2ndDiffCallFwd() ));		
		//MMAFF
		blackComponentMap.insert(std::make_pair(AQLString(AFF)	+ AQLString(PREM) + AQLString(CALL),	new MMAFFpremCall()	)	);
		blackComponentMap.insert(std::make_pair(AQLString(AFF)	+ AQLString(PREM) + AQLString(PUT),	new MMAFFpremPut()	)	);
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
	
	//MMZeroForError
	blackComponentMap.insert(std::make_pair(AQLString("ERROR"),	new MMZeroForError()));
	//LABlackPayOff
	blackComponentMap.insert(std::make_pair(AQLString(BKPAYOFF)	+ AQLString(PREM) + AQLString(CALL),	new AQLBlackPayOffpremCall())	);
	blackComponentMap.insert(std::make_pair(AQLString(BKPAYOFF)	+ AQLString(PREM) + AQLString(PUT),	new AQLBlackPayOffpremPut()	)	);
	
	
	
	
	//LABlack
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
	//LAGreek
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
	//MMCapFloor
	blackComponentMap.insert(std::make_pair(AQLString(CF) + AQLString(PREM) + AQLString(CALL), new MMCFpremCall()));
	blackComponentMap.insert(std::make_pair(AQLString(CF) + AQLString(VEGA) + AQLString(CALL), new MMCFvegaCall()));
	blackComponentMap.insert(std::make_pair(AQLString(CF) + AQLString(PREM) + AQLString(PUT), new MMCFpremPut()));
	blackComponentMap.insert(std::make_pair(AQLString(CF) + AQLString(VEGA) + AQLString(PUT), new MMCFvegaPut()));
	//MMDG
	blackComponentMap.insert(std::make_pair(AQLString(DG) + AQLString(PREM)	+ AQLString(CALL), new MMDGpremCall()	)	);
	blackComponentMap.insert(std::make_pair(AQLString(DG) + AQLString(DELTA)	+ AQLString(CALL), new MMDGdeltaCall() )	);
	blackComponentMap.insert(std::make_pair(AQLString(DG) + AQLString(GAMMA)	+ AQLString(CALL), new MMDGgammaCall() )	);
	blackComponentMap.insert(std::make_pair(AQLString(DG) + AQLString(VEGA)	+ AQLString(CALL), new MMDGvegaCall()	)	);
	blackComponentMap.insert(std::make_pair(AQLString(DG) + AQLString(THETA)	+ AQLString(CALL), new MMDGthetaCall() )	);
	blackComponentMap.insert(std::make_pair(AQLString(DG) + AQLString(RHO)	+ AQLString(CALL), new MMDGrhoCall()	)	);
	blackComponentMap.insert(std::make_pair(AQLString(DG) + AQLString(PHI)	+ AQLString(CALL), new MMDGphiCall()	)	);
	blackComponentMap.insert(std::make_pair(AQLString(DG) + AQLString(PREM)	+ AQLString(PUT),	new MMDGpremPut()	)	);
	blackComponentMap.insert(std::make_pair(AQLString(DG) + AQLString(DELTA)	+ AQLString(PUT),	new MMDGdeltaPut()	)	);
	blackComponentMap.insert(std::make_pair(AQLString(DG) + AQLString(GAMMA)	+ AQLString(PUT),	new MMDGgammaPut()	)	);
	blackComponentMap.insert(std::make_pair(AQLString(DG) + AQLString(VEGA)	+ AQLString(PUT),	new MMDGvegaPut()	)	);
	blackComponentMap.insert(std::make_pair(AQLString(DG) + AQLString(THETA)	+ AQLString(PUT),	new MMDGthetaPut()	)	);
	blackComponentMap.insert(std::make_pair(AQLString(DG) + AQLString(RHO)	+ AQLString(PUT),	new MMDGrhoPut()	)	);
	blackComponentMap.insert(std::make_pair(AQLString(DG) + AQLString(PHI)	+ AQLString(PUT),	new MMDGphiPut()	)	);
	//hishida vannavolga
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(PROB)	+ AQLString(SBUP) + AQLString(NOTOUCH), new MMSBprobUNT() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(PROB)	+ AQLString(SBDOWN) + AQLString(NOTOUCH), new MMSBprobDNT() ));
	//hishida vannavolga
	//MMSB
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(PREM)	+ AQLString(CALL) + AQLString(SBDOWN)	+ AQLString(SBIN)  , new MMSBpremDIC() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(PREM)	+ AQLString(PUT)  + AQLString(SBDOWN)	+ AQLString(SBIN)  , new MMSBpremDIP() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(PREM)	+ AQLString(CALL) + AQLString(SBDOWN)	+ AQLString(SBOUT) , new MMSBpremDOC() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(PREM)	+ AQLString(PUT)  + AQLString(SBDOWN)	+ AQLString(SBOUT) , new MMSBpremDOP() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(PREM)	+ AQLString(CALL) + AQLString(SBUP)	+ AQLString(SBIN)  , new MMSBpremUIC() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(PREM)	+ AQLString(PUT)  + AQLString(SBUP)	+ AQLString(SBIN)  , new MMSBpremUIP() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(PREM)	+ AQLString(CALL) + AQLString(SBUP)	+ AQLString(SBOUT) , new MMSBpremUOC() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(PREM)	+ AQLString(PUT)  + AQLString(SBUP)	+ AQLString(SBOUT) , new MMSBpremUOP() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(DELTA)	+ AQLString(CALL) + AQLString(SBDOWN)	+ AQLString(SBIN)  , new MMSBdeltaDIC() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(DELTA)	+ AQLString(PUT)  + AQLString(SBDOWN)	+ AQLString(SBIN)  , new MMSBdeltaDIP() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(DELTA)	+ AQLString(CALL) + AQLString(SBDOWN)	+ AQLString(SBOUT) , new MMSBdeltaDOC() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(DELTA)	+ AQLString(PUT)  + AQLString(SBDOWN)	+ AQLString(SBOUT) , new MMSBdeltaDOP() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(DELTA)	+ AQLString(CALL) + AQLString(SBUP)	+ AQLString(SBIN)  , new MMSBdeltaUIC() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(DELTA)	+ AQLString(PUT)  + AQLString(SBUP)	+ AQLString(SBIN)  , new MMSBdeltaUIP() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(DELTA)	+ AQLString(CALL) + AQLString(SBUP)	+ AQLString(SBOUT) , new MMSBdeltaUOC() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(DELTA)	+ AQLString(PUT)  + AQLString(SBUP)	+ AQLString(SBOUT) , new MMSBdeltaUOP() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(GAMMA)	+ AQLString(CALL) + AQLString(SBDOWN)	+ AQLString(SBIN)  , new MMSBgammaDIC() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(GAMMA)	+ AQLString(PUT)  + AQLString(SBDOWN)	+ AQLString(SBIN)  , new MMSBgammaDIP() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(GAMMA)	+ AQLString(CALL) + AQLString(SBDOWN)	+ AQLString(SBOUT) , new MMSBgammaDOC() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(GAMMA)	+ AQLString(PUT)  + AQLString(SBDOWN)	+ AQLString(SBOUT) , new MMSBgammaDOP() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(GAMMA)	+ AQLString(CALL) + AQLString(SBUP)	+ AQLString(SBIN)  , new MMSBgammaUIC() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(GAMMA)	+ AQLString(PUT)  + AQLString(SBUP)	+ AQLString(SBIN)  , new MMSBgammaUIP() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(GAMMA)	+ AQLString(CALL) + AQLString(SBUP)	+ AQLString(SBOUT) , new MMSBgammaUOC() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(GAMMA)	+ AQLString(PUT)  + AQLString(SBUP)	+ AQLString(SBOUT) , new MMSBgammaUOP() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(VEGA)	+ AQLString(CALL) + AQLString(SBDOWN)	+ AQLString(SBIN)  , new MMSBvegaDIC() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(VEGA)	+ AQLString(PUT)  + AQLString(SBDOWN)	+ AQLString(SBIN)  , new MMSBvegaDIP() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(VEGA)	+ AQLString(CALL) + AQLString(SBDOWN)	+ AQLString(SBOUT) , new MMSBvegaDOC() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(VEGA)	+ AQLString(PUT)  + AQLString(SBDOWN)	+ AQLString(SBOUT) , new MMSBvegaDOP() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(VEGA)	+ AQLString(CALL) + AQLString(SBUP)	+ AQLString(SBIN)  , new MMSBvegaUIC() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(VEGA)	+ AQLString(PUT)  + AQLString(SBUP)	+ AQLString(SBIN)  , new MMSBvegaUIP() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(VEGA)	+ AQLString(CALL) + AQLString(SBUP)	+ AQLString(SBOUT) , new MMSBvegaUOC() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(VEGA)	+ AQLString(PUT)  + AQLString(SBUP)	+ AQLString(SBOUT) , new MMSBvegaUOP() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(THETA)	+ AQLString(CALL) + AQLString(SBDOWN)	+ AQLString(SBIN)  , new MMSBthetaDIC() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(THETA)	+ AQLString(PUT)  + AQLString(SBDOWN)	+ AQLString(SBIN)  , new MMSBthetaDIP() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(THETA)	+ AQLString(CALL) + AQLString(SBDOWN)	+ AQLString(SBOUT) , new MMSBthetaDOC() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(THETA)	+ AQLString(PUT)  + AQLString(SBDOWN)	+ AQLString(SBOUT) , new MMSBthetaDOP() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(THETA)	+ AQLString(CALL) + AQLString(SBUP)	+ AQLString(SBIN)  , new MMSBthetaUIC() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(THETA)	+ AQLString(PUT)  + AQLString(SBUP)	+ AQLString(SBIN)  , new MMSBthetaUIP() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(THETA)	+ AQLString(CALL) + AQLString(SBUP)	+ AQLString(SBOUT) , new MMSBthetaUOC() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(THETA)	+ AQLString(PUT)  + AQLString(SBUP)	+ AQLString(SBOUT) , new MMSBthetaUOP() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(RHO)	+ AQLString(CALL) + AQLString(SBDOWN)	+ AQLString(SBIN)  , new MMSBrhoDIC() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(RHO)	+ AQLString(PUT)  + AQLString(SBDOWN)	+ AQLString(SBIN)  , new MMSBrhoDIP() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(RHO)	+ AQLString(CALL) + AQLString(SBDOWN)	+ AQLString(SBOUT) , new MMSBrhoDOC() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(RHO)	+ AQLString(PUT)  + AQLString(SBDOWN)	+ AQLString(SBOUT) , new MMSBrhoDOP() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(RHO)	+ AQLString(CALL) + AQLString(SBUP)	+ AQLString(SBIN)  , new MMSBrhoUIC() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(RHO)	+ AQLString(PUT)  + AQLString(SBUP)	+ AQLString(SBIN)  , new MMSBrhoUIP() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(RHO)	+ AQLString(CALL) + AQLString(SBUP)	+ AQLString(SBOUT) , new MMSBrhoUOC() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(RHO)	+ AQLString(PUT)  + AQLString(SBUP)	+ AQLString(SBOUT) , new MMSBrhoUOP() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(PHI)	+ AQLString(CALL) + AQLString(SBDOWN)	+ AQLString(SBIN)  , new MMSBphiDIC() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(PHI)	+ AQLString(PUT)  + AQLString(SBDOWN)	+ AQLString(SBIN)  , new MMSBphiDIP() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(PHI)	+ AQLString(CALL) + AQLString(SBDOWN)	+ AQLString(SBOUT) , new MMSBphiDOC() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(PHI)	+ AQLString(PUT)  + AQLString(SBDOWN)	+ AQLString(SBOUT) , new MMSBphiDOP() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(PHI)	+ AQLString(CALL) + AQLString(SBUP)	+ AQLString(SBIN)  , new MMSBphiUIC() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(PHI)	+ AQLString(PUT)  + AQLString(SBUP)	+ AQLString(SBIN)  , new MMSBphiUIP() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(PHI)	+ AQLString(CALL) + AQLString(SBUP)	+ AQLString(SBOUT) , new MMSBphiUOC() ));
	blackComponentMap.insert(std::make_pair(AQLString(SB) + AQLString(PHI)	+ AQLString(PUT)  + AQLString(SBUP)	+ AQLString(SBOUT) , new MMSBphiUOP() ));

	//MMDB
	blackComponentMap.insert(std::make_pair(AQLString(DB) + AQLString(PREM)	+ AQLString(CALL) + AQLString(DBIN)	+ AQLString(DBIN)  , new MMDBpremKIC() ));
	blackComponentMap.insert(std::make_pair(AQLString(DB) + AQLString(PREM)	+ AQLString(CALL) + AQLString(DBOUT)	+ AQLString(DBOUT) , new MMDBpremKOC()	));
	blackComponentMap.insert(std::make_pair(AQLString(DB) + AQLString(PREM)	+ AQLString(PUT)  + AQLString(DBIN)	+ AQLString(DBIN)  , new MMDBpremKIP() ));
	blackComponentMap.insert(std::make_pair(AQLString(DB) + AQLString(PREM)	+ AQLString(PUT)  + AQLString(DBOUT)	+ AQLString(DBOUT) , new MMDBpremKOP()	));
	blackComponentMap.insert(std::make_pair(AQLString(DB) + AQLString(REBATE)	+ AQLString(RHIGH),	new MMDBpremRH()				));
	blackComponentMap.insert(std::make_pair(AQLString(DB) + AQLString(REBATE)	+ AQLString(RLOW),		new MMDBpremRL()				));
	blackComponentMap.insert(std::make_pair(AQLString(DB) + AQLString(REBATE)	+ AQLString(RIN),		new MMDBpremRI()				));
	
	blackComponentMap.insert(std::make_pair(AQLString(FD) + AQLString(DELTA)	+ AQLString(CALL) + AQLString(SPOT)  , new MMFDdeltaCallSpot() ));
	blackComponentMap.insert(std::make_pair(AQLString(FD) + AQLString(DELTA)	+ AQLString(PUT) + AQLString(SPOT)  , new MMFDdeltaPutSpot() ));
	blackComponentMap.insert(std::make_pair(AQLString(FD) + AQLString(FIRSTDIFF)	+ AQLString(CALL) + AQLString(SPOT)  , new MMFD1stDiffCallSpot() ));
	blackComponentMap.insert(std::make_pair(AQLString(FD) + AQLString(FIRSTDIFF)	+ AQLString(PUT) + AQLString(SPOT)  , new MMFD1stDiffPutSpot() ));
	blackComponentMap.insert(std::make_pair(AQLString(FD) + AQLString(DELTA)	+ AQLString(CALL) + AQLString(FORWARD)  , new MMFDdeltaCallFwd() ));
	blackComponentMap.insert(std::make_pair(AQLString(FD) + AQLString(DELTA)	+ AQLString(PUT) + AQLString(FORWARD)  , new MMFDdeltaPutFwd() ));
	blackComponentMap.insert(std::make_pair(AQLString(FD) + AQLString(FIRSTDIFF)	+ AQLString(CALL) + AQLString(FORWARD)  , new MMFD1stDiffCallFwd() ));
	blackComponentMap.insert(std::make_pair(AQLString(FD) + AQLString(FIRSTDIFF)	+ AQLString(PUT) + AQLString(FORWARD)  , new MMFD1stDiffPutFwd() ));
	blackComponentMap.insert(std::make_pair(AQLString(FD) + AQLString(SECONDDIFF)	+ AQLString(CALL) + AQLString(SPOT)  , new MMFD2ndDiffCallSpot() ));
	blackComponentMap.insert(std::make_pair(AQLString(FD) + AQLString(SECONDDIFF)	+ AQLString(CALL) + AQLString(FORWARD)  , new MMFD2ndDiffCallFwd() ));
	//MMAFF
	blackComponentMap.insert(std::make_pair(AQLString(AFF)	+ AQLString(PREM) + AQLString(CALL),	new MMAFFpremCall()	)	);
	blackComponentMap.insert(std::make_pair(AQLString(AFF)	+ AQLString(PREM) + AQLString(PUT),	new MMAFFpremPut()	)	);


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
