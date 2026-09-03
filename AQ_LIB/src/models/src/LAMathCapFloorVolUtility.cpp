#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include <vector>
#include <LADataInstance.h>
#include <LADataMatrix.h>
#include <LAPriceDataInterpolation.h>
#include <LAFunctionManager.h>
#include <LACoreFunctionHolder.h>
#include "LAInterpolationBase.h"
#include <LAMathDateCalculations.h>
#include "LAMathDateUtilities.h"
#include <LAPriceCashFlowGenerator.h>
#include <LAMathDefine.h>
#include <LAMathYieldCurve.h>
#include "LACoreComponentManager.h"
#include "LAMathCapFloorVolUtility.h"
#include "LAMathCapFloorSABR.h"
#include "LAMathSABR.h"
#include "LAFunctionUtilities.h"

using namespace std;

void
LAMathCapFloorVolUtility::setUpCapFloorVolSurf( LADataInstance* dataInstance, const LAString& capFloorID,
        LAStringMatrix convMat, const DoubleMatrix& volMat, const DoubleVector& strike,
        LAStringVector capTerm, const DoubleMatrix& sabrParamMat, LAStringMatrix sabrLimiter,
        LAString target, LAString interpo, const DoubleVector& weight, bool isSABR,
		const LAString& curveID, LAString foreCurveName, LAString dfCurveName)
{
    LAString curveID_;
	if(curveID==LAString(""))
	{
		curveID_ = searchbyrow(convMat, "CurveID",1,true);
	}
	else
	{
		curveID_ = curveID;
	}

    upper(convMat);
    LAString freq = searchbyrow(convMat, CURVEINPUT_FREQUENCY,1,true);
    LAString daycount = searchbyrow(convMat, CURVEINPUT_DAYCOUNT,1,true);
    LAString paySlr = searchbyrow(convMat, CURVEINPUT_SLIDINGRULE,1,true);
    LAString spotLag = searchbyrow(convMat, CURVEINPUT_SPOTLAG,1,true);
    LAString payCal = searchbyrow(convMat, "PAYMENTCALENDAR",1,true);
    LAString fixCal = searchbyrow(convMat, "FIXINGCALENDAR",1,true);
    LAMathCapFloorConvention conv(freq, spotLag, daycount, paySlr, payCal, fixCal);

    LAPriceDataSlidingRule paySlr_att; paySlr_att.convertFromString(paySlr);
    LAPriceDataCalendar payCal_att; payCal_att.convertFromString(payCal);
    LAPriceDataCalendar fixCal_att; fixCal_att.convertFromString(fixCal);

    //intertplation 
    upper(interpo);
	std::map<LAString, LAString>& ivar = LACoreComponentManager::getInterpolationMap();
	std::map<LAString, LAString>::iterator it = ivar.find(interpo);
    if(it==ivar.end())
		throw LACoreInvalidData("Interpolation is not registered in interpolation methods",	__FILE__,__LINE__);
	LAString interpolation = it->second;

    LAInterpolationBase* pInterpo = NULL;
    LAFunctionManager& em = dataInstance->getFunctionMaster();
    const LACoreFunctionHolder& h = em.getFunction(interpolation);
    if (! h.isDefined() || ! h.isTypeOf(FN_INTERPOLATION)) 
    {
        LAString msg(interpo);
        msg += " is not found in Function Master as LAInterpolationBase";
        throw LACoreInvalidData(msg.getCString(), __FILE__,__LINE__);
    }
    pInterpo = dynamic_cast<LAInterpolationBase*>(h.get().clone());
    LACoreFunctionHolder fh; fh.set(pInterpo,true);

    upper(capTerm);
    IntVector capTerm_mon = changeDateFromStringToMonth(capTerm);
    unsigned int span = LAMathYieldCurve::setSpanFromFrequency( conv.freq );
    IntVector expiryMonth;
    for(size_t i=1; i*span < capTerm_mon.back(); i++)      
    {
        expiryMonth.push_back( i*span );
    }
    LAStringVector expiryTerm_str = changeDateFromMonthToString(expiryMonth);
    upper(target);
    DoubleMatrix capletVol, capVol;    
    DoubleMatrix sabr_result(expiryMonth.size(), DoubleArray(4));

    if( isSABR )
    {
        upper(sabrLimiter);
        LAMathSABRLimiter limiter(sabrLimiter);
        vector<LAMathSABR_Hagan> sabrParam(expiryMonth.size());
        for(size_t i=0; i<expiryMonth.size(); i++)   
        {
            sabrParam[i].setSABRParam(sabrParamMat[i][0], sabrParamMat[i][1], sabrParamMat[i][2], sabrParamMat[i][3]);
        }
        LAMathCapFloorSABR sabr(expiryMonth, sabrParam, pInterpo, conv, curveID_, foreCurveName, dfCurveName);
        sabr.calibrateToCapVol(dataInstance, limiter, volMat, strike, capTerm_mon, weight, target, capletVol, capVol);
        for(size_t i=0; i<expiryMonth.size(); i++)   
        {
            sabr_result[i][0] = sabr.getSABRParam(i).getAlpha();
            sabr_result[i][1] = sabr.getSABRParam(i).getBeta();
            sabr_result[i][2] = sabr.getSABRParam(i).getNu();
            sabr_result[i][3] = sabr.getSABRParam(i).getRho();
        }
    }
    else
    {
        vector<LAMathSABR_Hagan> sabrParam(expiryMonth.size());
        LAMathCapFloorSABR sabr(expiryMonth, sabrParam, pInterpo, conv, curveID_, foreCurveName, dfCurveName);
        if( target == LAString("CAP") )
        {
            sabr.getCapletVolMat(dataInstance, volMat, strike, capTerm_mon, capletVol, capVol);
        }
        else if( target == LAString("CAPLET") )
        {
            sabr.getCapletVolMat2(dataInstance, volMat, strike, capTerm_mon, capletVol, capVol);
        }
		else
		{
			throw LACoreInvalidData("Target is not supported!",	__FILE__,__LINE__);
		}
    }
    
    LAObjectPool &objPool = dataInstance->getObjectPool();
    LAObjectHolder objHolder = objPool.getObject(capFloorID ,ENCHKTYPE_NOCHECK);
    if(!objHolder.isDefined())
	{
		LAObject* e = new LAObject();
		e->add( "NAME", new LADataString()).convertFromString(capFloorID);
        e->add( "CurveID", new LADataString()).convertFromString(curveID_);
		e->add( "ForecastCurveName", new LADataString()).convertFromString(foreCurveName);
		e->add( "DiscountCurveName", new LADataString()).convertFromString(dfCurveName);
        e->add( "STRIKE", new LADataDoubles(strike));
        e->add( "EXPIRYDATE", new LADataStrings(expiryTerm_str));
        e->add( "EXPIRYMONTH", new LADataInts(expiryMonth));
        e->add( IR_CALIBRATION_DATA_FREQUENCY, new LADataString(freq));
        e->add( IR_CALIBRATION_DATA_DAYCOUNT, new LADataString(daycount));
        e->add( CURVEINPUT_SPOTLAG, new LADataString(spotLag));
        e->add( CALIBRATION_DATA_SLIDINGRULE, new LAPriceDataSlidingRule(paySlr_att));
        e->add( CALIBRATION_DATA_CALENDAR, new LAPriceDataCalendar(payCal_att));
        e->add( PRICING_DATA_FIXINGCALENDAR, new LAPriceDataCalendar(fixCal_att));
        e->add( "CAPLETVOLMATRIX", new LADataDoubleMatrix(capletVol));
        e->add( "CAPVOLMATRIX", new LADataDoubleMatrix(capVol));
        e->add( "SABRPARAM", new LADataDoubleMatrix(sabr_result));
        e->add( CALIBRATION_DATA_INTERPOLATION, new LAPriceDataInterpolation()).convertFromString(interpolation);

		objPool.set( capFloorID , e );
	}
	else if(objHolder.isDefined())
	{
        LADataHolder* dh;
		dh = &objHolder.getData("CurveID",ISDEFINED);
		LADataString& curveid_att = dynamic_cast<LADataString &>(dh->get());
		curveid_att.set(curveID_);

        dh = &objHolder.getData("STRIKE",ISDEFINED);
		LADataDoubles& strike_= dynamic_cast<LADataDoubles &>(dh->get());
        strike_.set(strike);

		dh = &objHolder.getData("EXPIRYDATE",ISDEFINED);
		LADataStrings& expiryTerm_ = dynamic_cast<LADataStrings &>(dh->get());
		expiryTerm_.set(expiryTerm_str);

        dh = &objHolder.getData("EXPIRYMONTH",ISDEFINED);
		LADataInts& expiryMonth_ = dynamic_cast<LADataInts &>(dh->get());
		expiryMonth_.set(expiryMonth);

		dh = &objHolder.getData(IR_CALIBRATION_DATA_FREQUENCY,ISDEFINED);
		LADataString& freq_ = dynamic_cast<LADataString &>(dh->get());
		freq_.set(freq);

        dh = &objHolder.getData(IR_CALIBRATION_DATA_DAYCOUNT,ISDEFINED);
		LADataString& daycount_ = dynamic_cast<LADataString &>(dh->get());
		daycount_.set(daycount);          

        dh = &objHolder.getData(CURVEINPUT_SPOTLAG,ISDEFINED);
		LADataString& spotLag_ = dynamic_cast<LADataString& >(dh->get());
		spotLag_.set(spotLag);

        dh = &objHolder.getData(CALIBRATION_DATA_SLIDINGRULE,ISDEFINED);
		LAPriceDataSlidingRule& paySlr_att_ = dynamic_cast<LAPriceDataSlidingRule& >(dh->get());
		paySlr_att_ = paySlr_att;

        dh = &objHolder.getData(CALIBRATION_DATA_CALENDAR,ISDEFINED);
		LAPriceDataCalendar& payCal_att_ = dynamic_cast<LAPriceDataCalendar& >(dh->get());
		payCal_att_ = payCal_att;

        dh = &objHolder.getData(PRICING_DATA_FIXINGCALENDAR,ISDEFINED);
        LAPriceDataCalendar& fixCal_att_ = dynamic_cast<LAPriceDataCalendar& >(dh->get());
		fixCal_att_ = fixCal_att;

        dh = &objHolder.getData("CAPLETVOLMATRIX",ISDEFINED);
		LADataDoubleMatrix& capletVol_ = dynamic_cast<LADataDoubleMatrix &>(dh->get());
		capletVol_.set(capletVol);

        dh = &objHolder.getData("CAPVOLMATRIX",ISDEFINED);
		LADataDoubleMatrix& capVol_ = dynamic_cast<LADataDoubleMatrix &>(dh->get());
		capVol_.set(capVol);

        dh = &objHolder.getData("SABRPARAM",ISDEFINED);
		LADataDoubleMatrix& sabr_result_ = dynamic_cast<LADataDoubleMatrix &>(dh->get());
		sabr_result_.set(sabr_result);

        dh = &objHolder.getData(CALIBRATION_DATA_INTERPOLATION,ISDEFINED);
		LAPriceDataInterpolation& interpo_att = dynamic_cast<LAPriceDataInterpolation &>(dh->get());
		interpo_att.convertFromString(interpolation);
	}
}

