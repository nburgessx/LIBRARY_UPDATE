//
// LACurveCalibrationHelpers.cpp 
// *** This file used to be called "LAMathCurveGenerateFuncUti1ity.cpp" ***
//

#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

// LA Includes
#include "LACurveCalibrationHelpers.h"  // was #include "LAMathCurveGenerateFuncUti1ity.h"
#include "LADateHelpers.h"              // was #include "LAMathDateCalcUti1ity.h"
#include "LADateScheduleHelpers.h"      // was #include "LAMathDateFuncUti1ity.h"
#include "LACurvePricingObject.h"      // was #include "LAMathYie1dCurve.h"

// External Includes
#include <cmath>
#include <map>
#include <algorithm>

// Internal Includes
#include "AQLFunctionUtilities.h"
#include "AQLObject.h"
#include "AQLDataBasics.h"
#include "AQLDataVector.h"
#include "AQLDataMatrix.h"
#include "AQLCoreTemplateType.h"
#include "AQLMathDefine.h"
#include "AQLPriceDataCalendar.h"
#include "AQLPriceDataSlidingRule.h"
#include "AQLBasic.h"
#include "AQLAlgorithm.h"
#include "AQLSplineInterpolation.h"
#include "AQLStepInterpolation.h"
#include "AQLLinearInterpolation.h"
#include "AQLInterpolationBase.h"
#include "AQLPriceDataDayCount.h"
#include "AQLPriceCFGenUtility.h"
#include "AQLPriceYieldGenerator.h"
#include "AQLDataReference.h"
#include "AQLAnalyticFormula.h"
#include "AQLBlackScholesCalc.h"
#include "AQLMathYieldCurvePro.h"
#include "AQLDataProcedure.h"
#include "AQLPriceDataInterpolation.h"
#include "AQLCoreComponentManager.h"
#include "AQLPriceDataConvention.h"
#include "AQLDataMultiReference.h"
#include "AQLMathValuableEntity.h"
#include "AQLPriceDataFunction.h"
#include "AQLPriceArbFreeGenerator.h"
#include "AQLCoreUtility.h"

// TODO - Try to remove this file. It contains lots of #defines that clash with external binaries
#include "ConstantDeclarations.h"

using namespace std;

// TODO: Remove this define statement, bad practice!
#ifndef YIELD_TYPE_BASIS
#define YIELD_TYPE_BASIS  "BASISRATE"
#endif


namespace etrading
{

    void  
    LACurveCalibrationHelpers::setUpCurveFromMarket(AQLDataInstance* dataInstance, const AQLString& CurveID, 
						    AQLStringMatrix mgrid, AQLStringMatrix lgrid, 
						    AQLStringMatrix fgrid, AQLStringMatrix sgrid,
						    AQLStringMatrix mdata, AQLStringMatrix ldata, 
						    AQLStringMatrix fdata, AQLStringMatrix sdata,
						    AQLStringMatrix ylddata, bool isCheckDF, bool isAUDadjust)
    {
	    //ylddata
	    AQLString	CurveIDTool			= CurveID+TOOL;
	    AQLString	CurveIDManager		= CurveID + STD+ MANAGER;

	    AQLObjectPool& objPool = dataInstance->getObjectPool(); 
	    const AQLObjectHolder objHolder = objPool.getObject(CurveIDManager,ENCHKTYPE_NOCHECK);
	
	    //str upper and delete space
	    upper(mgrid);
	    upper(lgrid);
	    upper(fgrid);
	    upper(sgrid);
	    upper(mdata);
	    upper(ldata);
	    upper(fdata);
	    upper(sdata);
	    upper(ylddata);
	
	    AQLDate		asofdate			= LADateScheduleHelpers::getLADate(chgrow(ylddata,CURVEINPUT_ASOFDATE,1));
	    AQLString	Currency			= chgrow(ylddata,CURVEINPUT_CURRENCY,	1);
	    AQLString	strfutureuse		= chgrow(ylddata,CURVEINPUT_ISFUTUREUSE,1);
	
	    // check STD curve 
	    AQLString curveType;
	    if (AQLFunctionUtilities::findRowsNumber(ylddata,CURVEINPUT_CURVETYPE) < 0)
	    {
		    curveType = BASECURVE;
	    }
	    else
	    {
		    curveType = chgrow(ylddata, CURVEINPUT_CURVETYPE, 1);
	    } 
	    AQLString suffix = AQLString("");
	    bool isSTD = true;
	    if (curveType != BASECURVE && curveType != OISCURVE)
	    {
		    suffix = AQLString("_") + curveType;
		    isSTD = false;
	    }

	    //intertplation 
	    std::map<AQLString, AQLString>& ivar = AQLCoreComponentManager::getInterpolationMap();
	    std::map<AQLString, AQLString>::iterator it;
	
	    it = ivar.find(chgrow(ylddata,CURVEINPUT_FUTUREINTERPOLATION,1		));
	    if(it==ivar.end())
		    throw AQLCoreInvalidData("Future interpolation is not registered in interpolation methods", __FILE__,__LINE__);
	    AQLString futureinterpolation	= it->second;
	
	    it = ivar.find(chgrow(ylddata,CURVEINPUT_YIELDGENINTERPOLATION,1	));
	    if(it==ivar.end())
		    throw AQLCoreInvalidData("YieldGenInterpolation is not registered in interpolation methods", __FILE__,__LINE__);
	    AQLString yieldgeninterpolation	= it->second;
	
	    it = ivar.find(chgrow(ylddata,CURVEINPUT_INTERPOLATION,1			));
	    if(it==ivar.end())
		    throw AQLCoreInvalidData("Interpolation is not registered in interpolation methods",			__FILE__,__LINE__);
	    AQLString interpolation = it->second;

	    AQLDataBool bl;
	    bl.convertFromString(strfutureuse);
	    bool isfuteruse = bl.get();
	    //YieldData
	    AQLObject* e = NULL;
	    if(!objPool.getObject(CurveID).isDefined())
	    {	
		    e = new AQLObject;
		    objPool.set(CurveID,e);
		    e->add(CALIBRATION_DATA_NAME, new AQLDataString(CurveID));
	    }
	    else
	    {
		    if (isSTD)	objPool.getObject(CurveID).get().clear();
		    e= &objPool.getObject(CurveID).get();
	    }
	
	    //CurveCalibrationData
	    const AQLObjectHolder ehCur = objPool.getObject(CurveIDTool);
	    AQLMathYieldCurvePro * curveCalibrationData = NULL; 
	    if(!ehCur.isDefined())
	    {	
		    curveCalibrationData= new AQLMathYieldCurvePro(dataInstance);
		    objPool.set(CurveIDTool,curveCalibrationData);
	    }
	    else
	    {
		    dynamic_cast<AQLMathYieldCurvePro &>(objPool.getObject(CurveIDTool).get()).reset();
		    curveCalibrationData	= &dynamic_cast<AQLMathYieldCurvePro &>(objPool.getObject(CurveIDTool).get());
	    }
	    curveCalibrationData->getName().convertFromString(CurveIDTool);
	    curveCalibrationData->setDFGenerator(FN_IRYIELDGENERATOR_STR);
	    curveCalibrationData->getAsOfDate().set(asofdate);
	    curveCalibrationData->getIsFutureUse().set(isfuteruse);
	    curveCalibrationData->setInterpolation(interpolation);
	    curveCalibrationData->getInterpolation_fw().convertFromString(futureinterpolation);
	    curveCalibrationData->getInterpolation_yg().convertFromString(yieldgeninterpolation);
	    curveCalibrationData->getYieldData().convertFromString(CurveID);
	    curveCalibrationData->getDayCount().convertFromString(AC_365I);
	    curveCalibrationData->getIsSwapTenorAdjust().set(false);
	    curveCalibrationData->getIsFRAUse().set(false);
	    if (!isSTD)
	    {
		    curveCalibrationData->AQLObject::add(IR_CALIBRATION_DATA_GENTARGETDF, new AQLDataString(curveType));
		    AQLStringVector tmpCurveType(1,curveType);
		    curveCalibrationData->AQLObject::add(IR_CALIBRATION_DATA_GENERATEDFS, new AQLDataStrings(tmpCurveType));

		    curveCalibrationData->setAssignedCurveMktMap(curveType,curveType);
	    }
	    //RatePriority
	    const AQLString& ratepriority = AQLFunctionUtilities::findElement(ylddata,RATEPRIORITY,0,1,true,false);
	    if (ratepriority == "SWAP:LIBOR:FUTURE"||ratepriority == "SWAP:FUTURE:LIBOR"||ratepriority == "FUTURE:LIBOR:SWAP"||ratepriority == "FUTURE:SWAP:LIBOR"||ratepriority == "LIBOR:SWAP:FUTURE"||ratepriority == "LIBOR:FUTURE:SWAP")
		    curveCalibrationData->getRatePriority().convertFromString(ratepriority);
	    else if (ratepriority == "")
	    {	
		    const AQLStringVector tmpstrvec(0);
		    curveCalibrationData->getRatePriority().set(tmpstrvec);
	    }
	    else
		    throw AQLCoreInvalidData("RatePriority is incorrect",			__FILE__,__LINE__);
	
	    //daycount
	    std::map<AQLString, AQLString>& dvar = AQLCoreComponentManager::getDayCountMap();
	    std::map<AQLString, AQLString>::iterator itd;
	
	    AQLDate spotdate;
	    AQLString ref;
	    AQLString Term;
	    AQLString srule;
	    AQLStringVector city;
        AQLString calendar;
	    AQLString dc;
	    int N=0;
	    double rate=0.0;
	    //Money Market
        //spotdate	= LADateScheduleHelpers::getLADate(chgrow(mdata,CURVEINPUT_SPOTDATE,1));
	    itd			= dvar.find(chgrow(mdata,CURVEINPUT_DAYCOUNT,1));
	    if(itd==dvar.end())
		    throw AQLCoreInvalidData("Money Market Daycount is not registered ", __FILE__,__LINE__);
	    dc			= itd->second;
	    city		= chgrow(mdata,CURVEINPUT_CALENDAR,1).toToken(':');
        calendar    = chgrow(mdata,CURVEINPUT_CALENDAR,1);
	    srule		= chgrow(mdata,CURVEINPUT_SLIDINGRULE,1);
	    N			= mgrid.size();

        int tmprow1,tmprow2;
        tmprow1 = AQLFunctionUtilities::findRowsNumber(mdata,CURVEINPUT_SPOTLAG);
        tmprow2= AQLFunctionUtilities::findRowsNumber(mdata,CURVEINPUT_SPOTDATE);
        if( tmprow1<0 && tmprow2<0) 
        {
            throw AQLCoreInvalidData("input spot date or spot lag", __FILE__,__LINE__);
        }
        else if(tmprow1>=0 && tmprow2>=0)
        {
		    throw AQLCoreInvalidData("do not input spot date and spot lag", __FILE__,__LINE__);
	    }
	    else if(tmprow1>=0)
	    {		
		    AQLString spotLag = AQLFunctionUtilities::findElement(mdata,CURVEINPUT_SPOTLAG,0,1,true);
            spotdate = LADateScheduleHelpers::getDate(asofdate,spotLag,srule,calendar);
        }
        else if(tmprow2>=0)
        {
            spotdate = LADateScheduleHelpers::getLADate(chgrow(mdata,CURVEINPUT_SPOTDATE,1));
        }

	    //i=row=0 is input row (not grid);
	    for(int i=1; i<N; i++)
	    {
		    AQLString name;
		    AQLString type;
		    AQLObject* mme = NULL;

		    type = chgcol(mgrid,CURVEINPUT_TYPE,i).getCString();
		    name = CurveID + Currency + type;

		    if(!objPool.getObject(name).isDefined())
		    {	
			    mme = new AQLObject;
			    objPool.set(name, mme);
		    }
		    else
		    {
			    objPool.getObject(name).get().clear();
			    mme= &objPool.getObject(name).get();
		    }

		    rate = chgcol(mgrid,CURVEINPUT_RATE,i).getDoubleValue();
		    mme->add(CALIBRATION_DATA_NAME,				new AQLDataString(name)							);
		    mme->add(IR_CALIBRATION_DATA_DATATYPE,		new AQLDataString(type)							);
		    mme->add(IR_CALIBRATION_DATA_DAYCOUNT,		new AQLPriceDataDayCount()							).convertFromString(dc);
		    mme->add(CALIBRATION_DATA_CALENDAR,			new AQLPriceDataCalendar(city)						);
		    mme->add(CALIBRATION_DATA_SLIDINGRULE,		new AQLPriceDataSlidingRule()							).convertFromString(srule);
		    mme->add(IR_CALIBRATION_DATA_SPOTDATE,		new AQLDataDate(spotdate)						);
		    mme->add(CALIBRATION_DATA_RATE,				new AQLDataDouble(rate)							);
		    //ref+= (1==i) ? name + ":" : name;
		    ref+= name + ":";
	    }
	    ////LiborData
	    //spotdate	= LADateScheduleHelpers::getLADate(chgrow(ldata,CURVEINPUT_SPOTDATE,1));
	    itd			= dvar.find(chgrow(ldata,CURVEINPUT_DAYCOUNT,1));
	    if(itd==dvar.end())
		    throw AQLCoreInvalidData("Libor Daycount is not registered ", __FILE__,__LINE__);
	    dc			= itd->second;
	    city		= chgrow(ldata,CURVEINPUT_CALENDAR,1).toToken(':');
        calendar	= chgrow(ldata,CURVEINPUT_CALENDAR,1);
	    srule		= chgrow(ldata,CURVEINPUT_SLIDINGRULE,1);
	    tmprow1= AQLFunctionUtilities::findRowsNumber(ylddata,CURVEINPUT_ISONFORSPOTADJUST);
	    AQLString isOnForSpotAdjust_str = "FALSE";
	    if (tmprow1 >= 0)
	    {
		     isOnForSpotAdjust_str = chgrow(ylddata,CURVEINPUT_ISONFORSPOTADJUST,1);
	    }
	    N = lgrid.size();
	    AQLPriceDataDayCount dc_Libor_attr; dc_Libor_attr.convertFromString(dc);
	    AQLPriceDataCalendar cal_Libor_attr; cal_Libor_attr.convertFromString(calendar);
	    AQLPriceDataSlidingRule sr_Libor_attr; sr_Libor_attr.convertFromString(srule);
	    AQLDataBool isOnForSpotAdjust_attr; isOnForSpotAdjust_attr.convertFromString(isOnForSpotAdjust_str);

        tmprow1= AQLFunctionUtilities::findRowsNumber(ldata,CURVEINPUT_SPOTLAG);
        tmprow2= AQLFunctionUtilities::findRowsNumber(ldata,CURVEINPUT_SPOTDATE);
        if( tmprow1<0 && tmprow2<0) 
        {
            throw AQLCoreInvalidData("input spot date or spot lag", __FILE__,__LINE__);
        }
	    else if(tmprow1>=0 && tmprow2>=0)
        {
		    throw AQLCoreInvalidData("do not input spot date and spot lag", __FILE__,__LINE__);
	    }
	    else if(tmprow1>=0)
	    {		
		    AQLString spotLag = AQLFunctionUtilities::findElement(ldata,CURVEINPUT_SPOTLAG,0,1,true);
            spotdate = LADateScheduleHelpers::getDate(asofdate,spotLag,srule,calendar);
        }
        else if(tmprow2>=0)
        {
            spotdate = LADateScheduleHelpers::getLADate(chgrow(ldata,CURVEINPUT_SPOTDATE,1));
        }

	    //i=row=0 is input row (not grid);
	    for(int i=1; i<N; i++)
	    {
		    Term			=	chgcol(lgrid,CURVEINPUT_TERM,i);
		    AQLString name	=	CurveID + Currency + LIBOR + Term;
		
		    AQLObject* le=NULL; 
		    if(!objPool.getObject(name).isDefined())
		    {	
			    le = new AQLObject;
			    objPool.set(name, le);
		    }
		    else
		    {
			    objPool.getObject(name).get().clear();
			    le= &objPool.getObject(name).get();
		    }
		    rate = chgcol(lgrid,CURVEINPUT_RATE,i).getDoubleValue();
		    le->add(CALIBRATION_DATA_NAME,				new AQLDataString(name)							);
		    le->add(IR_CALIBRATION_DATA_TERM,				new AQLDataString(Term)							);
		    le->add(IR_CALIBRATION_DATA_DATATYPE,			new AQLDataString(ZERO)							);
		    le->add(IR_CALIBRATION_DATA_FREQUENCY,		new AQLDataString(SIMPLE)						);
		    if (Currency.toUpper() == "AUD" && Term == "7D")
		    {
			    le->add(IR_CALIBRATION_DATA_DAYCOUNT, new AQLPriceDataDayCount()).convertFromString(AC_360);
		    }
		    else
		    {
			    le->add(IR_CALIBRATION_DATA_DAYCOUNT, new AQLPriceDataDayCount(dc_Libor_attr) );
		    }
		    le->add(CALIBRATION_DATA_CALENDAR,			new AQLPriceDataCalendar(cal_Libor_attr)				);
		    le->add(CALIBRATION_DATA_SLIDINGRULE,			new AQLPriceDataSlidingRule(sr_Libor_attr)			);
		    le->add(IR_CALIBRATION_DATA_SPOTDATE,			new AQLDataDate(spotdate)						);
		    le->add(CALIBRATION_DATA_RATE,				new AQLDataDouble(rate)							);
		    le->add(IR_CALIBRATION_DATA_ISONFORSPOTADJUST,	new AQLDataBool(isOnForSpotAdjust_attr)		);
		    ref+= name + ":";
	    }
	    ////SwapData
	    //spotdate	= LADateScheduleHelpers::getLADate(chgrow(sdata,CURVEINPUT_SPOTDATE,1));
	    itd			= dvar.find(chgrow(sdata,CURVEINPUT_DAYCOUNT,1));
	    if(itd==dvar.end())
		    throw AQLCoreInvalidData("Swap Daycount is not registered ", __FILE__,__LINE__);
	    dc			= itd->second;
	    city		= chgrow(sdata,CURVEINPUT_CALENDAR,1).toToken(':');
        calendar    = chgrow(sdata,CURVEINPUT_CALENDAR,1);
	    srule		= chgrow(sdata,CURVEINPUT_SLIDINGRULE,1);
	    tmprow1= AQLFunctionUtilities::findRowsNumber(ylddata,CURVEINPUT_ISNEWTONRAPHSON);
	    AQLString isNewtonRaphson_str = "FALSE";
	    if (tmprow1 >= 0)
	    {
		     isNewtonRaphson_str = chgrow(ylddata,CURVEINPUT_ISNEWTONRAPHSON,1);
	    }
	    N = sgrid.size();
	    AQLString frequency_base = chgrow(sdata,CURVEINPUT_FREQUENCY,1);
	    AQLPriceDataDayCount dc_Swap_attr; dc_Swap_attr.convertFromString(dc);
	    AQLPriceDataCalendar cal_Swap_attr; cal_Swap_attr.convertFromString(calendar);
	    AQLPriceDataSlidingRule sr_Swap_attr; sr_Swap_attr.convertFromString(srule);
	    AQLDataBool isNewtonRaphson_attr; isNewtonRaphson_attr.convertFromString(isNewtonRaphson_str);

        tmprow1= AQLFunctionUtilities::findRowsNumber(sdata,CURVEINPUT_SPOTLAG);
        tmprow2= AQLFunctionUtilities::findRowsNumber(sdata,CURVEINPUT_SPOTDATE);
        if( tmprow1<0 && tmprow2<0) 
        {
            throw AQLCoreInvalidData("input spot date or spot lag", __FILE__,__LINE__);
        }
	    else if(tmprow1>=0 && tmprow2>=0)
        {
		    throw AQLCoreInvalidData("do not input spot date and spot lag", __FILE__,__LINE__);
	    }
        else if(tmprow1>=0)
        {
            AQLString spotLag = AQLFunctionUtilities::findElement(sdata,CURVEINPUT_SPOTLAG,0,1,true);
            spotdate = LADateScheduleHelpers::getDate(asofdate,spotLag,srule,calendar);
        }
        else if(tmprow2>=0)
        {
            spotdate = LADateScheduleHelpers::getLADate(chgrow(sdata,CURVEINPUT_SPOTDATE,1));
        }

	    //i=row=0 is input row (not grid);
	    bool resetFlg = true;
	    bool tempresetFlg = true;
	    map<AQLString, double> aud_origSwapRate;
	    AQLString frequency;
	    for(int i=1; i<N; i++)
	    {
		    Term = chgcol(sgrid,CURVEINPUT_TERM,i);
		    Term.toUpper();
		    if(isAUDadjust && Currency.toUpper() == "AUD" &&
			    Term.findString("Y") == static_cast<int>(Term.size() - 1 ) &&
				    Term.subString(0, Term.size() - 2).getIntValue() > 3 && resetFlg)
		    {
			    curveCalibrationData->getMarketData().convertFromString(ref.subString(0,ref.size() - 2));
			    dataInstance->getReferencePool().completeDependency();

			    // generate yield data
			    AQLDataProcedure &modelDataObj = dynamic_cast<AQLDataProcedure &>
								    (curveCalibrationData->getData(CALIBRATION_DATA_CURVEGENERATOR, ISNOTNULL).get());

			    dataInstance->getReferencePool().completeDependency();
			    modelDataObj.calibrateModel(asofdate);

			    //		//reset market data as libor
			    resetMarketDataUseL(*curveCalibrationData,Currency,ldata);
			    resetFlg = false;
		    }
		
		    if(!isAUDadjust && Currency.toUpper() == "AUD" &&
			    Term.findString("Y") == static_cast<int>(Term.size() - 1 ) &&
			    Term.subString(0, Term.size() - 2).getIntValue() <= 3)
		    {
			    frequency = QUARTERLY;
		    }
		    else
		    {
			    frequency = frequency_base;
		    }

		    //Term = chgcol(sgrid,CURVEINPUT_TERM,i);
		    AQLObject* se	=	NULL;
		    AQLString name	=	CurveID + Currency + SWAP + Term;
		    if(!objPool.getObject(name).isDefined())
		    {	
			    se = new AQLObject;
			    objPool.set(name, se);
		    }
		    else
		    {
			    objPool.getObject(name).get().clear();
			    se= &objPool.getObject(name).get();
		    }
		    rate = chgcol(sgrid,CURVEINPUT_RATE,i).getDoubleValue();
		    se->add(CALIBRATION_DATA_NAME,				new AQLDataString(name)							);
		    se->add(IR_CALIBRATION_DATA_TERM,				new AQLDataString(Term)							);
		    se->add(IR_CALIBRATION_DATA_DATATYPE,			new AQLDataString(PAR)							);
		    se->add(IR_CALIBRATION_DATA_FREQUENCY,		new AQLDataString(frequency)						);
		    se->add(IR_CALIBRATION_DATA_BASEFREQUENCY_FLOAT,		new AQLDataString(frequency)				);
		    se->add(IR_CALIBRATION_DATA_FREQUENCY_FLOAT,		new AQLDataString(frequency)				);
		    se->add(IR_CALIBRATION_DATA_DAYCOUNT,			new AQLPriceDataDayCount(dc_Swap_attr)				);
		    se->add(CALIBRATION_DATA_CALENDAR,			new AQLPriceDataCalendar(cal_Swap_attr)				);
		    se->add(CALIBRATION_DATA_SLIDINGRULE,			new AQLPriceDataSlidingRule(sr_Swap_attr)				);
		    se->add(IR_CALIBRATION_DATA_SPOTDATE,			new AQLDataDate(spotdate)						);
		    se->add(CALIBRATION_DATA_RATE,				new AQLDataDouble(rate)							);
		    se->add(IR_CALIBRATION_DATA_ISNEWTONRAPHSONSW,	new AQLDataBool(isNewtonRaphson_attr)		);
		    ref+= name + ":";

		    if (isAUDadjust && Currency.toUpper() == "AUD" && resetFlg)
		    {
			    aud_origSwapRate.insert(make_pair(Term, rate));
		    }
		    tempresetFlg = resetFlg;
	    }
	    ////Futuredata
	    if(isfuteruse)
	    {
		    //spotdate	= LADateScheduleHelpers::getLADate(chgrow(fdata,CURVEINPUT_SPOTDATE,1));
		    itd			= dvar.find(chgrow(fdata,CURVEINPUT_DAYCOUNT,1));
		    if(itd==dvar.end())
			    throw AQLCoreInvalidData("Future Daycount is not registered ", __FILE__,__LINE__);
		    dc			= itd->second;
		    city		= chgrow(fdata,CURVEINPUT_CALENDAR,1).toToken(':');
            calendar		= chgrow(fdata,CURVEINPUT_CALENDAR,1);
		    srule		= chgrow(fdata,CURVEINPUT_SLIDINGRULE,1);
		    N			= fgrid.size();

            tmprow1= AQLFunctionUtilities::findRowsNumber(fdata,CURVEINPUT_SPOTLAG);
            tmprow2= AQLFunctionUtilities::findRowsNumber(fdata,CURVEINPUT_SPOTDATE);
            if( tmprow1<0 && tmprow2<0) 
            {
                throw AQLCoreInvalidData("input spot date or spot lag", __FILE__,__LINE__);
            }
		    else if(tmprow1>=0 && tmprow2>=0)
		    {
			    throw AQLCoreInvalidData("do not input spot date and spot lag", __FILE__,__LINE__);
		    }
            else if(tmprow1>=0)
            {
                AQLString spotLag = AQLFunctionUtilities::findElement(fdata,CURVEINPUT_SPOTLAG,0,1,true);
                spotdate = LADateScheduleHelpers::getDate(asofdate,spotLag,srule,calendar);
            }
            else if(tmprow2>=0)
            {
                spotdate = LADateScheduleHelpers::getLADate(chgrow(fdata,CURVEINPUT_SPOTDATE,1));
            }
		    //i=row=0 is input row (not grid);
		    for(int i=1; i<N; i++)
		    {
			    AQLDate	StartDate	=	LADateScheduleHelpers::getLADate(chgcol(fgrid,CURVEINPUT_STARTDATE,	i));
			    AQLDate EndDate		=	LADateScheduleHelpers::getLADate(chgcol(fgrid,CURVEINPUT_ENDDATE,		i));
			    AQLString name		=  CurveID + Currency + FUTURE + AQLDataDate(StartDate).convertToString();
			    AQLObject* fe		=	NULL;
			    if(!objPool.getObject(name).isDefined())
			    {	
				    fe = new AQLObject;
				    objPool.set(name, fe);
			    }
			    else
			    {
				    objPool.getObject(name).get().clear();
				    fe= &objPool.getObject(name).get();
			    }
			    rate = chgcol(fgrid,CURVEINPUT_RATE,i).getDoubleValue();
			    fe->add(CALIBRATION_DATA_NAME,				new AQLDataString(name)							);
			    fe->add(IR_CALIBRATION_DATA_DATATYPE,			new AQLDataString(FUTURE)						);
			    fe->add(IR_CALIBRATION_DATA_FREQUENCY,		new AQLDataString(SIMPLE)						);
			    fe->add(PRICING_DATA_STARTDATE,		new AQLDataDate(StartDate)						);
			    fe->add(PRICING_DATA_ENDDATE,		new AQLDataDate(EndDate)							);
			    fe->add(IR_CALIBRATION_DATA_DAYCOUNT,			new AQLPriceDataDayCount()							).convertFromString(dc);
			    fe->add(CALIBRATION_DATA_CALENDAR,			new AQLPriceDataCalendar(city)						);
			    fe->add(CALIBRATION_DATA_SLIDINGRULE,			new AQLPriceDataSlidingRule()							).convertFromString(srule);
			    fe->add(IR_CALIBRATION_DATA_SPOTDATE,			new AQLDataDate(spotdate)						);
			    fe->add(CALIBRATION_DATA_RATE,				new AQLDataDouble(rate)							);
			    fe->add(PRICING_DATA_FUTUREOLDMETHOD,	new AQLDataBool(true)						);
                fe->add(IR_CALIBRATION_DATA_APPLYTENSION,	    new AQLDataBool(false));
                fe->add(IR_CALIBRATION_DATA_TENSIONGAP,	    new AQLDataInt(1));
			    ref+= name + ":";
		    }
	    }
	    //set market object
	    if (ref.size() < 2)
		    throw AQLCoreInvalidData("Market Data is not set !!", __FILE__, __LINE__); 
	    ref = ref.subString(0, ref.size() - 2);
	    if (isSTD)
	    {
		    curveCalibrationData->getMarketData().convertFromString(ref);
	    }
	    else
	    {
		    curveCalibrationData->AQLObject::add(CALIBRATION_DATA_MARKETDATA + suffix, new AQLDataMultiReference()).convertFromString(ref);
	    }
	    //generate Object
	    dataInstance->getReferencePool().completeDependency();
	    AQLDataHolder* dh		= &objPool.getObject(CurveIDTool).getData(CALIBRATION_DATA_CURVEGENERATOR);
	    AQLDataProcedure& modelDataObj	= dynamic_cast<AQLDataProcedure&>(dh->get());
	    modelDataObj.calibrateModel(asofdate);
	    if (!resetFlg)
		    restoreSwapRateFromLibor(*curveCalibrationData,Currency,aud_origSwapRate,sdata);
	    if (!isSTD)
	    {
		    curveCalibrationData->AQLObject::remove(CALIBRATION_DATA_MARKETDATA + suffix);
	    }
	    if(isCheckDF)
	    {
		    // check df
		    DoubleArray terms = dynamic_cast<AQLDataDoubles &>(e->getData(CALIBRATION_DATA_TERMS, ISNOTNULL).get()).get();
		    DoubleArray dfs = dynamic_cast<AQLDataDoubles &>(e->getData(IR_CALIBRATION_DATA_DFS, ISNOTNULL).get()).get();

		    DoubleArray vals = dfs;
		    double b_df = 1.0;
		    double b_term = 0.0;
		    const unsigned int size = terms.size();
		    for (unsigned int i = 0; i < size; ++i)
		    {
			    if  (b_df < dfs[i] && i > 0)
			    {
				    DoubleArray tmpTerms(1, b_term);
				    DoubleArray tmpDfs(1, b_df);

				    unsigned int j = i;
				    for (; j < size && b_df < dfs[j]; ++j);

				    if (j != i && dfs[j] <= b_df)
				    {
					    tmpTerms.push_back(terms[j]);
					    tmpDfs.push_back(dfs[j]);
				    }

				    AQLLinearInterpolation interp = AQLLinearInterpolation();
				    interp.set(tmpTerms, tmpDfs);
				    vals[i] = interp.value(terms[i]);
			    }
			    else
			    {
				    b_df = dfs[i];
				    b_term = terms[i];
			    }
		    }
		    dynamic_cast<AQLDataDoubles &>(e->getData(IR_CALIBRATION_DATA_DFS, ISNOTNULL).get()).set(vals);
	    }

	    //curve entities manager (curve, grids, curveinformation entities)
	    AQLObject* mae = NULL;
	    if(!objPool.getObject(CurveIDManager).isDefined())
	    {	
		    mae = new AQLObject;
		    objPool.set(CurveIDManager,mae);
	    }
	    else
	    {
		    objPool.getObject(CurveIDManager).get().clear();
		    mae	= &objPool.getObject(CurveIDManager).get();
	    }
	    mae->add("Time",			new AQLDataString()			).convertFromString(AQLString(AQLTime::now()));
	    mae->add(CALIBRATION_DATA_NAME,		new AQLDataString()			).convertFromString(CurveIDManager);
	    mae->add(GRIDREFS,			new AQLDataMultiReference()	).convertFromString(ref);
	    mae->add(YIELDDATAREF,		new AQLDataReference()		).convertFromString(CurveID);
	    mae->add(YIELDTOOLREF,		new AQLDataReference()		).convertFromString(CurveIDTool);
    }

