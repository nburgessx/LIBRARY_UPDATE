
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

#include "LAMathBaseFuncUtility.h"
#include "LAObject.h"
#include "LADataBasics.h"
#include "LADataVector.h"
#include "LACoreTemplateType.h"
#include "LAMathDefine.h"
#include "LAPriceDataCalendar.h"
#include "LAPriceDataSlidingRule.h"
#include "LAPriceDataInterpolation.h"
#include "LABasic.h"
#include "LAAlgorithm.h"
#include "LAObjectHolder.h"
#include "LADataInstance.h"
#include "LAObjectMaster.h"
#include "LAPriceDataManager.h"
#include "LALinearInterpolation.h"
#include "LASplineInterpolation.h"
#include "LAStepInterpolation.h"
#include "LAConstrainedSplineInterpolation.h"
#include "LAMonotoneConvexInterpolation.h"
#include "LAParabolicInterpolation.h"
#include "LALinearSplineInterpolation.h"
#include "LALinearMonotoneSplineInterpolation.h"
#include "LAMonotoneSplineInterpolation.h"
#include "LAMathYieldCurvePro.h"

#include "LACoreComponentManager.h"
#include <cmath>
#include <float.h>

using namespace std;
//static-mem
std::map<LAString, LAString> LACoreComponentManager::interpolationMap;
std::map<LAString, LAString> LACoreComponentManager::dayCountMap;
std::map<LAString, LAString> LACoreComponentManager::basisTypeMap;
std::map<LAString, LAString> LACoreComponentManager::basisIndexMap;
std::map<LAString, LABlackScholesBase* > LACoreComponentManager::blackComponentMap ;
std::map<LAString, AnalyticParam* > LACoreComponentManager::blackParamComponentMap;

//static-method////////

std::map<LAString, LAString>& 
LACoreComponentManager::getInterpolationMap()
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

std::map<LAString, LAString>&
LACoreComponentManager::getDayCountMap()
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

std::map<LAString, LAString>&
LACoreComponentManager::getBasisTypeMap()
{
	if(basisTypeMap.empty())
	{
		basisTypeMap.insert(std::make_pair(XCCYBASIS,	XCCYBASIS));
	}
	return basisTypeMap;
}

std::map<LAString, LAString>&
LACoreComponentManager::getBasisIndexMap()
{
	if(basisIndexMap.empty())
	{
		basisIndexMap.insert(std::make_pair(LIBOR,	LIBOR));
	}
	return basisIndexMap;
}