void
LAMathCapFloorVolUtility::outPutCapFloor( LADataInstance* dataInstance, const LAString& capFloorID, LAString capFloorFlag, 
        DoubleVector& ret, size_t& row, size_t& colum )
{
    ret.clear();

    LAObject object = dataInstance->getObjectPool().getObject( capFloorID, ENCHKTYPE_ISDEFINED ).get();
    const LAStringVector& expiryDate = 
        dynamic_cast<const LADataStrings& >(object.getData("EXPIRYDATE",ISDEFINED).get()).get();
    const DoubleVector& strike = 
        dynamic_cast<const LADataDoubles& >(object.getData("STRIKE",ISDEFINED).get()).get();
    size_t expirySize = expiryDate.size();
    size_t strikeSize = strike.size();
    upper(capFloorFlag);
    if( capFloorFlag == "CAPFLOORVOL" )
    {
        const DoubleMatrix& capletVol = 
            dynamic_cast<const LADataDoubleMatrix& >(object.getData("CAPVOLMATRIX",ISDEFINED).get()).get();
		if(capletVol.size() != expirySize) throw LACoreInvalidData("caplet vol matrix and expiry date are inconsistent!",	__FILE__,__LINE__);
		if(capletVol[0].size() != strikeSize) throw LACoreInvalidData("caplet vol matrix and strike are inconsistent!",	__FILE__,__LINE__);

        for(size_t j=0; j<strikeSize; j++)
        {
            ret.push_back( strike[j] );
        }
        for(size_t i=0; i<expirySize; i++)
        {
            for(size_t j=0; j<strikeSize; j++)
            {
                ret.push_back( capletVol[i][j] );
            }
        }
        row = expirySize+1;
        colum = strikeSize;
    }
    else if( capFloorFlag == "CAPFLOORLETVOL" )
    {
        const DoubleMatrix& capVol = 
            dynamic_cast<const LADataDoubleMatrix& >(object.getData("CAPLETVOLMATRIX",ISDEFINED).get()).get();
		if(capVol.size() != expirySize) throw LACoreInvalidData("cap vol matrix and expiry date are inconsistent!",	__FILE__,__LINE__);
		if(capVol[0].size() != strikeSize) throw LACoreInvalidData("cap vol matrix and strike are inconsistent!",	__FILE__,__LINE__);

        for(size_t j=0; j<strikeSize; j++)
        {
            ret.push_back( strike[j] );
        }
        for(size_t i=0; i<expirySize; i++)
        {
            for(size_t j=0; j<strikeSize; j++)
            {
                ret.push_back( capVol[i][j] );
            }
        }
        row = expirySize+1;
        colum = strikeSize;
    }
    else if( capFloorFlag == "SABRPARAM" )
    {
        const DoubleMatrix& sabrPARAM = 
            dynamic_cast<const LADataDoubleMatrix& >(object.getData("SABRPARAM",ISDEFINED).get()).get();
		if(sabrPARAM.size() != expirySize) throw LACoreInvalidData("sabr param number and expiry date are inconsistent!",	__FILE__,__LINE__);
		if(sabrPARAM[0].size() != 4) throw LACoreInvalidData("sabr param number and strike are inconsistent!",	__FILE__,__LINE__);

        for(size_t i=0; i<expirySize; i++)
        {
            ret.push_back( sabrPARAM[i][0] );
            ret.push_back( sabrPARAM[i][1] );
            ret.push_back( sabrPARAM[i][2] );
            ret.push_back( sabrPARAM[i][3] );
        }
        row = expirySize;
        colum = 4;
    }
    else
    {
        throw LACoreInvalidData("cap floor flag is not supported.",	__FILE__,__LINE__);
    }
        
}