    void 
    LACurveCalibrationHelpers::setUpXccyBasisCurve(AQLDataInstance* dataInstance,const AQLString& BCurveID, 
						    AQLStringMatrix ylddata, AQLStringMatrix bgrid, AQLStringMatrix bdata)
    {
	    //ylddata
	    AQLString	CurveIDTool			= BCurveID + TOOL;
	    AQLString	CurveIDManager		= BCurveID + STD + MANAGER;

	    upper(ylddata);
	    upper(bgrid);
	    upper(bdata);

	    //basis market data
	    AQLString bref;
	    AQLString term;
        AQLDate spotdate;
	    //AQLDate spotdate		= LADateScheduleHelpers::getLADate(chgrow(bdata,CURVEINPUT_SPOTDATE,1));		
	    AQLString daycount(AC_365I);

	    //BasisType
	    std::map<AQLString, AQLString>& bvar = AQLCoreComponentManager::getBasisTypeMap();
	    std::map<AQLString, AQLString>::iterator itb;
	    itb = bvar.find(chgrow(bdata,CURVEINPUT_BASISTYPE,1));
	    if(itb == bvar.end())
		    throw AQLCoreInvalidData("BasisType is not registered ", __FILE__,__LINE__);
	    AQLString basistype = itb->second;

	    //daycount
	    std::map<AQLString, AQLString>& dvar = AQLCoreComponentManager::getDayCountMap();
	    std::map<AQLString, AQLString>::iterator itd;
	    itd = dvar.find(chgrow(bdata,CURVEINPUT_BASISDAYCOUNT,1));
	    if(itd == dvar.end())
	    {
		    itd = dvar.find(chgrow(bdata,CURVEINPUT_BASISDAYCOUNT1,1));
		    if(itd == dvar.end())
			    throw AQLCoreInvalidData("basisDaycount is not registered ", __FILE__,__LINE__);
	    }
	    AQLString bdaycount = itd->second;
	
	    AQLString bcalendar	= chgrow(bdata,CURVEINPUT_BASISCALENDAR,1);
	    if(bcalendar=="")
		    bcalendar		= chgrow(bdata,CURVEINPUT_BASISCALENDAR1, 1);
	    AQLString bsrule		= chgrow(bdata,CURVEINPUT_BASISSLIDINGRULE,1);
	    if(bsrule=="")
		    bsrule			= chgrow(bdata,CURVEINPUT_BASISSLIDINGRULE1, 1);
	    AQLString bfreq		= chgrow(bdata,CURVEINPUT_BASISFREQUENCY,1);
	    if(bfreq=="")
		    bfreq			= chgrow(bdata,CURVEINPUT_BASISFREQUENCY1, 1);

	    AQLPriceDataDayCount dc_Basis_attr; dc_Basis_attr.convertFromString(daycount);
	    AQLPriceDataDayCount bdc_Basis_attr; bdc_Basis_attr.convertFromString(bdaycount);
	    AQLPriceDataCalendar cal_Basis_attr; cal_Basis_attr.convertFromString(bcalendar);
	    AQLPriceDataSlidingRule sr_Basis_attr; sr_Basis_attr.convertFromString(bsrule);

        int tmprow1= AQLFunctionUtilities::findRowsNumber(bdata,CURVEINPUT_SPOTLAG);
        int tmprow2= AQLFunctionUtilities::findRowsNumber(bdata,CURVEINPUT_SPOTDATE);
        const AQLDate asofdate = dynamic_cast<const AQLDataDate& >(dataInstance->getObjectPool().getObject(BCurveID,ENCHKTYPE_ISDEFINED).
                                    get().getData(CALIBRATION_DATA_ASOFDATE,ISNOTNULL).get()).get();
        if( tmprow1<0 && tmprow2<0) 
        {
            throw AQLCoreInvalidData("input spot date or spot lag", __FILE__,__LINE__);
        }
        else if(tmprow1>=0)
        {
            AQLString spotLag = AQLFunctionUtilities::findElement(bdata,CURVEINPUT_SPOTLAG,0,1,true);
            spotdate = LADateScheduleHelpers::getDate(asofdate,spotLag,bsrule,bcalendar);
        }
        else
        {
            spotdate = LADateScheduleHelpers::getLADate(chgrow(bdata,CURVEINPUT_SPOTDATE,1));
        }

	    //Interpolation
	    std::map<AQLString, AQLString>& ivar = AQLCoreComponentManager::getInterpolationMap();
	    std::map<AQLString, AQLString>::iterator iti;
	    iti = ivar.find(chgrow(ylddata,CURVEINPUT_INTERPOLATION,1));
	    if(iti == ivar.end())
		    throw AQLCoreInvalidData("Interpolation is not registered ", __FILE__,__LINE__);
	    AQLString interpolation = iti->second;
	
	    AQLObjectPool& objPool = dataInstance->getObjectPool(); 
	    //first check isYieldCurveIDexist
	    if(!objPool.getObject(BCurveID).isDefined())
		    throw AQLCoreInvalidData("CurveID does not exsist",__FILE__,__LINE__);


	    int N=bgrid.size();
	    double rate =0.0;
	    for(int i=1;i<N;i++)
	    {
		    term = chgcol(bgrid,CURVEINPUT_TERM,i);
		    AQLString name = BCurveID + BASIS + term;
		    AQLObject* be = NULL;
		    if(!objPool.getObject(name).isDefined())
		    {
			    be = new AQLObject;
			    objPool.set(name,be);
		    }
		    else
		    {
			    objPool.getObject(name).get().clear();
			    be = &objPool.getObject(name).get();
		    }
		    rate = chgcol(bgrid,CURVEINPUT_RATE,i).getDoubleValue();
		    be->add(CALIBRATION_DATA_NAME,				new AQLDataString(name)				);
		    be->add(IR_CALIBRATION_DATA_TERM,				new AQLDataString(term)				);
		    be->add(IR_CALIBRATION_DATA_DATATYPE,			new AQLDataString("BasisRate")		);
		    be->add(CALIBRATION_DATA_RATE,				new AQLDataDouble(rate)				);
		    be->add(IR_CALIBRATION_DATA_FREQUENCYBASE,	new AQLDataString(bfreq)				);
		    be->add(IR_CALIBRATION_DATA_DAYCOUNTBASE,		new AQLPriceDataDayCount(bdc_Basis_attr)	);
		    be->add(IR_CALIBRATION_DATA_CALENDARBASE,		new AQLPriceDataCalendar(cal_Basis_attr)	);
		    be->add(IR_CALIBRATION_DATA_SLIDINGRULEBASE,	new AQLPriceDataSlidingRule(sr_Basis_attr));
		    be->add(IR_CALIBRATION_DATA_DAYCOUNT,			new AQLPriceDataDayCount(dc_Basis_attr)	);
		    be->add(IR_CALIBRATION_DATA_SPOTDATE,			new AQLDataDate(spotdate)			);
		    bref+= name+":";
	    }

	    //LACurvePricingObject
	    AQLString yldname =  BCurveID+ "FROMMAKINGBASISCURVE";
	    const AQLObjectHolder ehyld = objPool.getObject(yldname);
	    LACurvePricingObject* ycp = NULL;
	    if(!ehyld.isDefined())
	    {
		    ycp = new LACurvePricingObject(dataInstance);
		    objPool.set(yldname,ycp);
	    }
	    else
	    {
		    dynamic_cast<LACurvePricingObject &>(objPool.getObject(yldname).get()).reset();
		    ycp = &dynamic_cast<LACurvePricingObject &>(objPool.getObject(yldname).get());
	    }
	    ycp->getName().convertFromString(yldname);
	    ycp->getInterpolation().convertFromString(interpolation);
	    AQLString clonename = BCurveID + "Clone";
	    const AQLObjectHolder ehclo = objPool.getObject(clonename);
	    AQLObject* clonecurve = NULL;
	    if(!ehclo.isDefined())
	    {
		    clonecurve = objPool.getObject(BCurveID,ENCHKTYPE_ISDEFINED).get().clone();
		    objPool.set(clonename,clonecurve);
	    }
	    else
	    {
		    objPool.remove(clonename);
		    clonecurve = objPool.getObject(BCurveID,ENCHKTYPE_ISDEFINED).get().clone();
		    objPool.set(clonename,clonecurve);

	    }
	    ycp->getYieldData().convertFromString(clonename);


	    //AQLMathYieldCurvePro
	    const AQLObjectHolder ehCur = objPool.getObject(CurveIDTool);
	    AQLMathYieldCurvePro * curveCalibrationData	=NULL; 
	    if(!ehCur.isDefined())
	    {	
		    curveCalibrationData= new AQLMathYieldCurvePro(dataInstance);
		    objPool.set(CurveIDTool,curveCalibrationData);
	    }
	    else
	    {
		    //dynamic_cast<LACurvePricingObject &>(objPool.getObject(CurveIDTool).get()).reset();
		    curveCalibrationData	= &dynamic_cast<AQLMathYieldCurvePro &>(objPool.getObject(CurveIDTool).get());
	    }
	
	    AQLString ref = bref.subString(0,bref.size()-2);
	    curveCalibrationData->getYieldData().convertFromString(BCurveID);
	    curveCalibrationData->getBaseYieldCurve().convertFromString(yldname);
	    curveCalibrationData->getInterpolation_bs().convertFromString(FN_SPLINEINTERPOLATION_STR);
	    curveCalibrationData->getInterpolation().convertFromString(FN_SPLINEINTERPOLATION_STR);
	    curveCalibrationData->getDayCount().convertFromString(AC_365I);
	    AQLDataMultiReference &refMarket = curveCalibrationData->getMarketData();
	    if (!refMarket.isNull())
	    {
		    AQLString n_ref = "";
		    unsigned int mSize = refMarket.getSize();
		    for (unsigned int i = 0; i < mSize; ++i)
		    {
			    AQLString type = dynamic_cast<const AQLDataString&>((refMarket.get(i).getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL)).get()).get();
			    if (type.toUpper() != "BASISRATE")
			    {
				    n_ref += refMarket.get(i).getName() + ":";
			    }
		    }
		    n_ref += ref;
		    curveCalibrationData->getMarketData().convertFromString(n_ref);
	    }
	    else
	    {
		    curveCalibrationData->getMarketData().convertFromString(ref);
	
	    }
	    dataInstance->getReferencePool().completeDependency();
	    curveCalibrationData->setBasisRates();
    }

    void 
    LACurveCalibrationHelpers::setUpBasisCurve(AQLDataInstance* dataInstance,
                                                  const AQLString& stdCurveID, 
                                                  const AQLString& basisCurveID, 
                                                  AQLStringMatrix ylddata, 
                                                  AQLStringMatrix bgrid, 
                                                  AQLStringMatrix bdata)
    {
	    //ylddata
	    AQLString	CurveIDTool			= stdCurveID + TOOL;

	    upper(ylddata);
	    upper(bgrid);
	    upper(bdata);

	    //basis market data
	    AQLString bref;
	    AQLString term;
        AQLDate spotdate;		
	    AQLString daycount(AC_365I);

	    //BasisType
	    std::map<AQLString, AQLString>& bvar = AQLCoreComponentManager::getBasisTypeMap();
	    std::map<AQLString, AQLString>::iterator itb;
	    itb = bvar.find(chgrow(bdata,CURVEINPUT_BASISTYPE,1));
	    if(itb == bvar.end())
		    throw AQLCoreInvalidData("BasisType is not registered ", __FILE__,__LINE__);
	    AQLString basistype = itb->second;

	    //daycount
	    std::map<AQLString, AQLString>& dvar = AQLCoreComponentManager::getDayCountMap();
	    std::map<AQLString, AQLString>::iterator itd;
	    itd = dvar.find(chgrow(bdata,CURVEINPUT_BASISDAYCOUNT,1));
	    if(itd == dvar.end())
	    {
		    itd = dvar.find(chgrow(bdata,CURVEINPUT_BASISDAYCOUNT1,1));
		    if(itd == dvar.end())
			    throw AQLCoreInvalidData("basisDaycount is not registered ", __FILE__,__LINE__);
	    }
	    AQLString bdaycount = itd->second;
	
	    AQLString bcalendar	= chgrow(bdata,CURVEINPUT_BASISCALENDAR,1);
	    if(bcalendar=="")
		    bcalendar		= chgrow(bdata,CURVEINPUT_BASISCALENDAR1, 1);
	    AQLString bsrule		= chgrow(bdata,CURVEINPUT_BASISSLIDINGRULE,1);
	    if(bsrule=="")
		    bsrule			= chgrow(bdata,CURVEINPUT_BASISSLIDINGRULE1, 1);
	    AQLString bfreq		= chgrow(bdata,CURVEINPUT_BASISFREQUENCY,1);
	    if(bfreq=="")
		    bfreq			= chgrow(bdata,CURVEINPUT_BASISFREQUENCY1, 1);

        int tmprow1= AQLFunctionUtilities::findRowsNumber(bdata,CURVEINPUT_SPOTLAG);
        int tmprow2= AQLFunctionUtilities::findRowsNumber(bdata,CURVEINPUT_SPOTDATE);
        const AQLDate asofdate = dynamic_cast<const AQLDataDate& >(dataInstance->getObjectPool().getObject(stdCurveID,ENCHKTYPE_ISDEFINED).
                                    get().getData(CALIBRATION_DATA_ASOFDATE,ISNOTNULL).get()).get();
        if( tmprow1<0 && tmprow2<0) 
        {
            throw AQLCoreInvalidData("input spot date or spot lag", __FILE__,__LINE__);
        }
        else if(tmprow1>=0)
        {
            AQLString spotLag = AQLFunctionUtilities::findElement(bdata,CURVEINPUT_SPOTLAG,0,1,true);
            spotdate = LADateScheduleHelpers::getDate(asofdate,spotLag,bsrule,bcalendar);
        }
        else
        {
            spotdate = LADateScheduleHelpers::getLADate(chgrow(bdata,CURVEINPUT_SPOTDATE,1));
        }

	    //Interpolation
	    std::map<AQLString, AQLString>& ivar = AQLCoreComponentManager::getInterpolationMap();
	    std::map<AQLString, AQLString>::iterator iti;
	    iti = ivar.find(chgrow(ylddata,CURVEINPUT_INTERPOLATION,1));
	    if(iti == ivar.end())
		    throw AQLCoreInvalidData("Interpolation is not registered ", __FILE__,__LINE__);
	    AQLString interpolation = iti->second;
	
	    AQLObjectPool& objPool = dataInstance->getObjectPool(); 
	    //first check isYieldCurveIDexist
	    if(!objPool.getObject(stdCurveID).isDefined())
		    throw AQLCoreInvalidData("CurveID does not exsist",__FILE__,__LINE__);


	    int N=bgrid.size();
	    double rate =0.0;
	    for(int i=1;i<N;i++)
	    {
		    term = chgcol(bgrid,CURVEINPUT_TERM,i);
		    AQLString name = stdCurveID + BASIS + term;
		    AQLObject* be = NULL;
		    if(!objPool.getObject(name).isDefined())
		    {
			    be = new AQLObject;
			    objPool.set(name,be);
		    }
		    else
		    {
			    objPool.getObject(name).get().clear();
			    be = &objPool.getObject(name).get();
		    }
		    rate = chgcol(bgrid,CURVEINPUT_RATE,i).getDoubleValue();
		    be->add(CALIBRATION_DATA_NAME,				new AQLDataString(name)				);
		    be->add(IR_CALIBRATION_DATA_TERM,				new AQLDataString(term)				);
		    be->add(IR_CALIBRATION_DATA_DATATYPE,			new AQLDataString("BasisRate")		);
		    be->add(CALIBRATION_DATA_RATE,				new AQLDataDouble(rate)				);
		    be->add(IR_CALIBRATION_DATA_FREQUENCYBASE,	new AQLDataString(bfreq)				);
		    be->add(IR_CALIBRATION_DATA_DAYCOUNTBASE,		new AQLPriceDataDayCount()				).convertFromString(bdaycount);
		    be->add(IR_CALIBRATION_DATA_CALENDARBASE,		new AQLPriceDataCalendar()				).convertFromString(bcalendar);
		    be->add(IR_CALIBRATION_DATA_SLIDINGRULEBASE,	new AQLPriceDataSlidingRule()				).convertFromString(bsrule);
		    be->add(IR_CALIBRATION_DATA_DAYCOUNT,			new AQLPriceDataDayCount()				).convertFromString(daycount);
		    be->add(IR_CALIBRATION_DATA_SPOTDATE,			new AQLDataDate(spotdate)			);
		    bref+= name+":";
	    }

	    //LACurvePricingObject
	    AQLString yldname =  stdCurveID+ "FROMMAKINGBASISCURVE";
	    const AQLObjectHolder ehyld = objPool.getObject(yldname);
	    LACurvePricingObject* ycp = NULL;
	    if(!ehyld.isDefined())
	    {
		    ycp = new LACurvePricingObject(dataInstance);
		    objPool.set(yldname,ycp);
	    }
	    else
	    {
		    dynamic_cast<LACurvePricingObject &>(objPool.getObject(yldname).get()).reset();
		    ycp = &dynamic_cast<LACurvePricingObject &>(objPool.getObject(yldname).get());
	    }
	    ycp->getName().convertFromString(yldname);
	    ycp->getInterpolation().convertFromString(interpolation);
	    ycp->getYieldData().convertFromString(stdCurveID);

	    //AQLMathYieldCurvePro
	    const AQLObjectHolder ehCur = objPool.getObject(CurveIDTool);
	    AQLMathYieldCurvePro * curveCalibrationData	=NULL; 
	    if(!ehCur.isDefined())
	    {	
		    curveCalibrationData= new AQLMathYieldCurvePro(dataInstance);
		    objPool.set(CurveIDTool,curveCalibrationData);
	    }
	    else
	    {
		    curveCalibrationData	= &dynamic_cast<AQLMathYieldCurvePro &>(objPool.getObject(CurveIDTool).get());
	    }
	
	    AQLString ref = bref.subString(0,bref.size()-2);
	    curveCalibrationData->getYieldData().convertFromString(stdCurveID);
	    curveCalibrationData->getBaseYieldCurve().convertFromString(yldname);
	    curveCalibrationData->getInterpolation_bs().convertFromString(FN_SPLINEINTERPOLATION_STR);
	    curveCalibrationData->getInterpolation().convertFromString(FN_SPLINEINTERPOLATION_STR);
	    curveCalibrationData->getDayCount().convertFromString(AC_365I);
	    AQLDataMultiReference &refMarket = curveCalibrationData->getMarketData();
	    if (!refMarket.isNull())
	    {
		    AQLString n_ref = "";
		    unsigned int mSize = refMarket.getSize();
		    for (unsigned int i = 0; i < mSize; ++i)
		    {
			    AQLString type = dynamic_cast<const AQLDataString&>((refMarket.get(i).getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL)).get()).get();
			    if (type.toUpper() != "BASISRATE")
			    {
				    n_ref += refMarket.get(i).getName() + ":";
			    }
		    }
		    n_ref += ref;
		    curveCalibrationData->getMarketData().convertFromString(n_ref);
	    }
	    else
	    {
		    curveCalibrationData->getMarketData().convertFromString(ref);
	
	    }
	    dataInstance->getReferencePool().completeDependency();
	    curveCalibrationData->setBasisRates2(basisCurveID);
    }

    void 
    LACurveCalibrationHelpers::setUpBasisCurveFromMarket(AQLDataInstance* dataInstance,const AQLString& CurveID, 
						    AQLStringMatrix mgrid, AQLStringMatrix lgrid, AQLStringMatrix fgrid, 
						    AQLStringMatrix sgrid,AQLStringMatrix mdata, AQLStringMatrix ldata, 
						    AQLStringMatrix fdata, AQLStringMatrix sdata,AQLStringMatrix ylddata,
						    AQLStringMatrix bgrid,AQLStringMatrix bdata,bool isCheckDF, bool isAUDadjust)
    {
	    LACurveCalibrationHelpers::setUpCurveFromMarket(dataInstance,CurveID,mgrid,lgrid,fgrid,sgrid,mdata,ldata,fdata,sdata,ylddata,isCheckDF,isAUDadjust);
	    LACurveCalibrationHelpers::setUpXccyBasisCurve(dataInstance,CurveID,ylddata,bgrid,bdata);
    }

    void  
    LACurveCalibrationHelpers::
    setUpArbFreeCurveFromMarket( AQLDataInstance* dataInstance, const AQLString& CurveID, 
                                 AQLStringMatrix monGrid, AQLStringMatrix monConv, //money market
                                 AQLStringMatrix libGrid, AQLStringMatrix libConv, //libor market
                                 AQLStringMatrix swapGrid, AQLStringMatrix swapConv, //swap market
                                 AQLStringMatrix currGrid, AQLStringMatrix currConv, //currency basis swap market
                                 AQLStringMatrix libBasisGrid, AQLStringMatrix libBasisConv, //3M6M libor basis swap market
                                 AQLStringMatrix fraGrid3M, AQLStringMatrix fraGrid6M, AQLStringMatrix fraConv, //FRA market
                                 AQLStringMatrix yldData, AQLString& warning )
    {
	    //ylddata
	    AQLString	CurveIDTool			= CurveID + TOOL;
	    AQLString	CurveIDManager		= CurveID + STD + MANAGER;

	    AQLObjectPool& objPool = dataInstance->getObjectPool(); 
	    const AQLObjectHolder objHolder = objPool.getObject(CurveIDManager,ENCHKTYPE_NOCHECK);
	
	    //curve yielddata clear;
	    if(objPool.getObject(CurveID).isDefined())
	    {
		    objPool.getObject(CurveID).clear();
	    }
	
        AQLString Currency = chgrow(yldData,"Currency",	1);
        AQLString parCurve = chgrow(yldData,"ParCurve", 1);
	    AQLString usdCurve;
	    bool isRenAdj = false;
	    if(Currency != CURRENCY_JPY && parCurve != CURRENCY_USD) 
        {
		    int tmprow1= AQLFunctionUtilities::findRowsNumber(yldData,"isRenotionalAdjust");
		    if(tmprow1>=0)
		    {
			    AQLString isRenAdj_str = chgrow(yldData,"isRenotionalAdjust", 1);
			    if(isRenAdj_str=="TRUE") isRenAdj = true;
		    }

		    if(isRenAdj && Currency != CURRENCY_USD) usdCurve = chgrow(yldData,"USDCurve", 1);
	    }
  
	    //str uppea and delete space
	    upper(monGrid); upper(monConv);
        upper(libGrid); upper(libConv);
        upper(swapGrid); upper(swapConv);
        upper(currGrid); upper(currConv);
        upper(libBasisGrid); upper(libBasisConv);
        upper(fraGrid3M);upper(fraGrid6M);upper(fraConv);
	    upper(yldData);
	
	    AQLDate asofdate = LADateScheduleHelpers::getLADate(chgrow(yldData,CURVEINPUT_ASOFDATE,1));
        AQLString isFRAUSE_str = chgrow(yldData,"ISFRAUSE",1);
        bool isFRAUse = false;
        if( isFRAUSE_str == "TRUE" ) 
	    {
		    isFRAUse = true;
		    if(1>fraGrid3M.size() || 1>fraGrid6M.size())
			    throw AQLCoreInvalidData("Input FRA Data",__FILE__,__LINE__);
		    if(2>fraGrid3M[0].size() || 2>fraGrid6M[0].size())
			    throw AQLCoreInvalidData("Matrix column size must be 2",__FILE__,__LINE__);
		    if(fraConv.empty())
			    throw AQLCoreInvalidData("Input Matrix is empty",__FILE__,__LINE__);
	    }

        //intertplation 
	    std::map<AQLString, AQLString>& ivar = AQLCoreComponentManager::getInterpolationMap();
        std::map<AQLString, AQLString>::iterator it = ivar.find(chgrow(yldData,CURVEINPUT_INTERPOLATION,1			));
	    if(it==ivar.end())
		    throw AQLCoreInvalidData("Interpolation is not registered in interpolation methods",			__FILE__,__LINE__);
	    AQLString interpolation = it->second;

	    //YieldData
	    AQLObject* e = NULL;
	    if(!objPool.getObject(CurveID).isDefined())
	    {	
		    e = new AQLObject;
		    objPool.set(CurveID,e);
	    }
	    else
	    {
		    objPool.getObject(CurveID).get().clear();
		    e= &objPool.getObject(CurveID).get();
	    }
	    e->add(CALIBRATION_DATA_NAME, new AQLDataString(CurveID));
        e->add(IR_CALIBRATION_DATA_CURRENCY, new AQLDataString(Currency));
        e->add(IR_CALIBRATION_DATA_PARCURVE, new AQLDataString(parCurve));

	    //CurveCalibrationData
	    const AQLObjectHolder ehCur = objPool.getObject(CurveIDTool);
	    AQLMathYieldCurvePro * curveCalibrationData	=NULL; 
	    if(!ehCur.isDefined())
	    {	
		    curveCalibrationData= new AQLMathYieldCurvePro(dataInstance);
		    objPool.set(CurveIDTool,curveCalibrationData);
	    }
	    else
	    {
		    dynamic_cast<AQLMathYieldCurvePro &>(objPool.getObject(CurveIDTool).get()).reset();
		    curveCalibrationData	= &dynamic_cast<AQLMathYieldCurvePro &>(objPool.getObject(CurveIDTool).get());
	    }
	    curveCalibrationData->getName().convertFromString(CurveIDTool);

	    /*if (Currency == CURRENCY_JPY && (parCurve == "JPY3ML" || parCurve == "JPY6ML" ))
	    {
		    curveCalibrationData->getData(CALIBRATION_DATA_CURVEGENERATOR, NOCHECK).convertFromString(FN_IRARBFREEGENERATOR_JPYBASE_STR);
	    }
	    else
	    {*/
	    curveCalibrationData->getData(CALIBRATION_DATA_CURVEGENERATOR, NOCHECK).convertFromString(FN_IRARBFREEGENERATOR_STR);
	    //}

	    curveCalibrationData->getIsFRAUse().set(isFRAUse);
	    curveCalibrationData->getIsFutureUse().set(false);
        curveCalibrationData->getName().convertFromString(CurveIDTool);
	    curveCalibrationData->getAsOfDate().set(asofdate);
	    curveCalibrationData->getYieldData().convertFromString(CurveID);
        curveCalibrationData->setInterpolation(interpolation);
	    curveCalibrationData->getDayCount().convertFromString(AC_365I);
	    curveCalibrationData->AQLObject::add(IR_CALIBRATION_DATA_ISRENOTIONALADJUST, new AQLDataBool(isRenAdj));
		curveCalibrationData->AQLObject::add(IR_CALIBRATION_DATA_ISXCCYMARKEDTOMARKET, new AQLDataBool(isRenAdj)); // Alias for renotional adjust

	    AQLStringVector curveNames_6ML;curveNames_6ML.push_back("6ML");
	    AQLStringVector curveNames_3ML;curveNames_3ML.push_back("3ML");
	    AQLStringVector curveNames_DF;curveNames_DF.push_back("DF");
	    curveCalibrationData->setArbFreeCurveName(curveNames_6ML,curveNames_DF,curveNames_3ML);
	    e->add(IR_CALIBRATION_DATA_6MLCURVENAMES, new AQLDataStrings(curveNames_6ML));
	    e->add(IR_CALIBRATION_DATA_DFCURVENAMES, new AQLDataStrings(curveNames_DF));
	    e->add(IR_CALIBRATION_DATA_3MLCURVENAMES, new AQLDataStrings(curveNames_3ML));
	
	    //daycount
	    std::map<AQLString, AQLString>& dvar = AQLCoreComponentManager::getDayCountMap();
	    std::map<AQLString, AQLString>::iterator itd,itd2;
	
	    AQLString name, type;
	    AQLDate spotdate;
	    AQLString ref;
	    AQLStringVector term;
        DoubleArray rate;
	    AQLString srule;
	    //Furuya
	    AQLString lrule;
	    AQLString lcalendar;
	    AQLDate lspotdate;
	    int ltmprow1,ltmprow2;
	    //
	    AQLStringVector city;
        AQLString calendar;
	    AQLString dc,dc2;
	    int N=0;
	    //Money Market
	    itd			= dvar.find(chgrow(monConv,CURVEINPUT_DAYCOUNT,1));
	    if(itd==dvar.end())
		    throw AQLCoreInvalidData("Money Market Daycount is not registered ", __FILE__,__LINE__);
	    dc			= itd->second;
	    city		= chgrow(monConv,CURVEINPUT_CALENDAR,1).toToken(':');
        calendar    = chgrow(monConv,CURVEINPUT_CALENDAR,1);
	    srule		= chgrow(monConv,CURVEINPUT_SLIDINGRULE,1);

        int tmprow1,tmprow2;
	    term = searchvecbycol(monGrid,CURVEINPUT_TYPE,true);
	    for (size_t i=0; i<2; i++)
	    {
		    if (term[i] == "O_N") 
		    {
			    term[i] = "ON";
			    name = Currency + MONEY + "ON";
			    type = "O_N";
		    }
		    if (term[i] == "T_N")
		    {
			    term[i] = "TN";
			    name = Currency + MONEY + "TN";
			    type = "T_N";
		    }

		    AQLObject* mme = NULL;
		
		    if(!objPool.getObject(name).isDefined())
		    {	
			    mme = new AQLObject;
			    objPool.set(name, mme);
		    }
		    else
		    {
			    objPool.getObject(name).get().clear();
			    mme= &objPool.getObject(name).get();
		    }

		    rate = AQLCoreUtility::changeDoubleFromString(searchvecbycol(monGrid,CURVEINPUT_RATE,true));
		    mme->add(CALIBRATION_DATA_NAME,				new AQLDataString(name)							);
		    mme->add(IR_CALIBRATION_DATA_DATATYPE,		new AQLDataString(type)							);
		    mme->add(IR_CALIBRATION_DATA_TERM,    		new AQLDataString(term[i])							);
		    mme->add(IR_CALIBRATION_DATA_DAYCOUNT,		new AQLPriceDataDayCount()							).convertFromString(dc);
		    mme->add(CALIBRATION_DATA_CALENDAR,			new AQLPriceDataCalendar(city)						);
		    mme->add(CALIBRATION_DATA_SLIDINGRULE,		new AQLPriceDataSlidingRule()							).convertFromString(srule);
		    mme->add(CALIBRATION_DATA_RATE,				new AQLDataDouble(rate[i])							);
		    ref += name + ":";
	    }

	    ////LiborData
	    itd			= dvar.find(chgrow(libConv,CURVEINPUT_DAYCOUNT,1));
	    if(itd==dvar.end())
		    throw AQLCoreInvalidData("Libor Daycount is not registered ", __FILE__,__LINE__);
	    AQLString ldc			= itd->second;
	    const AQLStringVector term_tmp = searchvecbycol(libGrid,CURVEINPUT_TERM,true);
        const AQLStringVector rate_str = searchvecbycol(libGrid,CURVEINPUT_RATE,true);
	    /*Furuya*/
	    lcalendar    = chgrow(swapConv,CURVEINPUT_CALENDAR,1);
	    lrule		= chgrow(swapConv,CURVEINPUT_SLIDINGRULE,1);
	    AQLPriceDataCalendar calL;
	    calL.convertFromString(calendar);
	    ltmprow1= AQLFunctionUtilities::findRowsNumber(swapConv,CURVEINPUT_SPOTLAG);
        ltmprow2= AQLFunctionUtilities::findRowsNumber(swapConv,CURVEINPUT_SPOTDATE);
        if( ltmprow1<0 && ltmprow2<0 ) 
        {
            throw AQLCoreInvalidData("input spot date or spot lag", __FILE__,__LINE__);
        }
	    else if(ltmprow1>=0 && ltmprow2>=0)
        {
		    throw AQLCoreInvalidData("do not input spot date and spot lag", __FILE__,__LINE__);
	    }
        else if(ltmprow1>=0)
        {
            AQLString lspotLag = AQLFunctionUtilities::findElement(swapConv,CURVEINPUT_SPOTLAG,0,1,true);
		    lspotdate = LADateScheduleHelpers::getDate(asofdate,lspotLag,lrule,lcalendar);
        }
        else if(ltmprow2>=0)
        {
            lspotdate = LADateScheduleHelpers::getLADate(chgrow(swapConv,CURVEINPUT_SPOTDATE,1));
        }
	    /**/

	    //set three month libor
	    const int row_3M = AQLCoreUtility::findRowsNumber(libGrid,"3M");
	    if( row_3M<0 ) throw AQLCoreInvalidData("3 month libor does not exist.", __FILE__,__LINE__);
	    //set six month libor
	    const int row_6M = AQLCoreUtility::findRowsNumber(libGrid,"6M");
	    if( row_6M<0 ) throw AQLCoreInvalidData("6 month libor does not exist.", __FILE__,__LINE__);

	    for (size_t i=0; i<term_tmp.size(); i++)
	    {
		    //e->add("LiborDaycount", new AQLPriceDataDayCount() ).convertFromString(ldc);
		    if (term_tmp[i] != "3M" && term_tmp[i] != "6M") continue;

		    name = Currency + LIBOR + term_tmp[i];
		    AQLObject* le=NULL; 
		    if(!objPool.getObject(name).isDefined())
		    {	
			    le = new AQLObject;
			    objPool.set(name, le);
		    }
		    else
		    {
			    objPool.getObject(name).get().clear();
			    le= &objPool.getObject(name).get();
		    }
	    
		    /*double threeMLibor = rate_str[row_3M-1].getDoubleValue();
		    rate = DoubleArray(1, threeMLibor);
		
		    double sixMLibor = rate_str[row_6M-1].getDoubleValue();
		    rate.push_back( sixMLibor );

		    term = AQLStringVector( 1, term_tmp[row_3M-1] );
		    term.push_back( term_tmp[row_6M-1] );*/

		    le->add(CALIBRATION_DATA_NAME,				new AQLDataString(name)							);
		    le->add(IR_CALIBRATION_DATA_TERM,    		    new AQLDataString(term_tmp[i])							);
		    le->add(IR_CALIBRATION_DATA_DAYCOUNT,			new AQLPriceDataDayCount()							).convertFromString(ldc);
		    le->add(CALIBRATION_DATA_RATE,				new AQLDataDouble(rate_str[i].getDoubleValue())	);
		    le->add(IR_CALIBRATION_DATA_DATATYPE,			new AQLDataString(ZERO)							);
		    //Furuya 
		    le->add(CALIBRATION_DATA_CALENDAR,			new AQLPriceDataCalendar(calL)						);
		    le->add(CALIBRATION_DATA_SLIDINGRULE,			new AQLPriceDataSlidingRule()							).convertFromString(lrule);
		    le->add(IR_CALIBRATION_DATA_SPOTDATE,			new AQLDataDate(lspotdate)						);
		    ref+= name + ":";
	    }

	    e->add("LiborDaycount", new AQLPriceDataDayCount()).convertFromString(ldc);

	    ////SwapData
	    itd			= dvar.find(chgrow(swapConv,"DAYCOUNTFIX",1));
        itd2		= dvar.find(chgrow(swapConv,"DAYCOUNTFLOAT",1));
	    if ( itd==dvar.end() && itd2==dvar.end() ) 
        {
            throw AQLCoreInvalidData("Swap Daycount is not registered ", __FILE__,__LINE__);
        }
        else if ( itd == dvar.end() && itd2 != dvar.end() ) 
        {
		    throw AQLCoreInvalidData("Libor Basis Daycount is not registered ", __FILE__,__LINE__);
        }
        else if ( itd != dvar.end() && itd2 == dvar.end() ) 
        {
		    throw AQLCoreInvalidData("Libor Basis Daycount is not registered ", __FILE__,__LINE__);
        }
        else
        {
            dc	= itd->second;
            dc2 = itd2->second;
        }
	    if( dc2 != ldc ) warning += AQLString(" Swap floating daycount is not Libor's!");

	    city		= chgrow(swapConv,CURVEINPUT_CALENDAR,1).toToken(':');
        calendar    = chgrow(swapConv,CURVEINPUT_CALENDAR,1);
	    srule		= chgrow(swapConv,CURVEINPUT_SLIDINGRULE,1);
	    AQLString frequency = chgrow(swapConv,CURVEINPUT_FREQUENCY,1);
	    AQLPriceDataCalendar calS;
	    calS.convertFromString(calendar);

	    term = searchvecbycol(swapGrid,CURVEINPUT_TERM,true);
	    rate = AQLCoreUtility::changeDoubleFromString(searchvecbycol(swapGrid,CURVEINPUT_RATE,true));
	
	    if( rate.size() == 0 ) throw AQLCoreInvalidData("input swap market data.", __FILE__,__LINE__);
	    if( Currency == CURRENCY_USD && !isFRAUse )
	    {
		    tmprow1= AQLFunctionUtilities::findRowsNumber(swapGrid,"6M");
		    if( tmprow1 < 0 ) throw AQLCoreInvalidData("input 6M swap market data.", __FILE__,__LINE__);
	    }

        tmprow1= AQLFunctionUtilities::findRowsNumber(swapConv,CURVEINPUT_SPOTLAG);
        tmprow2= AQLFunctionUtilities::findRowsNumber(swapConv,CURVEINPUT_SPOTDATE);
        if( tmprow1<0 && tmprow2<0 ) 
        {
            throw AQLCoreInvalidData("input spot date or spot lag", __FILE__,__LINE__);
        }
	    else if(tmprow1>=0 && tmprow2>=0)
        {
		    throw AQLCoreInvalidData("do not input spot date and spot lag", __FILE__,__LINE__);
	    }
        else if(tmprow1>=0)
        {
            AQLString spotLag = AQLFunctionUtilities::findElement(swapConv,CURVEINPUT_SPOTLAG,0,1,true);
            spotdate = LADateScheduleHelpers::getDate(asofdate,spotLag,srule,calendar);
        }
        else if(tmprow2>=0)
        {
            spotdate = LADateScheduleHelpers::getLADate(chgrow(swapConv,CURVEINPUT_SPOTDATE,1));
        }

	    for (size_t i=0; i<term.size(); i++)
	    {
		    AQLObject* se	=	NULL;
		    name	=	Currency + SWAP + term[i];
		
		    if(!objPool.getObject(name).isDefined())
		    {	
			    se = new AQLObject;
			    objPool.set(name, se);
		    }
		    else
		    {
			    objPool.getObject(name).get().clear();
			    se= &objPool.getObject(name).get();
		    }

		    //intertplation 
		    it = ivar.find(chgrow(swapConv,CURVEINPUT_INTERPOLATION,1			));
		    if(it==ivar.end())
			    throw AQLCoreInvalidData("Interpolation is not registered in interpolation methods",			__FILE__,__LINE__);
		    interpolation = it->second;

		    se->add(CALIBRATION_DATA_NAME,				new AQLDataString(name)							);
		    se->add(IR_CALIBRATION_DATA_TERM,				new AQLDataString(term[i])							);
		    se->add(IR_CALIBRATION_DATA_DATATYPE,			new AQLDataString(PAR)							);
		    se->add(IR_CALIBRATION_DATA_FREQUENCY_FIX,	new AQLDataString(frequency)	);
		    se->add(IR_CALIBRATION_DATA_BASEFREQUENCY_FIX,	new AQLDataString(frequency)	);
		
		    if ( Currency == CURRENCY_USD ) 
		    {
			    se->add(IR_CALIBRATION_DATA_FREQUENCY_FLOAT,	new AQLDataString(QUARTERLY)	);
			    se->add(IR_CALIBRATION_DATA_BASEFREQUENCY_FLOAT,	new AQLDataString(QUARTERLY)	);
		    }
		    else if ( Currency == CURRENCY_AUD || Currency == CURRENCY_JPY  || Currency == CURRENCY_EUR )
		    {
			    tmprow1= AQLFunctionUtilities::findRowsNumber(swapConv,CURVEINPUT_FREQUENCYFLOAT);
			    if (tmprow1>=0)
			    {
				    AQLString frequency_Float = chgrow(swapConv,CURVEINPUT_FREQUENCYFLOAT,1);
				    se->add(IR_CALIBRATION_DATA_FREQUENCY_FLOAT,	new AQLDataString(frequency_Float)	);
				    se->add(IR_CALIBRATION_DATA_BASEFREQUENCY_FLOAT,	new AQLDataString(frequency_Float)	);
			    }
			    else
			    {
				    se->add(IR_CALIBRATION_DATA_FREQUENCY_FLOAT, new AQLDataString(SEMI_ANNUAL) );
				    se->add(IR_CALIBRATION_DATA_BASEFREQUENCY_FLOAT, new AQLDataString(SEMI_ANNUAL) );
			    }
		    }
		    else
		    {
			    AQLString frequency_Float = chgrow(swapConv,CURVEINPUT_FREQUENCYFLOAT,1);
			    se->add(IR_CALIBRATION_DATA_FREQUENCY_FLOAT, new AQLDataString(frequency_Float) );
			    se->add(IR_CALIBRATION_DATA_BASEFREQUENCY_FLOAT, new AQLDataString(frequency_Float) );
		    }

		    tmprow1= AQLFunctionUtilities::findRowsNumber(swapConv,CURVEINPUT_ISEOMROLL);
		    if (tmprow1>=0)
		    {
			    AQLString isEOMRoll_str = chgrow(swapConv,CURVEINPUT_ISEOMROLL,1);
			    AQLDataBool tmpIsEOMRollSW;
			    tmpIsEOMRollSW.convertFromString(isEOMRoll_str);
			    bool isEOMRoll = tmpIsEOMRollSW.get();
			    if (isEOMRoll)
			    {
				    AQLDate eomDate = calS.getEOMDay(spotdate);
				    if (eomDate != spotdate)
				    {
					    isEOMRoll = false;
				    }
			    }
			    se->add(IR_CALIBRATION_DATA_ISEOMROLLSW, new AQLDataBool(isEOMRoll));
		    }

		    // Swap Frequency of AUD is irregular
		    if ( Currency == CURRENCY_AUD && (term[i] == "1Y" || term[i] == "2Y" || term[i] == "3Y") ) 
		    {
			    se->add(IR_CALIBRATION_DATA_ISSWAPTENORADJUST, new AQLDataBool() ).convertFromString("TRUE");
			    se->remove(IR_CALIBRATION_DATA_FREQUENCY_FLOAT);
			    se->add(IR_CALIBRATION_DATA_FREQUENCY_FLOAT,	new AQLDataString(QUARTERLY) );
			    se->remove(IR_CALIBRATION_DATA_FREQUENCY_FIX);
			    se->add(IR_CALIBRATION_DATA_FREQUENCY_FIX, new AQLDataString(QUARTERLY) );
			    se->remove(IR_CALIBRATION_DATA_FREQUENCY);
			    se->add(IR_CALIBRATION_DATA_FREQUENCY, new AQLDataString(QUARTERLY) );
		    }
		
		    se->add(IR_CALIBRATION_DATA_DAYCOUNT,		new AQLPriceDataDayCount()							).convertFromString(dc);
		    se->add(IR_CALIBRATION_DATA_DAYCOUNT_FIX,		new AQLPriceDataDayCount()							).convertFromString(dc);
		    se->add(IR_CALIBRATION_DATA_DAYCOUNT_FLOAT,	new AQLPriceDataDayCount()							).convertFromString(dc2);
		    se->add(CALIBRATION_DATA_CALENDAR,			new AQLPriceDataCalendar(calS)						);
		    se->add(CALIBRATION_DATA_SLIDINGRULE,			new AQLPriceDataSlidingRule()							).convertFromString(srule);
		    se->add(IR_CALIBRATION_DATA_SPOTDATE,			new AQLDataDate(spotdate)						);
		    se->add(CALIBRATION_DATA_RATE,				new AQLDataDouble(rate[i])							);
		    se->add(CALIBRATION_DATA_INTERPOLATION,		new AQLPriceDataInterpolation()						).convertFromString(interpolation);
		    ref+= name + ":";
	    }
	
        //Currency Basis Data
	    AQLString ref_XCCYBasis = "";
        if( Currency != CURRENCY_USD )
        {
		    term = searchvecbycol(currGrid,CURVEINPUT_TERM,true);
		    rate = AQLCoreUtility::changeDoubleFromString(searchvecbycol(currGrid,CURVEINPUT_RATE,true));

		    for (size_t i=0; i<term.size(); i++)
		    {
			    AQLObject* be	=	NULL;
			    name	=	Currency + XCCYBASIS + term[i];

			    if(!objPool.getObject(name).isDefined())
			    {	
				    be = new AQLObject;
				    objPool.set(name, be);
			    }
			    else
			    {
				    objPool.getObject(name).get().clear();
				    be = &objPool.getObject(name).get();
			    }
			    be->add(CALIBRATION_DATA_NAME,				new AQLDataString(name)							);

			    if( currConv.empty() ) throw AQLCoreInvalidData("input currency basis swap convention.", __FILE__,__LINE__);
			    itd			= dvar.find(chgrow(currConv,CURVEINPUT_BASISDAYCOUNT,1));
			    if(itd==dvar.end())
				    throw AQLCoreInvalidData("Currency Basis Daycount is not registered ", __FILE__,__LINE__);
			    dc			= itd->second;
			    if( dc != ldc ) warning += AQLString(" Currency basis daycount is not Libor's!");
			    be->add(IR_CALIBRATION_DATA_INDEXDAYCOUNT,	new AQLPriceDataDayCount() ).convertFromString(dc);

			    city		= chgrow(currConv,CURVEINPUT_BASISCALENDAR,1).toToken(':');
			    calendar    = chgrow(currConv,CURVEINPUT_BASISCALENDAR,1);
			    if(calendar=="")
				    calendar = chgrow(currConv,CURVEINPUT_BASISCALENDAR1, 1);
			    srule = chgrow(currConv,CURVEINPUT_BASISSLIDINGRULE,1);
			    if(srule=="")
				    srule = chgrow(currConv,CURVEINPUT_BASISSLIDINGRULE1, 1);

			    tmprow1= AQLFunctionUtilities::findRowsNumber(currConv,CURVEINPUT_SPOTLAG);
			    tmprow2= AQLFunctionUtilities::findRowsNumber(currConv,CURVEINPUT_SPOTDATE);
			    if( tmprow1<0 && tmprow2<0) 
			    {
				    throw AQLCoreInvalidData("input spot date or spot lag", __FILE__,__LINE__);
			    }
			    else if(tmprow1>=0 && tmprow2>=0)
			    {
				    throw AQLCoreInvalidData("do not input spot date and spot lag", __FILE__,__LINE__);
			    }
			    else if(tmprow1>=0)
			    {
				    AQLString spotLag = AQLFunctionUtilities::findElement(currConv,CURVEINPUT_SPOTLAG,0,1,true);
				    spotdate = LADateScheduleHelpers::getDate(asofdate,spotLag,srule,calendar);
			    }
			    else if(tmprow2>=0)
			    {
				    spotdate = LADateScheduleHelpers::getLADate(chgrow(currConv,CURVEINPUT_SPOTDATE,1));
			    }

			    if( rate.size() == 0 ) throw AQLCoreInvalidData("input currency basis market data.", __FILE__,__LINE__);
			    tmprow1= AQLFunctionUtilities::findRowsNumber(currGrid,"3M");
			    if( tmprow1 < 0 ) throw AQLCoreInvalidData("input 3M currency basis market data.", __FILE__,__LINE__);

			    it = ivar.find(chgrow(currConv,CURVEINPUT_INTERPOLATION,1			));
			    if(it==ivar.end())
				    throw AQLCoreInvalidData("Interpolation is not registered in interpolation methods",			__FILE__,__LINE__);
			    interpolation = it->second;
			    be->add(CALIBRATION_DATA_INTERPOLATION, new AQLPriceDataInterpolation() ).convertFromString(interpolation);
			    be->add(IR_CALIBRATION_DATA_TERM, new AQLDataString(term[i]) );
			    be->add(CALIBRATION_DATA_RATE, new AQLDataDouble(rate[i]) );
			    be->add(IR_CALIBRATION_DATA_CASHLETCALENDAR, new AQLPriceDataCalendar(city) );
			    be->add(IR_CALIBRATION_DATA_CASHLETSLIDINGRULE, new AQLPriceDataSlidingRule() ).convertFromString(srule);
			    be->add(IR_CALIBRATION_DATA_CASHLETSPOTDATE, new AQLDataDate(spotdate) );
			    be->add(IR_CALIBRATION_DATA_DATATYPE, new AQLDataString()).convertFromString(YIELD_TYPE_BASIS);
			    if (parCurve == CURRENCY_USD || parCurve == "JPY3ML" || parCurve == "JPY6ML" ) 
			    {
				    be->add(IR_CALIBRATION_DATA_AGTFORECAST, new AQLDataString()).convertFromString("DUMMY");
				    be->add(IR_CALIBRATION_DATA_AGTDISCOUNT, new AQLDataString()).convertFromString("DUMMY");
			    }
			    else
			    {
				    if (isRenAdj)
				    {
					    curveCalibrationData->getForeignYieldData().convertFromString(usdCurve);
					    be->add(IR_CALIBRATION_DATA_AGTFORECAST, new AQLDataString()).convertFromString("3ML");
					    be->add(IR_CALIBRATION_DATA_AGTDISCOUNT, new AQLDataString()).convertFromString("DF");
					    be->add(IR_CALIBRATION_DATA_AGTCURRENCY, new AQLDataString()).convertFromString(CURRENCY_USD);
				    }
				    else
				    {
					    curveCalibrationData->getForeignYieldData().convertFromString(parCurve);
					    be->add(IR_CALIBRATION_DATA_AGTFORECAST, new AQLDataString()).convertFromString("FLOATER");
					    be->add(IR_CALIBRATION_DATA_AGTDISCOUNT, new AQLDataString()).convertFromString("DUMMY");
				    }				
			    }
			    ref_XCCYBasis+= name + ":";
		    }

		    ref_XCCYBasis = ref_XCCYBasis.subString(0, ref_XCCYBasis.size() - 2);
		    curveCalibrationData->AQLObject::add(CALIBRATION_DATA_MARKETDATA + AQLString("_") + XCCYBASIS, new AQLDataMultiReference()).convertFromString(ref_XCCYBasis);
        }
	    else
	    {
		    AQLObject* be	=	NULL;
		    name	=	Currency + XCCYBASIS;

		    if(!objPool.getObject(name).isDefined())
		    {	
			    be = new AQLObject;
			    objPool.set(name, be);
		    }
		    else
		    {
			    objPool.getObject(name).get().clear();
			    be = &objPool.getObject(name).get();
		    }

		    // set name
		    be->add(CALIBRATION_DATA_NAME, new AQLDataString()).convertFromString(name);
		    // set data type
		    be->add(IR_CALIBRATION_DATA_DATATYPE, new AQLDataString()).convertFromString(YIELD_TYPE_BASIS);
		    if (parCurve != CURRENCY_USD)
		    {
			    const AQLObject& object = objPool.getObject(parCurve, ENCHKTYPE_ISDEFINED).get();
			    const AQLString& parCurve_baseccy = dynamic_cast<const AQLDataString& > ((object.getData(IR_CALIBRATION_DATA_PARCURVE, ISNOTNULL)).get()).get();
			    if( parCurve_baseccy == CURRENCY_USD )
				    throw AQLCoreInvalidData("par curves are inconsistent!",__FILE__,__LINE__);
			    be->add(IR_CALIBRATION_DATA_AGTFORECAST, new AQLDataString()).convertFromString("FLOATER");
			    curveCalibrationData->getForeignYieldData().convertFromString(parCurve);
			    if (isRenAdj)
			    {
				    be->add(IR_CALIBRATION_DATA_AGTDISCOUNT, new AQLDataString()).convertFromString("DF");
				    be->add(IR_CALIBRATION_DATA_AGTCURRENCY, new AQLDataString()).convertFromString(parCurve_baseccy);
			    }
		    }
		    else
		    {
			    be->add(IR_CALIBRATION_DATA_AGTFORECAST, new AQLDataString()).convertFromString("DUMMY");
		    }

		    ref+= name + ":";
	    }

        //3MLibor 6MLibor Basis Data
	    itd			= dvar.find(chgrow(libBasisConv,"DAYCOUNTTHREE",1));
        itd2		= dvar.find(chgrow(libBasisConv,"DAYCOUNTSIX",1));
	    if(itd==dvar.end() && itd2==dvar.end() ) 
        {
            throw AQLCoreInvalidData("Libor Basis Daycount is not registered ", __FILE__,__LINE__);
        }
        else if(itd == dvar.end() && itd2 != dvar.end()) 
        {
		    throw AQLCoreInvalidData("Libor Basis Daycount is not registered ", __FILE__,__LINE__);
        }
        else if(itd != dvar.end() && itd2 == dvar.end()) 
        {
		    throw AQLCoreInvalidData("Libor Basis Daycount is not registered ", __FILE__,__LINE__);
        }
        else
        {
            dc	= itd->second;
            dc2 = itd2->second;
        }
	    if( dc != ldc ) warning += AQLString(" 3ML Leg daycount is not Libor's!");
	    if( dc2 != ldc ) warning += AQLString(" 6ML Leg daycount is not Libor's!");
	    term = searchvecbycol(libBasisGrid,CURVEINPUT_TERM,true);
	    rate = AQLCoreUtility::changeDoubleFromString(searchvecbycol(libBasisGrid,CURVEINPUT_RATE,true));
	    if( rate.size() == 0 ) throw AQLCoreInvalidData("input 3M/6M Libor basis market data.", __FILE__,__LINE__);
	    if( Currency != CURRENCY_USD && !isFRAUse )
	    {
		    tmprow1= AQLFunctionUtilities::findRowsNumber(libBasisGrid,"6M");
		    if( tmprow1 < 0 ) throw AQLCoreInvalidData("input 6M 3M/6M Libor basis market data.", __FILE__,__LINE__);
	    }

	    AQLString ref_36Basis = "";
	    for (size_t i=0; i<term.size(); i++)
	    {
		    AQLObject* lbe	=	NULL;
		    name	=	Currency + THREESIXBASIS + term[i];
		
		    if(!objPool.getObject(name).isDefined())
		    {	
			    lbe = new AQLObject;
			    objPool.set(name, lbe);
		    }
		    else
		    {
			    objPool.getObject(name).get().clear();
			    lbe = &objPool.getObject(name).get();
		    }

		    //intertplation 
		    it = ivar.find(chgrow(libBasisConv,CURVEINPUT_INTERPOLATION,1			));
		    if(it==ivar.end())
			    throw AQLCoreInvalidData("Interpolation is not registered in interpolation methods",			__FILE__,__LINE__);
		    interpolation = it->second;
		
		    lbe->add(CALIBRATION_DATA_NAME,						new AQLDataString(name) );
		    lbe->add(IR_CALIBRATION_DATA_TERM,					new AQLDataString(term[i]) );
		    lbe->add(IR_CALIBRATION_DATA_DAYCOUNT_THREE,		new AQLPriceDataDayCount() ).convertFromString(dc);
		    lbe->add(IR_CALIBRATION_DATA_DAYCOUNT_SIX,			new AQLPriceDataDayCount() ).convertFromString(dc2);
		    lbe->add(CALIBRATION_DATA_RATE,						new AQLDataDouble(rate[i]) );
		    lbe->add(CALIBRATION_DATA_INTERPOLATION,			new AQLPriceDataInterpolation() ).convertFromString(interpolation);
		    lbe->add(IR_CALIBRATION_DATA_DATATYPE,				new AQLDataString() ).convertFromString(YIELD_TYPE_BASIS);
		    lbe->add(IR_CALIBRATION_DATA_ISAGTSPREAD,			new AQLDataBool() ).convertFromString("FALSE");
		    lbe->add(IR_CALIBRATION_DATA_INDEXDAYCOUNT,			new AQLPriceDataDayCount() ).convertFromString(dc);
		    lbe->add(IR_CALIBRATION_DATA_AGTINDEXDAYCOUNT,		new AQLPriceDataDayCount() ).convertFromString(dc2);
		    lbe->add(IR_CALIBRATION_DATA_CASHLETFREQUENCY,		new AQLDataString() ).convertFromString(QUARTERLY);
		    lbe->add(IR_CALIBRATION_DATA_CASHLETCALENDAR,		new AQLPriceDataCalendar(calS) );
		    lbe->add(IR_CALIBRATION_DATA_CASHLETSLIDINGRULE,	new AQLPriceDataSlidingRule() ).convertFromString(srule);
		    lbe->add(IR_CALIBRATION_DATA_CASHLETSPOTDATE,		new AQLDataDate(spotdate) );
		    lbe->add(IR_CALIBRATION_DATA_AGTCASHLETFREQUENCY,	new AQLDataString() ).convertFromString(SEMI_ANNUAL);
		    ref_36Basis += name + ":";	
	    }
		
	    ref_36Basis = ref_36Basis.subString(0, ref_36Basis.size() - 2);
	    curveCalibrationData->AQLObject::add(CALIBRATION_DATA_MARKETDATA + AQLString("_") + THREESIXBASIS, new AQLDataMultiReference()).convertFromString(ref_36Basis);


        //FRA Data
        if(isFRAUse)
        {
            //error check
            AQLString errorMsg,term_FRA;

	        city		= chgrow(fraConv,CURVEINPUT_CALENDAR,1).toToken(':');
            calendar    = chgrow(fraConv,CURVEINPUT_CALENDAR,1);
	        srule = chgrow(fraConv,CURVEINPUT_SLIDINGRULE,1);
		    itd			= dvar.find(chgrow(fraConv,CURVEINPUT_DAYCOUNT,1));
		    if(itd==dvar.end())
			    throw AQLCoreInvalidData("FRA Daycount is not registered ", __FILE__,__LINE__);
		    dc	= itd->second;
		    if( dc != ldc ) warning += AQLString(" FRA daycount is not Libor's!");

            tmprow1= AQLFunctionUtilities::findRowsNumber(fraConv,CURVEINPUT_SPOTLAG);
            tmprow2= AQLFunctionUtilities::findRowsNumber(fraConv,CURVEINPUT_SPOTDATE);
            if( tmprow1<0 && tmprow2<0) 
            {
                throw AQLCoreInvalidData("input spot date or spot lag", __FILE__,__LINE__);
            }
	        else if(tmprow1>=0 && tmprow2>=0)
            {
		        throw AQLCoreInvalidData("do not input spot date and spot lag", __FILE__,__LINE__);
	        }
            else if(tmprow1>=0)
            {
                AQLString spotLag = AQLFunctionUtilities::findElement(fraConv,CURVEINPUT_SPOTLAG,0,1,true);
                spotdate = LADateScheduleHelpers::getDate(asofdate,spotLag,srule,calendar);
            }
            else if(tmprow2>=0)
            {
                spotdate = LADateScheduleHelpers::getLADate(chgrow(fraConv,CURVEINPUT_SPOTDATE,1));
            }

		    term = searchvecbycol(fraGrid3M,CURVEINPUT_TERM,true);
	        rate = AQLCoreUtility::changeDoubleFromString(searchvecbycol(fraGrid3M,CURVEINPUT_RATE,true));
		    for (size_t i=0; i<term.size(); i++)
		    {
			    AQLObject* fra3e	=	NULL;
			    //name	=	CurveID + Currency + FRA3ML;
			    name	=	Currency + FRA3ML + term[i];
		    
			    if(!objPool.getObject(name).isDefined())
			    {	
				    fra3e = new AQLObject;
				    objPool.set(name, fra3e);
			    }
			    else
			    {
				    objPool.getObject(name).get().clear();
				    fra3e = &objPool.getObject(name).get();
			    }
			
			    int where = -1;
			    where = term[i].findString("M");
			    int term_month = term[i].subString(0, where - 1).getIntValue();
			    AQLString term_X = AQLString(term_month) + AQLString("X") + AQLString(term_month + 3);

			    fra3e->add(CALIBRATION_DATA_NAME,				new AQLDataString(name)							);
			    fra3e->add(IR_CALIBRATION_DATA_TERM,			new AQLDataString(term_X)							);
			    fra3e->add(CALIBRATION_DATA_CALENDAR,			new AQLPriceDataCalendar(city)						);
			    fra3e->add(CALIBRATION_DATA_SLIDINGRULE,		new AQLPriceDataSlidingRule()							).convertFromString(srule);
			    fra3e->add(IR_CALIBRATION_DATA_SPOTDATE,		new AQLDataDate(spotdate)						);
			    fra3e->add(CALIBRATION_DATA_RATE,				new AQLDataDouble(rate[i])							);
			    fra3e->add(IR_CALIBRATION_DATA_DAYCOUNT,		new AQLPriceDataDayCount()							).convertFromString(dc);
			    fra3e->add(IR_CALIBRATION_DATA_DATATYPE,		new AQLDataString()).convertFromString(FRA3M);
                fra3e->add(IR_CALIBRATION_DATA_ISEOMROLL,		new AQLDataBool(false));
                fra3e->add(IR_CALIBRATION_DATA_APPLYTENSION,	new AQLDataBool(false));
                fra3e->add(IR_CALIBRATION_DATA_TENSIONGAP,	new AQLDataInt(1));
			    ref+= name + ":";
		    }

		    term = searchvecbycol(fraGrid6M,CURVEINPUT_TERM,true);
	        rate = AQLCoreUtility::changeDoubleFromString(searchvecbycol(fraGrid6M,CURVEINPUT_RATE,true));
		    for (size_t i=0; i<term.size(); i++)
		    {
			    AQLObject* fra6e	=	NULL;
			    name	=	CurveID + Currency + FRA6ML + term[i];
			    if(!objPool.getObject(name).isDefined())
			    {	
				    fra6e = new AQLObject;
				    objPool.set(name, fra6e);
			    }
			    else
			    {
				    objPool.getObject(name).get().clear();
				    fra6e = &objPool.getObject(name).get();
			    }

			    int where = -1;
			    where = term[i].findString("M");
			    int term_month = term[i].subString(0, where - 1).getIntValue();
			    AQLString term_X = AQLString(term_month) + AQLString("X") + AQLString(term_month + 6);

			    fra6e->add(CALIBRATION_DATA_NAME,				new AQLDataString(name)							);
			    fra6e->add(IR_CALIBRATION_DATA_TERM,			new AQLDataString(term_X)							);
			    fra6e->add(CALIBRATION_DATA_CALENDAR,			new AQLPriceDataCalendar(city)						);
			    fra6e->add(CALIBRATION_DATA_SLIDINGRULE,		new AQLPriceDataSlidingRule()							).convertFromString(srule);
			    fra6e->add(IR_CALIBRATION_DATA_SPOTDATE,		new AQLDataDate(spotdate)						);
			    fra6e->add(CALIBRATION_DATA_RATE,				new AQLDataDouble(rate[i])							);
			    fra6e->add(IR_CALIBRATION_DATA_DAYCOUNT,		new AQLPriceDataDayCount()							).convertFromString(dc);
			    fra6e->add(IR_CALIBRATION_DATA_DATATYPE,		new AQLDataString()).convertFromString(FRA6M);
			    fra6e->add(IR_CALIBRATION_DATA_ISEOMROLL,		new AQLDataBool(false));
                fra6e->add(IR_CALIBRATION_DATA_APPLYTENSION,	new AQLDataBool(false));
			    fra6e->add(IR_CALIBRATION_DATA_TENSIONGAP,	new AQLDataInt(1));
			    ref+= name + ":";
		    }
        }

	    //set market object
	    if (ref.size() < 2)
		    throw AQLCoreInvalidData("Market Data is not set !!", __FILE__, __LINE__); 
	    ref = ref.subString(0, ref.size() - 2);
	    curveCalibrationData->getMarketData().convertFromString(ref);
	    //generate Object
	    dataInstance->getReferencePool().completeDependency();
	    AQLDataHolder* dh		= &objPool.getObject(CurveIDTool).getData(CALIBRATION_DATA_CURVEGENERATOR);
	    AQLDataProcedure& modelDataObj	= dynamic_cast<AQLDataProcedure&>(dh->get());
	    modelDataObj.calibrateModel(asofdate);

	    curveCalibrationData->AQLObject::add(IR_CALIBRATION_DATA_MAINBASISDF, new AQLDataString("DF"));
	    curveCalibrationData->setBasisRates();

	    //curve entities manager (curve, grids, curveinformation entities)
	    AQLObject* mae = NULL;
	    if(!objPool.getObject(CurveIDManager).isDefined())
	    {	
		    mae = new AQLObject;
		    objPool.set(CurveIDManager,mae);
	    }
	    else
	    {
		    objPool.getObject(CurveIDManager).get().clear();
		    mae	= &objPool.getObject(CurveIDManager).get();
	    }
	    mae->add("Time",			new AQLDataString()			).convertFromString(AQLString(AQLTime::now()));
	    mae->add(CALIBRATION_DATA_NAME,		new AQLDataString()			).convertFromString(CurveIDManager);
	    mae->add(GRIDREFS,			new AQLDataMultiReference()	).convertFromString(ref);
	    mae->add(YIELDDATAREF,		new AQLDataReference()		).convertFromString(CurveID);
	    mae->add(YIELDTOOLREF,		new AQLDataReference()		).convertFromString(CurveIDTool);
    }

    void 
    LACurveCalibrationHelpers::setUpForecastCurveFromMarket(AQLDataInstance* dataInstance,
                                                               const AQLString& setUpCurveID, 
						                                       AQLStringMatrix bgrid, 
                                                               AQLStringMatrix bdata)
    {
        AQLString basisLeg = chgrow(bdata,"BasisLeg",1);    
        AQLString arbFreeCurveID = chgrow(bdata,"ArbFreeCurveID",1);
	    AQLString baseForecastCurveName = chgrow(bdata,"BaseForecastCurveName",1);
	    AQLString dfCurveName = chgrow(bdata,"DiscountCurveName",1);

        //ylddata
	    AQLString CurveIDTool = arbFreeCurveID + TOOL;
	    upper(bgrid);
	    upper(bdata);

	    //basis market data
	    AQLStringVector terms_str;
        AQLDate spotdate;		
	    AQLString daycount(AC_365I);

	    //BasisType
	    std::map<AQLString, AQLString>& dvar = AQLCoreComponentManager::getDayCountMap();
	    std::map<AQLString, AQLString>::iterator itd = dvar.find(chgrow(bdata,"SETUPCURVEDAYCOUNT",1));
        if(itd == dvar.end()) throw AQLCoreInvalidData("basisDaycount is not registered ", __FILE__,__LINE__);
	    AQLString sDaycount = itd->second;

        itd = dvar.find(chgrow(bdata,"BASECURVEDAYCOUNT",1));
        if(itd == dvar.end()) throw AQLCoreInvalidData("basisDaycount is not registered ", __FILE__,__LINE__);
	    AQLString bDaycount = itd->second;
	
	    AQLString bcalendar	= chgrow(bdata,CURVEINPUT_BASISCALENDAR,1);
	    if(bcalendar=="")
		    bcalendar		= chgrow(bdata,CURVEINPUT_BASISCALENDAR1, 1);
	    AQLString bsrule		= chgrow(bdata,CURVEINPUT_BASISSLIDINGRULE,1);
	    if(bsrule=="")
		    bsrule			= chgrow(bdata,CURVEINPUT_BASISSLIDINGRULE1, 1);
        AQLString sFreq		= chgrow(bdata,"SETUPCURVEFREQUENCY",1);
	    AQLString bFreq		= chgrow(bdata,"BASECURVEFREQUENCY",1);
    
        //set initial rate
        unsigned int setSpan,baseSpan;
        if (bFreq == ANNUAL) baseSpan = 12;
	    else if (bFreq == SEMI_ANNUAL) baseSpan = 6;
	    else if (bFreq == QUARTERLY) baseSpan = 3;
	    else if (bFreq == MONTHLY) baseSpan = 1;
	    else
	    {
		    //error
		    throw AQLCoreInvalidData("Input freq type is not supported", __FILE__, __LINE__);
	    }
        if (sFreq == ANNUAL) setSpan = 12;
	    else if (sFreq == SEMI_ANNUAL) setSpan = 6;
	    else if (sFreq == QUARTERLY) setSpan = 3;
	    else if (sFreq == MONTHLY) setSpan = 1;
	    else
	    {
		    //error
		    throw AQLCoreInvalidData("Input freq type is not supported", __FILE__, __LINE__);
	    }
        int tempRow = AQLFunctionUtilities::findRowsNumber(bdata,CURVEINPUT_BASISINITIALRATE);
        AQLString isIniRateUse_str = chgrow(bdata,"ISINITIALRATEUSE",1);
        bool isIniRateUse = true;
        if( setSpan >= baseSpan )
        {
            if( isIniRateUse_str == "TRUE" ) isIniRateUse = true;
            else if( isIniRateUse_str == "FALSE" ) isIniRateUse = false;
            else throw AQLCoreInvalidData("Input initial rate use", __FILE__, __LINE__);
        }
        if( isIniRateUse == true && (tempRow < 0 || chgrow(bdata,CURVEINPUT_BASISINITIALRATE,1) == "" ) ) 
            throw AQLCoreInvalidData("Input initial rate", __FILE__, __LINE__);
        double initialRate	= chgrow(bdata,CURVEINPUT_BASISINITIALRATE,1).getDoubleValue();

        int tmprow1= AQLFunctionUtilities::findRowsNumber(bdata,CURVEINPUT_SPOTLAG);
        int tmprow2= AQLFunctionUtilities::findRowsNumber(bdata,CURVEINPUT_SPOTDATE);
        const AQLDate asofdate = dynamic_cast<const AQLDataDate& >(dataInstance->getObjectPool().getObject(arbFreeCurveID,ENCHKTYPE_ISDEFINED).
                                    get().getData(CALIBRATION_DATA_ASOFDATE,ISNOTNULL).get()).get();
        if( tmprow1<0 && tmprow2<0) 
        {
            throw AQLCoreInvalidData("input spot date or spot lag", __FILE__,__LINE__);
        }
        else if(tmprow1>=0)
        {
            AQLString spotLag = AQLFunctionUtilities::findElement(bdata,CURVEINPUT_SPOTLAG,0,1,true);
            spotdate = LADateScheduleHelpers::getDate(asofdate,spotLag,bsrule,bcalendar);
        }
        else
        {
            spotdate = LADateScheduleHelpers::getLADate(chgrow(bdata,CURVEINPUT_SPOTDATE,1));
        }
	
	    AQLObjectPool& objPool = dataInstance->getObjectPool(); 
	    //first check isYieldCurveIDexist
	    if(!objPool.getObject(arbFreeCurveID).isDefined())
		    throw AQLCoreInvalidData("CurveID does not exsist",__FILE__,__LINE__);

	    double rate =0.0;

        AQLStringVector term_str = searchvecbycol(bgrid,CURVEINPUT_TERM,true);
        DoubleVector basisRates = AQLCoreUtility::changeDoubleFromString(searchvecbycol(bgrid,CURVEINPUT_RATE,true));
	    AQLString name = setUpCurveID + "Market";
	    AQLObject* be = NULL;
	    if(!objPool.getObject(name).isDefined())
	    {
		    be = new AQLObject;
		    objPool.set(name,be);
	    }
	    else
	    {
		    objPool.getObject(name).get().clear();
            be = &objPool.getObject(name).get();
	    }

	    be->add(CALIBRATION_DATA_NAME,				new AQLDataString(name)				);
	    be->add(IR_CALIBRATION_DATA_TERM,				new AQLDataStrings(term_str)			);
	    be->add("BasisRate",    			new AQLDataDoubles(basisRates)		);
	    be->add("SetUpCurveFrequency",	    new AQLDataString(sFreq)				);
        be->add("BaseCurveFrequency",	    new AQLDataString(bFreq)				);
        be->add("SetUpCurveDayCount",		new AQLPriceDataDayCount()				).convertFromString(sDaycount);
	    be->add("BaseCurveDayCount",		new AQLPriceDataDayCount()				).convertFromString(bDaycount);
	    be->add(IR_CALIBRATION_DATA_CALENDARBASE,		new AQLPriceDataCalendar()				).convertFromString(bcalendar);
	    be->add(IR_CALIBRATION_DATA_SLIDINGRULEBASE,	new AQLPriceDataSlidingRule()				).convertFromString(bsrule);
	    be->add(IR_CALIBRATION_DATA_SPOTDATE,			new AQLDataDate(spotdate)			);
        be->add("InitialRate",	            new AQLDataDouble(initialRate)		);
        be->add("IsInitialRateUse",         new AQLDataBool(isIniRateUse)		);
        be->add("ArbFreeCurveID",	        new AQLDataString(arbFreeCurveID)    );
	    be->add("BaseForecastCurveName",    new AQLDataString(baseForecastCurveName));
	    be->add("DiscountCurveName",		new AQLDataString(dfCurveName));
        be->add("BasisLeg",	                new AQLDataString(basisLeg)		    );
 
        AQLPriceArbFreeGenerator::setForecastCurve(dataInstance, setUpCurveID);
    }

    void 
    LACurveCalibrationHelpers::setUpForecastCurve(AQLDataInstance* dataInstance,
                                                     const AQLString& arbFreeCurveID, 
                                                     const AQLString& forecastCurveID, 
                                                     const AQLDate& asofdate,
                                                     const DoubleMatrix& ratesArray)
    {
	    unsigned int ratesize = ratesArray.size();
	    if (ratesize < 2)
	    {
		    AQLString msg = "rate matrix size must be more than 2";
		    throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	    }
        AQLObjectPool &objPool = dataInstance->getObjectPool();
        AQLString name = arbFreeCurveID + "_" + forecastCurveID;
	    AQLObjectHolder objHolder = objPool.getObject(name,ENCHKTYPE_NOCHECK);
	    if(!objHolder.isDefined())
	    {
		    AQLObject* e = new AQLObject();
		    e->add("Name", new AQLDataString()).convertFromString(name);
		    e->add("AsOfDate", new AQLDataDate(asofdate));
		    e->add("Terms_Rate",	new AQLDataDoubles(ratesArray[0]));
		    e->add("ForecastRates",new AQLDataDoubles(ratesArray[1]));
            e->add("ArbFreeCurve",new AQLDataString(arbFreeCurveID));
		    objPool.set(name,e);
	    }
	    else if(objHolder.isDefined())
	    {
		    AQLDataHolder* dh;
		    dh = &objHolder.getData("AsOfDate",ISDEFINED);
		    AQLDataDate& date = dynamic_cast<AQLDataDate &>(dh->get());
		    date.set(asofdate);
		    dh = &objHolder.getData("Terms_Rate",ISDEFINED);
		    AQLDataDoubles& terms = dynamic_cast<AQLDataDoubles& >(dh->get());
		    terms.set(ratesArray[0]);
		    dh = &objHolder.getData("ForecastRates",ISDEFINED);
		    AQLDataDoubles& rates = dynamic_cast<AQLDataDoubles& >(dh->get());
		    rates.set(ratesArray[1]);
            dh = &objHolder.getData("ArbFreeCurve",ISDEFINED);
		    AQLDataString& arbFreeName = dynamic_cast<AQLDataString &>(dh->get());
		    arbFreeName.set(arbFreeCurveID);
	    }
    }

    void 
    LACurveCalibrationHelpers::setUpCdtDFCurve(AQLDataInstance* dataInstance, AQLString curveID, AQLStringMatrix data)
    {
        AQLString arbFreeCurveID = chgrow(data,"ArbFreeCurveID",1);
        AQLString forecastCurveID = chgrow(data,"ForecastCurveID",1);
        const AQLDate& asOfDate = dynamic_cast<const AQLDataDate&> ((dataInstance->getObjectPool().getObject(arbFreeCurveID,ENCHKTYPE_ISDEFINED).get().
                getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL)).get()).get();
        upper(data);
        //daycount
	    std::map<AQLString, AQLString>& dvar = AQLCoreComponentManager::getDayCountMap();
	    std::map<AQLString, AQLString>::iterator itd = dvar.find(chgrow(data,CURVEINPUT_DAYCOUNT,1));
	    if(itd==dvar.end()) throw AQLCoreInvalidData("Money Market Daycount is not registered ", __FILE__,__LINE__);
	    AQLPriceDataDayCount dc;
        dc.convertFromString(itd->second);
        //calendar,slidingrule,frequency
        AQLString srule_str = chgrow(data,CURVEINPUT_SLIDINGRULE,1);
        AQLString calendar_str = chgrow(data,CURVEINPUT_CALENDAR,1);
	
	    AQLString frequency = chgrow(data,CURVEINPUT_FREQUENCY,1);
        //spotdate
        AQLDate spotdate;
        int tmprow1,tmprow2;
        tmprow1 = AQLFunctionUtilities::findRowsNumber(data,CURVEINPUT_SPOTLAG);
        tmprow2= AQLFunctionUtilities::findRowsNumber(data,CURVEINPUT_SPOTDATE);
        if( tmprow1<0 && tmprow2<0) 
        {
            throw AQLCoreInvalidData("input spot date or spot lag", __FILE__,__LINE__);
        }
        else if(tmprow1>=0 && tmprow2>=0)
        {
		    throw AQLCoreInvalidData("do not input spot date and spot lag", __FILE__,__LINE__);
	    }
	    else if(tmprow1>=0)
	    {		
		    AQLString spotLag = AQLFunctionUtilities::findElement(data,CURVEINPUT_SPOTLAG,0,1,true);
            spotdate = LADateScheduleHelpers::getDate(asOfDate,spotLag,srule_str,calendar_str);
        }
        else if(tmprow2>=0)
        {
            spotdate = LADateScheduleHelpers::getLADate(chgrow(data,CURVEINPUT_SPOTDATE,1));
        }
        double spread = chgrow(data,"CREDITSPREAD",1).getDoubleValue();
        AQLPriceDataSlidingRule srule;
        srule.convertFromString( srule_str );
        AQLPriceDataCalendar calendar;
        calendar.convertFromString( calendar_str );

        AQLPriceArbFreeGenerator::generateCdtDFCurve(dataInstance, arbFreeCurveID, forecastCurveID, curveID, spotdate, 
            calendar, srule, frequency, dc, spread);
    }

    AQLString 
    LACurveCalibrationHelpers::getCurveGenerateTime(AQLDataInstance* dataInstance, const AQLString& curveid, const AQLString& curvename)
    {

	    AQLString CurveManager = curveid + curvename + MANAGER;
	    const AQLObjectHolder& objHolder = dataInstance->getObjectPool().getObject(CurveManager,ENCHKTYPE_NOCHECK);
	    if(!objHolder.isDefined())
	    {		AQLString msg = "Curve :" + curveid + " is not generated by mirCurveGenerate";
		    throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);		
	    }
	    const AQLDataString &str = dynamic_cast<const AQLDataString &>(objHolder.getData("Time",ISDEFINED).get());
	    AQLString ret = curveid + " is generated at " + str.convertToString();
	    return ret;
    }

    void	
    LACurveCalibrationHelpers::setUpCurve2(AQLDataInstance* dataInstance, const AQLString& curveid, const AQLDate& asofdate, 
										      const DoubleMatrix& mat, const AQLString& curveName)
    {
	    LACurveCalibrationHelpers::setUpCurve(dataInstance,curveid,asofdate,mat,curveName);
	    AQLString suffix;
	    if (curveName == STD)
	    {
		    suffix = AQLString("");
	    }
	    else
	    {
		    suffix = AQLString("_") + curveName;
	    }

	    unsigned int N=mat.size();
	    if (N < 2) 
	    {
		    AQLString msg = "rate matrix size must be more than 2";
		    throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);		
	    }
	    AQLObjectPool& objPool  = dataInstance->getObjectPool();
	    AQLObject& e = objPool.getObject(curveid,ENCHKTYPE_ISDEFINED).get();
	    e.remove(IR_CALIBRATION_DATA_DFS2 + suffix);
	    if(N<=2)
		    return;
	    else
		    e.add(IR_CALIBRATION_DATA_DFS2 + suffix,new AQLDataDoubles(mat[2]));
	    return;
    }

    void 
    LACurveCalibrationHelpers::setUpCurve(AQLDataInstance* dataInstance, const AQLString& curveid, const AQLDate& asofdate, 
										     const DoubleMatrix& mat, const AQLString& curveName)
    {
	    unsigned int N=mat.size();
	    if (N < 2) 
	    {
		    AQLString msg = "rate matrix size must be more than 2";
		    throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);		
	    }
	    AQLObjectPool &objPool = dataInstance->getObjectPool();
	    AQLObjectHolder objHolder = objPool.getObject(curveid,ENCHKTYPE_NOCHECK);
	    AQLString suffix;
	    if (curveName == STD)
	    {
		    suffix = AQLString("");
	    }
	    else
	    {
		    suffix = AQLString("_") + curveName;
	    }

	    if(!objHolder.isDefined())
	    {
		    AQLObject* e = new AQLObject();
		    e->add("Name", new AQLDataString()).convertFromString(curveid);
		    e->add("AsOfDate", new AQLDataDate(asofdate));
		    e->add("Terms" + suffix, new AQLDataDoubles(mat[0]));
		    e->add("DiscountFactors" + suffix,new AQLDataDoubles(mat[1]));
		    objPool.set(curveid,e);
	    }
	    else if(objHolder.isDefined())
	    {
		    objHolder.remove("AsOfDate");
		    objHolder.remove("Terms" + suffix);
		    objHolder.remove("DiscountFactors" + suffix);
		    objHolder.add("AsOfDate", new AQLDataDate(asofdate));
		    objHolder.add("Terms" + suffix, new AQLDataDoubles(mat[0]));
		    objHolder.add("DiscountFactors" + suffix,new AQLDataDoubles(mat[1]));
	    }
    }

    /////////////furuytos20120813///////////
    void 
    LACurveCalibrationHelpers::setUpCurve3(AQLDataInstance* dataInstance, const AQLString& curveid, const AQLDate& asofdate, 
										     const DoubleMatrix& mat, const AQLStringMatrix& conv, const AQLString& curveName)
    {
	    unsigned int N=mat.size();
	    if (N < 2) 
	    {
		    AQLString msg = "rate matrix size must be more than 2";
		    throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);		
	    }
	    AQLObjectPool &objPool = dataInstance->getObjectPool();
	    AQLObjectHolder objHolder = objPool.getObject(curveid,ENCHKTYPE_NOCHECK);
	    AQLString suffix;
	    if (curveName == STD)
	    {
		    suffix = AQLString("");
	    }
	    else
	    {
		    suffix = AQLString("_") + curveName;
	    }

	    AQLStringMatrix tempconv = conv;
	    upper(tempconv);
	    AQLString accessary = "";
	    AQLString dc = "";
	
	    dc = chgrow(tempconv,CURVEINPUT_DAYCOUNT,1);
	    accessary = chgrow(tempconv,"ACCESSARY",1);

	    // If not "objHolder" exists
	    if(!objHolder.isDefined())
	    {
		    AQLObject* e = new AQLObject();
		    e->add(CALIBRATION_DATA_NAME, new AQLDataString()).convertFromString(curveid);
		    e->add(CALIBRATION_DATA_ASOFDATE, new AQLDataDate(asofdate));
		    e->add(CALIBRATION_DATA_TERMS + suffix, new AQLDataDoubles(mat[0]));
		    e->add(IR_CALIBRATION_DATA_DFS + suffix,new AQLDataDoubles(mat[1]));

		    e->remove(IR_CALIBRATION_DATA_FREQUENCY + suffix);
		    e->remove(IR_CALIBRATION_DATA_DAYCOUNT + suffix);
		    e->remove(IR_CALIBRATION_DATA_ACCESSARY + suffix);
		    e->add(IR_CALIBRATION_DATA_FREQUENCY + suffix, new AQLDataString(SIMPLE));
		    e->add(IR_CALIBRATION_DATA_DAYCOUNT+ suffix,		new AQLPriceDataDayCount()							).convertFromString(dc);
		    e->add(IR_CALIBRATION_DATA_ACCESSARY + suffix, new AQLDataString(accessary));

		    objPool.set(curveid,e);
	    }

	     // If "objHolder" exists
	    else if(objHolder.isDefined())
	    {
		    objHolder.remove(CALIBRATION_DATA_ASOFDATE);
		    objHolder.remove(CALIBRATION_DATA_TERMS + suffix);
		    objHolder.remove(IR_CALIBRATION_DATA_DFS + suffix);
		    objHolder.add(CALIBRATION_DATA_ASOFDATE, new AQLDataDate(asofdate));
		    objHolder.add(CALIBRATION_DATA_TERMS + suffix, new AQLDataDoubles(mat[0]));
		    objHolder.add(IR_CALIBRATION_DATA_DFS + suffix,new AQLDataDoubles(mat[1]));
		
		    objHolder.remove(IR_CALIBRATION_DATA_FREQUENCY + suffix);
		    objHolder.remove(IR_CALIBRATION_DATA_DAYCOUNT + suffix);
		    objHolder.remove(IR_CALIBRATION_DATA_ACCESSARY + suffix);
		    objHolder.add(IR_CALIBRATION_DATA_FREQUENCY + suffix,	new AQLDataString(SIMPLE));
		    objHolder.add(IR_CALIBRATION_DATA_DAYCOUNT+ suffix,		new AQLPriceDataDayCount()							).convertFromString(dc);
		    objHolder.add(IR_CALIBRATION_DATA_ACCESSARY + suffix,	new AQLDataString(accessary));

	    }

    }
    ////////////////////////////////

    void 
    LACurveCalibrationHelpers::setUpCurve(AQLDataInstance* dataInstance, const AQLString& curveid, const AQLDate& asofdate,
										     DoubleArray& termarray, DoubleArray& dfarray, const AQLString& curveName)
    {
	    if(termarray.size() != dfarray.size())
	    {
		    throw AQLCoreInvalidData("Input the same data size between Term and DF",__FILE__,__LINE__);
	    }

	    unsigned int col = termarray.size();
	    DoubleVector tmp(col, 0.0);
	    DoubleMatrix mat(2, tmp);
	    mat[0] = termarray;
	    mat[1] = dfarray;

	    LACurveCalibrationHelpers::setUpCurve(dataInstance, curveid, asofdate, mat, curveName);
    }

    DoubleArray 
    LACurveCalibrationHelpers::outPutCurveFromYieldData(AQLDataInstance* dataInstance, const AQLString& curveid, const AQLString& curveName)
    {
	    AQLObjectPool& objPool	= dataInstance->getObjectPool();
	
	    AQLString suffix;
	    if (curveName == STD)
	    {
		    suffix = "";
	    }
	    else
	    {
		    suffix = "_" + curveName;
	    }

	    const AQLObject& ycp	= objPool.getObject(curveid,ENCHKTYPE_ISDEFINED).get();
	    const AQLDataDoubles& terms = dynamic_cast<const AQLDataDoubles& >(ycp.getData(CALIBRATION_DATA_TERMS + suffix,ISDEFINED).get());
	    DoubleArray x = terms.get();
	    const AQLDataDoubles & dfs = dynamic_cast<const AQLDataDoubles& >(ycp.getData(IR_CALIBRATION_DATA_DFS + suffix,ISDEFINED).get());
	    DoubleArray	y = dfs.get();
	
	    DoubleArray ret;
	    int N = x.size();
	    const AQLDataHolder* dh = &(ycp.getData(IR_CALIBRATION_DATA_DFS2 + suffix));
	    if(dh->isDefined() && !dh->isNull())
	    {
		    DoubleArray z = dynamic_cast<const AQLDataDoubles &>(dh->get()).get();
		    z.resize(N);
		    for(int i=0; i< N;i++)
		    {
			    ret.push_back(x[i]);
			    ret.push_back(y[i]);
			    ret.push_back(z[i]);
		    }
	    }
	    else
	    {
		    for(int i=0; i< N;i++)
		    {
			    ret.push_back(x[i]);
			    ret.push_back(y[i]);
		    }
	    }
	    return ret;
    }

    DoubleArray 
    LACurveCalibrationHelpers::outPutCurveFromYieldData2(AQLDataInstance* dataInstance, const AQLString& curveID, const AQLString& curveID2,
														    const AQLString& curveName, const AQLString& curveName2)
    {
	    AQLObjectPool& objPool	= dataInstance->getObjectPool();
	    AQLString suffix;
	    AQLObject& ycp1 = objPool.getObject(curveID,ENCHKTYPE_ISDEFINED).get();
	    if (curveName == STD)
	    {
		    suffix = "";
	    }
	    else
	    {
		    suffix = "_" + curveName;
	    }
	    const DoubleArray& x1 = dynamic_cast<const AQLDataDoubles& >(ycp1.getData(CALIBRATION_DATA_TERMS + suffix,ISDEFINED).get()).get();
	    const DoubleArray& y1 = dynamic_cast<const AQLDataDoubles& >(ycp1.getData(IR_CALIBRATION_DATA_DFS + suffix,ISDEFINED ).get()).get();
    
        AQLObject& ycp2 = objPool.getObject(curveID2,ENCHKTYPE_ISDEFINED).get();
	    if (curveName2 == STD)
	    {
		    suffix = "";
	    }
	    else
	    {
		    suffix = "_" + curveName2;
	    }
	    const DoubleArray& x2 = dynamic_cast<const AQLDataDoubles& >(ycp2.getData(CALIBRATION_DATA_TERMS + suffix,ISDEFINED).get()).get();
	    const DoubleArray& y2 = dynamic_cast<const AQLDataDoubles& >(ycp2.getData(IR_CALIBRATION_DATA_DFS + suffix,ISDEFINED).get()).get();
	
	    DoubleArray ret;
	    if(x1.size() <= x2.size())
	    {
            AQLInterpolationBase* pInter = dynamic_cast<AQLInterpolationBase*>
                (dynamic_cast<const AQLPriceDataInterpolation& >(ycp1.getData(CALIBRATION_DATA_INTERPOLATION,ISDEFINED).get()).getMethod().clone());
	        AQLCoreFunctionHolder fh(pInter, true);
            pInter->set(x1,y1);
		    for(size_t i=0; i<x2.size(); i++)
		    {
			    ret.push_back(x2[i]);
			    ret.push_back(pInter->value(x2[i]));
			    ret.push_back(y2[i]);
		    }
	    }
        else
        {
            AQLInterpolationBase* pInter = dynamic_cast<AQLInterpolationBase*>
                (dynamic_cast<const AQLPriceDataInterpolation& >(ycp2.getData(CALIBRATION_DATA_INTERPOLATION,ISDEFINED).get()).getMethod().clone());
	        AQLCoreFunctionHolder fh(pInter, true);
            pInter->set(x2,y2);
		    for(size_t i=0; i<x1.size(); i++)
		    {
			    ret.push_back(x1[i]);
			    ret.push_back(y1[i]);
			    ret.push_back(pInter->value(x1[i]));
		    }
	    }


	    return ret;
    }

    /* @brief			Function to get the discount factors from a curve returning a discountFactorTable structure containing dates, terms and discount factors
    *  @param [in]		dataInstance		            The curveCollection to use when accessing the curveIndices
    *  @param [in]		curveid		            The curveindex within the curve
    *  @param [in]		curveName               The payment dates corresponding to our discount factors
    *  @param [out]     DiscountFactorTable     The discount factor table results
    */
    DiscountFactorTable 
    LACurveCalibrationHelpers::getCurveDiscountFactorTable(AQLDataInstance* dataInstance, const AQLString& curveid, const AQLString& curveName)
    {
	    AQLObjectPool& objPool	= dataInstance->getObjectPool();
	
	    AQLString suffix;
	    if (curveName == STD)
	    {
		    suffix = "";
	    }
	    else
	    {
		    suffix = "_" + curveName;
	    }

	    const AQLObject& yieldCurveProps	= objPool.getObject(curveid,ENCHKTYPE_ISDEFINED).get();
	    const AQLDataDoubles& terms = dynamic_cast<const AQLDataDoubles& >(yieldCurveProps.getData(CALIBRATION_DATA_TERMS + suffix,ISDEFINED).get());
        const AQLDataDoubles & dfs = dynamic_cast<const AQLDataDoubles& >(yieldCurveProps.getData(IR_CALIBRATION_DATA_DFS + suffix,ISDEFINED).get());
    
        // Get the terms and discount factor vectors
        DoubleArray termsVector     = terms.get();
        DoubleArray	dfsVector       = dfs.get();
    
        // Check Result Dimensions
        if ( termsVector.size() != dfsVector.size() )
        {
            throw AQLCoreInvalidData("#Error: Invalid Curve Results - Inconsistent number of discount factor dates and values",__FILE__,__LINE__);
        }
    
        // Get the Payment Dates corresponding to the terms daycount fractions
        // --------------------------------------
        DateVector  paymentDatesVector( termsVector.size(), AQLDate() );

        // Get the Curve AsOfDate - note curveid is also known as curve collection
        AQLDate asOfDate = dynamic_cast<const AQLDataDate& > ( objPool.getObject( curveid ).get().getData( CALIBRATION_DATA_ASOFDATE, ISNOTNULL ).get() ).get();

        // Imply the corresponding term payment dates
        for( size_t i = 0; i<termsVector.size() ; ++i )
        {
            // Curve terms are always calculated using ACT/365_ISDA as defined in ConstantDeclarations.h [sic] - see #define AC_365I
            // Etrading curves have been migrated to ACT/365 because ACT/365_ISDA causes leap year irregularities in the yield curve
		    AQLString ACT_365 = AQLString("ACT/365");
            paymentDatesVector[i] = LADateScheduleHelpers::getDateFromTerm( asOfDate, termsVector[i], ACT_365);
        }
    

        // Return Results
        DiscountFactorTable resultsTable;
        resultsTable.terms_             = termsVector;
	    resultsTable.discountFactors_   = dfsVector;
        resultsTable.paymentDates_      = paymentDatesVector;

	    return resultsTable;
    }

    /* @brief			Function to set the discount factors from a curve
    *  @param [in]		dataInstance		    The object pool dataInstance std::map
    *  @param [in]		curveid		            The curveCollection to use when accessing the curveIndices
    *  @param [in]		curveNames              A vector of curveindex names to set within the curve
    *  @param [in]      discountFactorTable     The discount factor table inputs
    */
    void LACurveCalibrationHelpers::setCurveDiscountFactorTable(AQLDataInstance* dataInstance, const AQLString& curveid, const std::vector<AQLString>& curveNames, const DiscountFactorTable& discountFactorTable )
    {
        // 1. Get the terms and discount factors from the discountFactorTable
        // ---------------------------------------
        DoubleArray termsVector     = discountFactorTable.terms_;
        DoubleArray	dfsVector       = discountFactorTable.discountFactors_;
    
        // Check Input Dimensions
        if ( termsVector.size() != dfsVector.size() )
        {
            throw AQLCoreInvalidData("#Error: Invalid Curve Inputs - Inconsistent number of discount factor dates and values",__FILE__,__LINE__);
        }

        // 2. Initialize the Yield Curve Properties; Required to Get and Set Curve Index Info
        // ---------------------------------------
        AQLObjectPool& objPool = dataInstance->getObjectPool();
        AQLObjectHolder objHolder = objPool.getObject( curveid, ENCHKTYPE_NOCHECK );

        // 3. Update the Curve Data
        // Interate Over Each Curve Index in the Curve Collection and Update
        // ---------------------------------------
        AQLString curveSuffix = "";
        for ( size_t i = 0; i < curveNames.size(); ++i )
	    {
            // Skip Blanks
            if( curveNames[i].size() == 0 )
            {
                continue;
            }

            if (curveNames[i] != STD) 
		    {
			    curveSuffix = "_" + curveNames[i];
		    }
	
            // If object exists: remove first then add
	        if(objHolder.isDefined())
	        {
	            objHolder.remove( CALIBRATION_DATA_TERMS + curveSuffix );
                objHolder.remove( IR_CALIBRATION_DATA_DFS + curveSuffix );
            
                // Clear this legacy Discount Factor 2 data also
                objHolder.remove( IR_CALIBRATION_DATA_DFS2 + curveSuffix ); 
            
                objHolder.add( CALIBRATION_DATA_TERMS + curveSuffix,  new AQLDataDoubles( termsVector ) );
	    	    objHolder.add( IR_CALIBRATION_DATA_DFS + curveSuffix, new AQLDataDoubles( dfsVector ) );
            }
            else
            {
                AQLObject* e = new AQLObject();
                e->add( CALIBRATION_DATA_TERMS + curveSuffix,  new AQLDataDoubles( termsVector ) );
	    	    e->add( IR_CALIBRATION_DATA_DFS + curveSuffix, new AQLDataDoubles( dfsVector ) );
            }
        }
    
    }

    /* @brief			Function to get the forward rates from a curve returning a forwardRateTable structure containing dates, terms and forward rates
    *  @param [in]		dataInstance		            The object pool dataInstance std::map
    *  @param [in]		curveid		            The curveCollection to use when accessing the curveIndices
    *  @param [in]		curveName               The curveindex within the curve
    *  @param [out]     ForwardRateTable        The forward rate table results
    */
    ForwardRateTable LACurveCalibrationHelpers::getCurveForwardRateTable( AQLDataInstance* dataInstance, const AQLString& curveid, const AQLString& curveName )
    {
        AQLObjectPool& objPool = dataInstance->getObjectPool();
	
	    AQLString suffix = "_" + curveName;
	    if (curveName == STD)
	    {
		    suffix = "";
	    }
	
        // Get the terms matrix and forward rates vector
        const AQLObject& yieldCurveProps	= objPool.getObject( curveid, ENCHKTYPE_ISDEFINED ).get();
        DoubleMatrix termsMatrix    = dynamic_cast<const AQLDataDoubleMatrix &>( yieldCurveProps.getData(CALIBRATION_DATA_FWDTERMSMATRIX + suffix, ISNOTNULL ).get() ).get();
        DoubleArray	forwardsVector  = dynamic_cast<const AQLDataDoubles& >( yieldCurveProps.getData( IR_CALIBRATION_DATA_FORWARDRATES + suffix, ISDEFINED ).get() ).get();
    
         // Check Input Dimensions
        if ( termsMatrix.size() != 2 ) // Matrix with 2 rows: terms[0] forward start date terms, terms[1] forward end date terms
        {
            throw AQLCoreInvalidData("#Error: Invalid Curve Inputs - Inconsistent forward date dimensions; A matrix of forward start and end dates are required",__FILE__,__LINE__);
        }

        if ( termsMatrix[0].size() != termsMatrix[1].size() ) // Matrix with 2 rows: terms[0] forward start date terms, terms[1] forward end date terms
        {
            throw AQLCoreInvalidData("#Error: Invalid Curve Inputs - Inconsistent number of forward start- and end dates",__FILE__,__LINE__);
        }

        // Check Input Dimensions
        if ( termsMatrix[0].size() != forwardsVector.size() )
        {
            throw AQLCoreInvalidData("#Error: Invalid Curve Inputs - Inconsistent number of forward rate dates and values",__FILE__,__LINE__);
        }

        // Get the Fixing Dates corresponding to the terms daycount fractions
        // --------------------------------------
        DateVector  fixingDatesVector( termsMatrix[0].size(), AQLDate() );

        // Get the Curve AsOfDate - note curveid is also known as curve collection
        AQLDate asOfDate = dynamic_cast<const AQLDataDate& > ( objPool.getObject( curveid ).get().getData( CALIBRATION_DATA_ASOFDATE, ISNOTNULL ).get() ).get();

        // Imply the corresponding term fixing dates
        for( size_t i = 0; i<termsMatrix[0].size() ; ++i )
        {
            // Curve terms are always calculated using ACT/365_ISDA as defined in ConstantDeclarations.h [sic] - see #define AC_365I
		    // Etrading curves have been migrated to ACT/365 because ACT/365_ISDA causes leap year irregularities in the yield curve
		    AQLString ACT_365 = AQLString("ACT/365");
            fixingDatesVector[i] = LADateScheduleHelpers::getDateFromTerm( asOfDate, termsMatrix[0][i], ACT_365);
        }
   
        // Return Results
        ForwardRateTable resultsTable;
        resultsTable.forwardStartTerms_ = termsMatrix[0];
        resultsTable.forwardEndTerms_   = termsMatrix[1];
	    resultsTable.forwardRates_      = forwardsVector;
        resultsTable.fixingDates_       = fixingDatesVector;

	    return resultsTable;
    }

    /* @brief			Function to set the forward rates on a curve
    *  @param [in]		dataInstance		            The object pool dataInstance std::map
    *  @param [in]		curveid		            The curveCollection to use when accessing the curveIndices
    *  @param [in]		curveNames              A vector of curveindex names to set within the curve
    *  @param [in]      ForwardRateTable        The forward rate table inputs
    */
    void LACurveCalibrationHelpers::setCurveForwardRateTable( AQLDataInstance* dataInstance, const AQLString& curveid, const std::vector<AQLString>& curveNames, const ForwardRateTable& forwardRateTable )
    {
            // 1. Get the terms and discount factors from the discountFactorTable
        // ---------------------------------------
        DoubleVector startTerms     = forwardRateTable.forwardStartTerms_;
        DoubleVector endTerms       = forwardRateTable.forwardEndTerms_;
    
        DoubleMatrix fixingStartAndEndTerms(2);
        fixingStartAndEndTerms[0]   = startTerms;
        fixingStartAndEndTerms[1]   = endTerms;

        // Matrix with 2 rows: terms[0] forward start date terms, terms[1] forward end date terms
        DoubleMatrix termsMatrix    = fixingStartAndEndTerms; 
        DoubleArray	forwardsVector  = forwardRateTable.forwardRates_;
    
        // Check Input Dimensions
        if ( termsMatrix.size() != 2 ) // Matrix with 2 rows: terms[0] forward start date terms, terms[1] forward end date terms
        {
            throw AQLCoreInvalidData("#Error: Invalid Curve Inputs - Inconsistent forward date dimensions; A matrix of forward start and end dates are required",__FILE__,__LINE__);
        }

        // Matrix with 2 rows: terms[0] forward start date terms, terms[1] forward end date terms
        if ( termsMatrix[0].size() != termsMatrix[1].size() )
        {
            throw AQLCoreInvalidData("#Error: Invalid Curve Inputs - Inconsistent number of forward start- and end dates", __FILE__,__LINE__);
        }

        // Check Input Dimensions
        if ( termsMatrix[0].size() != forwardsVector.size() )
        {
            throw AQLCoreInvalidData("#Error: Invalid Curve Inputs - Inconsistent number of forward rate dates and values", __FILE__,__LINE__);
        }

        // 2. Initialize the Yield Curve Properties; Required to Get and Set Curve Index Info
        // ---------------------------------------
        AQLObjectPool& objPool = dataInstance->getObjectPool();
        AQLObjectHolder objHolder = objPool.getObject( curveid, ENCHKTYPE_NOCHECK );

        // 3. Update the Curve Data
        // Interate Over Each Curve Index in the Curve Collection and Update
        // ---------------------------------------
        AQLString curveSuffix = "";
        for ( size_t i = 0; i < curveNames.size(); ++i )
	    {
            // Skip Blanks
            if( curveNames[i].size() == 0 )
            {
                continue;
            }

            if (curveNames[i] != STD) 
		    {
			    curveSuffix = "_" + curveNames[i];
		    }
                        
		    // If object exists: remove first then add
	        if(objHolder.isDefined())
	        {
	            objHolder.remove( CALIBRATION_DATA_FWDTERMSMATRIX + curveSuffix );
                objHolder.remove( IR_CALIBRATION_DATA_FORWARDRATES + curveSuffix );
            
                objHolder.add( CALIBRATION_DATA_FWDTERMSMATRIX + curveSuffix, new AQLDataDoubleMatrix( termsMatrix ) );
	    	    objHolder.add( IR_CALIBRATION_DATA_FORWARDRATES + curveSuffix,new AQLDataDoubles( forwardsVector ) );
            }
            else
            {
                AQLObject* e = new AQLObject();
                e->add( CALIBRATION_DATA_FWDTERMSMATRIX + curveSuffix, new AQLDataDoubleMatrix( termsMatrix ) );
	    	    e->add( IR_CALIBRATION_DATA_FORWARDRATES + curveSuffix,new AQLDataDoubles( forwardsVector ) );
            }
        }

    }

    void 
    LACurveCalibrationHelpers::restoreSwapRateFromLibor(AQLMathYieldCurvePro &curve, const AQLString &ccy, const std::map<AQLString, double> &sRateMap, AQLStringMatrix sdata)
    {
	    //AQLDate spotDate	= LADateScheduleHelpers::getLADate(chgrow(sdata,CURVEINPUT_SPOTDATE,1));
	    std::map<AQLString, AQLString>& dvar = AQLCoreComponentManager::getDayCountMap();
	    std::map<AQLString, AQLString>::iterator itd = dvar.find(chgrow(sdata,CURVEINPUT_DAYCOUNT,1));
	    if(itd==dvar.end())
		    throw AQLCoreInvalidData("Swap Daycount is not registered ", __FILE__,__LINE__);
	    AQLString dcStr			= itd->second;
	    AQLString calStr			= chgrow(sdata,CURVEINPUT_CALENDAR,1);
	    AQLString slidingStr		= chgrow(sdata,CURVEINPUT_SLIDINGRULE,1);
	    AQLString freq			= chgrow(sdata,CURVEINPUT_FREQUENCY,1);

	    AQLDataMultiReference &refMarkets = curve.getMarketData();
	    const int mSize = refMarkets.getSize();
	    // set data for holiday adjustment
	    // daycount
	    AQLPriceDataDayCount dc;
	    dc.convertFromString(dcStr);
	    // sliding rule
	    AQLPriceDataSlidingRule sliding;
	    sliding.convertFromString(slidingStr);
	    // calendar
	    AQLPriceDataCalendar cal;
	    cal.convertFromString(calStr);
	    // asOfDate
	    const AQLDate asOfDate = curve.getAsOfDate();
	    // spotDate
	    AQLDate spotDate;
	    int tmprow1,tmprow2;
        tmprow1 = AQLFunctionUtilities::findRowsNumber(sdata,CURVEINPUT_SPOTLAG);
        tmprow2= AQLFunctionUtilities::findRowsNumber(sdata,CURVEINPUT_SPOTDATE);
        if( tmprow1<0 && tmprow2<0) 
        {
            throw AQLCoreInvalidData("input spot date or spot lag", __FILE__,__LINE__);
        }
        else if(tmprow1>=0 && tmprow2>=0)
        {
		    throw AQLCoreInvalidData("do not input spot date and spot lag", __FILE__,__LINE__);
	    }
	    else if(tmprow1>=0)
	    {		
		    AQLString spotLag = AQLFunctionUtilities::findElement(sdata,CURVEINPUT_SPOTLAG,0,1,true);
            spotDate = LADateScheduleHelpers::getDate(asOfDate,spotLag,slidingStr,calStr);
        }
        else if(tmprow2>=0)
        {
            spotDate = LADateScheduleHelpers::getLADate(chgrow(sdata,CURVEINPUT_SPOTDATE,1));
        }
	    if (asOfDate > spotDate)
		    throw AQLCoreInvalidData("AsofDate > spotDate, cannnot calc. ", __FILE__, __LINE__);

	    for (size_t i = 0; i < size_t(mSize); ++i)
	    {
		    AQLObjectHolder &mktData = refMarkets.get(i);

		    AQLString dataType = dynamic_cast<const AQLDataString &>
						    (mktData.getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL).get()).get();

		    dataType.toUpper();
		    if (dataType == ZERO || dataType == PAR)
		    {
			    const AQLString &termStr = dynamic_cast<const AQLDataString &>(mktData.getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL).get());
			    map<AQLString, double>::const_iterator it = sRateMap.find(termStr);
			    if (it != sRateMap.end())
			    {
				    // set swap rate and information
				    // set original swap value
				    mktData.remove(CALIBRATION_DATA_RATE);
				    mktData.add(CALIBRATION_DATA_RATE, new AQLDataDouble(it->second));
				    // set spot date
				    mktData.remove(IR_CALIBRATION_DATA_SPOTDATE);
				    mktData.add(IR_CALIBRATION_DATA_SPOTDATE, new AQLDataDate(spotDate));
				    // set calendar
				    mktData.remove(CALIBRATION_DATA_CALENDAR);
				    mktData.add(CALIBRATION_DATA_CALENDAR , new AQLPriceDataCalendar()).convertFromString(calStr);
				    // set daycount
				    mktData.remove(IR_CALIBRATION_DATA_DAYCOUNT);
				    mktData.add(IR_CALIBRATION_DATA_DAYCOUNT, new AQLPriceDataDayCount()).convertFromString(dcStr);
				    // set data type
				    mktData.remove(IR_CALIBRATION_DATA_DATATYPE);
				    mktData.add(IR_CALIBRATION_DATA_DATATYPE, new AQLDataString()).convertFromString(PAR);
				    // set frequency
				    mktData.remove(IR_CALIBRATION_DATA_FREQUENCY);
				    mktData.add(IR_CALIBRATION_DATA_FREQUENCY, new AQLDataString()).convertFromString(freq);
				    // set slidingrule
				    mktData.remove(CALIBRATION_DATA_SLIDINGRULE);
				    mktData.add(CALIBRATION_DATA_SLIDINGRULE, new AQLPriceDataSlidingRule()).convertFromString(slidingStr);
			    }
		    }
	    }
	    return;
    }

    void 
    LACurveCalibrationHelpers::resetMarketDataUseL(AQLMathYieldCurvePro &curve, const AQLString &ccy, AQLStringMatrix ldata)
    {
	    //AQLDate spotDate	= LADateScheduleHelpers::getLADate(chgrow(ldata,CURVEINPUT_SPOTDATE,1));
	    std::map<AQLString, AQLString>& dvar = AQLCoreComponentManager::getDayCountMap();
	    std::map<AQLString, AQLString>::iterator itd = dvar.find(chgrow(ldata,CURVEINPUT_DAYCOUNT,1));
	    if(itd==dvar.end())
		    throw AQLCoreInvalidData("Libor Daycount is not registered ", __FILE__,__LINE__);
	    AQLString dcStr			= itd->second;
	    AQLString calStr			= chgrow(ldata,CURVEINPUT_CALENDAR,1);
	    AQLString slidingStr		= chgrow(ldata,CURVEINPUT_SLIDINGRULE,1);
	
	    AQLDataMultiReference &refMarkets = curve.getMarketData();
	    const int mSize = refMarkets.getSize();
	    // set data for holiday adjustment
	    // daycount
	    AQLPriceDataDayCount dc;
	    dc.convertFromString(dcStr);
	    // sliding rule
	    AQLPriceDataSlidingRule sliding;
	    sliding.convertFromString(slidingStr);
	    // calendar
	    AQLPriceDataCalendar cal;
	    cal.convertFromString(calStr);
	    // asOfDate
	    const AQLDate asOfDate = curve.getAsOfDate();
	    // spotDate
	    AQLDate spotDate;
	    int tmprow1,tmprow2;
        tmprow1 = AQLFunctionUtilities::findRowsNumber(ldata,CURVEINPUT_SPOTLAG);
        tmprow2= AQLFunctionUtilities::findRowsNumber(ldata,CURVEINPUT_SPOTDATE);
        if( tmprow1<0 && tmprow2<0) 
        {
            throw AQLCoreInvalidData("input spot date or spot lag", __FILE__,__LINE__);
        }
        else if(tmprow1>=0 && tmprow2>=0)
        {
		    throw AQLCoreInvalidData("do not input spot date and spot lag", __FILE__,__LINE__);
	    }
	    else if(tmprow1>=0)
	    {		
		    AQLString spotLag = AQLFunctionUtilities::findElement(ldata,CURVEINPUT_SPOTLAG,0,1,true);
            spotDate = LADateScheduleHelpers::getDate(asOfDate,spotLag,slidingStr,calStr);
        }
        else if(tmprow2>=0)
        {
            spotDate = LADateScheduleHelpers::getLADate(chgrow(ldata,CURVEINPUT_SPOTDATE,1));
        }
	    if (asOfDate > spotDate)
		    throw AQLCoreInvalidData("AsofDate > spotDate, cannnot calc. ", __FILE__, __LINE__);

	    double termSpot = dc.getTerm(asOfDate, spotDate);
	    double dfSpot   = curve.getDF(termSpot);

	    AQLStringVector liborYTerm;
	    // calc rate
	    for (int i = 0; i < mSize; ++i)
	    {
		    AQLObjectHolder &mktData = refMarkets.get(i);
		    AQLString dataType = dynamic_cast<const AQLDataString &>
								    (mktData.getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL).get()).get();

		    dataType.toUpper();

		    if (dataType != O_N && dataType != T_N 
			    && dataType != ZERO)
		    {
			    const AQLString &termStr = dynamic_cast<const AQLDataString &>
								    (mktData.getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL).get()).get();

			    AQLString searchTerm = termStr;
			    if (termStr == "12M")
			    {
				    searchTerm = "1Y";
			    }
			    if (std::find(liborYTerm.begin(), liborYTerm.end(), searchTerm) == liborYTerm.end())
			    {
				    // calc date from spotDate
				    AQLDate date = LADateHelpers::getDate(spotDate, termStr, sliding, &cal, true);
					
				    double term     = dc.getTerm(asOfDate, date);
				    double df       = curve.getDF(term);
				    double delta    = term - termSpot;

				    // calc libor rate
				    double rate = (dfSpot - df) / (df * delta);

				    // reset attr libor value
				    mktData.remove(CALIBRATION_DATA_RATE);
				    mktData.add(CALIBRATION_DATA_RATE, new AQLDataDouble(rate));
				    // set spot date
				    mktData.remove(IR_CALIBRATION_DATA_SPOTDATE);
				    mktData.add(IR_CALIBRATION_DATA_SPOTDATE, new AQLDataDate(spotDate));
				    // set calendar
				    mktData.remove(CALIBRATION_DATA_CALENDAR);
				    mktData.add(CALIBRATION_DATA_CALENDAR , new AQLPriceDataCalendar()).convertFromString(calStr);
				    // set daycount
				    mktData.remove(IR_CALIBRATION_DATA_DAYCOUNT);
				    mktData.add(IR_CALIBRATION_DATA_DAYCOUNT, new AQLPriceDataDayCount()).convertFromString(dcStr);
				    // set data type
				    mktData.remove(IR_CALIBRATION_DATA_DATATYPE);
				    mktData.add(IR_CALIBRATION_DATA_DATATYPE, new AQLDataString()).convertFromString(ZERO);
				    // set frequency
				    mktData.remove(IR_CALIBRATION_DATA_FREQUENCY);
				    mktData.add(IR_CALIBRATION_DATA_FREQUENCY, new AQLDataString()).convertFromString(SIMPLE);
				    // set slidingrule
				    mktData.remove(CALIBRATION_DATA_SLIDINGRULE);
				    mktData.add(CALIBRATION_DATA_SLIDINGRULE, new AQLPriceDataSlidingRule()).convertFromString(slidingStr);
			    }
		    }
		    else if (dataType == ZERO)
		    {
			    const AQLString &termStr = dynamic_cast<const AQLDataString &>
								    (mktData.getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL).get()).get();

			    if (termStr == "12M")
			    {
				    liborYTerm.push_back("1Y");
			    }
			    else if (termStr.findString("Y") >= 0)
			    {
				    liborYTerm.push_back(termStr);
			    }
		    }
	    }
	
	

	    return;
    }
    
}

