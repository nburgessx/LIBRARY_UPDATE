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
#include "LAFunctionUtilities.h"
#include "LAObject.h"
#include "LADataBasics.h"
#include "LADataVector.h"
#include "LADataMatrix.h"
#include "LACoreTemplateType.h"
#include "LAMathDefine.h"
#include "LAPriceDataCalendar.h"
#include "LAPriceDataSlidingRule.h"
#include "LABasic.h"
#include "LAAlgorithm.h"
#include "LASplineInterpolation.h"
#include "LAStepInterpolation.h"
#include "LALinearInterpolation.h"
#include "LAInterpolationBase.h"
#include "LAPriceDataDayCount.h"
#include "LAPriceCFGenUtility.h"
#include "LAPriceYieldGenerator.h"
#include "LADataReference.h"
#include "LAAnalyticFormula.h"
#include "LABlackScholesCalc.h"
#include "LAMathYieldCurvePro.h"
#include "LADataProcedure.h"
#include "LAPriceDataInterpolation.h"
#include "LACoreComponentManager.h"
#include "LAPriceDataConvention.h"
#include "LADataMultiReference.h"
#include "LAMathValuableEntity.h"
#include "LAPriceDataFunction.h"
#include "LAPriceArbFreeGenerator.h"
#include "LACoreUtility.h"

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
    LACurveCalibrationHelpers::setUpCurveFromMarket(LADataInstance* dataInstance, const LAString& CurveID, 
						    LAStringMatrix mgrid, LAStringMatrix lgrid, 
						    LAStringMatrix fgrid, LAStringMatrix sgrid,
						    LAStringMatrix mdata, LAStringMatrix ldata, 
						    LAStringMatrix fdata, LAStringMatrix sdata,
						    LAStringMatrix ylddata, bool isCheckDF, bool isAUDadjust)
    {
	    //ylddata
	    LAString	CurveIDTool			= CurveID+TOOL;
	    LAString	CurveIDManager		= CurveID + STD+ MANAGER;

	    LAObjectPool& objPool = dataInstance->getObjectPool(); 
	    const LAObjectHolder objHolder = objPool.getObject(CurveIDManager,ENCHKTYPE_NOCHECK);
	
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
	
	    LADate		asofdate			= LADateScheduleHelpers::getLADate(chgrow(ylddata,CURVEINPUT_ASOFDATE,1));
	    LAString	Currency			= chgrow(ylddata,CURVEINPUT_CURRENCY,	1);
	    LAString	strfutureuse		= chgrow(ylddata,CURVEINPUT_ISFUTUREUSE,1);
	
	    // check STD curve 
	    LAString curveType;
	    if (LAFunctionUtilities::findRowsNumber(ylddata,CURVEINPUT_CURVETYPE) < 0)
	    {
		    curveType = BASECURVE;
	    }
	    else
	    {
		    curveType = chgrow(ylddata, CURVEINPUT_CURVETYPE, 1);
	    } 
	    LAString suffix = LAString("");
	    bool isSTD = true;
	    if (curveType != BASECURVE && curveType != OISCURVE)
	    {
		    suffix = LAString("_") + curveType;
		    isSTD = false;
	    }

	    //intertplation 
	    std::map<LAString, LAString>& ivar = LACoreComponentManager::getInterpolationMap();
	    std::map<LAString, LAString>::iterator it;
	
	    it = ivar.find(chgrow(ylddata,CURVEINPUT_FUTUREINTERPOLATION,1		));
	    if(it==ivar.end())
		    throw LACoreInvalidData("Future interpolation is not registered in interpolation methods", __FILE__,__LINE__);
	    LAString futureinterpolation	= it->second;
	
	    it = ivar.find(chgrow(ylddata,CURVEINPUT_YIELDGENINTERPOLATION,1	));
	    if(it==ivar.end())
		    throw LACoreInvalidData("YieldGenInterpolation is not registered in interpolation methods", __FILE__,__LINE__);
	    LAString yieldgeninterpolation	= it->second;
	
	    it = ivar.find(chgrow(ylddata,CURVEINPUT_INTERPOLATION,1			));
	    if(it==ivar.end())
		    throw LACoreInvalidData("Interpolation is not registered in interpolation methods",			__FILE__,__LINE__);
	    LAString interpolation = it->second;

	    LADataBool bl;
	    bl.convertFromString(strfutureuse);
	    bool isfuteruse = bl.get();
	    //YieldData
	    LAObject* e = NULL;
	    if(!objPool.getObject(CurveID).isDefined())
	    {	
		    e = new LAObject;
		    objPool.set(CurveID,e);
		    e->add(CALIBRATION_DATA_NAME, new LADataString(CurveID));
	    }
	    else
	    {
		    if (isSTD)	objPool.getObject(CurveID).get().clear();
		    e= &objPool.getObject(CurveID).get();
	    }
	
	    //CurveCalibrationData
	    const LAObjectHolder ehCur = objPool.getObject(CurveIDTool);
	    LAMathYieldCurvePro * curveCalibrationData = NULL; 
	    if(!ehCur.isDefined())
	    {	
		    curveCalibrationData= new LAMathYieldCurvePro(dataInstance);
		    objPool.set(CurveIDTool,curveCalibrationData);
	    }
	    else
	    {
		    dynamic_cast<LAMathYieldCurvePro &>(objPool.getObject(CurveIDTool).get()).reset();
		    curveCalibrationData	= &dynamic_cast<LAMathYieldCurvePro &>(objPool.getObject(CurveIDTool).get());
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
		    curveCalibrationData->LAObject::add(IR_CALIBRATION_DATA_GENTARGETDF, new LADataString(curveType));
		    LAStringVector tmpCurveType(1,curveType);
		    curveCalibrationData->LAObject::add(IR_CALIBRATION_DATA_GENERATEDFS, new LADataStrings(tmpCurveType));

		    curveCalibrationData->setAssignedCurveMktMap(curveType,curveType);
	    }
	    //RatePriority
	    const LAString& ratepriority = LAFunctionUtilities::findElement(ylddata,RATEPRIORITY,0,1,true,false);
	    if (ratepriority == "SWAP:LIBOR:FUTURE"||ratepriority == "SWAP:FUTURE:LIBOR"||ratepriority == "FUTURE:LIBOR:SWAP"||ratepriority == "FUTURE:SWAP:LIBOR"||ratepriority == "LIBOR:SWAP:FUTURE"||ratepriority == "LIBOR:FUTURE:SWAP")
		    curveCalibrationData->getRatePriority().convertFromString(ratepriority);
	    else if (ratepriority == "")
	    {	
		    const LAStringVector tmpstrvec(0);
		    curveCalibrationData->getRatePriority().set(tmpstrvec);
	    }
	    else
		    throw LACoreInvalidData("RatePriority is incorrect",			__FILE__,__LINE__);
	
	    //daycount
	    std::map<LAString, LAString>& dvar = LACoreComponentManager::getDayCountMap();
	    std::map<LAString, LAString>::iterator itd;
	
	    LADate spotdate;
	    LAString ref;
	    LAString Term;
	    LAString srule;
	    LAStringVector city;
        LAString calendar;
	    LAString dc;
	    int N=0;
	    double rate=0.0;
	    //Money Market
        //spotdate	= LADateScheduleHelpers::getLADate(chgrow(mdata,CURVEINPUT_SPOTDATE,1));
	    itd			= dvar.find(chgrow(mdata,CURVEINPUT_DAYCOUNT,1));
	    if(itd==dvar.end())
		    throw LACoreInvalidData("Money Market Daycount is not registered ", __FILE__,__LINE__);
	    dc			= itd->second;
	    city		= chgrow(mdata,CURVEINPUT_CALENDAR,1).toToken(':');
        calendar    = chgrow(mdata,CURVEINPUT_CALENDAR,1);
	    srule		= chgrow(mdata,CURVEINPUT_SLIDINGRULE,1);
	    N			= mgrid.size();

        int tmprow1,tmprow2;
        tmprow1 = LAFunctionUtilities::findRowsNumber(mdata,CURVEINPUT_SPOTLAG);
        tmprow2= LAFunctionUtilities::findRowsNumber(mdata,CURVEINPUT_SPOTDATE);
        if( tmprow1<0 && tmprow2<0) 
        {
            throw LACoreInvalidData("input spot date or spot lag", __FILE__,__LINE__);
        }
        else if(tmprow1>=0 && tmprow2>=0)
        {
		    throw LACoreInvalidData("do not input spot date and spot lag", __FILE__,__LINE__);
	    }
	    else if(tmprow1>=0)
	    {		
		    LAString spotLag = LAFunctionUtilities::findElement(mdata,CURVEINPUT_SPOTLAG,0,1,true);
            spotdate = LADateScheduleHelpers::getDate(asofdate,spotLag,srule,calendar);
        }
        else if(tmprow2>=0)
        {
            spotdate = LADateScheduleHelpers::getLADate(chgrow(mdata,CURVEINPUT_SPOTDATE,1));
        }

	    //i=row=0 is input row (not grid);
	    for(int i=1; i<N; i++)
	    {
		    LAString name;
		    LAString type;
		    LAObject* mme = NULL;

		    type = chgcol(mgrid,CURVEINPUT_TYPE,i).getCString();
		    name = CurveID + Currency + type;

		    if(!objPool.getObject(name).isDefined())
		    {	
			    mme = new LAObject;
			    objPool.set(name, mme);
		    }
		    else
		    {
			    objPool.getObject(name).get().clear();
			    mme= &objPool.getObject(name).get();
		    }

		    rate = chgcol(mgrid,CURVEINPUT_RATE,i).getDoubleValue();
		    mme->add(CALIBRATION_DATA_NAME,				new LADataString(name)							);
		    mme->add(IR_CALIBRATION_DATA_DATATYPE,		new LADataString(type)							);
		    mme->add(IR_CALIBRATION_DATA_DAYCOUNT,		new LAPriceDataDayCount()							).convertFromString(dc);
		    mme->add(CALIBRATION_DATA_CALENDAR,			new LAPriceDataCalendar(city)						);
		    mme->add(CALIBRATION_DATA_SLIDINGRULE,		new LAPriceDataSlidingRule()							).convertFromString(srule);
		    mme->add(IR_CALIBRATION_DATA_SPOTDATE,		new LADataDate(spotdate)						);
		    mme->add(CALIBRATION_DATA_RATE,				new LADataDouble(rate)							);
		    //ref+= (1==i) ? name + ":" : name;
		    ref+= name + ":";
	    }
	    ////LiborData
	    //spotdate	= LADateScheduleHelpers::getLADate(chgrow(ldata,CURVEINPUT_SPOTDATE,1));
	    itd			= dvar.find(chgrow(ldata,CURVEINPUT_DAYCOUNT,1));
	    if(itd==dvar.end())
		    throw LACoreInvalidData("Libor Daycount is not registered ", __FILE__,__LINE__);
	    dc			= itd->second;
	    city		= chgrow(ldata,CURVEINPUT_CALENDAR,1).toToken(':');
        calendar	= chgrow(ldata,CURVEINPUT_CALENDAR,1);
	    srule		= chgrow(ldata,CURVEINPUT_SLIDINGRULE,1);
	    tmprow1= LAFunctionUtilities::findRowsNumber(ylddata,CURVEINPUT_ISONFORSPOTADJUST);
	    LAString isOnForSpotAdjust_str = "FALSE";
	    if (tmprow1 >= 0)
	    {
		     isOnForSpotAdjust_str = chgrow(ylddata,CURVEINPUT_ISONFORSPOTADJUST,1);
	    }
	    N = lgrid.size();
	    LAPriceDataDayCount dc_Libor_attr; dc_Libor_attr.convertFromString(dc);
	    LAPriceDataCalendar cal_Libor_attr; cal_Libor_attr.convertFromString(calendar);
	    LAPriceDataSlidingRule sr_Libor_attr; sr_Libor_attr.convertFromString(srule);
	    LADataBool isOnForSpotAdjust_attr; isOnForSpotAdjust_attr.convertFromString(isOnForSpotAdjust_str);

        tmprow1= LAFunctionUtilities::findRowsNumber(ldata,CURVEINPUT_SPOTLAG);
        tmprow2= LAFunctionUtilities::findRowsNumber(ldata,CURVEINPUT_SPOTDATE);
        if( tmprow1<0 && tmprow2<0) 
        {
            throw LACoreInvalidData("input spot date or spot lag", __FILE__,__LINE__);
        }
	    else if(tmprow1>=0 && tmprow2>=0)
        {
		    throw LACoreInvalidData("do not input spot date and spot lag", __FILE__,__LINE__);
	    }
	    else if(tmprow1>=0)
	    {		
		    LAString spotLag = LAFunctionUtilities::findElement(ldata,CURVEINPUT_SPOTLAG,0,1,true);
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
		    LAString name	=	CurveID + Currency + LIBOR + Term;
		
		    LAObject* le=NULL; 
		    if(!objPool.getObject(name).isDefined())
		    {	
			    le = new LAObject;
			    objPool.set(name, le);
		    }
		    else
		    {
			    objPool.getObject(name).get().clear();
			    le= &objPool.getObject(name).get();
		    }
		    rate = chgcol(lgrid,CURVEINPUT_RATE,i).getDoubleValue();
		    le->add(CALIBRATION_DATA_NAME,				new LADataString(name)							);
		    le->add(IR_CALIBRATION_DATA_TERM,				new LADataString(Term)							);
		    le->add(IR_CALIBRATION_DATA_DATATYPE,			new LADataString(ZERO)							);
		    le->add(IR_CALIBRATION_DATA_FREQUENCY,		new LADataString(SIMPLE)						);
		    if (Currency.toUpper() == "AUD" && Term == "7D")
		    {
			    le->add(IR_CALIBRATION_DATA_DAYCOUNT, new LAPriceDataDayCount()).convertFromString(AC_360);
		    }
		    else
		    {
			    le->add(IR_CALIBRATION_DATA_DAYCOUNT, new LAPriceDataDayCount(dc_Libor_attr) );
		    }
		    le->add(CALIBRATION_DATA_CALENDAR,			new LAPriceDataCalendar(cal_Libor_attr)				);
		    le->add(CALIBRATION_DATA_SLIDINGRULE,			new LAPriceDataSlidingRule(sr_Libor_attr)			);
		    le->add(IR_CALIBRATION_DATA_SPOTDATE,			new LADataDate(spotdate)						);
		    le->add(CALIBRATION_DATA_RATE,				new LADataDouble(rate)							);
		    le->add(IR_CALIBRATION_DATA_ISONFORSPOTADJUST,	new LADataBool(isOnForSpotAdjust_attr)		);
		    ref+= name + ":";
	    }
	    ////SwapData
	    //spotdate	= LADateScheduleHelpers::getLADate(chgrow(sdata,CURVEINPUT_SPOTDATE,1));
	    itd			= dvar.find(chgrow(sdata,CURVEINPUT_DAYCOUNT,1));
	    if(itd==dvar.end())
		    throw LACoreInvalidData("Swap Daycount is not registered ", __FILE__,__LINE__);
	    dc			= itd->second;
	    city		= chgrow(sdata,CURVEINPUT_CALENDAR,1).toToken(':');
        calendar    = chgrow(sdata,CURVEINPUT_CALENDAR,1);
	    srule		= chgrow(sdata,CURVEINPUT_SLIDINGRULE,1);
	    tmprow1= LAFunctionUtilities::findRowsNumber(ylddata,CURVEINPUT_ISNEWTONRAPHSON);
	    LAString isNewtonRaphson_str = "FALSE";
	    if (tmprow1 >= 0)
	    {
		     isNewtonRaphson_str = chgrow(ylddata,CURVEINPUT_ISNEWTONRAPHSON,1);
	    }
	    N = sgrid.size();
	    LAString frequency_base = chgrow(sdata,CURVEINPUT_FREQUENCY,1);
	    LAPriceDataDayCount dc_Swap_attr; dc_Swap_attr.convertFromString(dc);
	    LAPriceDataCalendar cal_Swap_attr; cal_Swap_attr.convertFromString(calendar);
	    LAPriceDataSlidingRule sr_Swap_attr; sr_Swap_attr.convertFromString(srule);
	    LADataBool isNewtonRaphson_attr; isNewtonRaphson_attr.convertFromString(isNewtonRaphson_str);

        tmprow1= LAFunctionUtilities::findRowsNumber(sdata,CURVEINPUT_SPOTLAG);
        tmprow2= LAFunctionUtilities::findRowsNumber(sdata,CURVEINPUT_SPOTDATE);
        if( tmprow1<0 && tmprow2<0) 
        {
            throw LACoreInvalidData("input spot date or spot lag", __FILE__,__LINE__);
        }
	    else if(tmprow1>=0 && tmprow2>=0)
        {
		    throw LACoreInvalidData("do not input spot date and spot lag", __FILE__,__LINE__);
	    }
        else if(tmprow1>=0)
        {
            LAString spotLag = LAFunctionUtilities::findElement(sdata,CURVEINPUT_SPOTLAG,0,1,true);
            spotdate = LADateScheduleHelpers::getDate(asofdate,spotLag,srule,calendar);
        }
        else if(tmprow2>=0)
        {
            spotdate = LADateScheduleHelpers::getLADate(chgrow(sdata,CURVEINPUT_SPOTDATE,1));
        }

	    //i=row=0 is input row (not grid);
	    bool resetFlg = true;
	    bool tempresetFlg = true;
	    map<LAString, double> aud_origSwapRate;
	    LAString frequency;
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
			    LADataProcedure &modelDataObj = dynamic_cast<LADataProcedure &>
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
		    LAObject* se	=	NULL;
		    LAString name	=	CurveID + Currency + SWAP + Term;
		    if(!objPool.getObject(name).isDefined())
		    {	
			    se = new LAObject;
			    objPool.set(name, se);
		    }
		    else
		    {
			    objPool.getObject(name).get().clear();
			    se= &objPool.getObject(name).get();
		    }
		    rate = chgcol(sgrid,CURVEINPUT_RATE,i).getDoubleValue();
		    se->add(CALIBRATION_DATA_NAME,				new LADataString(name)							);
		    se->add(IR_CALIBRATION_DATA_TERM,				new LADataString(Term)							);
		    se->add(IR_CALIBRATION_DATA_DATATYPE,			new LADataString(PAR)							);
		    se->add(IR_CALIBRATION_DATA_FREQUENCY,		new LADataString(frequency)						);
		    se->add(IR_CALIBRATION_DATA_BASEFREQUENCY_FLOAT,		new LADataString(frequency)				);
		    se->add(IR_CALIBRATION_DATA_FREQUENCY_FLOAT,		new LADataString(frequency)				);
		    se->add(IR_CALIBRATION_DATA_DAYCOUNT,			new LAPriceDataDayCount(dc_Swap_attr)				);
		    se->add(CALIBRATION_DATA_CALENDAR,			new LAPriceDataCalendar(cal_Swap_attr)				);
		    se->add(CALIBRATION_DATA_SLIDINGRULE,			new LAPriceDataSlidingRule(sr_Swap_attr)				);
		    se->add(IR_CALIBRATION_DATA_SPOTDATE,			new LADataDate(spotdate)						);
		    se->add(CALIBRATION_DATA_RATE,				new LADataDouble(rate)							);
		    se->add(IR_CALIBRATION_DATA_ISNEWTONRAPHSONSW,	new LADataBool(isNewtonRaphson_attr)		);
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
			    throw LACoreInvalidData("Future Daycount is not registered ", __FILE__,__LINE__);
		    dc			= itd->second;
		    city		= chgrow(fdata,CURVEINPUT_CALENDAR,1).toToken(':');
            calendar		= chgrow(fdata,CURVEINPUT_CALENDAR,1);
		    srule		= chgrow(fdata,CURVEINPUT_SLIDINGRULE,1);
		    N			= fgrid.size();

            tmprow1= LAFunctionUtilities::findRowsNumber(fdata,CURVEINPUT_SPOTLAG);
            tmprow2= LAFunctionUtilities::findRowsNumber(fdata,CURVEINPUT_SPOTDATE);
            if( tmprow1<0 && tmprow2<0) 
            {
                throw LACoreInvalidData("input spot date or spot lag", __FILE__,__LINE__);
            }
		    else if(tmprow1>=0 && tmprow2>=0)
		    {
			    throw LACoreInvalidData("do not input spot date and spot lag", __FILE__,__LINE__);
		    }
            else if(tmprow1>=0)
            {
                LAString spotLag = LAFunctionUtilities::findElement(fdata,CURVEINPUT_SPOTLAG,0,1,true);
                spotdate = LADateScheduleHelpers::getDate(asofdate,spotLag,srule,calendar);
            }
            else if(tmprow2>=0)
            {
                spotdate = LADateScheduleHelpers::getLADate(chgrow(fdata,CURVEINPUT_SPOTDATE,1));
            }
		    //i=row=0 is input row (not grid);
		    for(int i=1; i<N; i++)
		    {
			    LADate	StartDate	=	LADateScheduleHelpers::getLADate(chgcol(fgrid,CURVEINPUT_STARTDATE,	i));
			    LADate EndDate		=	LADateScheduleHelpers::getLADate(chgcol(fgrid,CURVEINPUT_ENDDATE,		i));
			    LAString name		=  CurveID + Currency + FUTURE + LADataDate(StartDate).convertToString();
			    LAObject* fe		=	NULL;
			    if(!objPool.getObject(name).isDefined())
			    {	
				    fe = new LAObject;
				    objPool.set(name, fe);
			    }
			    else
			    {
				    objPool.getObject(name).get().clear();
				    fe= &objPool.getObject(name).get();
			    }
			    rate = chgcol(fgrid,CURVEINPUT_RATE,i).getDoubleValue();
			    fe->add(CALIBRATION_DATA_NAME,				new LADataString(name)							);
			    fe->add(IR_CALIBRATION_DATA_DATATYPE,			new LADataString(FUTURE)						);
			    fe->add(IR_CALIBRATION_DATA_FREQUENCY,		new LADataString(SIMPLE)						);
			    fe->add(PRICING_DATA_STARTDATE,		new LADataDate(StartDate)						);
			    fe->add(PRICING_DATA_ENDDATE,		new LADataDate(EndDate)							);
			    fe->add(IR_CALIBRATION_DATA_DAYCOUNT,			new LAPriceDataDayCount()							).convertFromString(dc);
			    fe->add(CALIBRATION_DATA_CALENDAR,			new LAPriceDataCalendar(city)						);
			    fe->add(CALIBRATION_DATA_SLIDINGRULE,			new LAPriceDataSlidingRule()							).convertFromString(srule);
			    fe->add(IR_CALIBRATION_DATA_SPOTDATE,			new LADataDate(spotdate)						);
			    fe->add(CALIBRATION_DATA_RATE,				new LADataDouble(rate)							);
			    fe->add(PRICING_DATA_FUTUREOLDMETHOD,	new LADataBool(true)						);
                fe->add(IR_CALIBRATION_DATA_APPLYTENSION,	    new LADataBool(false));
                fe->add(IR_CALIBRATION_DATA_TENSIONGAP,	    new LADataInt(1));
			    ref+= name + ":";
		    }
	    }
	    //set market object
	    if (ref.size() < 2)
		    throw LACoreInvalidData("Market Data is not set !!", __FILE__, __LINE__); 
	    ref = ref.subString(0, ref.size() - 2);
	    if (isSTD)
	    {
		    curveCalibrationData->getMarketData().convertFromString(ref);
	    }
	    else
	    {
		    curveCalibrationData->LAObject::add(CALIBRATION_DATA_MARKETDATA + suffix, new LADataMultiReference()).convertFromString(ref);
	    }
	    //generate Object
	    dataInstance->getReferencePool().completeDependency();
	    LADataHolder* dh		= &objPool.getObject(CurveIDTool).getData(CALIBRATION_DATA_CURVEGENERATOR);
	    LADataProcedure& modelDataObj	= dynamic_cast<LADataProcedure&>(dh->get());
	    modelDataObj.calibrateModel(asofdate);
	    if (!resetFlg)
		    restoreSwapRateFromLibor(*curveCalibrationData,Currency,aud_origSwapRate,sdata);
	    if (!isSTD)
	    {
		    curveCalibrationData->LAObject::remove(CALIBRATION_DATA_MARKETDATA + suffix);
	    }
	    if(isCheckDF)
	    {
		    // check df
		    DoubleArray terms = dynamic_cast<LADataDoubles &>(e->getData(CALIBRATION_DATA_TERMS, ISNOTNULL).get()).get();
		    DoubleArray dfs = dynamic_cast<LADataDoubles &>(e->getData(IR_CALIBRATION_DATA_DFS, ISNOTNULL).get()).get();

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

				    LALinearInterpolation interp = LALinearInterpolation();
				    interp.set(tmpTerms, tmpDfs);
				    vals[i] = interp.value(terms[i]);
			    }
			    else
			    {
				    b_df = dfs[i];
				    b_term = terms[i];
			    }
		    }
		    dynamic_cast<LADataDoubles &>(e->getData(IR_CALIBRATION_DATA_DFS, ISNOTNULL).get()).set(vals);
	    }

	    //curve entities manager (curve, grids, curveinformation entities)
	    LAObject* mae = NULL;
	    if(!objPool.getObject(CurveIDManager).isDefined())
	    {	
		    mae = new LAObject;
		    objPool.set(CurveIDManager,mae);
	    }
	    else
	    {
		    objPool.getObject(CurveIDManager).get().clear();
		    mae	= &objPool.getObject(CurveIDManager).get();
	    }
	    mae->add("Time",			new LADataString()			).convertFromString(LAString(LATime::now()));
	    mae->add(CALIBRATION_DATA_NAME,		new LADataString()			).convertFromString(CurveIDManager);
	    mae->add(GRIDREFS,			new LADataMultiReference()	).convertFromString(ref);
	    mae->add(YIELDDATAREF,		new LADataReference()		).convertFromString(CurveID);
	    mae->add(YIELDTOOLREF,		new LADataReference()		).convertFromString(CurveIDTool);
    }

    void 
    LACurveCalibrationHelpers::setUpXccyBasisCurve(LADataInstance* dataInstance,const LAString& BCurveID, 
						    LAStringMatrix ylddata, LAStringMatrix bgrid, LAStringMatrix bdata)
    {
	    //ylddata
	    LAString	CurveIDTool			= BCurveID + TOOL;
	    LAString	CurveIDManager		= BCurveID + STD + MANAGER;

	    upper(ylddata);
	    upper(bgrid);
	    upper(bdata);

	    //basis market data
	    LAString bref;
	    LAString term;
        LADate spotdate;
	    //LADate spotdate		= LADateScheduleHelpers::getLADate(chgrow(bdata,CURVEINPUT_SPOTDATE,1));		
	    LAString daycount(AC_365I);

	    //BasisType
	    std::map<LAString, LAString>& bvar = LACoreComponentManager::getBasisTypeMap();
	    std::map<LAString, LAString>::iterator itb;
	    itb = bvar.find(chgrow(bdata,CURVEINPUT_BASISTYPE,1));
	    if(itb == bvar.end())
		    throw LACoreInvalidData("BasisType is not registered ", __FILE__,__LINE__);
	    LAString basistype = itb->second;

	    //daycount
	    std::map<LAString, LAString>& dvar = LACoreComponentManager::getDayCountMap();
	    std::map<LAString, LAString>::iterator itd;
	    itd = dvar.find(chgrow(bdata,CURVEINPUT_BASISDAYCOUNT,1));
	    if(itd == dvar.end())
	    {
		    itd = dvar.find(chgrow(bdata,CURVEINPUT_BASISDAYCOUNT1,1));
		    if(itd == dvar.end())
			    throw LACoreInvalidData("basisDaycount is not registered ", __FILE__,__LINE__);
	    }
	    LAString bdaycount = itd->second;
	
	    LAString bcalendar	= chgrow(bdata,CURVEINPUT_BASISCALENDAR,1);
	    if(bcalendar=="")
		    bcalendar		= chgrow(bdata,CURVEINPUT_BASISCALENDAR1, 1);
	    LAString bsrule		= chgrow(bdata,CURVEINPUT_BASISSLIDINGRULE,1);
	    if(bsrule=="")
		    bsrule			= chgrow(bdata,CURVEINPUT_BASISSLIDINGRULE1, 1);
	    LAString bfreq		= chgrow(bdata,CURVEINPUT_BASISFREQUENCY,1);
	    if(bfreq=="")
		    bfreq			= chgrow(bdata,CURVEINPUT_BASISFREQUENCY1, 1);

	    LAPriceDataDayCount dc_Basis_attr; dc_Basis_attr.convertFromString(daycount);
	    LAPriceDataDayCount bdc_Basis_attr; bdc_Basis_attr.convertFromString(bdaycount);
	    LAPriceDataCalendar cal_Basis_attr; cal_Basis_attr.convertFromString(bcalendar);
	    LAPriceDataSlidingRule sr_Basis_attr; sr_Basis_attr.convertFromString(bsrule);

        int tmprow1= LAFunctionUtilities::findRowsNumber(bdata,CURVEINPUT_SPOTLAG);
        int tmprow2= LAFunctionUtilities::findRowsNumber(bdata,CURVEINPUT_SPOTDATE);
        const LADate asofdate = dynamic_cast<const LADataDate& >(dataInstance->getObjectPool().getObject(BCurveID,ENCHKTYPE_ISDEFINED).
                                    get().getData(CALIBRATION_DATA_ASOFDATE,ISNOTNULL).get()).get();
        if( tmprow1<0 && tmprow2<0) 
        {
            throw LACoreInvalidData("input spot date or spot lag", __FILE__,__LINE__);
        }
        else if(tmprow1>=0)
        {
            LAString spotLag = LAFunctionUtilities::findElement(bdata,CURVEINPUT_SPOTLAG,0,1,true);
            spotdate = LADateScheduleHelpers::getDate(asofdate,spotLag,bsrule,bcalendar);
        }
        else
        {
            spotdate = LADateScheduleHelpers::getLADate(chgrow(bdata,CURVEINPUT_SPOTDATE,1));
        }

	    //Interpolation
	    std::map<LAString, LAString>& ivar = LACoreComponentManager::getInterpolationMap();
	    std::map<LAString, LAString>::iterator iti;
	    iti = ivar.find(chgrow(ylddata,CURVEINPUT_INTERPOLATION,1));
	    if(iti == ivar.end())
		    throw LACoreInvalidData("Interpolation is not registered ", __FILE__,__LINE__);
	    LAString interpolation = iti->second;
	
	    LAObjectPool& objPool = dataInstance->getObjectPool(); 
	    //first check isYieldCurveIDexist
	    if(!objPool.getObject(BCurveID).isDefined())
		    throw LACoreInvalidData("CurveID does not exsist",__FILE__,__LINE__);


	    int N=bgrid.size();
	    double rate =0.0;
	    for(int i=1;i<N;i++)
	    {
		    term = chgcol(bgrid,CURVEINPUT_TERM,i);
		    LAString name = BCurveID + BASIS + term;
		    LAObject* be = NULL;
		    if(!objPool.getObject(name).isDefined())
		    {
			    be = new LAObject;
			    objPool.set(name,be);
		    }
		    else
		    {
			    objPool.getObject(name).get().clear();
			    be = &objPool.getObject(name).get();
		    }
		    rate = chgcol(bgrid,CURVEINPUT_RATE,i).getDoubleValue();
		    be->add(CALIBRATION_DATA_NAME,				new LADataString(name)				);
		    be->add(IR_CALIBRATION_DATA_TERM,				new LADataString(term)				);
		    be->add(IR_CALIBRATION_DATA_DATATYPE,			new LADataString("BasisRate")		);
		    be->add(CALIBRATION_DATA_RATE,				new LADataDouble(rate)				);
		    be->add(IR_CALIBRATION_DATA_FREQUENCYBASE,	new LADataString(bfreq)				);
		    be->add(IR_CALIBRATION_DATA_DAYCOUNTBASE,		new LAPriceDataDayCount(bdc_Basis_attr)	);
		    be->add(IR_CALIBRATION_DATA_CALENDARBASE,		new LAPriceDataCalendar(cal_Basis_attr)	);
		    be->add(IR_CALIBRATION_DATA_SLIDINGRULEBASE,	new LAPriceDataSlidingRule(sr_Basis_attr));
		    be->add(IR_CALIBRATION_DATA_DAYCOUNT,			new LAPriceDataDayCount(dc_Basis_attr)	);
		    be->add(IR_CALIBRATION_DATA_SPOTDATE,			new LADataDate(spotdate)			);
		    bref+= name+":";
	    }

	    //LACurvePricingObject
	    LAString yldname =  BCurveID+ "FROMMAKINGBASISCURVE";
	    const LAObjectHolder ehyld = objPool.getObject(yldname);
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
	    LAString clonename = BCurveID + "Clone";
	    const LAObjectHolder ehclo = objPool.getObject(clonename);
	    LAObject* clonecurve = NULL;
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


	    //LAMathYieldCurvePro
	    const LAObjectHolder ehCur = objPool.getObject(CurveIDTool);
	    LAMathYieldCurvePro * curveCalibrationData	=NULL; 
	    if(!ehCur.isDefined())
	    {	
		    curveCalibrationData= new LAMathYieldCurvePro(dataInstance);
		    objPool.set(CurveIDTool,curveCalibrationData);
	    }
	    else
	    {
		    //dynamic_cast<LACurvePricingObject &>(objPool.getObject(CurveIDTool).get()).reset();
		    curveCalibrationData	= &dynamic_cast<LAMathYieldCurvePro &>(objPool.getObject(CurveIDTool).get());
	    }
	
	    LAString ref = bref.subString(0,bref.size()-2);
	    curveCalibrationData->getYieldData().convertFromString(BCurveID);
	    curveCalibrationData->getBaseYieldCurve().convertFromString(yldname);
	    curveCalibrationData->getInterpolation_bs().convertFromString(FN_SPLINEINTERPOLATION_STR);
	    curveCalibrationData->getInterpolation().convertFromString(FN_SPLINEINTERPOLATION_STR);
	    curveCalibrationData->getDayCount().convertFromString(AC_365I);
	    LADataMultiReference &refMarket = curveCalibrationData->getMarketData();
	    if (!refMarket.isNull())
	    {
		    LAString n_ref = "";
		    unsigned int mSize = refMarket.getSize();
		    for (unsigned int i = 0; i < mSize; ++i)
		    {
			    LAString type = dynamic_cast<const LADataString&>((refMarket.get(i).getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL)).get()).get();
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
    LACurveCalibrationHelpers::setUpBasisCurve(LADataInstance* dataInstance,
                                                  const LAString& stdCurveID, 
                                                  const LAString& basisCurveID, 
                                                  LAStringMatrix ylddata, 
                                                  LAStringMatrix bgrid, 
                                                  LAStringMatrix bdata)
    {
	    //ylddata
	    LAString	CurveIDTool			= stdCurveID + TOOL;

	    upper(ylddata);
	    upper(bgrid);
	    upper(bdata);

	    //basis market data
	    LAString bref;
	    LAString term;
        LADate spotdate;		
	    LAString daycount(AC_365I);

	    //BasisType
	    std::map<LAString, LAString>& bvar = LACoreComponentManager::getBasisTypeMap();
	    std::map<LAString, LAString>::iterator itb;
	    itb = bvar.find(chgrow(bdata,CURVEINPUT_BASISTYPE,1));
	    if(itb == bvar.end())
		    throw LACoreInvalidData("BasisType is not registered ", __FILE__,__LINE__);
	    LAString basistype = itb->second;

	    //daycount
	    std::map<LAString, LAString>& dvar = LACoreComponentManager::getDayCountMap();
	    std::map<LAString, LAString>::iterator itd;
	    itd = dvar.find(chgrow(bdata,CURVEINPUT_BASISDAYCOUNT,1));
	    if(itd == dvar.end())
	    {
		    itd = dvar.find(chgrow(bdata,CURVEINPUT_BASISDAYCOUNT1,1));
		    if(itd == dvar.end())
			    throw LACoreInvalidData("basisDaycount is not registered ", __FILE__,__LINE__);
	    }
	    LAString bdaycount = itd->second;
	
	    LAString bcalendar	= chgrow(bdata,CURVEINPUT_BASISCALENDAR,1);
	    if(bcalendar=="")
		    bcalendar		= chgrow(bdata,CURVEINPUT_BASISCALENDAR1, 1);
	    LAString bsrule		= chgrow(bdata,CURVEINPUT_BASISSLIDINGRULE,1);
	    if(bsrule=="")
		    bsrule			= chgrow(bdata,CURVEINPUT_BASISSLIDINGRULE1, 1);
	    LAString bfreq		= chgrow(bdata,CURVEINPUT_BASISFREQUENCY,1);
	    if(bfreq=="")
		    bfreq			= chgrow(bdata,CURVEINPUT_BASISFREQUENCY1, 1);

        int tmprow1= LAFunctionUtilities::findRowsNumber(bdata,CURVEINPUT_SPOTLAG);
        int tmprow2= LAFunctionUtilities::findRowsNumber(bdata,CURVEINPUT_SPOTDATE);
        const LADate asofdate = dynamic_cast<const LADataDate& >(dataInstance->getObjectPool().getObject(stdCurveID,ENCHKTYPE_ISDEFINED).
                                    get().getData(CALIBRATION_DATA_ASOFDATE,ISNOTNULL).get()).get();
        if( tmprow1<0 && tmprow2<0) 
        {
            throw LACoreInvalidData("input spot date or spot lag", __FILE__,__LINE__);
        }
        else if(tmprow1>=0)
        {
            LAString spotLag = LAFunctionUtilities::findElement(bdata,CURVEINPUT_SPOTLAG,0,1,true);
            spotdate = LADateScheduleHelpers::getDate(asofdate,spotLag,bsrule,bcalendar);
        }
        else
        {
            spotdate = LADateScheduleHelpers::getLADate(chgrow(bdata,CURVEINPUT_SPOTDATE,1));
        }

	    //Interpolation
	    std::map<LAString, LAString>& ivar = LACoreComponentManager::getInterpolationMap();
	    std::map<LAString, LAString>::iterator iti;
	    iti = ivar.find(chgrow(ylddata,CURVEINPUT_INTERPOLATION,1));
	    if(iti == ivar.end())
		    throw LACoreInvalidData("Interpolation is not registered ", __FILE__,__LINE__);
	    LAString interpolation = iti->second;
	
	    LAObjectPool& objPool = dataInstance->getObjectPool(); 
	    //first check isYieldCurveIDexist
	    if(!objPool.getObject(stdCurveID).isDefined())
		    throw LACoreInvalidData("CurveID does not exsist",__FILE__,__LINE__);


	    int N=bgrid.size();
	    double rate =0.0;
	    for(int i=1;i<N;i++)
	    {
		    term = chgcol(bgrid,CURVEINPUT_TERM,i);
		    LAString name = stdCurveID + BASIS + term;
		    LAObject* be = NULL;
		    if(!objPool.getObject(name).isDefined())
		    {
			    be = new LAObject;
			    objPool.set(name,be);
		    }
		    else
		    {
			    objPool.getObject(name).get().clear();
			    be = &objPool.getObject(name).get();
		    }
		    rate = chgcol(bgrid,CURVEINPUT_RATE,i).getDoubleValue();
		    be->add(CALIBRATION_DATA_NAME,				new LADataString(name)				);
		    be->add(IR_CALIBRATION_DATA_TERM,				new LADataString(term)				);
		    be->add(IR_CALIBRATION_DATA_DATATYPE,			new LADataString("BasisRate")		);
		    be->add(CALIBRATION_DATA_RATE,				new LADataDouble(rate)				);
		    be->add(IR_CALIBRATION_DATA_FREQUENCYBASE,	new LADataString(bfreq)				);
		    be->add(IR_CALIBRATION_DATA_DAYCOUNTBASE,		new LAPriceDataDayCount()				).convertFromString(bdaycount);
		    be->add(IR_CALIBRATION_DATA_CALENDARBASE,		new LAPriceDataCalendar()				).convertFromString(bcalendar);
		    be->add(IR_CALIBRATION_DATA_SLIDINGRULEBASE,	new LAPriceDataSlidingRule()				).convertFromString(bsrule);
		    be->add(IR_CALIBRATION_DATA_DAYCOUNT,			new LAPriceDataDayCount()				).convertFromString(daycount);
		    be->add(IR_CALIBRATION_DATA_SPOTDATE,			new LADataDate(spotdate)			);
		    bref+= name+":";
	    }

	    //LACurvePricingObject
	    LAString yldname =  stdCurveID+ "FROMMAKINGBASISCURVE";
	    const LAObjectHolder ehyld = objPool.getObject(yldname);
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

	    //LAMathYieldCurvePro
	    const LAObjectHolder ehCur = objPool.getObject(CurveIDTool);
	    LAMathYieldCurvePro * curveCalibrationData	=NULL; 
	    if(!ehCur.isDefined())
	    {	
		    curveCalibrationData= new LAMathYieldCurvePro(dataInstance);
		    objPool.set(CurveIDTool,curveCalibrationData);
	    }
	    else
	    {
		    curveCalibrationData	= &dynamic_cast<LAMathYieldCurvePro &>(objPool.getObject(CurveIDTool).get());
	    }
	
	    LAString ref = bref.subString(0,bref.size()-2);
	    curveCalibrationData->getYieldData().convertFromString(stdCurveID);
	    curveCalibrationData->getBaseYieldCurve().convertFromString(yldname);
	    curveCalibrationData->getInterpolation_bs().convertFromString(FN_SPLINEINTERPOLATION_STR);
	    curveCalibrationData->getInterpolation().convertFromString(FN_SPLINEINTERPOLATION_STR);
	    curveCalibrationData->getDayCount().convertFromString(AC_365I);
	    LADataMultiReference &refMarket = curveCalibrationData->getMarketData();
	    if (!refMarket.isNull())
	    {
		    LAString n_ref = "";
		    unsigned int mSize = refMarket.getSize();
		    for (unsigned int i = 0; i < mSize; ++i)
		    {
			    LAString type = dynamic_cast<const LADataString&>((refMarket.get(i).getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL)).get()).get();
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
    LACurveCalibrationHelpers::setUpBasisCurveFromMarket(LADataInstance* dataInstance,const LAString& CurveID, 
						    LAStringMatrix mgrid, LAStringMatrix lgrid, LAStringMatrix fgrid, 
						    LAStringMatrix sgrid,LAStringMatrix mdata, LAStringMatrix ldata, 
						    LAStringMatrix fdata, LAStringMatrix sdata,LAStringMatrix ylddata,
						    LAStringMatrix bgrid,LAStringMatrix bdata,bool isCheckDF, bool isAUDadjust)
    {
	    LACurveCalibrationHelpers::setUpCurveFromMarket(dataInstance,CurveID,mgrid,lgrid,fgrid,sgrid,mdata,ldata,fdata,sdata,ylddata,isCheckDF,isAUDadjust);
	    LACurveCalibrationHelpers::setUpXccyBasisCurve(dataInstance,CurveID,ylddata,bgrid,bdata);
    }

    void  
    LACurveCalibrationHelpers::
    setUpArbFreeCurveFromMarket( LADataInstance* dataInstance, const LAString& CurveID, 
                                 LAStringMatrix monGrid, LAStringMatrix monConv, //money market
                                 LAStringMatrix libGrid, LAStringMatrix libConv, //libor market
                                 LAStringMatrix swapGrid, LAStringMatrix swapConv, //swap market
                                 LAStringMatrix currGrid, LAStringMatrix currConv, //currency basis swap market
                                 LAStringMatrix libBasisGrid, LAStringMatrix libBasisConv, //3M6M libor basis swap market
                                 LAStringMatrix fraGrid3M, LAStringMatrix fraGrid6M, LAStringMatrix fraConv, //FRA market
                                 LAStringMatrix yldData, LAString& warning )
    {
	    //ylddata
	    LAString	CurveIDTool			= CurveID + TOOL;
	    LAString	CurveIDManager		= CurveID + STD + MANAGER;

	    LAObjectPool& objPool = dataInstance->getObjectPool(); 
	    const LAObjectHolder objHolder = objPool.getObject(CurveIDManager,ENCHKTYPE_NOCHECK);
	
	    //curve yielddata clear;
	    if(objPool.getObject(CurveID).isDefined())
	    {
		    objPool.getObject(CurveID).clear();
	    }
	
        LAString Currency = chgrow(yldData,"Currency",	1);
        LAString parCurve = chgrow(yldData,"ParCurve", 1);
	    LAString usdCurve;
	    bool isRenAdj = false;
	    if(Currency != CURRENCY_JPY && parCurve != CURRENCY_USD) 
        {
		    int tmprow1= LAFunctionUtilities::findRowsNumber(yldData,"isRenotionalAdjust");
		    if(tmprow1>=0)
		    {
			    LAString isRenAdj_str = chgrow(yldData,"isRenotionalAdjust", 1);
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
	
	    LADate asofdate = LADateScheduleHelpers::getLADate(chgrow(yldData,CURVEINPUT_ASOFDATE,1));
        LAString isFRAUSE_str = chgrow(yldData,"ISFRAUSE",1);
        bool isFRAUse = false;
        if( isFRAUSE_str == "TRUE" ) 
	    {
		    isFRAUse = true;
		    if(1>fraGrid3M.size() || 1>fraGrid6M.size())
			    throw LACoreInvalidData("Input FRA Data",__FILE__,__LINE__);
		    if(2>fraGrid3M[0].size() || 2>fraGrid6M[0].size())
			    throw LACoreInvalidData("Matrix column size must be 2",__FILE__,__LINE__);
		    if(fraConv.empty())
			    throw LACoreInvalidData("Input Matrix is empty",__FILE__,__LINE__);
	    }

        //intertplation 
	    std::map<LAString, LAString>& ivar = LACoreComponentManager::getInterpolationMap();
        std::map<LAString, LAString>::iterator it = ivar.find(chgrow(yldData,CURVEINPUT_INTERPOLATION,1			));
	    if(it==ivar.end())
		    throw LACoreInvalidData("Interpolation is not registered in interpolation methods",			__FILE__,__LINE__);
	    LAString interpolation = it->second;

	    //YieldData
	    LAObject* e = NULL;
	    if(!objPool.getObject(CurveID).isDefined())
	    {	
		    e = new LAObject;
		    objPool.set(CurveID,e);
	    }
	    else
	    {
		    objPool.getObject(CurveID).get().clear();
		    e= &objPool.getObject(CurveID).get();
	    }
	    e->add(CALIBRATION_DATA_NAME, new LADataString(CurveID));
        e->add(IR_CALIBRATION_DATA_CURRENCY, new LADataString(Currency));
        e->add(IR_CALIBRATION_DATA_PARCURVE, new LADataString(parCurve));

	    //CurveCalibrationData
	    const LAObjectHolder ehCur = objPool.getObject(CurveIDTool);
	    LAMathYieldCurvePro * curveCalibrationData	=NULL; 
	    if(!ehCur.isDefined())
	    {	
		    curveCalibrationData= new LAMathYieldCurvePro(dataInstance);
		    objPool.set(CurveIDTool,curveCalibrationData);
	    }
	    else
	    {
		    dynamic_cast<LAMathYieldCurvePro &>(objPool.getObject(CurveIDTool).get()).reset();
		    curveCalibrationData	= &dynamic_cast<LAMathYieldCurvePro &>(objPool.getObject(CurveIDTool).get());
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
	    curveCalibrationData->LAObject::add(IR_CALIBRATION_DATA_ISRENOTIONALADJUST, new LADataBool(isRenAdj));
		curveCalibrationData->LAObject::add(IR_CALIBRATION_DATA_ISXCCYMARKEDTOMARKET, new LADataBool(isRenAdj)); // Alias for renotional adjust

	    LAStringVector curveNames_6ML;curveNames_6ML.push_back("6ML");
	    LAStringVector curveNames_3ML;curveNames_3ML.push_back("3ML");
	    LAStringVector curveNames_DF;curveNames_DF.push_back("DF");
	    curveCalibrationData->setArbFreeCurveName(curveNames_6ML,curveNames_DF,curveNames_3ML);
	    e->add(IR_CALIBRATION_DATA_6MLCURVENAMES, new LADataStrings(curveNames_6ML));
	    e->add(IR_CALIBRATION_DATA_DFCURVENAMES, new LADataStrings(curveNames_DF));
	    e->add(IR_CALIBRATION_DATA_3MLCURVENAMES, new LADataStrings(curveNames_3ML));
	
	    //daycount
	    std::map<LAString, LAString>& dvar = LACoreComponentManager::getDayCountMap();
	    std::map<LAString, LAString>::iterator itd,itd2;
	
	    LAString name, type;
	    LADate spotdate;
	    LAString ref;
	    LAStringVector term;
        DoubleArray rate;
	    LAString srule;
	    //Furuya
	    LAString lrule;
	    LAString lcalendar;
	    LADate lspotdate;
	    int ltmprow1,ltmprow2;
	    //
	    LAStringVector city;
        LAString calendar;
	    LAString dc,dc2;
	    int N=0;
	    //Money Market
	    itd			= dvar.find(chgrow(monConv,CURVEINPUT_DAYCOUNT,1));
	    if(itd==dvar.end())
		    throw LACoreInvalidData("Money Market Daycount is not registered ", __FILE__,__LINE__);
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

		    LAObject* mme = NULL;
		
		    if(!objPool.getObject(name).isDefined())
		    {	
			    mme = new LAObject;
			    objPool.set(name, mme);
		    }
		    else
		    {
			    objPool.getObject(name).get().clear();
			    mme= &objPool.getObject(name).get();
		    }

		    rate = LACoreUtility::changeDoubleFromString(searchvecbycol(monGrid,CURVEINPUT_RATE,true));
		    mme->add(CALIBRATION_DATA_NAME,				new LADataString(name)							);
		    mme->add(IR_CALIBRATION_DATA_DATATYPE,		new LADataString(type)							);
		    mme->add(IR_CALIBRATION_DATA_TERM,    		new LADataString(term[i])							);
		    mme->add(IR_CALIBRATION_DATA_DAYCOUNT,		new LAPriceDataDayCount()							).convertFromString(dc);
		    mme->add(CALIBRATION_DATA_CALENDAR,			new LAPriceDataCalendar(city)						);
		    mme->add(CALIBRATION_DATA_SLIDINGRULE,		new LAPriceDataSlidingRule()							).convertFromString(srule);
		    mme->add(CALIBRATION_DATA_RATE,				new LADataDouble(rate[i])							);
		    ref += name + ":";
	    }

	    ////LiborData
	    itd			= dvar.find(chgrow(libConv,CURVEINPUT_DAYCOUNT,1));
	    if(itd==dvar.end())
		    throw LACoreInvalidData("Libor Daycount is not registered ", __FILE__,__LINE__);
	    LAString ldc			= itd->second;
	    const LAStringVector term_tmp = searchvecbycol(libGrid,CURVEINPUT_TERM,true);
        const LAStringVector rate_str = searchvecbycol(libGrid,CURVEINPUT_RATE,true);
	    /*Furuya*/
	    lcalendar    = chgrow(swapConv,CURVEINPUT_CALENDAR,1);
	    lrule		= chgrow(swapConv,CURVEINPUT_SLIDINGRULE,1);
	    LAPriceDataCalendar calL;
	    calL.convertFromString(calendar);
	    ltmprow1= LAFunctionUtilities::findRowsNumber(swapConv,CURVEINPUT_SPOTLAG);
        ltmprow2= LAFunctionUtilities::findRowsNumber(swapConv,CURVEINPUT_SPOTDATE);
        if( ltmprow1<0 && ltmprow2<0 ) 
        {
            throw LACoreInvalidData("input spot date or spot lag", __FILE__,__LINE__);
        }
	    else if(ltmprow1>=0 && ltmprow2>=0)
        {
		    throw LACoreInvalidData("do not input spot date and spot lag", __FILE__,__LINE__);
	    }
        else if(ltmprow1>=0)
        {
            LAString lspotLag = LAFunctionUtilities::findElement(swapConv,CURVEINPUT_SPOTLAG,0,1,true);
		    lspotdate = LADateScheduleHelpers::getDate(asofdate,lspotLag,lrule,lcalendar);
        }
        else if(ltmprow2>=0)
        {
            lspotdate = LADateScheduleHelpers::getLADate(chgrow(swapConv,CURVEINPUT_SPOTDATE,1));
        }
	    /**/

	    //set three month libor
	    const int row_3M = LACoreUtility::findRowsNumber(libGrid,"3M");
	    if( row_3M<0 ) throw LACoreInvalidData("3 month libor does not exist.", __FILE__,__LINE__);
	    //set six month libor
	    const int row_6M = LACoreUtility::findRowsNumber(libGrid,"6M");
	    if( row_6M<0 ) throw LACoreInvalidData("6 month libor does not exist.", __FILE__,__LINE__);

	    for (size_t i=0; i<term_tmp.size(); i++)
	    {
		    //e->add("LiborDaycount", new LAPriceDataDayCount() ).convertFromString(ldc);
		    if (term_tmp[i] != "3M" && term_tmp[i] != "6M") continue;

		    name = Currency + LIBOR + term_tmp[i];
		    LAObject* le=NULL; 
		    if(!objPool.getObject(name).isDefined())
		    {	
			    le = new LAObject;
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

		    term = LAStringVector( 1, term_tmp[row_3M-1] );
		    term.push_back( term_tmp[row_6M-1] );*/

		    le->add(CALIBRATION_DATA_NAME,				new LADataString(name)							);
		    le->add(IR_CALIBRATION_DATA_TERM,    		    new LADataString(term_tmp[i])							);
		    le->add(IR_CALIBRATION_DATA_DAYCOUNT,			new LAPriceDataDayCount()							).convertFromString(ldc);
		    le->add(CALIBRATION_DATA_RATE,				new LADataDouble(rate_str[i].getDoubleValue())	);
		    le->add(IR_CALIBRATION_DATA_DATATYPE,			new LADataString(ZERO)							);
		    //Furuya 
		    le->add(CALIBRATION_DATA_CALENDAR,			new LAPriceDataCalendar(calL)						);
		    le->add(CALIBRATION_DATA_SLIDINGRULE,			new LAPriceDataSlidingRule()							).convertFromString(lrule);
		    le->add(IR_CALIBRATION_DATA_SPOTDATE,			new LADataDate(lspotdate)						);
		    ref+= name + ":";
	    }

	    e->add("LiborDaycount", new LAPriceDataDayCount()).convertFromString(ldc);

	    ////SwapData
	    itd			= dvar.find(chgrow(swapConv,"DAYCOUNTFIX",1));
        itd2		= dvar.find(chgrow(swapConv,"DAYCOUNTFLOAT",1));
	    if ( itd==dvar.end() && itd2==dvar.end() ) 
        {
            throw LACoreInvalidData("Swap Daycount is not registered ", __FILE__,__LINE__);
        }
        else if ( itd == dvar.end() && itd2 != dvar.end() ) 
        {
		    throw LACoreInvalidData("Libor Basis Daycount is not registered ", __FILE__,__LINE__);
        }
        else if ( itd != dvar.end() && itd2 == dvar.end() ) 
        {
		    throw LACoreInvalidData("Libor Basis Daycount is not registered ", __FILE__,__LINE__);
        }
        else
        {
            dc	= itd->second;
            dc2 = itd2->second;
        }
	    if( dc2 != ldc ) warning += LAString(" Swap floating daycount is not Libor's!");

	    city		= chgrow(swapConv,CURVEINPUT_CALENDAR,1).toToken(':');
        calendar    = chgrow(swapConv,CURVEINPUT_CALENDAR,1);
	    srule		= chgrow(swapConv,CURVEINPUT_SLIDINGRULE,1);
	    LAString frequency = chgrow(swapConv,CURVEINPUT_FREQUENCY,1);
	    LAPriceDataCalendar calS;
	    calS.convertFromString(calendar);

	    term = searchvecbycol(swapGrid,CURVEINPUT_TERM,true);
	    rate = LACoreUtility::changeDoubleFromString(searchvecbycol(swapGrid,CURVEINPUT_RATE,true));
	
	    if( rate.size() == 0 ) throw LACoreInvalidData("input swap market data.", __FILE__,__LINE__);
	    if( Currency == CURRENCY_USD && !isFRAUse )
	    {
		    tmprow1= LAFunctionUtilities::findRowsNumber(swapGrid,"6M");
		    if( tmprow1 < 0 ) throw LACoreInvalidData("input 6M swap market data.", __FILE__,__LINE__);
	    }

        tmprow1= LAFunctionUtilities::findRowsNumber(swapConv,CURVEINPUT_SPOTLAG);
        tmprow2= LAFunctionUtilities::findRowsNumber(swapConv,CURVEINPUT_SPOTDATE);
        if( tmprow1<0 && tmprow2<0 ) 
        {
            throw LACoreInvalidData("input spot date or spot lag", __FILE__,__LINE__);
        }
	    else if(tmprow1>=0 && tmprow2>=0)
        {
		    throw LACoreInvalidData("do not input spot date and spot lag", __FILE__,__LINE__);
	    }
        else if(tmprow1>=0)
        {
            LAString spotLag = LAFunctionUtilities::findElement(swapConv,CURVEINPUT_SPOTLAG,0,1,true);
            spotdate = LADateScheduleHelpers::getDate(asofdate,spotLag,srule,calendar);
        }
        else if(tmprow2>=0)
        {
            spotdate = LADateScheduleHelpers::getLADate(chgrow(swapConv,CURVEINPUT_SPOTDATE,1));
        }

	    for (size_t i=0; i<term.size(); i++)
	    {
		    LAObject* se	=	NULL;
		    name	=	Currency + SWAP + term[i];
		
		    if(!objPool.getObject(name).isDefined())
		    {	
			    se = new LAObject;
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
			    throw LACoreInvalidData("Interpolation is not registered in interpolation methods",			__FILE__,__LINE__);
		    interpolation = it->second;

		    se->add(CALIBRATION_DATA_NAME,				new LADataString(name)							);
		    se->add(IR_CALIBRATION_DATA_TERM,				new LADataString(term[i])							);
		    se->add(IR_CALIBRATION_DATA_DATATYPE,			new LADataString(PAR)							);
		    se->add(IR_CALIBRATION_DATA_FREQUENCY_FIX,	new LADataString(frequency)	);
		    se->add(IR_CALIBRATION_DATA_BASEFREQUENCY_FIX,	new LADataString(frequency)	);
		
		    if ( Currency == CURRENCY_USD ) 
		    {
			    se->add(IR_CALIBRATION_DATA_FREQUENCY_FLOAT,	new LADataString(QUARTERLY)	);
			    se->add(IR_CALIBRATION_DATA_BASEFREQUENCY_FLOAT,	new LADataString(QUARTERLY)	);
		    }
		    else if ( Currency == CURRENCY_AUD || Currency == CURRENCY_JPY  || Currency == CURRENCY_EUR )
		    {
			    tmprow1= LAFunctionUtilities::findRowsNumber(swapConv,CURVEINPUT_FREQUENCYFLOAT);
			    if (tmprow1>=0)
			    {
				    LAString frequency_Float = chgrow(swapConv,CURVEINPUT_FREQUENCYFLOAT,1);
				    se->add(IR_CALIBRATION_DATA_FREQUENCY_FLOAT,	new LADataString(frequency_Float)	);
				    se->add(IR_CALIBRATION_DATA_BASEFREQUENCY_FLOAT,	new LADataString(frequency_Float)	);
			    }
			    else
			    {
				    se->add(IR_CALIBRATION_DATA_FREQUENCY_FLOAT, new LADataString(SEMI_ANNUAL) );
				    se->add(IR_CALIBRATION_DATA_BASEFREQUENCY_FLOAT, new LADataString(SEMI_ANNUAL) );
			    }
		    }
		    else
		    {
			    LAString frequency_Float = chgrow(swapConv,CURVEINPUT_FREQUENCYFLOAT,1);
			    se->add(IR_CALIBRATION_DATA_FREQUENCY_FLOAT, new LADataString(frequency_Float) );
			    se->add(IR_CALIBRATION_DATA_BASEFREQUENCY_FLOAT, new LADataString(frequency_Float) );
		    }

		    tmprow1= LAFunctionUtilities::findRowsNumber(swapConv,CURVEINPUT_ISEOMROLL);
		    if (tmprow1>=0)
		    {
			    LAString isEOMRoll_str = chgrow(swapConv,CURVEINPUT_ISEOMROLL,1);
			    LADataBool tmpIsEOMRollSW;
			    tmpIsEOMRollSW.convertFromString(isEOMRoll_str);
			    bool isEOMRoll = tmpIsEOMRollSW.get();
			    if (isEOMRoll)
			    {
				    LADate eomDate = calS.getEOMDay(spotdate);
				    if (eomDate != spotdate)
				    {
					    isEOMRoll = false;
				    }
			    }
			    se->add(IR_CALIBRATION_DATA_ISEOMROLLSW, new LADataBool(isEOMRoll));
		    }

		    // Swap Frequency of AUD is irregular
		    if ( Currency == CURRENCY_AUD && (term[i] == "1Y" || term[i] == "2Y" || term[i] == "3Y") ) 
		    {
			    se->add(IR_CALIBRATION_DATA_ISSWAPTENORADJUST, new LADataBool() ).convertFromString("TRUE");
			    se->remove(IR_CALIBRATION_DATA_FREQUENCY_FLOAT);
			    se->add(IR_CALIBRATION_DATA_FREQUENCY_FLOAT,	new LADataString(QUARTERLY) );
			    se->remove(IR_CALIBRATION_DATA_FREQUENCY_FIX);
			    se->add(IR_CALIBRATION_DATA_FREQUENCY_FIX, new LADataString(QUARTERLY) );
			    se->remove(IR_CALIBRATION_DATA_FREQUENCY);
			    se->add(IR_CALIBRATION_DATA_FREQUENCY, new LADataString(QUARTERLY) );
		    }
		
		    se->add(IR_CALIBRATION_DATA_DAYCOUNT,		new LAPriceDataDayCount()							).convertFromString(dc);
		    se->add(IR_CALIBRATION_DATA_DAYCOUNT_FIX,		new LAPriceDataDayCount()							).convertFromString(dc);
		    se->add(IR_CALIBRATION_DATA_DAYCOUNT_FLOAT,	new LAPriceDataDayCount()							).convertFromString(dc2);
		    se->add(CALIBRATION_DATA_CALENDAR,			new LAPriceDataCalendar(calS)						);
		    se->add(CALIBRATION_DATA_SLIDINGRULE,			new LAPriceDataSlidingRule()							).convertFromString(srule);
		    se->add(IR_CALIBRATION_DATA_SPOTDATE,			new LADataDate(spotdate)						);
		    se->add(CALIBRATION_DATA_RATE,				new LADataDouble(rate[i])							);
		    se->add(CALIBRATION_DATA_INTERPOLATION,		new LAPriceDataInterpolation()						).convertFromString(interpolation);
		    ref+= name + ":";
	    }
	
        //Currency Basis Data
	    LAString ref_XCCYBasis = "";
        if( Currency != CURRENCY_USD )
        {
		    term = searchvecbycol(currGrid,CURVEINPUT_TERM,true);
		    rate = LACoreUtility::changeDoubleFromString(searchvecbycol(currGrid,CURVEINPUT_RATE,true));

		    for (size_t i=0; i<term.size(); i++)
		    {
			    LAObject* be	=	NULL;
			    name	=	Currency + XCCYBASIS + term[i];

			    if(!objPool.getObject(name).isDefined())
			    {	
				    be = new LAObject;
				    objPool.set(name, be);
			    }
			    else
			    {
				    objPool.getObject(name).get().clear();
				    be = &objPool.getObject(name).get();
			    }
			    be->add(CALIBRATION_DATA_NAME,				new LADataString(name)							);

			    if( currConv.empty() ) throw LACoreInvalidData("input currency basis swap convention.", __FILE__,__LINE__);
			    itd			= dvar.find(chgrow(currConv,CURVEINPUT_BASISDAYCOUNT,1));
			    if(itd==dvar.end())
				    throw LACoreInvalidData("Currency Basis Daycount is not registered ", __FILE__,__LINE__);
			    dc			= itd->second;
			    if( dc != ldc ) warning += LAString(" Currency basis daycount is not Libor's!");
			    be->add(IR_CALIBRATION_DATA_INDEXDAYCOUNT,	new LAPriceDataDayCount() ).convertFromString(dc);

			    city		= chgrow(currConv,CURVEINPUT_BASISCALENDAR,1).toToken(':');
			    calendar    = chgrow(currConv,CURVEINPUT_BASISCALENDAR,1);
			    if(calendar=="")
				    calendar = chgrow(currConv,CURVEINPUT_BASISCALENDAR1, 1);
			    srule = chgrow(currConv,CURVEINPUT_BASISSLIDINGRULE,1);
			    if(srule=="")
				    srule = chgrow(currConv,CURVEINPUT_BASISSLIDINGRULE1, 1);

			    tmprow1= LAFunctionUtilities::findRowsNumber(currConv,CURVEINPUT_SPOTLAG);
			    tmprow2= LAFunctionUtilities::findRowsNumber(currConv,CURVEINPUT_SPOTDATE);
			    if( tmprow1<0 && tmprow2<0) 
			    {
				    throw LACoreInvalidData("input spot date or spot lag", __FILE__,__LINE__);
			    }
			    else if(tmprow1>=0 && tmprow2>=0)
			    {
				    throw LACoreInvalidData("do not input spot date and spot lag", __FILE__,__LINE__);
			    }
			    else if(tmprow1>=0)
			    {
				    LAString spotLag = LAFunctionUtilities::findElement(currConv,CURVEINPUT_SPOTLAG,0,1,true);
				    spotdate = LADateScheduleHelpers::getDate(asofdate,spotLag,srule,calendar);
			    }
			    else if(tmprow2>=0)
			    {
				    spotdate = LADateScheduleHelpers::getLADate(chgrow(currConv,CURVEINPUT_SPOTDATE,1));
			    }

			    if( rate.size() == 0 ) throw LACoreInvalidData("input currency basis market data.", __FILE__,__LINE__);
			    tmprow1= LAFunctionUtilities::findRowsNumber(currGrid,"3M");
			    if( tmprow1 < 0 ) throw LACoreInvalidData("input 3M currency basis market data.", __FILE__,__LINE__);

			    it = ivar.find(chgrow(currConv,CURVEINPUT_INTERPOLATION,1			));
			    if(it==ivar.end())
				    throw LACoreInvalidData("Interpolation is not registered in interpolation methods",			__FILE__,__LINE__);
			    interpolation = it->second;
			    be->add(CALIBRATION_DATA_INTERPOLATION, new LAPriceDataInterpolation() ).convertFromString(interpolation);
			    be->add(IR_CALIBRATION_DATA_TERM, new LADataString(term[i]) );
			    be->add(CALIBRATION_DATA_RATE, new LADataDouble(rate[i]) );
			    be->add(IR_CALIBRATION_DATA_CASHLETCALENDAR, new LAPriceDataCalendar(city) );
			    be->add(IR_CALIBRATION_DATA_CASHLETSLIDINGRULE, new LAPriceDataSlidingRule() ).convertFromString(srule);
			    be->add(IR_CALIBRATION_DATA_CASHLETSPOTDATE, new LADataDate(spotdate) );
			    be->add(IR_CALIBRATION_DATA_DATATYPE, new LADataString()).convertFromString(YIELD_TYPE_BASIS);
			    if (parCurve == CURRENCY_USD || parCurve == "JPY3ML" || parCurve == "JPY6ML" ) 
			    {
				    be->add(IR_CALIBRATION_DATA_AGTFORECAST, new LADataString()).convertFromString("DUMMY");
				    be->add(IR_CALIBRATION_DATA_AGTDISCOUNT, new LADataString()).convertFromString("DUMMY");
			    }
			    else
			    {
				    if (isRenAdj)
				    {
					    curveCalibrationData->getForeignYieldData().convertFromString(usdCurve);
					    be->add(IR_CALIBRATION_DATA_AGTFORECAST, new LADataString()).convertFromString("3ML");
					    be->add(IR_CALIBRATION_DATA_AGTDISCOUNT, new LADataString()).convertFromString("DF");
					    be->add(IR_CALIBRATION_DATA_AGTCURRENCY, new LADataString()).convertFromString(CURRENCY_USD);
				    }
				    else
				    {
					    curveCalibrationData->getForeignYieldData().convertFromString(parCurve);
					    be->add(IR_CALIBRATION_DATA_AGTFORECAST, new LADataString()).convertFromString("FLOATER");
					    be->add(IR_CALIBRATION_DATA_AGTDISCOUNT, new LADataString()).convertFromString("DUMMY");
				    }				
			    }
			    ref_XCCYBasis+= name + ":";
		    }

		    ref_XCCYBasis = ref_XCCYBasis.subString(0, ref_XCCYBasis.size() - 2);
		    curveCalibrationData->LAObject::add(CALIBRATION_DATA_MARKETDATA + LAString("_") + XCCYBASIS, new LADataMultiReference()).convertFromString(ref_XCCYBasis);
        }
	    else
	    {
		    LAObject* be	=	NULL;
		    name	=	Currency + XCCYBASIS;

		    if(!objPool.getObject(name).isDefined())
		    {	
			    be = new LAObject;
			    objPool.set(name, be);
		    }
		    else
		    {
			    objPool.getObject(name).get().clear();
			    be = &objPool.getObject(name).get();
		    }

		    // set name
		    be->add(CALIBRATION_DATA_NAME, new LADataString()).convertFromString(name);
		    // set data type
		    be->add(IR_CALIBRATION_DATA_DATATYPE, new LADataString()).convertFromString(YIELD_TYPE_BASIS);
		    if (parCurve != CURRENCY_USD)
		    {
			    const LAObject& object = objPool.getObject(parCurve, ENCHKTYPE_ISDEFINED).get();
			    const LAString& parCurve_baseccy = dynamic_cast<const LADataString& > ((object.getData(IR_CALIBRATION_DATA_PARCURVE, ISNOTNULL)).get()).get();
			    if( parCurve_baseccy == CURRENCY_USD )
				    throw LACoreInvalidData("par curves are inconsistent!",__FILE__,__LINE__);
			    be->add(IR_CALIBRATION_DATA_AGTFORECAST, new LADataString()).convertFromString("FLOATER");
			    curveCalibrationData->getForeignYieldData().convertFromString(parCurve);
			    if (isRenAdj)
			    {
				    be->add(IR_CALIBRATION_DATA_AGTDISCOUNT, new LADataString()).convertFromString("DF");
				    be->add(IR_CALIBRATION_DATA_AGTCURRENCY, new LADataString()).convertFromString(parCurve_baseccy);
			    }
		    }
		    else
		    {
			    be->add(IR_CALIBRATION_DATA_AGTFORECAST, new LADataString()).convertFromString("DUMMY");
		    }

		    ref+= name + ":";
	    }

        //3MLibor 6MLibor Basis Data
	    itd			= dvar.find(chgrow(libBasisConv,"DAYCOUNTTHREE",1));
        itd2		= dvar.find(chgrow(libBasisConv,"DAYCOUNTSIX",1));
	    if(itd==dvar.end() && itd2==dvar.end() ) 
        {
            throw LACoreInvalidData("Libor Basis Daycount is not registered ", __FILE__,__LINE__);
        }
        else if(itd == dvar.end() && itd2 != dvar.end()) 
        {
		    throw LACoreInvalidData("Libor Basis Daycount is not registered ", __FILE__,__LINE__);
        }
        else if(itd != dvar.end() && itd2 == dvar.end()) 
        {
		    throw LACoreInvalidData("Libor Basis Daycount is not registered ", __FILE__,__LINE__);
        }
        else
        {
            dc	= itd->second;
            dc2 = itd2->second;
        }
	    if( dc != ldc ) warning += LAString(" 3ML Leg daycount is not Libor's!");
	    if( dc2 != ldc ) warning += LAString(" 6ML Leg daycount is not Libor's!");
	    term = searchvecbycol(libBasisGrid,CURVEINPUT_TERM,true);
	    rate = LACoreUtility::changeDoubleFromString(searchvecbycol(libBasisGrid,CURVEINPUT_RATE,true));
	    if( rate.size() == 0 ) throw LACoreInvalidData("input 3M/6M Libor basis market data.", __FILE__,__LINE__);
	    if( Currency != CURRENCY_USD && !isFRAUse )
	    {
		    tmprow1= LAFunctionUtilities::findRowsNumber(libBasisGrid,"6M");
		    if( tmprow1 < 0 ) throw LACoreInvalidData("input 6M 3M/6M Libor basis market data.", __FILE__,__LINE__);
	    }

	    LAString ref_36Basis = "";
	    for (size_t i=0; i<term.size(); i++)
	    {
		    LAObject* lbe	=	NULL;
		    name	=	Currency + THREESIXBASIS + term[i];
		
		    if(!objPool.getObject(name).isDefined())
		    {	
			    lbe = new LAObject;
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
			    throw LACoreInvalidData("Interpolation is not registered in interpolation methods",			__FILE__,__LINE__);
		    interpolation = it->second;
		
		    lbe->add(CALIBRATION_DATA_NAME,						new LADataString(name) );
		    lbe->add(IR_CALIBRATION_DATA_TERM,					new LADataString(term[i]) );
		    lbe->add(IR_CALIBRATION_DATA_DAYCOUNT_THREE,		new LAPriceDataDayCount() ).convertFromString(dc);
		    lbe->add(IR_CALIBRATION_DATA_DAYCOUNT_SIX,			new LAPriceDataDayCount() ).convertFromString(dc2);
		    lbe->add(CALIBRATION_DATA_RATE,						new LADataDouble(rate[i]) );
		    lbe->add(CALIBRATION_DATA_INTERPOLATION,			new LAPriceDataInterpolation() ).convertFromString(interpolation);
		    lbe->add(IR_CALIBRATION_DATA_DATATYPE,				new LADataString() ).convertFromString(YIELD_TYPE_BASIS);
		    lbe->add(IR_CALIBRATION_DATA_ISAGTSPREAD,			new LADataBool() ).convertFromString("FALSE");
		    lbe->add(IR_CALIBRATION_DATA_INDEXDAYCOUNT,			new LAPriceDataDayCount() ).convertFromString(dc);
		    lbe->add(IR_CALIBRATION_DATA_AGTINDEXDAYCOUNT,		new LAPriceDataDayCount() ).convertFromString(dc2);
		    lbe->add(IR_CALIBRATION_DATA_CASHLETFREQUENCY,		new LADataString() ).convertFromString(QUARTERLY);
		    lbe->add(IR_CALIBRATION_DATA_CASHLETCALENDAR,		new LAPriceDataCalendar(calS) );
		    lbe->add(IR_CALIBRATION_DATA_CASHLETSLIDINGRULE,	new LAPriceDataSlidingRule() ).convertFromString(srule);
		    lbe->add(IR_CALIBRATION_DATA_CASHLETSPOTDATE,		new LADataDate(spotdate) );
		    lbe->add(IR_CALIBRATION_DATA_AGTCASHLETFREQUENCY,	new LADataString() ).convertFromString(SEMI_ANNUAL);
		    ref_36Basis += name + ":";	
	    }
		
	    ref_36Basis = ref_36Basis.subString(0, ref_36Basis.size() - 2);
	    curveCalibrationData->LAObject::add(CALIBRATION_DATA_MARKETDATA + LAString("_") + THREESIXBASIS, new LADataMultiReference()).convertFromString(ref_36Basis);


        //FRA Data
        if(isFRAUse)
        {
            //error check
            LAString errorMsg,term_FRA;

	        city		= chgrow(fraConv,CURVEINPUT_CALENDAR,1).toToken(':');
            calendar    = chgrow(fraConv,CURVEINPUT_CALENDAR,1);
	        srule = chgrow(fraConv,CURVEINPUT_SLIDINGRULE,1);
		    itd			= dvar.find(chgrow(fraConv,CURVEINPUT_DAYCOUNT,1));
		    if(itd==dvar.end())
			    throw LACoreInvalidData("FRA Daycount is not registered ", __FILE__,__LINE__);
		    dc	= itd->second;
		    if( dc != ldc ) warning += LAString(" FRA daycount is not Libor's!");

            tmprow1= LAFunctionUtilities::findRowsNumber(fraConv,CURVEINPUT_SPOTLAG);
            tmprow2= LAFunctionUtilities::findRowsNumber(fraConv,CURVEINPUT_SPOTDATE);
            if( tmprow1<0 && tmprow2<0) 
            {
                throw LACoreInvalidData("input spot date or spot lag", __FILE__,__LINE__);
            }
	        else if(tmprow1>=0 && tmprow2>=0)
            {
		        throw LACoreInvalidData("do not input spot date and spot lag", __FILE__,__LINE__);
	        }
            else if(tmprow1>=0)
            {
                LAString spotLag = LAFunctionUtilities::findElement(fraConv,CURVEINPUT_SPOTLAG,0,1,true);
                spotdate = LADateScheduleHelpers::getDate(asofdate,spotLag,srule,calendar);
            }
            else if(tmprow2>=0)
            {
                spotdate = LADateScheduleHelpers::getLADate(chgrow(fraConv,CURVEINPUT_SPOTDATE,1));
            }

		    term = searchvecbycol(fraGrid3M,CURVEINPUT_TERM,true);
	        rate = LACoreUtility::changeDoubleFromString(searchvecbycol(fraGrid3M,CURVEINPUT_RATE,true));
		    for (size_t i=0; i<term.size(); i++)
		    {
			    LAObject* fra3e	=	NULL;
			    //name	=	CurveID + Currency + FRA3ML;
			    name	=	Currency + FRA3ML + term[i];
		    
			    if(!objPool.getObject(name).isDefined())
			    {	
				    fra3e = new LAObject;
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
			    LAString term_X = LAString(term_month) + LAString("X") + LAString(term_month + 3);

			    fra3e->add(CALIBRATION_DATA_NAME,				new LADataString(name)							);
			    fra3e->add(IR_CALIBRATION_DATA_TERM,			new LADataString(term_X)							);
			    fra3e->add(CALIBRATION_DATA_CALENDAR,			new LAPriceDataCalendar(city)						);
			    fra3e->add(CALIBRATION_DATA_SLIDINGRULE,		new LAPriceDataSlidingRule()							).convertFromString(srule);
			    fra3e->add(IR_CALIBRATION_DATA_SPOTDATE,		new LADataDate(spotdate)						);
			    fra3e->add(CALIBRATION_DATA_RATE,				new LADataDouble(rate[i])							);
			    fra3e->add(IR_CALIBRATION_DATA_DAYCOUNT,		new LAPriceDataDayCount()							).convertFromString(dc);
			    fra3e->add(IR_CALIBRATION_DATA_DATATYPE,		new LADataString()).convertFromString(FRA3M);
                fra3e->add(IR_CALIBRATION_DATA_ISEOMROLL,		new LADataBool(false));
                fra3e->add(IR_CALIBRATION_DATA_APPLYTENSION,	new LADataBool(false));
                fra3e->add(IR_CALIBRATION_DATA_TENSIONGAP,	new LADataInt(1));
			    ref+= name + ":";
		    }

		    term = searchvecbycol(fraGrid6M,CURVEINPUT_TERM,true);
	        rate = LACoreUtility::changeDoubleFromString(searchvecbycol(fraGrid6M,CURVEINPUT_RATE,true));
		    for (size_t i=0; i<term.size(); i++)
		    {
			    LAObject* fra6e	=	NULL;
			    name	=	CurveID + Currency + FRA6ML + term[i];
			    if(!objPool.getObject(name).isDefined())
			    {	
				    fra6e = new LAObject;
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
			    LAString term_X = LAString(term_month) + LAString("X") + LAString(term_month + 6);

			    fra6e->add(CALIBRATION_DATA_NAME,				new LADataString(name)							);
			    fra6e->add(IR_CALIBRATION_DATA_TERM,			new LADataString(term_X)							);
			    fra6e->add(CALIBRATION_DATA_CALENDAR,			new LAPriceDataCalendar(city)						);
			    fra6e->add(CALIBRATION_DATA_SLIDINGRULE,		new LAPriceDataSlidingRule()							).convertFromString(srule);
			    fra6e->add(IR_CALIBRATION_DATA_SPOTDATE,		new LADataDate(spotdate)						);
			    fra6e->add(CALIBRATION_DATA_RATE,				new LADataDouble(rate[i])							);
			    fra6e->add(IR_CALIBRATION_DATA_DAYCOUNT,		new LAPriceDataDayCount()							).convertFromString(dc);
			    fra6e->add(IR_CALIBRATION_DATA_DATATYPE,		new LADataString()).convertFromString(FRA6M);
			    fra6e->add(IR_CALIBRATION_DATA_ISEOMROLL,		new LADataBool(false));
                fra6e->add(IR_CALIBRATION_DATA_APPLYTENSION,	new LADataBool(false));
			    fra6e->add(IR_CALIBRATION_DATA_TENSIONGAP,	new LADataInt(1));
			    ref+= name + ":";
		    }
        }

	    //set market object
	    if (ref.size() < 2)
		    throw LACoreInvalidData("Market Data is not set !!", __FILE__, __LINE__); 
	    ref = ref.subString(0, ref.size() - 2);
	    curveCalibrationData->getMarketData().convertFromString(ref);
	    //generate Object
	    dataInstance->getReferencePool().completeDependency();
	    LADataHolder* dh		= &objPool.getObject(CurveIDTool).getData(CALIBRATION_DATA_CURVEGENERATOR);
	    LADataProcedure& modelDataObj	= dynamic_cast<LADataProcedure&>(dh->get());
	    modelDataObj.calibrateModel(asofdate);

	    curveCalibrationData->LAObject::add(IR_CALIBRATION_DATA_MAINBASISDF, new LADataString("DF"));
	    curveCalibrationData->setBasisRates();

	    //curve entities manager (curve, grids, curveinformation entities)
	    LAObject* mae = NULL;
	    if(!objPool.getObject(CurveIDManager).isDefined())
	    {	
		    mae = new LAObject;
		    objPool.set(CurveIDManager,mae);
	    }
	    else
	    {
		    objPool.getObject(CurveIDManager).get().clear();
		    mae	= &objPool.getObject(CurveIDManager).get();
	    }
	    mae->add("Time",			new LADataString()			).convertFromString(LAString(LATime::now()));
	    mae->add(CALIBRATION_DATA_NAME,		new LADataString()			).convertFromString(CurveIDManager);
	    mae->add(GRIDREFS,			new LADataMultiReference()	).convertFromString(ref);
	    mae->add(YIELDDATAREF,		new LADataReference()		).convertFromString(CurveID);
	    mae->add(YIELDTOOLREF,		new LADataReference()		).convertFromString(CurveIDTool);
    }

    void 
    LACurveCalibrationHelpers::setUpForecastCurveFromMarket(LADataInstance* dataInstance,
                                                               const LAString& setUpCurveID, 
						                                       LAStringMatrix bgrid, 
                                                               LAStringMatrix bdata)
    {
        LAString basisLeg = chgrow(bdata,"BasisLeg",1);    
        LAString arbFreeCurveID = chgrow(bdata,"ArbFreeCurveID",1);
	    LAString baseForecastCurveName = chgrow(bdata,"BaseForecastCurveName",1);
	    LAString dfCurveName = chgrow(bdata,"DiscountCurveName",1);

        //ylddata
	    LAString CurveIDTool = arbFreeCurveID + TOOL;
	    upper(bgrid);
	    upper(bdata);

	    //basis market data
	    LAStringVector terms_str;
        LADate spotdate;		
	    LAString daycount(AC_365I);

	    //BasisType
	    std::map<LAString, LAString>& dvar = LACoreComponentManager::getDayCountMap();
	    std::map<LAString, LAString>::iterator itd = dvar.find(chgrow(bdata,"SETUPCURVEDAYCOUNT",1));
        if(itd == dvar.end()) throw LACoreInvalidData("basisDaycount is not registered ", __FILE__,__LINE__);
	    LAString sDaycount = itd->second;

        itd = dvar.find(chgrow(bdata,"BASECURVEDAYCOUNT",1));
        if(itd == dvar.end()) throw LACoreInvalidData("basisDaycount is not registered ", __FILE__,__LINE__);
	    LAString bDaycount = itd->second;
	
	    LAString bcalendar	= chgrow(bdata,CURVEINPUT_BASISCALENDAR,1);
	    if(bcalendar=="")
		    bcalendar		= chgrow(bdata,CURVEINPUT_BASISCALENDAR1, 1);
	    LAString bsrule		= chgrow(bdata,CURVEINPUT_BASISSLIDINGRULE,1);
	    if(bsrule=="")
		    bsrule			= chgrow(bdata,CURVEINPUT_BASISSLIDINGRULE1, 1);
        LAString sFreq		= chgrow(bdata,"SETUPCURVEFREQUENCY",1);
	    LAString bFreq		= chgrow(bdata,"BASECURVEFREQUENCY",1);
    
        //set initial rate
        unsigned int setSpan,baseSpan;
        if (bFreq == ANNUAL) baseSpan = 12;
	    else if (bFreq == SEMI_ANNUAL) baseSpan = 6;
	    else if (bFreq == QUARTERLY) baseSpan = 3;
	    else if (bFreq == MONTHLY) baseSpan = 1;
	    else
	    {
		    //error
		    throw LACoreInvalidData("Input freq type is not supported", __FILE__, __LINE__);
	    }
        if (sFreq == ANNUAL) setSpan = 12;
	    else if (sFreq == SEMI_ANNUAL) setSpan = 6;
	    else if (sFreq == QUARTERLY) setSpan = 3;
	    else if (sFreq == MONTHLY) setSpan = 1;
	    else
	    {
		    //error
		    throw LACoreInvalidData("Input freq type is not supported", __FILE__, __LINE__);
	    }
        int tempRow = LAFunctionUtilities::findRowsNumber(bdata,CURVEINPUT_BASISINITIALRATE);
        LAString isIniRateUse_str = chgrow(bdata,"ISINITIALRATEUSE",1);
        bool isIniRateUse = true;
        if( setSpan >= baseSpan )
        {
            if( isIniRateUse_str == "TRUE" ) isIniRateUse = true;
            else if( isIniRateUse_str == "FALSE" ) isIniRateUse = false;
            else throw LACoreInvalidData("Input initial rate use", __FILE__, __LINE__);
        }
        if( isIniRateUse == true && (tempRow < 0 || chgrow(bdata,CURVEINPUT_BASISINITIALRATE,1) == "" ) ) 
            throw LACoreInvalidData("Input initial rate", __FILE__, __LINE__);
        double initialRate	= chgrow(bdata,CURVEINPUT_BASISINITIALRATE,1).getDoubleValue();

        int tmprow1= LAFunctionUtilities::findRowsNumber(bdata,CURVEINPUT_SPOTLAG);
        int tmprow2= LAFunctionUtilities::findRowsNumber(bdata,CURVEINPUT_SPOTDATE);
        const LADate asofdate = dynamic_cast<const LADataDate& >(dataInstance->getObjectPool().getObject(arbFreeCurveID,ENCHKTYPE_ISDEFINED).
                                    get().getData(CALIBRATION_DATA_ASOFDATE,ISNOTNULL).get()).get();
        if( tmprow1<0 && tmprow2<0) 
        {
            throw LACoreInvalidData("input spot date or spot lag", __FILE__,__LINE__);
        }
        else if(tmprow1>=0)
        {
            LAString spotLag = LAFunctionUtilities::findElement(bdata,CURVEINPUT_SPOTLAG,0,1,true);
            spotdate = LADateScheduleHelpers::getDate(asofdate,spotLag,bsrule,bcalendar);
        }
        else
        {
            spotdate = LADateScheduleHelpers::getLADate(chgrow(bdata,CURVEINPUT_SPOTDATE,1));
        }
	
	    LAObjectPool& objPool = dataInstance->getObjectPool(); 
	    //first check isYieldCurveIDexist
	    if(!objPool.getObject(arbFreeCurveID).isDefined())
		    throw LACoreInvalidData("CurveID does not exsist",__FILE__,__LINE__);

	    double rate =0.0;

        LAStringVector term_str = searchvecbycol(bgrid,CURVEINPUT_TERM,true);
        DoubleVector basisRates = LACoreUtility::changeDoubleFromString(searchvecbycol(bgrid,CURVEINPUT_RATE,true));
	    LAString name = setUpCurveID + "Market";
	    LAObject* be = NULL;
	    if(!objPool.getObject(name).isDefined())
	    {
		    be = new LAObject;
		    objPool.set(name,be);
	    }
	    else
	    {
		    objPool.getObject(name).get().clear();
            be = &objPool.getObject(name).get();
	    }

	    be->add(CALIBRATION_DATA_NAME,				new LADataString(name)				);
	    be->add(IR_CALIBRATION_DATA_TERM,				new LADataStrings(term_str)			);
	    be->add("BasisRate",    			new LADataDoubles(basisRates)		);
	    be->add("SetUpCurveFrequency",	    new LADataString(sFreq)				);
        be->add("BaseCurveFrequency",	    new LADataString(bFreq)				);
        be->add("SetUpCurveDayCount",		new LAPriceDataDayCount()				).convertFromString(sDaycount);
	    be->add("BaseCurveDayCount",		new LAPriceDataDayCount()				).convertFromString(bDaycount);
	    be->add(IR_CALIBRATION_DATA_CALENDARBASE,		new LAPriceDataCalendar()				).convertFromString(bcalendar);
	    be->add(IR_CALIBRATION_DATA_SLIDINGRULEBASE,	new LAPriceDataSlidingRule()				).convertFromString(bsrule);
	    be->add(IR_CALIBRATION_DATA_SPOTDATE,			new LADataDate(spotdate)			);
        be->add("InitialRate",	            new LADataDouble(initialRate)		);
        be->add("IsInitialRateUse",         new LADataBool(isIniRateUse)		);
        be->add("ArbFreeCurveID",	        new LADataString(arbFreeCurveID)    );
	    be->add("BaseForecastCurveName",    new LADataString(baseForecastCurveName));
	    be->add("DiscountCurveName",		new LADataString(dfCurveName));
        be->add("BasisLeg",	                new LADataString(basisLeg)		    );
 
        LAPriceArbFreeGenerator::setForecastCurve(dataInstance, setUpCurveID);
    }

    void 
    LACurveCalibrationHelpers::setUpForecastCurve(LADataInstance* dataInstance,
                                                     const LAString& arbFreeCurveID, 
                                                     const LAString& forecastCurveID, 
                                                     const LADate& asofdate,
                                                     const DoubleMatrix& ratesArray)
    {
	    unsigned int ratesize = ratesArray.size();
	    if (ratesize < 2)
	    {
		    LAString msg = "rate matrix size must be more than 2";
		    throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	    }
        LAObjectPool &objPool = dataInstance->getObjectPool();
        LAString name = arbFreeCurveID + "_" + forecastCurveID;
	    LAObjectHolder objHolder = objPool.getObject(name,ENCHKTYPE_NOCHECK);
	    if(!objHolder.isDefined())
	    {
		    LAObject* e = new LAObject();
		    e->add("Name", new LADataString()).convertFromString(name);
		    e->add("AsOfDate", new LADataDate(asofdate));
		    e->add("Terms_Rate",	new LADataDoubles(ratesArray[0]));
		    e->add("ForecastRates",new LADataDoubles(ratesArray[1]));
            e->add("ArbFreeCurve",new LADataString(arbFreeCurveID));
		    objPool.set(name,e);
	    }
	    else if(objHolder.isDefined())
	    {
		    LADataHolder* dh;
		    dh = &objHolder.getData("AsOfDate",ISDEFINED);
		    LADataDate& date = dynamic_cast<LADataDate &>(dh->get());
		    date.set(asofdate);
		    dh = &objHolder.getData("Terms_Rate",ISDEFINED);
		    LADataDoubles& terms = dynamic_cast<LADataDoubles& >(dh->get());
		    terms.set(ratesArray[0]);
		    dh = &objHolder.getData("ForecastRates",ISDEFINED);
		    LADataDoubles& rates = dynamic_cast<LADataDoubles& >(dh->get());
		    rates.set(ratesArray[1]);
            dh = &objHolder.getData("ArbFreeCurve",ISDEFINED);
		    LADataString& arbFreeName = dynamic_cast<LADataString &>(dh->get());
		    arbFreeName.set(arbFreeCurveID);
	    }
    }

    void 
    LACurveCalibrationHelpers::setUpCdtDFCurve(LADataInstance* dataInstance, LAString curveID, LAStringMatrix data)
    {
        LAString arbFreeCurveID = chgrow(data,"ArbFreeCurveID",1);
        LAString forecastCurveID = chgrow(data,"ForecastCurveID",1);
        const LADate& asOfDate = dynamic_cast<const LADataDate&> ((dataInstance->getObjectPool().getObject(arbFreeCurveID,ENCHKTYPE_ISDEFINED).get().
                getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL)).get()).get();
        upper(data);
        //daycount
	    std::map<LAString, LAString>& dvar = LACoreComponentManager::getDayCountMap();
	    std::map<LAString, LAString>::iterator itd = dvar.find(chgrow(data,CURVEINPUT_DAYCOUNT,1));
	    if(itd==dvar.end()) throw LACoreInvalidData("Money Market Daycount is not registered ", __FILE__,__LINE__);
	    LAPriceDataDayCount dc;
        dc.convertFromString(itd->second);
        //calendar,slidingrule,frequency
        LAString srule_str = chgrow(data,CURVEINPUT_SLIDINGRULE,1);
        LAString calendar_str = chgrow(data,CURVEINPUT_CALENDAR,1);
	
	    LAString frequency = chgrow(data,CURVEINPUT_FREQUENCY,1);
        //spotdate
        LADate spotdate;
        int tmprow1,tmprow2;
        tmprow1 = LAFunctionUtilities::findRowsNumber(data,CURVEINPUT_SPOTLAG);
        tmprow2= LAFunctionUtilities::findRowsNumber(data,CURVEINPUT_SPOTDATE);
        if( tmprow1<0 && tmprow2<0) 
        {
            throw LACoreInvalidData("input spot date or spot lag", __FILE__,__LINE__);
        }
        else if(tmprow1>=0 && tmprow2>=0)
        {
		    throw LACoreInvalidData("do not input spot date and spot lag", __FILE__,__LINE__);
	    }
	    else if(tmprow1>=0)
	    {		
		    LAString spotLag = LAFunctionUtilities::findElement(data,CURVEINPUT_SPOTLAG,0,1,true);
            spotdate = LADateScheduleHelpers::getDate(asOfDate,spotLag,srule_str,calendar_str);
        }
        else if(tmprow2>=0)
        {
            spotdate = LADateScheduleHelpers::getLADate(chgrow(data,CURVEINPUT_SPOTDATE,1));
        }
        double spread = chgrow(data,"CREDITSPREAD",1).getDoubleValue();
        LAPriceDataSlidingRule srule;
        srule.convertFromString( srule_str );
        LAPriceDataCalendar calendar;
        calendar.convertFromString( calendar_str );

        LAPriceArbFreeGenerator::generateCdtDFCurve(dataInstance, arbFreeCurveID, forecastCurveID, curveID, spotdate, 
            calendar, srule, frequency, dc, spread);
    }

    LAString 
    LACurveCalibrationHelpers::getCurveGenerateTime(LADataInstance* dataInstance, const LAString& curveid, const LAString& curvename)
    {

	    LAString CurveManager = curveid + curvename + MANAGER;
	    const LAObjectHolder& objHolder = dataInstance->getObjectPool().getObject(CurveManager,ENCHKTYPE_NOCHECK);
	    if(!objHolder.isDefined())
	    {		LAString msg = "Curve :" + curveid + " is not generated by mirCurveGenerate";
		    throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);		
	    }
	    const LADataString &str = dynamic_cast<const LADataString &>(objHolder.getData("Time",ISDEFINED).get());
	    LAString ret = curveid + " is generated at " + str.convertToString();
	    return ret;
    }

    void	
    LACurveCalibrationHelpers::setUpCurve2(LADataInstance* dataInstance, const LAString& curveid, const LADate& asofdate, 
										      const DoubleMatrix& mat, const LAString& curveName)
    {
	    LACurveCalibrationHelpers::setUpCurve(dataInstance,curveid,asofdate,mat,curveName);
	    LAString suffix;
	    if (curveName == STD)
	    {
		    suffix = LAString("");
	    }
	    else
	    {
		    suffix = LAString("_") + curveName;
	    }

	    unsigned int N=mat.size();
	    if (N < 2) 
	    {
		    LAString msg = "rate matrix size must be more than 2";
		    throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);		
	    }
	    LAObjectPool& objPool  = dataInstance->getObjectPool();
	    LAObject& e = objPool.getObject(curveid,ENCHKTYPE_ISDEFINED).get();
	    e.remove(IR_CALIBRATION_DATA_DFS2 + suffix);
	    if(N<=2)
		    return;
	    else
		    e.add(IR_CALIBRATION_DATA_DFS2 + suffix,new LADataDoubles(mat[2]));
	    return;
    }

    void 
    LACurveCalibrationHelpers::setUpCurve(LADataInstance* dataInstance, const LAString& curveid, const LADate& asofdate, 
										     const DoubleMatrix& mat, const LAString& curveName)
    {
	    unsigned int N=mat.size();
	    if (N < 2) 
	    {
		    LAString msg = "rate matrix size must be more than 2";
		    throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);		
	    }
	    LAObjectPool &objPool = dataInstance->getObjectPool();
	    LAObjectHolder objHolder = objPool.getObject(curveid,ENCHKTYPE_NOCHECK);
	    LAString suffix;
	    if (curveName == STD)
	    {
		    suffix = LAString("");
	    }
	    else
	    {
		    suffix = LAString("_") + curveName;
	    }

	    if(!objHolder.isDefined())
	    {
		    LAObject* e = new LAObject();
		    e->add("Name", new LADataString()).convertFromString(curveid);
		    e->add("AsOfDate", new LADataDate(asofdate));
		    e->add("Terms" + suffix, new LADataDoubles(mat[0]));
		    e->add("DiscountFactors" + suffix,new LADataDoubles(mat[1]));
		    objPool.set(curveid,e);
	    }
	    else if(objHolder.isDefined())
	    {
		    objHolder.remove("AsOfDate");
		    objHolder.remove("Terms" + suffix);
		    objHolder.remove("DiscountFactors" + suffix);
		    objHolder.add("AsOfDate", new LADataDate(asofdate));
		    objHolder.add("Terms" + suffix, new LADataDoubles(mat[0]));
		    objHolder.add("DiscountFactors" + suffix,new LADataDoubles(mat[1]));
	    }
    }

    /////////////furuytos20120813///////////
    void 
    LACurveCalibrationHelpers::setUpCurve3(LADataInstance* dataInstance, const LAString& curveid, const LADate& asofdate, 
										     const DoubleMatrix& mat, const LAStringMatrix& conv, const LAString& curveName)
    {
	    unsigned int N=mat.size();
	    if (N < 2) 
	    {
		    LAString msg = "rate matrix size must be more than 2";
		    throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);		
	    }
	    LAObjectPool &objPool = dataInstance->getObjectPool();
	    LAObjectHolder objHolder = objPool.getObject(curveid,ENCHKTYPE_NOCHECK);
	    LAString suffix;
	    if (curveName == STD)
	    {
		    suffix = LAString("");
	    }
	    else
	    {
		    suffix = LAString("_") + curveName;
	    }

	    LAStringMatrix tempconv = conv;
	    upper(tempconv);
	    LAString accessary = "";
	    LAString dc = "";
	
	    dc = chgrow(tempconv,CURVEINPUT_DAYCOUNT,1);
	    accessary = chgrow(tempconv,"ACCESSARY",1);

	    // If not "objHolder" exists
	    if(!objHolder.isDefined())
	    {
		    LAObject* e = new LAObject();
		    e->add(CALIBRATION_DATA_NAME, new LADataString()).convertFromString(curveid);
		    e->add(CALIBRATION_DATA_ASOFDATE, new LADataDate(asofdate));
		    e->add(CALIBRATION_DATA_TERMS + suffix, new LADataDoubles(mat[0]));
		    e->add(IR_CALIBRATION_DATA_DFS + suffix,new LADataDoubles(mat[1]));

		    e->remove(IR_CALIBRATION_DATA_FREQUENCY + suffix);
		    e->remove(IR_CALIBRATION_DATA_DAYCOUNT + suffix);
		    e->remove(IR_CALIBRATION_DATA_ACCESSARY + suffix);
		    e->add(IR_CALIBRATION_DATA_FREQUENCY + suffix, new LADataString(SIMPLE));
		    e->add(IR_CALIBRATION_DATA_DAYCOUNT+ suffix,		new LAPriceDataDayCount()							).convertFromString(dc);
		    e->add(IR_CALIBRATION_DATA_ACCESSARY + suffix, new LADataString(accessary));

		    objPool.set(curveid,e);
	    }

	     // If "objHolder" exists
	    else if(objHolder.isDefined())
	    {
		    objHolder.remove(CALIBRATION_DATA_ASOFDATE);
		    objHolder.remove(CALIBRATION_DATA_TERMS + suffix);
		    objHolder.remove(IR_CALIBRATION_DATA_DFS + suffix);
		    objHolder.add(CALIBRATION_DATA_ASOFDATE, new LADataDate(asofdate));
		    objHolder.add(CALIBRATION_DATA_TERMS + suffix, new LADataDoubles(mat[0]));
		    objHolder.add(IR_CALIBRATION_DATA_DFS + suffix,new LADataDoubles(mat[1]));
		
		    objHolder.remove(IR_CALIBRATION_DATA_FREQUENCY + suffix);
		    objHolder.remove(IR_CALIBRATION_DATA_DAYCOUNT + suffix);
		    objHolder.remove(IR_CALIBRATION_DATA_ACCESSARY + suffix);
		    objHolder.add(IR_CALIBRATION_DATA_FREQUENCY + suffix,	new LADataString(SIMPLE));
		    objHolder.add(IR_CALIBRATION_DATA_DAYCOUNT+ suffix,		new LAPriceDataDayCount()							).convertFromString(dc);
		    objHolder.add(IR_CALIBRATION_DATA_ACCESSARY + suffix,	new LADataString(accessary));

	    }

    }
    ////////////////////////////////

    void 
    LACurveCalibrationHelpers::setUpCurve(LADataInstance* dataInstance, const LAString& curveid, const LADate& asofdate,
										     DoubleArray& termarray, DoubleArray& dfarray, const LAString& curveName)
    {
	    if(termarray.size() != dfarray.size())
	    {
		    throw LACoreInvalidData("Input the same data size between Term and DF",__FILE__,__LINE__);
	    }

	    unsigned int col = termarray.size();
	    DoubleVector tmp(col, 0.0);
	    DoubleMatrix mat(2, tmp);
	    mat[0] = termarray;
	    mat[1] = dfarray;

	    LACurveCalibrationHelpers::setUpCurve(dataInstance, curveid, asofdate, mat, curveName);
    }

    DoubleArray 
    LACurveCalibrationHelpers::outPutCurveFromYieldData(LADataInstance* dataInstance, const LAString& curveid, const LAString& curveName)
    {
	    LAObjectPool& objPool	= dataInstance->getObjectPool();
	
	    LAString suffix;
	    if (curveName == STD)
	    {
		    suffix = "";
	    }
	    else
	    {
		    suffix = "_" + curveName;
	    }

	    const LAObject& ycp	= objPool.getObject(curveid,ENCHKTYPE_ISDEFINED).get();
	    const LADataDoubles& terms = dynamic_cast<const LADataDoubles& >(ycp.getData(CALIBRATION_DATA_TERMS + suffix,ISDEFINED).get());
	    DoubleArray x = terms.get();
	    const LADataDoubles & dfs = dynamic_cast<const LADataDoubles& >(ycp.getData(IR_CALIBRATION_DATA_DFS + suffix,ISDEFINED).get());
	    DoubleArray	y = dfs.get();
	
	    DoubleArray ret;
	    int N = x.size();
	    const LADataHolder* dh = &(ycp.getData(IR_CALIBRATION_DATA_DFS2 + suffix));
	    if(dh->isDefined() && !dh->isNull())
	    {
		    DoubleArray z = dynamic_cast<const LADataDoubles &>(dh->get()).get();
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
    LACurveCalibrationHelpers::outPutCurveFromYieldData2(LADataInstance* dataInstance, const LAString& curveID, const LAString& curveID2,
														    const LAString& curveName, const LAString& curveName2)
    {
	    LAObjectPool& objPool	= dataInstance->getObjectPool();
	    LAString suffix;
	    LAObject& ycp1 = objPool.getObject(curveID,ENCHKTYPE_ISDEFINED).get();
	    if (curveName == STD)
	    {
		    suffix = "";
	    }
	    else
	    {
		    suffix = "_" + curveName;
	    }
	    const DoubleArray& x1 = dynamic_cast<const LADataDoubles& >(ycp1.getData(CALIBRATION_DATA_TERMS + suffix,ISDEFINED).get()).get();
	    const DoubleArray& y1 = dynamic_cast<const LADataDoubles& >(ycp1.getData(IR_CALIBRATION_DATA_DFS + suffix,ISDEFINED ).get()).get();
    
        LAObject& ycp2 = objPool.getObject(curveID2,ENCHKTYPE_ISDEFINED).get();
	    if (curveName2 == STD)
	    {
		    suffix = "";
	    }
	    else
	    {
		    suffix = "_" + curveName2;
	    }
	    const DoubleArray& x2 = dynamic_cast<const LADataDoubles& >(ycp2.getData(CALIBRATION_DATA_TERMS + suffix,ISDEFINED).get()).get();
	    const DoubleArray& y2 = dynamic_cast<const LADataDoubles& >(ycp2.getData(IR_CALIBRATION_DATA_DFS + suffix,ISDEFINED).get()).get();
	
	    DoubleArray ret;
	    if(x1.size() <= x2.size())
	    {
            LAInterpolationBase* pInter = dynamic_cast<LAInterpolationBase*>
                (dynamic_cast<const LAPriceDataInterpolation& >(ycp1.getData(CALIBRATION_DATA_INTERPOLATION,ISDEFINED).get()).getMethod().clone());
	        LACoreFunctionHolder fh(pInter, true);
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
            LAInterpolationBase* pInter = dynamic_cast<LAInterpolationBase*>
                (dynamic_cast<const LAPriceDataInterpolation& >(ycp2.getData(CALIBRATION_DATA_INTERPOLATION,ISDEFINED).get()).getMethod().clone());
	        LACoreFunctionHolder fh(pInter, true);
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
    LACurveCalibrationHelpers::getCurveDiscountFactorTable(LADataInstance* dataInstance, const LAString& curveid, const LAString& curveName)
    {
	    LAObjectPool& objPool	= dataInstance->getObjectPool();
	
	    LAString suffix;
	    if (curveName == STD)
	    {
		    suffix = "";
	    }
	    else
	    {
		    suffix = "_" + curveName;
	    }

	    const LAObject& yieldCurveProps	= objPool.getObject(curveid,ENCHKTYPE_ISDEFINED).get();
	    const LADataDoubles& terms = dynamic_cast<const LADataDoubles& >(yieldCurveProps.getData(CALIBRATION_DATA_TERMS + suffix,ISDEFINED).get());
        const LADataDoubles & dfs = dynamic_cast<const LADataDoubles& >(yieldCurveProps.getData(IR_CALIBRATION_DATA_DFS + suffix,ISDEFINED).get());
    
        // Get the terms and discount factor vectors
        DoubleArray termsVector     = terms.get();
        DoubleArray	dfsVector       = dfs.get();
    
        // Check Result Dimensions
        if ( termsVector.size() != dfsVector.size() )
        {
            throw LACoreInvalidData("#Error: Invalid Curve Results - Inconsistent number of discount factor dates and values",__FILE__,__LINE__);
        }
    
        // Get the Payment Dates corresponding to the terms daycount fractions
        // --------------------------------------
        DateVector  paymentDatesVector( termsVector.size(), LADate() );

        // Get the Curve AsOfDate - note curveid is also known as curve collection
        LADate asOfDate = dynamic_cast<const LADataDate& > ( objPool.getObject( curveid ).get().getData( CALIBRATION_DATA_ASOFDATE, ISNOTNULL ).get() ).get();

        // Imply the corresponding term payment dates
        for( size_t i = 0; i<termsVector.size() ; ++i )
        {
            // Curve terms are always calculated using ACT/365_ISDA as defined in ConstantDeclarations.h [sic] - see #define AC_365I
            // Etrading curves have been migrated to ACT/365 because ACT/365_ISDA causes leap year irregularities in the yield curve
		    LAString ACT_365 = LAString("ACT/365");
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
    void LACurveCalibrationHelpers::setCurveDiscountFactorTable(LADataInstance* dataInstance, const LAString& curveid, const std::vector<LAString>& curveNames, const DiscountFactorTable& discountFactorTable )
    {
        // 1. Get the terms and discount factors from the discountFactorTable
        // ---------------------------------------
        DoubleArray termsVector     = discountFactorTable.terms_;
        DoubleArray	dfsVector       = discountFactorTable.discountFactors_;
    
        // Check Input Dimensions
        if ( termsVector.size() != dfsVector.size() )
        {
            throw LACoreInvalidData("#Error: Invalid Curve Inputs - Inconsistent number of discount factor dates and values",__FILE__,__LINE__);
        }

        // 2. Initialize the Yield Curve Properties; Required to Get and Set Curve Index Info
        // ---------------------------------------
        LAObjectPool& objPool = dataInstance->getObjectPool();
        LAObjectHolder objHolder = objPool.getObject( curveid, ENCHKTYPE_NOCHECK );

        // 3. Update the Curve Data
        // Interate Over Each Curve Index in the Curve Collection and Update
        // ---------------------------------------
        LAString curveSuffix = "";
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
            
                objHolder.add( CALIBRATION_DATA_TERMS + curveSuffix,  new LADataDoubles( termsVector ) );
	    	    objHolder.add( IR_CALIBRATION_DATA_DFS + curveSuffix, new LADataDoubles( dfsVector ) );
            }
            else
            {
                LAObject* e = new LAObject();
                e->add( CALIBRATION_DATA_TERMS + curveSuffix,  new LADataDoubles( termsVector ) );
	    	    e->add( IR_CALIBRATION_DATA_DFS + curveSuffix, new LADataDoubles( dfsVector ) );
            }
        }
    
    }

    /* @brief			Function to get the forward rates from a curve returning a forwardRateTable structure containing dates, terms and forward rates
    *  @param [in]		dataInstance		            The object pool dataInstance std::map
    *  @param [in]		curveid		            The curveCollection to use when accessing the curveIndices
    *  @param [in]		curveName               The curveindex within the curve
    *  @param [out]     ForwardRateTable        The forward rate table results
    */
    ForwardRateTable LACurveCalibrationHelpers::getCurveForwardRateTable( LADataInstance* dataInstance, const LAString& curveid, const LAString& curveName )
    {
        LAObjectPool& objPool = dataInstance->getObjectPool();
	
	    LAString suffix = "_" + curveName;
	    if (curveName == STD)
	    {
		    suffix = "";
	    }
	
        // Get the terms matrix and forward rates vector
        const LAObject& yieldCurveProps	= objPool.getObject( curveid, ENCHKTYPE_ISDEFINED ).get();
        DoubleMatrix termsMatrix    = dynamic_cast<const LADataDoubleMatrix &>( yieldCurveProps.getData(CALIBRATION_DATA_FWDTERMSMATRIX + suffix, ISNOTNULL ).get() ).get();
        DoubleArray	forwardsVector  = dynamic_cast<const LADataDoubles& >( yieldCurveProps.getData( IR_CALIBRATION_DATA_FORWARDRATES + suffix, ISDEFINED ).get() ).get();
    
         // Check Input Dimensions
        if ( termsMatrix.size() != 2 ) // Matrix with 2 rows: terms[0] forward start date terms, terms[1] forward end date terms
        {
            throw LACoreInvalidData("#Error: Invalid Curve Inputs - Inconsistent forward date dimensions; A matrix of forward start and end dates are required",__FILE__,__LINE__);
        }

        if ( termsMatrix[0].size() != termsMatrix[1].size() ) // Matrix with 2 rows: terms[0] forward start date terms, terms[1] forward end date terms
        {
            throw LACoreInvalidData("#Error: Invalid Curve Inputs - Inconsistent number of forward start- and end dates",__FILE__,__LINE__);
        }

        // Check Input Dimensions
        if ( termsMatrix[0].size() != forwardsVector.size() )
        {
            throw LACoreInvalidData("#Error: Invalid Curve Inputs - Inconsistent number of forward rate dates and values",__FILE__,__LINE__);
        }

        // Get the Fixing Dates corresponding to the terms daycount fractions
        // --------------------------------------
        DateVector  fixingDatesVector( termsMatrix[0].size(), LADate() );

        // Get the Curve AsOfDate - note curveid is also known as curve collection
        LADate asOfDate = dynamic_cast<const LADataDate& > ( objPool.getObject( curveid ).get().getData( CALIBRATION_DATA_ASOFDATE, ISNOTNULL ).get() ).get();

        // Imply the corresponding term fixing dates
        for( size_t i = 0; i<termsMatrix[0].size() ; ++i )
        {
            // Curve terms are always calculated using ACT/365_ISDA as defined in ConstantDeclarations.h [sic] - see #define AC_365I
		    // Etrading curves have been migrated to ACT/365 because ACT/365_ISDA causes leap year irregularities in the yield curve
		    LAString ACT_365 = LAString("ACT/365");
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
    void LACurveCalibrationHelpers::setCurveForwardRateTable( LADataInstance* dataInstance, const LAString& curveid, const std::vector<LAString>& curveNames, const ForwardRateTable& forwardRateTable )
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
            throw LACoreInvalidData("#Error: Invalid Curve Inputs - Inconsistent forward date dimensions; A matrix of forward start and end dates are required",__FILE__,__LINE__);
        }

        // Matrix with 2 rows: terms[0] forward start date terms, terms[1] forward end date terms
        if ( termsMatrix[0].size() != termsMatrix[1].size() )
        {
            throw LACoreInvalidData("#Error: Invalid Curve Inputs - Inconsistent number of forward start- and end dates", __FILE__,__LINE__);
        }

        // Check Input Dimensions
        if ( termsMatrix[0].size() != forwardsVector.size() )
        {
            throw LACoreInvalidData("#Error: Invalid Curve Inputs - Inconsistent number of forward rate dates and values", __FILE__,__LINE__);
        }

        // 2. Initialize the Yield Curve Properties; Required to Get and Set Curve Index Info
        // ---------------------------------------
        LAObjectPool& objPool = dataInstance->getObjectPool();
        LAObjectHolder objHolder = objPool.getObject( curveid, ENCHKTYPE_NOCHECK );

        // 3. Update the Curve Data
        // Interate Over Each Curve Index in the Curve Collection and Update
        // ---------------------------------------
        LAString curveSuffix = "";
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
            
                objHolder.add( CALIBRATION_DATA_FWDTERMSMATRIX + curveSuffix, new LADataDoubleMatrix( termsMatrix ) );
	    	    objHolder.add( IR_CALIBRATION_DATA_FORWARDRATES + curveSuffix,new LADataDoubles( forwardsVector ) );
            }
            else
            {
                LAObject* e = new LAObject();
                e->add( CALIBRATION_DATA_FWDTERMSMATRIX + curveSuffix, new LADataDoubleMatrix( termsMatrix ) );
	    	    e->add( IR_CALIBRATION_DATA_FORWARDRATES + curveSuffix,new LADataDoubles( forwardsVector ) );
            }
        }

    }

    void 
    LACurveCalibrationHelpers::restoreSwapRateFromLibor(LAMathYieldCurvePro &curve, const LAString &ccy, const std::map<LAString, double> &sRateMap, LAStringMatrix sdata)
    {
	    //LADate spotDate	= LADateScheduleHelpers::getLADate(chgrow(sdata,CURVEINPUT_SPOTDATE,1));
	    std::map<LAString, LAString>& dvar = LACoreComponentManager::getDayCountMap();
	    std::map<LAString, LAString>::iterator itd = dvar.find(chgrow(sdata,CURVEINPUT_DAYCOUNT,1));
	    if(itd==dvar.end())
		    throw LACoreInvalidData("Swap Daycount is not registered ", __FILE__,__LINE__);
	    LAString dcStr			= itd->second;
	    LAString calStr			= chgrow(sdata,CURVEINPUT_CALENDAR,1);
	    LAString slidingStr		= chgrow(sdata,CURVEINPUT_SLIDINGRULE,1);
	    LAString freq			= chgrow(sdata,CURVEINPUT_FREQUENCY,1);

	    LADataMultiReference &refMarkets = curve.getMarketData();
	    const int mSize = refMarkets.getSize();
	    // set data for holiday adjustment
	    // daycount
	    LAPriceDataDayCount dc;
	    dc.convertFromString(dcStr);
	    // sliding rule
	    LAPriceDataSlidingRule sliding;
	    sliding.convertFromString(slidingStr);
	    // calendar
	    LAPriceDataCalendar cal;
	    cal.convertFromString(calStr);
	    // asOfDate
	    const LADate asOfDate = curve.getAsOfDate();
	    // spotDate
	    LADate spotDate;
	    int tmprow1,tmprow2;
        tmprow1 = LAFunctionUtilities::findRowsNumber(sdata,CURVEINPUT_SPOTLAG);
        tmprow2= LAFunctionUtilities::findRowsNumber(sdata,CURVEINPUT_SPOTDATE);
        if( tmprow1<0 && tmprow2<0) 
        {
            throw LACoreInvalidData("input spot date or spot lag", __FILE__,__LINE__);
        }
        else if(tmprow1>=0 && tmprow2>=0)
        {
		    throw LACoreInvalidData("do not input spot date and spot lag", __FILE__,__LINE__);
	    }
	    else if(tmprow1>=0)
	    {		
		    LAString spotLag = LAFunctionUtilities::findElement(sdata,CURVEINPUT_SPOTLAG,0,1,true);
            spotDate = LADateScheduleHelpers::getDate(asOfDate,spotLag,slidingStr,calStr);
        }
        else if(tmprow2>=0)
        {
            spotDate = LADateScheduleHelpers::getLADate(chgrow(sdata,CURVEINPUT_SPOTDATE,1));
        }
	    if (asOfDate > spotDate)
		    throw LACoreInvalidData("AsofDate > spotDate, cannnot calc. ", __FILE__, __LINE__);

	    for (size_t i = 0; i < size_t(mSize); ++i)
	    {
		    LAObjectHolder &mktData = refMarkets.get(i);

		    LAString dataType = dynamic_cast<const LADataString &>
						    (mktData.getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL).get()).get();

		    dataType.toUpper();
		    if (dataType == ZERO || dataType == PAR)
		    {
			    const LAString &termStr = dynamic_cast<const LADataString &>(mktData.getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL).get());
			    map<LAString, double>::const_iterator it = sRateMap.find(termStr);
			    if (it != sRateMap.end())
			    {
				    // set swap rate and information
				    // set original swap value
				    mktData.remove(CALIBRATION_DATA_RATE);
				    mktData.add(CALIBRATION_DATA_RATE, new LADataDouble(it->second));
				    // set spot date
				    mktData.remove(IR_CALIBRATION_DATA_SPOTDATE);
				    mktData.add(IR_CALIBRATION_DATA_SPOTDATE, new LADataDate(spotDate));
				    // set calendar
				    mktData.remove(CALIBRATION_DATA_CALENDAR);
				    mktData.add(CALIBRATION_DATA_CALENDAR , new LAPriceDataCalendar()).convertFromString(calStr);
				    // set daycount
				    mktData.remove(IR_CALIBRATION_DATA_DAYCOUNT);
				    mktData.add(IR_CALIBRATION_DATA_DAYCOUNT, new LAPriceDataDayCount()).convertFromString(dcStr);
				    // set data type
				    mktData.remove(IR_CALIBRATION_DATA_DATATYPE);
				    mktData.add(IR_CALIBRATION_DATA_DATATYPE, new LADataString()).convertFromString(PAR);
				    // set frequency
				    mktData.remove(IR_CALIBRATION_DATA_FREQUENCY);
				    mktData.add(IR_CALIBRATION_DATA_FREQUENCY, new LADataString()).convertFromString(freq);
				    // set slidingrule
				    mktData.remove(CALIBRATION_DATA_SLIDINGRULE);
				    mktData.add(CALIBRATION_DATA_SLIDINGRULE, new LAPriceDataSlidingRule()).convertFromString(slidingStr);
			    }
		    }
	    }
	    return;
    }

    void 
    LACurveCalibrationHelpers::resetMarketDataUseL(LAMathYieldCurvePro &curve, const LAString &ccy, LAStringMatrix ldata)
    {
	    //LADate spotDate	= LADateScheduleHelpers::getLADate(chgrow(ldata,CURVEINPUT_SPOTDATE,1));
	    std::map<LAString, LAString>& dvar = LACoreComponentManager::getDayCountMap();
	    std::map<LAString, LAString>::iterator itd = dvar.find(chgrow(ldata,CURVEINPUT_DAYCOUNT,1));
	    if(itd==dvar.end())
		    throw LACoreInvalidData("Libor Daycount is not registered ", __FILE__,__LINE__);
	    LAString dcStr			= itd->second;
	    LAString calStr			= chgrow(ldata,CURVEINPUT_CALENDAR,1);
	    LAString slidingStr		= chgrow(ldata,CURVEINPUT_SLIDINGRULE,1);
	
	    LADataMultiReference &refMarkets = curve.getMarketData();
	    const int mSize = refMarkets.getSize();
	    // set data for holiday adjustment
	    // daycount
	    LAPriceDataDayCount dc;
	    dc.convertFromString(dcStr);
	    // sliding rule
	    LAPriceDataSlidingRule sliding;
	    sliding.convertFromString(slidingStr);
	    // calendar
	    LAPriceDataCalendar cal;
	    cal.convertFromString(calStr);
	    // asOfDate
	    const LADate asOfDate = curve.getAsOfDate();
	    // spotDate
	    LADate spotDate;
	    int tmprow1,tmprow2;
        tmprow1 = LAFunctionUtilities::findRowsNumber(ldata,CURVEINPUT_SPOTLAG);
        tmprow2= LAFunctionUtilities::findRowsNumber(ldata,CURVEINPUT_SPOTDATE);
        if( tmprow1<0 && tmprow2<0) 
        {
            throw LACoreInvalidData("input spot date or spot lag", __FILE__,__LINE__);
        }
        else if(tmprow1>=0 && tmprow2>=0)
        {
		    throw LACoreInvalidData("do not input spot date and spot lag", __FILE__,__LINE__);
	    }
	    else if(tmprow1>=0)
	    {		
		    LAString spotLag = LAFunctionUtilities::findElement(ldata,CURVEINPUT_SPOTLAG,0,1,true);
            spotDate = LADateScheduleHelpers::getDate(asOfDate,spotLag,slidingStr,calStr);
        }
        else if(tmprow2>=0)
        {
            spotDate = LADateScheduleHelpers::getLADate(chgrow(ldata,CURVEINPUT_SPOTDATE,1));
        }
	    if (asOfDate > spotDate)
		    throw LACoreInvalidData("AsofDate > spotDate, cannnot calc. ", __FILE__, __LINE__);

	    double termSpot = dc.getTerm(asOfDate, spotDate);
	    double dfSpot   = curve.getDF(termSpot);

	    LAStringVector liborYTerm;
	    // calc rate
	    for (int i = 0; i < mSize; ++i)
	    {
		    LAObjectHolder &mktData = refMarkets.get(i);
		    LAString dataType = dynamic_cast<const LADataString &>
								    (mktData.getData(IR_CALIBRATION_DATA_DATATYPE, ISNOTNULL).get()).get();

		    dataType.toUpper();

		    if (dataType != O_N && dataType != T_N 
			    && dataType != ZERO)
		    {
			    const LAString &termStr = dynamic_cast<const LADataString &>
								    (mktData.getData(IR_CALIBRATION_DATA_TERM, ISNOTNULL).get()).get();

			    LAString searchTerm = termStr;
			    if (termStr == "12M")
			    {
				    searchTerm = "1Y";
			    }
			    if (std::find(liborYTerm.begin(), liborYTerm.end(), searchTerm) == liborYTerm.end())
			    {
				    // calc date from spotDate
				    LADate date = LADateHelpers::getDate(spotDate, termStr, sliding, &cal, true);
					
				    double term     = dc.getTerm(asOfDate, date);
				    double df       = curve.getDF(term);
				    double delta    = term - termSpot;

				    // calc libor rate
				    double rate = (dfSpot - df) / (df * delta);

				    // reset attr libor value
				    mktData.remove(CALIBRATION_DATA_RATE);
				    mktData.add(CALIBRATION_DATA_RATE, new LADataDouble(rate));
				    // set spot date
				    mktData.remove(IR_CALIBRATION_DATA_SPOTDATE);
				    mktData.add(IR_CALIBRATION_DATA_SPOTDATE, new LADataDate(spotDate));
				    // set calendar
				    mktData.remove(CALIBRATION_DATA_CALENDAR);
				    mktData.add(CALIBRATION_DATA_CALENDAR , new LAPriceDataCalendar()).convertFromString(calStr);
				    // set daycount
				    mktData.remove(IR_CALIBRATION_DATA_DAYCOUNT);
				    mktData.add(IR_CALIBRATION_DATA_DAYCOUNT, new LAPriceDataDayCount()).convertFromString(dcStr);
				    // set data type
				    mktData.remove(IR_CALIBRATION_DATA_DATATYPE);
				    mktData.add(IR_CALIBRATION_DATA_DATATYPE, new LADataString()).convertFromString(ZERO);
				    // set frequency
				    mktData.remove(IR_CALIBRATION_DATA_FREQUENCY);
				    mktData.add(IR_CALIBRATION_DATA_FREQUENCY, new LADataString()).convertFromString(SIMPLE);
				    // set slidingrule
				    mktData.remove(CALIBRATION_DATA_SLIDINGRULE);
				    mktData.add(CALIBRATION_DATA_SLIDINGRULE, new LAPriceDataSlidingRule()).convertFromString(slidingStr);
			    }
		    }
		    else if (dataType == ZERO)
		    {
			    const LAString &termStr = dynamic_cast<const LADataString &>
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