double 
LAMathCapFloorVolUtility::lookUpCapFloorVol
( LADataInstance* dataInstance, const LAString& capFloorID, double strike, const LADate& fixingDate )
{
    LAObject object = dataInstance->getObjectPool().getObject( capFloorID, ENCHKTYPE_ISDEFINED ).get();
    LAString curveID = 
        dynamic_cast<const LADataString& >(object.getData("CurveID",ISDEFINED).get()).get();
    LAString freq = 
        dynamic_cast<const LADataString& >(object.getData(IR_CALIBRATION_DATA_FREQUENCY,ISDEFINED).get()).get();
    LAString spotLag = 
        dynamic_cast<const LADataString& >(object.getData(CURVEINPUT_SPOTLAG,ISDEFINED).get()).get();
    LAString daycount =
        dynamic_cast<const LADataString& >(object.getData(IR_CALIBRATION_DATA_DAYCOUNT,ISDEFINED).get()).get();
    LAPriceDataSlidingRule paySlr = 
        dynamic_cast<const LAPriceDataSlidingRule& >(object.getData(CALIBRATION_DATA_SLIDINGRULE,ISDEFINED).get());
    LAPriceDataCalendar payCal =
        dynamic_cast<const LAPriceDataCalendar& >(object.getData(CALIBRATION_DATA_CALENDAR,ISDEFINED).get());
    LAPriceDataCalendar fixCal =
        dynamic_cast<const LAPriceDataCalendar& >(object.getData(PRICING_DATA_FIXINGCALENDAR,ISDEFINED).get());
    LAMathCapFloorConvention conv(freq, spotLag, daycount, paySlr, payCal, fixCal);

    const IntVector& expiryMonth = 
        dynamic_cast<const LADataInts& >(object.getData("EXPIRYMONTH",ISDEFINED).get()).get();

    const DoubleMatrix& sabrParamMat = 
            dynamic_cast<const LADataDoubleMatrix& >(object.getData("SABRPARAM",ISDEFINED).get()).get();
    
    LAInterpolationBase* pInter = dynamic_cast<LAInterpolationBase*>
        (dynamic_cast<LAPriceDataInterpolation& >(object.getData(CALIBRATION_DATA_INTERPOLATION,ISDEFINED).get()).getMethod().clone());
    LACoreFunctionHolder fh(pInter, true);

    vector<LAMathSABR_Hagan> sabrParam(expiryMonth.size());
    for(size_t i=0; i<expiryMonth.size(); i++)   
    {
        sabrParam[i].setSABRParam(sabrParamMat[i][0], sabrParamMat[i][1], sabrParamMat[i][2], sabrParamMat[i][3]);
    }
    LAMathCapFloorSABR sabr(expiryMonth, sabrParam, pInter, conv, curveID);

    const LADate asOfDate = 
        dynamic_cast<const LADataDate& >(dataInstance->getObjectPool().getObject(curveID,ENCHKTYPE_ISDEFINED).
                                get().getData(CALIBRATION_DATA_ASOFDATE,ISNOTNULL).get()).get();
    double fixingTerm =  LAMathDateUtilities::getTerm(asOfDate,fixingDate,conv.daycount,true); 

    if( fixingTerm<0. ) throw LACoreInvalidData("fixing date is before as of date.!",__FILE__,__LINE__);

    return sabr.getVol(dataInstance, fixingTerm, strike);
}