std::map<LAString, LABlackScholesBase*>&
LACoreComponentManager::getBlackComponentMap()
{
	if(blackComponentMap.empty())
	{	
		//MMZeroForError
		blackComponentMap.insert(std::make_pair(LAString("ERROR"),	new MMZeroForError()));
		//LABlack
		blackComponentMap.insert(std::make_pair(LAString(BK)	+ LAString(PREM) + LAString(CALL),	new LABlackpremCall()	)	);
		blackComponentMap.insert(std::make_pair(LAString(BK)	+ LAString(PREM) + LAString(PUT),	new LABlackpremPut()	)	);
		blackComponentMap.insert(std::make_pair(LAString(BK)	+ LAString(DELTA) + LAString(CALL), new LABlackdeltaCall()	)	);
		blackComponentMap.insert(std::make_pair(LAString(BK)	+ LAString(GAMMA) + LAString(CALL), new LABlackgammaCall()	)	);
		blackComponentMap.insert(std::make_pair(LAString(BK)	+ LAString(VEGA)  + LAString(CALL), new LABlackvegaCall()	)	);
		blackComponentMap.insert(std::make_pair(LAString(BK)	+ LAString(THETA) + LAString(CALL), new LABlackthetaCall()	)	);
		blackComponentMap.insert(std::make_pair(LAString(BK)	+ LAString(DELTA) + LAString(PUT),  new LABlackdeltaPut()	)	);
		blackComponentMap.insert(std::make_pair(LAString(BK)	+ LAString(GAMMA) + LAString(PUT),  new LABlackgammaPut()	)	);
		blackComponentMap.insert(std::make_pair(LAString(BK)	+ LAString(VEGA)  + LAString(PUT),  new LABlackvegaPut()	)	);
		blackComponentMap.insert(std::make_pair(LAString(BK)	+ LAString(THETA) + LAString(PUT),  new LABlackthetaPut()	)	);
		//LABlackPayOff
		blackComponentMap.insert(std::make_pair(LAString(BKPAYOFF) + LAString(PREM) + LAString(CALL),	new LABlackPayOffpremCall()	)	);
		blackComponentMap.insert(std::make_pair(LAString(BKPAYOFF) + LAString(PREM) + LAString(PUT),	new LABlackPayOffpremPut()		)	);
		//LAGreek
		blackComponentMap.insert(std::make_pair(LAString(GK) + LAString(PREM)	+ LAString(CALL), new LAGreekpremCall()	)	);
		blackComponentMap.insert(std::make_pair(LAString(GK) + LAString(DELTA)	+ LAString(CALL), new LAGreekdeltaCall() )	);
		blackComponentMap.insert(std::make_pair(LAString(GK) + LAString(GAMMA)	+ LAString(CALL), new LAGreekgammaCall() )	);
		blackComponentMap.insert(std::make_pair(LAString(GK) + LAString(VEGA)	+ LAString(CALL), new LAGreekvegaCall()	)	);
		blackComponentMap.insert(std::make_pair(LAString(GK) + LAString(THETA)	+ LAString(CALL), new LAGreekthetaCall() )	);
		blackComponentMap.insert(std::make_pair(LAString(GK) + LAString(RHO)	+ LAString(CALL), new LAGreekrhoCall()	)	);
		blackComponentMap.insert(std::make_pair(LAString(GK) + LAString(PHI)	+ LAString(CALL), new LAGreekphiCall()	)	);
		blackComponentMap.insert(std::make_pair(LAString(GK) + LAString(VANNA)	+ LAString(CALL), new LAGreekvannaCall()	)	);
		blackComponentMap.insert(std::make_pair(LAString(GK) + LAString(VOLGA)	+ LAString(CALL), new LAGreekvolgaCall()	)	);
		blackComponentMap.insert(std::make_pair(LAString(GK) + LAString(PREM)	+ LAString(PUT),	new LAGreekpremPut()	)	);
		blackComponentMap.insert(std::make_pair(LAString(GK) + LAString(DELTA)	+ LAString(PUT),	new LAGreekdeltaPut()	)	);
		blackComponentMap.insert(std::make_pair(LAString(GK) + LAString(GAMMA)	+ LAString(PUT),	new LAGreekgammaPut()	)	);
		blackComponentMap.insert(std::make_pair(LAString(GK) + LAString(VEGA)	+ LAString(PUT),	new LAGreekvegaPut()	)	);
		blackComponentMap.insert(std::make_pair(LAString(GK) + LAString(THETA)	+ LAString(PUT),	new LAGreekthetaPut()	)	);
		blackComponentMap.insert(std::make_pair(LAString(GK) + LAString(RHO)	+ LAString(PUT),	new LAGreekrhoPut()	)	);
		blackComponentMap.insert(std::make_pair(LAString(GK) + LAString(PHI)	+ LAString(PUT),	new LAGreekphiPut()	)	);
		blackComponentMap.insert(std::make_pair(LAString(GK) + LAString(VANNA)	+ LAString(PUT), new LAGreekvannaPut()	)	);
		blackComponentMap.insert(std::make_pair(LAString(GK) + LAString(VOLGA)	+ LAString(PUT), new LAGreekvolgaPut()	)	);
        //MMCapFloor
		blackComponentMap.insert(std::make_pair(LAString(CF) + LAString(PREM)	+ LAString(CALL), new MMCFpremCall()	)	);
		blackComponentMap.insert(std::make_pair(LAString(CF) + LAString(VEGA)	+ LAString(CALL), new MMCFvegaCall() )	);
		blackComponentMap.insert(std::make_pair(LAString(CF) + LAString(PREM)	+ LAString(PUT), new MMCFpremPut()	)	);
		blackComponentMap.insert(std::make_pair(LAString(CF) + LAString(VEGA)	+ LAString(PUT), new MMCFvegaPut() )	);
		//MMDG
		blackComponentMap.insert(std::make_pair(LAString(DG) + LAString(PREM)	+ LAString(CALL), new MMDGpremCall()	)	);
		blackComponentMap.insert(std::make_pair(LAString(DG) + LAString(DELTA)	+ LAString(CALL), new MMDGdeltaCall() )	);
		blackComponentMap.insert(std::make_pair(LAString(DG) + LAString(GAMMA)	+ LAString(CALL), new MMDGgammaCall() )	);
		blackComponentMap.insert(std::make_pair(LAString(DG) + LAString(VEGA)	+ LAString(CALL), new MMDGvegaCall()	)	);
		blackComponentMap.insert(std::make_pair(LAString(DG) + LAString(THETA)	+ LAString(CALL), new MMDGthetaCall() )	);
		blackComponentMap.insert(std::make_pair(LAString(DG) + LAString(RHO)	+ LAString(CALL), new MMDGrhoCall()	)	);
		blackComponentMap.insert(std::make_pair(LAString(DG) + LAString(PHI)	+ LAString(CALL), new MMDGphiCall()	)	);
		blackComponentMap.insert(std::make_pair(LAString(DG) + LAString(PREM)	+ LAString(PUT),	new MMDGpremPut()	)	);
		blackComponentMap.insert(std::make_pair(LAString(DG) + LAString(DELTA)	+ LAString(PUT),	new MMDGdeltaPut()	)	);
		blackComponentMap.insert(std::make_pair(LAString(DG) + LAString(GAMMA)	+ LAString(PUT),	new MMDGgammaPut()	)	);
		blackComponentMap.insert(std::make_pair(LAString(DG) + LAString(VEGA)	+ LAString(PUT),	new MMDGvegaPut()	)	);
		blackComponentMap.insert(std::make_pair(LAString(DG) + LAString(THETA)	+ LAString(PUT),	new MMDGthetaPut()	)	);
		blackComponentMap.insert(std::make_pair(LAString(DG) + LAString(RHO)	+ LAString(PUT),	new MMDGrhoPut()	)	);
		blackComponentMap.insert(std::make_pair(LAString(DG) + LAString(PHI)	+ LAString(PUT),	new MMDGphiPut()	)	);
		//MMSB
		blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(PREM)	+ LAString(CALL) + LAString(SBDOWN)	+ LAString(SBIN)  , new MMSBpremDIC() ));
		blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(PREM)	+ LAString(PUT)  + LAString(SBDOWN)	+ LAString(SBIN)  , new MMSBpremDIP() ));
		blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(PREM)	+ LAString(CALL) + LAString(SBDOWN)	+ LAString(SBOUT) , new MMSBpremDOC() ));
		blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(PREM)	+ LAString(PUT)  + LAString(SBDOWN)	+ LAString(SBOUT) , new MMSBpremDOP() ));
		blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(PREM)	+ LAString(CALL) + LAString(SBUP)	+ LAString(SBIN)  , new MMSBpremUIC() ));
		blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(PREM)	+ LAString(PUT)  + LAString(SBUP)	+ LAString(SBIN)  , new MMSBpremUIP() ));
		blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(PREM)	+ LAString(CALL) + LAString(SBUP)	+ LAString(SBOUT) , new MMSBpremUOC() ));
		blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(PREM)	+ LAString(PUT)  + LAString(SBUP)	+ LAString(SBOUT) , new MMSBpremUOP() ));
		blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(DELTA)	+ LAString(CALL) + LAString(SBDOWN)	+ LAString(SBIN)  , new MMSBdeltaDIC() ));
		blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(DELTA)	+ LAString(PUT)  + LAString(SBDOWN)	+ LAString(SBIN)  , new MMSBdeltaDIP() ));
		blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(DELTA)	+ LAString(CALL) + LAString(SBDOWN)	+ LAString(SBOUT) , new MMSBdeltaDOC() ));
		blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(DELTA)	+ LAString(PUT)  + LAString(SBDOWN)	+ LAString(SBOUT) , new MMSBdeltaDOP() ));
		blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(DELTA)	+ LAString(CALL) + LAString(SBUP)	+ LAString(SBIN)  , new MMSBdeltaUIC() ));
		blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(DELTA)	+ LAString(PUT)  + LAString(SBUP)	+ LAString(SBIN)  , new MMSBdeltaUIP() ));
		blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(DELTA)	+ LAString(CALL) + LAString(SBUP)	+ LAString(SBOUT) , new MMSBdeltaUOC() ));
		blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(DELTA)	+ LAString(PUT)  + LAString(SBUP)	+ LAString(SBOUT) , new MMSBdeltaUOP() ));
		blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(GAMMA)	+ LAString(CALL) + LAString(SBDOWN)	+ LAString(SBIN)  , new MMSBgammaDIC() ));
		blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(GAMMA)	+ LAString(PUT)  + LAString(SBDOWN)	+ LAString(SBIN)  , new MMSBgammaDIP() ));
		blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(GAMMA)	+ LAString(CALL) + LAString(SBDOWN)	+ LAString(SBOUT) , new MMSBgammaDOC() ));
		blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(GAMMA)	+ LAString(PUT)  + LAString(SBDOWN)	+ LAString(SBOUT) , new MMSBgammaDOP() ));
		blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(GAMMA)	+ LAString(CALL) + LAString(SBUP)	+ LAString(SBIN)  , new MMSBgammaUIC() ));
		blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(GAMMA)	+ LAString(PUT)  + LAString(SBUP)	+ LAString(SBIN)  , new MMSBgammaUIP() ));
		blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(GAMMA)	+ LAString(CALL) + LAString(SBUP)	+ LAString(SBOUT) , new MMSBgammaUOC() ));
		blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(GAMMA)	+ LAString(PUT)  + LAString(SBUP)	+ LAString(SBOUT) , new MMSBgammaUOP() ));
		blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(VEGA)	+ LAString(CALL) + LAString(SBDOWN)	+ LAString(SBIN)  , new MMSBvegaDIC() ));
		blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(VEGA)	+ LAString(PUT)  + LAString(SBDOWN)	+ LAString(SBIN)  , new MMSBvegaDIP() ));
		blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(VEGA)	+ LAString(CALL) + LAString(SBDOWN)	+ LAString(SBOUT) , new MMSBvegaDOC() ));
		blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(VEGA)	+ LAString(PUT)  + LAString(SBDOWN)	+ LAString(SBOUT) , new MMSBvegaDOP() ));
		blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(VEGA)	+ LAString(CALL) + LAString(SBUP)	+ LAString(SBIN)  , new MMSBvegaUIC() ));
		blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(VEGA)	+ LAString(PUT)  + LAString(SBUP)	+ LAString(SBIN)  , new MMSBvegaUIP() ));
		blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(VEGA)	+ LAString(CALL) + LAString(SBUP)	+ LAString(SBOUT) , new MMSBvegaUOC() ));
		blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(VEGA)	+ LAString(PUT)  + LAString(SBUP)	+ LAString(SBOUT) , new MMSBvegaUOP() ));
		blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(THETA)	+ LAString(CALL) + LAString(SBDOWN)	+ LAString(SBIN)  , new MMSBthetaDIC() ));
		blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(THETA)	+ LAString(PUT)  + LAString(SBDOWN)	+ LAString(SBIN)  , new MMSBthetaDIP() ));
		blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(THETA)	+ LAString(CALL) + LAString(SBDOWN)	+ LAString(SBOUT) , new MMSBthetaDOC() ));
		blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(THETA)	+ LAString(PUT)  + LAString(SBDOWN)	+ LAString(SBOUT) , new MMSBthetaDOP() ));
		blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(THETA)	+ LAString(CALL) + LAString(SBUP)	+ LAString(SBIN)  , new MMSBthetaUIC() ));
		blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(THETA)	+ LAString(PUT)  + LAString(SBUP)	+ LAString(SBIN)  , new MMSBthetaUIP() ));
		blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(THETA)	+ LAString(CALL) + LAString(SBUP)	+ LAString(SBOUT) , new MMSBthetaUOC() ));
		blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(THETA)	+ LAString(PUT)  + LAString(SBUP)	+ LAString(SBOUT) , new MMSBthetaUOP() ));
		blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(RHO)	+ LAString(CALL) + LAString(SBDOWN)	+ LAString(SBIN)  , new MMSBrhoDIC() ));
		blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(RHO)	+ LAString(PUT)  + LAString(SBDOWN)	+ LAString(SBIN)  , new MMSBrhoDIP() ));
		blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(RHO)	+ LAString(CALL) + LAString(SBDOWN)	+ LAString(SBOUT) , new MMSBrhoDOC() ));
		blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(RHO)	+ LAString(PUT)  + LAString(SBDOWN)	+ LAString(SBOUT) , new MMSBrhoDOP() ));
		blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(RHO)	+ LAString(CALL) + LAString(SBUP)	+ LAString(SBIN)  , new MMSBrhoUIC() ));
		blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(RHO)	+ LAString(PUT)  + LAString(SBUP)	+ LAString(SBIN)  , new MMSBrhoUIP() ));
		blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(RHO)	+ LAString(CALL) + LAString(SBUP)	+ LAString(SBOUT) , new MMSBrhoUOC() ));
		blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(RHO)	+ LAString(PUT)  + LAString(SBUP)	+ LAString(SBOUT) , new MMSBrhoUOP() ));
		blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(PHI)	+ LAString(CALL) + LAString(SBDOWN)	+ LAString(SBIN)  , new MMSBphiDIC() ));
		blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(PHI)	+ LAString(PUT)  + LAString(SBDOWN)	+ LAString(SBIN)  , new MMSBphiDIP() ));
		blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(PHI)	+ LAString(CALL) + LAString(SBDOWN)	+ LAString(SBOUT) , new MMSBphiDOC() ));
		blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(PHI)	+ LAString(PUT)  + LAString(SBDOWN)	+ LAString(SBOUT) , new MMSBphiDOP() ));
		blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(PHI)	+ LAString(CALL) + LAString(SBUP)	+ LAString(SBIN)  , new MMSBphiUIC() ));
		blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(PHI)	+ LAString(PUT)  + LAString(SBUP)	+ LAString(SBIN)  , new MMSBphiUIP() ));
		blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(PHI)	+ LAString(CALL) + LAString(SBUP)	+ LAString(SBOUT) , new MMSBphiUOC() ));
		blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(PHI)	+ LAString(PUT)  + LAString(SBUP)	+ LAString(SBOUT) , new MMSBphiUOP() ));

		//MMDB
		blackComponentMap.insert(std::make_pair(LAString(DB) + LAString(PREM)	+ LAString(CALL) + LAString(DBIN)	+ LAString(DBIN)  , new MMDBpremKIC() ));
		blackComponentMap.insert(std::make_pair(LAString(DB) + LAString(PREM)	+ LAString(CALL) + LAString(DBOUT)	+ LAString(DBOUT) , new MMDBpremKOC()	));
		blackComponentMap.insert(std::make_pair(LAString(DB) + LAString(PREM)	+ LAString(PUT)  + LAString(DBIN)	+ LAString(DBIN)  , new MMDBpremKIP() ));
		blackComponentMap.insert(std::make_pair(LAString(DB) + LAString(PREM)	+ LAString(PUT)  + LAString(DBOUT)	+ LAString(DBOUT) , new MMDBpremKOP()	));
		blackComponentMap.insert(std::make_pair(LAString(DB) + LAString(REBATE)	+ LAString(RHIGH),	new MMDBpremRH()				));
		blackComponentMap.insert(std::make_pair(LAString(DB) + LAString(REBATE)	+ LAString(RLOW),		new MMDBpremRL()				));
		blackComponentMap.insert(std::make_pair(LAString(DB) + LAString(REBATE)	+ LAString(RIN),		new MMDBpremRI()				));
		
		blackComponentMap.insert(std::make_pair(LAString(FD) + LAString(DELTA)	+ LAString(CALL) + LAString(SPOT)  , new MMFDdeltaCallSpot() ));
		blackComponentMap.insert(std::make_pair(LAString(FD) + LAString(DELTA)	+ LAString(PUT) + LAString(SPOT)  , new MMFDdeltaPutSpot() ));
		blackComponentMap.insert(std::make_pair(LAString(FD) + LAString(FIRSTDIFF)	+ LAString(CALL) + LAString(SPOT)  , new MMFD1stDiffCallSpot() ));
		blackComponentMap.insert(std::make_pair(LAString(FD) + LAString(FIRSTDIFF)	+ LAString(PUT) + LAString(SPOT)  , new MMFD1stDiffPutSpot() ));
		blackComponentMap.insert(std::make_pair(LAString(FD) + LAString(DELTA)	+ LAString(CALL) + LAString(FORWARD)  , new MMFDdeltaCallFwd() ));
		blackComponentMap.insert(std::make_pair(LAString(FD) + LAString(DELTA)	+ LAString(PUT) + LAString(FORWARD)  , new MMFDdeltaPutFwd() ));
		blackComponentMap.insert(std::make_pair(LAString(FD) + LAString(FIRSTDIFF)	+ LAString(CALL) + LAString(FORWARD)  , new MMFD1stDiffCallFwd() ));
		blackComponentMap.insert(std::make_pair(LAString(FD) + LAString(FIRSTDIFF)	+ LAString(PUT) + LAString(FORWARD)  , new MMFD1stDiffPutFwd() ));
		blackComponentMap.insert(std::make_pair(LAString(FD) + LAString(SECONDDIFF)	+ LAString(CALL) + LAString(SPOT)  , new MMFD2ndDiffCallSpot() ));
		blackComponentMap.insert(std::make_pair(LAString(FD) + LAString(SECONDDIFF)	+ LAString(CALL) + LAString(FORWARD)  , new MMFD2ndDiffCallFwd() ));		
		//MMAFF
		blackComponentMap.insert(std::make_pair(LAString(AFF)	+ LAString(PREM) + LAString(CALL),	new MMAFFpremCall()	)	);
		blackComponentMap.insert(std::make_pair(LAString(AFF)	+ LAString(PREM) + LAString(PUT),	new MMAFFpremPut()	)	);
	}
	return blackComponentMap;
}
std::map<LAString, AnalyticParam*>& 
LACoreComponentManager::getBlackParamComponentMap()
{
	if(blackParamComponentMap.empty())
	{
		blackParamComponentMap.insert(std::make_pair(LAString(BK), new AnalyticBKParam() ));
		blackParamComponentMap.insert(std::make_pair(LAString(GK), new AnalyticGKParam() ));
		blackParamComponentMap.insert(std::make_pair(LAString(DG), new AnalyticDGParam() ));
		blackParamComponentMap.insert(std::make_pair(LAString(SB), new AnalyticSBParam() ));
		blackParamComponentMap.insert(std::make_pair(LAString(DB), new AnalyticDBParam() ));
	}
	return blackParamComponentMap;
}