//double 
//LAMathCapFloorVolUtility::lookUpCapFloorPrem( LADataInstance* dataInstance, const LAString& capFloorID, const LADate& firstFixDate, 
//        double strike, const LAString& capTerm, const LAString& capOrFloor )
//{
//    LAObject object = dataInstance->getObjectPool().getObject( capFloorID, ENCHKTYPE_ISDEFINED ).get();
//    LAString curveID = 
//        dynamic_cast<const LADataString& >(object.getData("CurveID",ISDEFINED).get()).get();
//    LAString freq = 
//        dynamic_cast<const LADataString& >(object.getData(IR_CALIBRATION_DATA_FREQUENCY,ISDEFINED).get()).get();
//    LAString spotLag = 
//        dynamic_cast<const LADataString& >(object.getData(CURVEINPUT_SPOTLAG,ISDEFINED).get()).get();
//    LAString daycount =
//        dynamic_cast<const LADataString& >(object.getData(IR_CALIBRATION_DATA_DAYCOUNT,ISDEFINED).get()).get();
//    LAPriceDataSlidingRule paySlr = 
//        dynamic_cast<const LAPriceDataSlidingRule& >(object.getData(CALIBRATION_DATA_SLIDINGRULE,ISDEFINED).get()).get();
//    LAPriceDataCalendar payCal =
//        dynamic_cast<const LAPriceDataCalendar& >(object.getData(CALIBRATION_DATA_CALENDAR,ISDEFINED).get()).get();
//    LAPriceDataCalendar fixCal =
//        dynamic_cast<const LAPriceDataCalendar& >(object.getData(PRICING_DATA_FIXINGCALENDAR,ISDEFINED).get()).get();
//    LAMathCapFloorConvention conv(curveID, freq, spotLag, daycount, paySlr, payCal, fixCal);
//
//    const IntVector& expiryMonth = 
//        dynamic_cast<const LADataInts& >(object.getData("EXPIRYMONTH",ISDEFINED).get()).get();
//
//    const DoubleMatrix& sabrPARAM = 
//            dynamic_cast<const LADataDoubleMatrix& >(object.getData("SABRPARAM",ISDEFINED).get()).get();
//    
//    const LAInterpolationBase& inter 
//        dynamic_cast<const LAPriceDataInterpolation& >(object.getData(CALIBRATION_DATA_INTERPOLATION,ISDEFINED).get()).getMethod();
//
//    LAMathCapFloorSABR sabr(expiryMonth, sabrPARAM, &inter, conv);
//
//    int y,m,d;
//    int capTerm_m;
//    LAMathDateCalculations::termStrtoYMD(capTerm, y, m, d);
//    capTerm_m = 12 * y + m;
//
//    if( capTerm_m % span != 0 ) throw LACoreInvalidData("cap term is inconsistent with frequency.",	__FILE__,__LINE__);
//
//    map<LAString, LABlackScholesBase*> &var = LACoreComponentManager::getBlackComponentMap();
//    LAString bscomponent;
//    if(capOrFloor == CAP) bscomponent = LAString(BK)+LAString(PREM)+LAString(CALL);
//    else if(capOrFloor == FLOOR) bscomponent = LAString(BK)+LAString(PREM)+LAString(PUT);
//    else throw LACoreInvalidData("option type is not supported.",	__FILE__,__LINE__)
//    map<LAString, LABlackScholesBase*>::iterator it = var.find(bscomponent);
//    if(it==var.end()) throw LACoreInvalidData("Product Type is not supported",__FILE__,__LINE__);
//	LABlackScholesBase* p = it->second;
//    double ret=0.;
//    LAPriceDataSlidingRule slr_Pre; slr_Pre.convertFromString(PRE);
//    LAPriceDataSlidingRule slr_Fol; slr_Fol.convertFromString(FOL);
//    const LADate asOfDate = dynamic_cast<const LADataDate& >(dataInstance->getObjectPool().getObject(conv.curveID,ENCHKTYPE_ISDEFINED).
//                                get().getData(CALIBRATION_DATA_ASOFDATE,ISNOTNULL).get()).get();
//    //LAMathYieldCurve
//	LAMathYieldCurve& yc = LAMathCurveFuncUtility::getYieldCurveForCurveID(dataInstance,conv.curveID);
//	yc.setInterpolation(FN_SPLINEINTERPOLATION_STR);
//	yc.getDayCount().setDayCount(conv.daycount);
//    yc.getSlidingRule().convertFromString(NO_CH);
//    yc.getCalendar() = conv.payCal;
//	yc.getFrequency().convertFromString(conv.freq);
//
//    LAPriceDataDayCount dc_act365(ACT_365_ISDA);
//    LAPriceDataDayCount daycount; daycount.convertFromString(conv.daycount);
//
//    AnalyticBKParam param; param.K = strike; 
//    for(size_t i=0; span*(i+1)<=capTerm_m; i++)
//    {
//        LADate fixingDate = 
//        param.Te = ;
//        param.F = F[i]; param.Nu = Nu[i]; 
//        ret += p->calc(param);
//    }
//    return ret;
//}

IntVector 
LAMathCapFloorVolUtility::changeDateFromStringToMonth(const LAStringVector& expiryDate)
{
    int y,m,d,w;
    IntVector expiryMonth;
    for(size_t i=0; i<expiryDate.size(); i++)
    {
        //expiryMonth.push_back(12*y + m);
		LAMathDateCalculations::termStrtoYMDW(expiryDate[i], y, m, d, w);
        expiryMonth.push_back(12*y + m + w / 4);
    }
    return expiryMonth;
}

LAStringVector
LAMathCapFloorVolUtility::changeDateFromMonthToString(const IntVector& expiryMonth)
{
    LAStringVector expiryDate;
    int y,m;
    for(size_t i=0; i<expiryMonth.size(); i++)
    {
        m = expiryMonth[i] % 12;
        y = (expiryMonth[i] - m) / 12;
        if( m != 0)
        {
            expiryDate.push_back( LAString(y) + LAString("Y") + LAString(m) + LAString("M") );
        }
        else
        {
            expiryDate.push_back( LAString(y) + LAString("Y") );
        }
    }
    return expiryDate;
}