/*!
	@brief getIntetpolation

	@param[in] key
	@return LAString

*/
LAString
LACoreComponentManager::getInterpolation(const LAString &key)
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
		LAString msg = key + " is not registered in interpolation methods";
		throw LACoreInvalidData(msg.getCString(), __FILE__,__LINE__);
	}

}

/*!
	@brief getDayCount

	@param[in] key
	@return LAString

*/
LAString
LACoreComponentManager::getDayCount(const LAString &key)
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
		LAString msg = key + " is not registered in daycount conventions";
		throw LACoreInvalidData(msg.getCString(), __FILE__,__LINE__);
	}

}

/*!
	@brief getBasisType

	@param[in] key
	@return LAString

*/
LAString
LACoreComponentManager::getBasisType(const LAString &key)
{
	if (key == XCCYBASIS)
	{
		return XCCYBASIS;
	}
	else
	{
		throw LACoreInvalidData("BasisType is not registered ", __FILE__,__LINE__);
	}
}

/*!
	@brief initialize

*/
void
LACoreComponentManager::initialize()
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
	blackComponentMap.insert(std::make_pair(LAString("ERROR"),	new MMZeroForError()));
	//LABlackPayOff
	blackComponentMap.insert(std::make_pair(LAString(BKPAYOFF)	+ LAString(PREM) + LAString(CALL),	new LABlackPayOffpremCall())	);
	blackComponentMap.insert(std::make_pair(LAString(BKPAYOFF)	+ LAString(PREM) + LAString(PUT),	new LABlackPayOffpremPut()	)	);
	
	
	
	
	//LABlack
	blackComponentMap.insert(std::make_pair(LAString(BK)	+ LAString(PREM) + LAString(CALL),	new LABlackpremCall()	)	);
	blackComponentMap.insert(std::make_pair(LAString(BK)	+ LAString(PREM) + LAString(PUT),	new LABlackpremPut()	)	);
	blackComponentMap.insert(std::make_pair(LAString(BK)	+ LAString(DELTA) + LAString(CALL), new LABlackdeltaCall()	)	);
	blackComponentMap.insert(std::make_pair(LAString(BK)	+ LAString(GAMMA) + LAString(CALL), new LABlackgammaCall()	)	);
	blackComponentMap.insert(std::make_pair(LAString(BK)	+ LAString(VEGA)  + LAString(CALL), new LABlackvegaCall()	)	);
	blackComponentMap.insert(std::make_pair(LAString(BK)	+ LAString(THETA) + LAString(CALL), new LABlackthetaCall()	)	);
	blackComponentMap.insert(std::make_pair(LAString(BK)	+ LAString(DELTA) + LAString(PUT),  new LABlackdeltaPut()	)	);
	blackComponentMap.insert(std::make_pair(LAString(BK)	+ LAString(GAMMA) + LAString(PUT),  new LABlackgammaPut()	)	);
	blackComponentMap.insert(std::make_pair(LAString(BK)	+ LAString(VEGA)  + LAString(PUT),  new LABlackvegaPut()	)	);
	blackComponentMap.insert(std::make_pair(LAString(BK)	+ LAString(THETA) + LAString(PUT),  new LABlackthetaPut()	)	);
	//LAGreek
	blackComponentMap.insert(std::make_pair(LAString(GK) + LAString(PREM)	+ LAString(CALL), new LAGreekpremCall()	)	);
	blackComponentMap.insert(std::make_pair(LAString(GK) + LAString(DELTA)	+ LAString(CALL), new LAGreekdeltaCall() )	);
	blackComponentMap.insert(std::make_pair(LAString(GK) + LAString(GAMMA)	+ LAString(CALL), new LAGreekgammaCall() )	);
	blackComponentMap.insert(std::make_pair(LAString(GK) + LAString(VEGA)	+ LAString(CALL), new LAGreekvegaCall()	)	);
	blackComponentMap.insert(std::make_pair(LAString(GK) + LAString(THETA)	+ LAString(CALL), new LAGreekthetaCall() )	);
	blackComponentMap.insert(std::make_pair(LAString(GK) + LAString(RHO)	+ LAString(CALL), new LAGreekrhoCall()	)	);
	blackComponentMap.insert(std::make_pair(LAString(GK) + LAString(PHI)	+ LAString(CALL), new LAGreekphiCall()	)	);
	blackComponentMap.insert(std::make_pair(LAString(GK) + LAString(VANNA)	+ LAString(CALL), new LAGreekvannaCall()	)	);
	blackComponentMap.insert(std::make_pair(LAString(GK) + LAString(VOLGA)	+ LAString(CALL), new LAGreekvolgaCall()	)	);
	blackComponentMap.insert(std::make_pair(LAString(GK) + LAString(PREM)	+ LAString(PUT),	new LAGreekpremPut()	)	);
	blackComponentMap.insert(std::make_pair(LAString(GK) + LAString(DELTA)	+ LAString(PUT),	new LAGreekdeltaPut()	)	);
	blackComponentMap.insert(std::make_pair(LAString(GK) + LAString(GAMMA)	+ LAString(PUT),	new LAGreekgammaPut()	)	);
	blackComponentMap.insert(std::make_pair(LAString(GK) + LAString(VEGA)	+ LAString(PUT),	new LAGreekvegaPut()	)	);
	blackComponentMap.insert(std::make_pair(LAString(GK) + LAString(THETA)	+ LAString(PUT),	new LAGreekthetaPut()	)	);
	blackComponentMap.insert(std::make_pair(LAString(GK) + LAString(RHO)	+ LAString(PUT),	new LAGreekrhoPut()	)	);
	blackComponentMap.insert(std::make_pair(LAString(GK) + LAString(PHI)	+ LAString(PUT),	new LAGreekphiPut()	)	);
	blackComponentMap.insert(std::make_pair(LAString(GK) + LAString(VANNA)	+ LAString(PUT), new LAGreekvannaPut()	)	);
	blackComponentMap.insert(std::make_pair(LAString(GK) + LAString(VOLGA)	+ LAString(PUT), new LAGreekvolgaPut()	)	);
	//MMCapFloor
	blackComponentMap.insert(std::make_pair(LAString(CF) + LAString(PREM) + LAString(CALL), new MMCFpremCall()));
	blackComponentMap.insert(std::make_pair(LAString(CF) + LAString(VEGA) + LAString(CALL), new MMCFvegaCall()));
	blackComponentMap.insert(std::make_pair(LAString(CF) + LAString(PREM) + LAString(PUT), new MMCFpremPut()));
	blackComponentMap.insert(std::make_pair(LAString(CF) + LAString(VEGA) + LAString(PUT), new MMCFvegaPut()));
	//MMDG
	blackComponentMap.insert(std::make_pair(LAString(DG) + LAString(PREM)	+ LAString(CALL), new MMDGpremCall()	)	);
	blackComponentMap.insert(std::make_pair(LAString(DG) + LAString(DELTA)	+ LAString(CALL), new MMDGdeltaCall() )	);
	blackComponentMap.insert(std::make_pair(LAString(DG) + LAString(GAMMA)	+ LAString(CALL), new MMDGgammaCall() )	);
	blackComponentMap.insert(std::make_pair(LAString(DG) + LAString(VEGA)	+ LAString(CALL), new MMDGvegaCall()	)	);
	blackComponentMap.insert(std::make_pair(LAString(DG) + LAString(THETA)	+ LAString(CALL), new MMDGthetaCall() )	);
	blackComponentMap.insert(std::make_pair(LAString(DG) + LAString(RHO)	+ LAString(CALL), new MMDGrhoCall()	)	);
	blackComponentMap.insert(std::make_pair(LAString(DG) + LAString(PHI)	+ LAString(CALL), new MMDGphiCall()	)	);
	blackComponentMap.insert(std::make_pair(LAString(DG) + LAString(PREM)	+ LAString(PUT),	new MMDGpremPut()	)	);
	blackComponentMap.insert(std::make_pair(LAString(DG) + LAString(DELTA)	+ LAString(PUT),	new MMDGdeltaPut()	)	);
	blackComponentMap.insert(std::make_pair(LAString(DG) + LAString(GAMMA)	+ LAString(PUT),	new MMDGgammaPut()	)	);
	blackComponentMap.insert(std::make_pair(LAString(DG) + LAString(VEGA)	+ LAString(PUT),	new MMDGvegaPut()	)	);
	blackComponentMap.insert(std::make_pair(LAString(DG) + LAString(THETA)	+ LAString(PUT),	new MMDGthetaPut()	)	);
	blackComponentMap.insert(std::make_pair(LAString(DG) + LAString(RHO)	+ LAString(PUT),	new MMDGrhoPut()	)	);
	blackComponentMap.insert(std::make_pair(LAString(DG) + LAString(PHI)	+ LAString(PUT),	new MMDGphiPut()	)	);
	//hishida vannavolga
	blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(PROB)	+ LAString(SBUP) + LAString(NOTOUCH), new MMSBprobUNT() ));
	blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(PROB)	+ LAString(SBDOWN) + LAString(NOTOUCH), new MMSBprobDNT() ));
	//hishida vannavolga
	//MMSB
	blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(PREM)	+ LAString(CALL) + LAString(SBDOWN)	+ LAString(SBIN)  , new MMSBpremDIC() ));
	blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(PREM)	+ LAString(PUT)  + LAString(SBDOWN)	+ LAString(SBIN)  , new MMSBpremDIP() ));
	blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(PREM)	+ LAString(CALL) + LAString(SBDOWN)	+ LAString(SBOUT) , new MMSBpremDOC() ));
	blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(PREM)	+ LAString(PUT)  + LAString(SBDOWN)	+ LAString(SBOUT) , new MMSBpremDOP() ));
	blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(PREM)	+ LAString(CALL) + LAString(SBUP)	+ LAString(SBIN)  , new MMSBpremUIC() ));
	blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(PREM)	+ LAString(PUT)  + LAString(SBUP)	+ LAString(SBIN)  , new MMSBpremUIP() ));
	blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(PREM)	+ LAString(CALL) + LAString(SBUP)	+ LAString(SBOUT) , new MMSBpremUOC() ));
	blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(PREM)	+ LAString(PUT)  + LAString(SBUP)	+ LAString(SBOUT) , new MMSBpremUOP() ));
	blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(DELTA)	+ LAString(CALL) + LAString(SBDOWN)	+ LAString(SBIN)  , new MMSBdeltaDIC() ));
	blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(DELTA)	+ LAString(PUT)  + LAString(SBDOWN)	+ LAString(SBIN)  , new MMSBdeltaDIP() ));
	blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(DELTA)	+ LAString(CALL) + LAString(SBDOWN)	+ LAString(SBOUT) , new MMSBdeltaDOC() ));
	blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(DELTA)	+ LAString(PUT)  + LAString(SBDOWN)	+ LAString(SBOUT) , new MMSBdeltaDOP() ));
	blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(DELTA)	+ LAString(CALL) + LAString(SBUP)	+ LAString(SBIN)  , new MMSBdeltaUIC() ));
	blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(DELTA)	+ LAString(PUT)  + LAString(SBUP)	+ LAString(SBIN)  , new MMSBdeltaUIP() ));
	blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(DELTA)	+ LAString(CALL) + LAString(SBUP)	+ LAString(SBOUT) , new MMSBdeltaUOC() ));
	blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(DELTA)	+ LAString(PUT)  + LAString(SBUP)	+ LAString(SBOUT) , new MMSBdeltaUOP() ));
	blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(GAMMA)	+ LAString(CALL) + LAString(SBDOWN)	+ LAString(SBIN)  , new MMSBgammaDIC() ));
	blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(GAMMA)	+ LAString(PUT)  + LAString(SBDOWN)	+ LAString(SBIN)  , new MMSBgammaDIP() ));
	blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(GAMMA)	+ LAString(CALL) + LAString(SBDOWN)	+ LAString(SBOUT) , new MMSBgammaDOC() ));
	blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(GAMMA)	+ LAString(PUT)  + LAString(SBDOWN)	+ LAString(SBOUT) , new MMSBgammaDOP() ));
	blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(GAMMA)	+ LAString(CALL) + LAString(SBUP)	+ LAString(SBIN)  , new MMSBgammaUIC() ));
	blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(GAMMA)	+ LAString(PUT)  + LAString(SBUP)	+ LAString(SBIN)  , new MMSBgammaUIP() ));
	blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(GAMMA)	+ LAString(CALL) + LAString(SBUP)	+ LAString(SBOUT) , new MMSBgammaUOC() ));
	blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(GAMMA)	+ LAString(PUT)  + LAString(SBUP)	+ LAString(SBOUT) , new MMSBgammaUOP() ));
	blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(VEGA)	+ LAString(CALL) + LAString(SBDOWN)	+ LAString(SBIN)  , new MMSBvegaDIC() ));
	blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(VEGA)	+ LAString(PUT)  + LAString(SBDOWN)	+ LAString(SBIN)  , new MMSBvegaDIP() ));
	blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(VEGA)	+ LAString(CALL) + LAString(SBDOWN)	+ LAString(SBOUT) , new MMSBvegaDOC() ));
	blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(VEGA)	+ LAString(PUT)  + LAString(SBDOWN)	+ LAString(SBOUT) , new MMSBvegaDOP() ));
	blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(VEGA)	+ LAString(CALL) + LAString(SBUP)	+ LAString(SBIN)  , new MMSBvegaUIC() ));
	blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(VEGA)	+ LAString(PUT)  + LAString(SBUP)	+ LAString(SBIN)  , new MMSBvegaUIP() ));
	blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(VEGA)	+ LAString(CALL) + LAString(SBUP)	+ LAString(SBOUT) , new MMSBvegaUOC() ));
	blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(VEGA)	+ LAString(PUT)  + LAString(SBUP)	+ LAString(SBOUT) , new MMSBvegaUOP() ));
	blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(THETA)	+ LAString(CALL) + LAString(SBDOWN)	+ LAString(SBIN)  , new MMSBthetaDIC() ));
	blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(THETA)	+ LAString(PUT)  + LAString(SBDOWN)	+ LAString(SBIN)  , new MMSBthetaDIP() ));
	blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(THETA)	+ LAString(CALL) + LAString(SBDOWN)	+ LAString(SBOUT) , new MMSBthetaDOC() ));
	blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(THETA)	+ LAString(PUT)  + LAString(SBDOWN)	+ LAString(SBOUT) , new MMSBthetaDOP() ));
	blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(THETA)	+ LAString(CALL) + LAString(SBUP)	+ LAString(SBIN)  , new MMSBthetaUIC() ));
	blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(THETA)	+ LAString(PUT)  + LAString(SBUP)	+ LAString(SBIN)  , new MMSBthetaUIP() ));
	blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(THETA)	+ LAString(CALL) + LAString(SBUP)	+ LAString(SBOUT) , new MMSBthetaUOC() ));
	blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(THETA)	+ LAString(PUT)  + LAString(SBUP)	+ LAString(SBOUT) , new MMSBthetaUOP() ));
	blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(RHO)	+ LAString(CALL) + LAString(SBDOWN)	+ LAString(SBIN)  , new MMSBrhoDIC() ));
	blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(RHO)	+ LAString(PUT)  + LAString(SBDOWN)	+ LAString(SBIN)  , new MMSBrhoDIP() ));
	blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(RHO)	+ LAString(CALL) + LAString(SBDOWN)	+ LAString(SBOUT) , new MMSBrhoDOC() ));
	blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(RHO)	+ LAString(PUT)  + LAString(SBDOWN)	+ LAString(SBOUT) , new MMSBrhoDOP() ));
	blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(RHO)	+ LAString(CALL) + LAString(SBUP)	+ LAString(SBIN)  , new MMSBrhoUIC() ));
	blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(RHO)	+ LAString(PUT)  + LAString(SBUP)	+ LAString(SBIN)  , new MMSBrhoUIP() ));
	blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(RHO)	+ LAString(CALL) + LAString(SBUP)	+ LAString(SBOUT) , new MMSBrhoUOC() ));
	blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(RHO)	+ LAString(PUT)  + LAString(SBUP)	+ LAString(SBOUT) , new MMSBrhoUOP() ));
	blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(PHI)	+ LAString(CALL) + LAString(SBDOWN)	+ LAString(SBIN)  , new MMSBphiDIC() ));
	blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(PHI)	+ LAString(PUT)  + LAString(SBDOWN)	+ LAString(SBIN)  , new MMSBphiDIP() ));
	blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(PHI)	+ LAString(CALL) + LAString(SBDOWN)	+ LAString(SBOUT) , new MMSBphiDOC() ));
	blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(PHI)	+ LAString(PUT)  + LAString(SBDOWN)	+ LAString(SBOUT) , new MMSBphiDOP() ));
	blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(PHI)	+ LAString(CALL) + LAString(SBUP)	+ LAString(SBIN)  , new MMSBphiUIC() ));
	blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(PHI)	+ LAString(PUT)  + LAString(SBUP)	+ LAString(SBIN)  , new MMSBphiUIP() ));
	blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(PHI)	+ LAString(CALL) + LAString(SBUP)	+ LAString(SBOUT) , new MMSBphiUOC() ));
	blackComponentMap.insert(std::make_pair(LAString(SB) + LAString(PHI)	+ LAString(PUT)  + LAString(SBUP)	+ LAString(SBOUT) , new MMSBphiUOP() ));

	//MMDB
	blackComponentMap.insert(std::make_pair(LAString(DB) + LAString(PREM)	+ LAString(CALL) + LAString(DBIN)	+ LAString(DBIN)  , new MMDBpremKIC() ));
	blackComponentMap.insert(std::make_pair(LAString(DB) + LAString(PREM)	+ LAString(CALL) + LAString(DBOUT)	+ LAString(DBOUT) , new MMDBpremKOC()	));
	blackComponentMap.insert(std::make_pair(LAString(DB) + LAString(PREM)	+ LAString(PUT)  + LAString(DBIN)	+ LAString(DBIN)  , new MMDBpremKIP() ));
	blackComponentMap.insert(std::make_pair(LAString(DB) + LAString(PREM)	+ LAString(PUT)  + LAString(DBOUT)	+ LAString(DBOUT) , new MMDBpremKOP()	));
	blackComponentMap.insert(std::make_pair(LAString(DB) + LAString(REBATE)	+ LAString(RHIGH),	new MMDBpremRH()				));
	blackComponentMap.insert(std::make_pair(LAString(DB) + LAString(REBATE)	+ LAString(RLOW),		new MMDBpremRL()				));
	blackComponentMap.insert(std::make_pair(LAString(DB) + LAString(REBATE)	+ LAString(RIN),		new MMDBpremRI()				));
	
	blackComponentMap.insert(std::make_pair(LAString(FD) + LAString(DELTA)	+ LAString(CALL) + LAString(SPOT)  , new MMFDdeltaCallSpot() ));
	blackComponentMap.insert(std::make_pair(LAString(FD) + LAString(DELTA)	+ LAString(PUT) + LAString(SPOT)  , new MMFDdeltaPutSpot() ));
	blackComponentMap.insert(std::make_pair(LAString(FD) + LAString(FIRSTDIFF)	+ LAString(CALL) + LAString(SPOT)  , new MMFD1stDiffCallSpot() ));
	blackComponentMap.insert(std::make_pair(LAString(FD) + LAString(FIRSTDIFF)	+ LAString(PUT) + LAString(SPOT)  , new MMFD1stDiffPutSpot() ));
	blackComponentMap.insert(std::make_pair(LAString(FD) + LAString(DELTA)	+ LAString(CALL) + LAString(FORWARD)  , new MMFDdeltaCallFwd() ));
	blackComponentMap.insert(std::make_pair(LAString(FD) + LAString(DELTA)	+ LAString(PUT) + LAString(FORWARD)  , new MMFDdeltaPutFwd() ));
	blackComponentMap.insert(std::make_pair(LAString(FD) + LAString(FIRSTDIFF)	+ LAString(CALL) + LAString(FORWARD)  , new MMFD1stDiffCallFwd() ));
	blackComponentMap.insert(std::make_pair(LAString(FD) + LAString(FIRSTDIFF)	+ LAString(PUT) + LAString(FORWARD)  , new MMFD1stDiffPutFwd() ));
	blackComponentMap.insert(std::make_pair(LAString(FD) + LAString(SECONDDIFF)	+ LAString(CALL) + LAString(SPOT)  , new MMFD2ndDiffCallSpot() ));
	blackComponentMap.insert(std::make_pair(LAString(FD) + LAString(SECONDDIFF)	+ LAString(CALL) + LAString(FORWARD)  , new MMFD2ndDiffCallFwd() ));
	//MMAFF
	blackComponentMap.insert(std::make_pair(LAString(AFF)	+ LAString(PREM) + LAString(CALL),	new MMAFFpremCall()	)	);
	blackComponentMap.insert(std::make_pair(LAString(AFF)	+ LAString(PREM) + LAString(PUT),	new MMAFFpremPut()	)	);


	// black param component map
	blackParamComponentMap.insert(std::make_pair(LAString(BK), new AnalyticBKParam() ));
	blackParamComponentMap.insert(std::make_pair(LAString(GK), new AnalyticGKParam() ));
	blackParamComponentMap.insert(std::make_pair(LAString(DG), new AnalyticDGParam() ));
	blackParamComponentMap.insert(std::make_pair(LAString(SB), new AnalyticSBParam() ));
	blackParamComponentMap.insert(std::make_pair(LAString(DB), new AnalyticDBParam() ));

}

/*!
	@brief finalize

*/
void
LACoreComponentManager::finalize()
{
	interpolationMap.clear();
	dayCountMap.clear();
	basisTypeMap.clear();
	basisIndexMap.clear();
	deleteBlackComponentMap();
	deleteBlackParamComponentMap();
}




void 
LACoreComponentManager::deleteBlackComponentMap()
{
	std::map<LAString, LABlackScholesBase*> &var = LACoreComponentManager::getBlackComponentMap();
	
	std::map<LAString, LABlackScholesBase*>::iterator it = var.begin();
	while (it != var.end())
	{
		delete it->second;
		++it;
	}
	var.clear();
}
void 
LACoreComponentManager::deleteBlackParamComponentMap()
{
	std::map<LAString, AnalyticParam*> &var = LACoreComponentManager::getBlackParamComponentMap();
	
	std::map<LAString, AnalyticParam*>::iterator it = var.begin();
	while (it != var.end())
	{
		delete it->second;
		++it;
	}
	var.clear();
}
