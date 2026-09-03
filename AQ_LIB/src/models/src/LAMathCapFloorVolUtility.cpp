#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include <vector>
#include <AQLDataInstance.h>
#include <AQLDataMatrix.h>
#include <AQLPriceDataInterpolation.h>
#include <AQLFunctionManager.h>
#include <AQLCoreFunctionHolder.h>
#include "AQLInterpolationBase.h"
#include <LAMathDateCalculations.h>
#include "LAMathDateUtilities.h"
#include <LAPriceCashFlowGenerator.h>
#include <AQLMathDefine.h>
#include <LAMathYieldCurve.h>
#include "AQLCoreComponentManager.h"
#include "LAMathCapFloorVolUtility.h"
#include "LAMathCapFloorSABR.h"
#include "LAMathSABR.h"
#include "AQLFunctionUtilities.h"

using namespace std;

void
LAMathCapFloorVolUtility::setUpCapFloorVolSurf( AQLDataInstance* dataInstance, const AQLString& capFloorID,
        AQLStringMatrix convMat, const DoubleMatrix& volMat, const DoubleVector& strike,
        AQLStringVector capTerm, const DoubleMatrix& sabrParamMat, AQLStringMatrix sabrLimiter,
        AQLString target, AQLString interpo, const DoubleVector& weight, bool isSABR,
		const AQLString& curveID, AQLString foreCurveName, AQLString dfCurveName)
{
    AQLString curveID_;
	if(curveID==AQLString(""))
	{
		curveID_ = searchbyrow(convMat, "CurveID",1,true);
	}
	else
	{
		curveID_ = curveID;
	}

    upper(convMat);
    AQLString freq = searchbyrow(convMat, CURVEINPUT_FREQUENCY,1,true);
    AQLString daycount = searchbyrow(convMat, CURVEINPUT_DAYCOUNT,1,true);
    AQLString paySlr = searchbyrow(convMat, CURVEINPUT_SLIDINGRULE,1,true);
    AQLString spotLag = searchbyrow(convMat, CURVEINPUT_SPOTLAG,1,true);
    AQLString payCal = searchbyrow(convMat, "PAYMENTCALENDAR",1,true);
    AQLString fixCal = searchbyrow(convMat, "FIXINGCALENDAR",1,true);
    LAMathCapFloorConvention conv(freq, spotLag, daycount, paySlr, payCal, fixCal);

    AQLPriceDataSlidingRule paySlr_att; paySlr_att.convertFromString(paySlr);
    AQLPriceDataCalendar payCal_att; payCal_att.convertFromString(payCal);
    AQLPriceDataCalendar fixCal_att; fixCal_att.convertFromString(fixCal);

    //intertplation 
    upper(interpo);
	std::map<AQLString, AQLString>& ivar = AQLCoreComponentManager::getInterpolationMap();
	std::map<AQLString, AQLString>::iterator it = ivar.find(interpo);
    if(it==ivar.end())
		throw AQLCoreInvalidData("Interpolation is not registered in interpolation methods",	__FILE__,__LINE__);
	AQLString interpolation = it->second;

    AQLInterpolationBase* pInterpo = NULL;
    AQLFunctionManager& em = dataInstance->getFunctionMaster();
    const AQLCoreFunctionHolder& h = em.getFunction(interpolation);
    if (! h.isDefined() || ! h.isTypeOf(FN_INTERPOLATION)) 
    {
        AQLString msg(interpo);
        msg += " is not found in Function Master as AQLInterpolationBase";
        throw AQLCoreInvalidData(msg.getCString(), __FILE__,__LINE__);
    }
    pInterpo = dynamic_cast<AQLInterpolationBase*>(h.get().clone());
    AQLCoreFunctionHolder fh; fh.set(pInterpo,true);

    upper(capTerm);
    IntVector capTerm_mon = changeDateFromStringToMonth(capTerm);
    unsigned int span = LAMathYieldCurve::setSpanFromFrequency( conv.freq );
    IntVector expiryMonth;
    for(size_t i=1; i*span < capTerm_mon.back(); i++)      
    {
        expiryMonth.push_back( i*span );
    }
    AQLStringVector expiryTerm_str = changeDateFromMonthToString(expiryMonth);
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
        if( target == AQLString("CAP") )
        {
            sabr.getCapletVolMat(dataInstance, volMat, strike, capTerm_mon, capletVol, capVol);
        }
        else if( target == AQLString("CAPLET") )
        {
            sabr.getCapletVolMat2(dataInstance, volMat, strike, capTerm_mon, capletVol, capVol);
        }
		else
		{
			throw AQLCoreInvalidData("Target is not supported!",	__FILE__,__LINE__);
		}
    }
    
    AQLObjectPool &objPool = dataInstance->getObjectPool();
    AQLObjectHolder objHolder = objPool.getObject(capFloorID ,ENCHKTYPE_NOCHECK);
    if(!objHolder.isDefined())
	{
		AQLObject* e = new AQLObject();
		e->add( "NAME", new AQLDataString()).convertFromString(capFloorID);
        e->add( "CurveID", new AQLDataString()).convertFromString(curveID_);
		e->add( "ForecastCurveName", new AQLDataString()).convertFromString(foreCurveName);
		e->add( "DiscountCurveName", new AQLDataString()).convertFromString(dfCurveName);
        e->add( "STRIKE", new AQLDataDoubles(strike));
        e->add( "EXPIRYDATE", new AQLDataStrings(expiryTerm_str));
        e->add( "EXPIRYMONTH", new AQLDataInts(expiryMonth));
        e->add( IR_CALIBRATION_DATA_FREQUENCY, new AQLDataString(freq));
        e->add( IR_CALIBRATION_DATA_DAYCOUNT, new AQLDataString(daycount));
        e->add( CURVEINPUT_SPOTLAG, new AQLDataString(spotLag));
        e->add( CALIBRATION_DATA_SLIDINGRULE, new AQLPriceDataSlidingRule(paySlr_att));
        e->add( CALIBRATION_DATA_CALENDAR, new AQLPriceDataCalendar(payCal_att));
        e->add( PRICING_DATA_FIXINGCALENDAR, new AQLPriceDataCalendar(fixCal_att));
        e->add( "CAPLETVOLMATRIX", new AQLDataDoubleMatrix(capletVol));
        e->add( "CAPVOLMATRIX", new AQLDataDoubleMatrix(capVol));
        e->add( "SABRPARAM", new AQLDataDoubleMatrix(sabr_result));
        e->add( CALIBRATION_DATA_INTERPOLATION, new AQLPriceDataInterpolation()).convertFromString(interpolation);

		objPool.set( capFloorID , e );
	}
	else if(objHolder.isDefined())
	{
        AQLDataHolder* dh;
		dh = &objHolder.getData("CurveID",ISDEFINED);
		AQLDataString& curveid_att = dynamic_cast<AQLDataString &>(dh->get());
		curveid_att.set(curveID_);

        dh = &objHolder.getData("STRIKE",ISDEFINED);
		AQLDataDoubles& strike_= dynamic_cast<AQLDataDoubles &>(dh->get());
        strike_.set(strike);

		dh = &objHolder.getData("EXPIRYDATE",ISDEFINED);
		AQLDataStrings& expiryTerm_ = dynamic_cast<AQLDataStrings &>(dh->get());
		expiryTerm_.set(expiryTerm_str);

        dh = &objHolder.getData("EXPIRYMONTH",ISDEFINED);
		AQLDataInts& expiryMonth_ = dynamic_cast<AQLDataInts &>(dh->get());
		expiryMonth_.set(expiryMonth);

		dh = &objHolder.getData(IR_CALIBRATION_DATA_FREQUENCY,ISDEFINED);
		AQLDataString& freq_ = dynamic_cast<AQLDataString &>(dh->get());
		freq_.set(freq);

        dh = &objHolder.getData(IR_CALIBRATION_DATA_DAYCOUNT,ISDEFINED);
		AQLDataString& daycount_ = dynamic_cast<AQLDataString &>(dh->get());
		daycount_.set(daycount);          

        dh = &objHolder.getData(CURVEINPUT_SPOTLAG,ISDEFINED);
		AQLDataString& spotLag_ = dynamic_cast<AQLDataString& >(dh->get());
		spotLag_.set(spotLag);

        dh = &objHolder.getData(CALIBRATION_DATA_SLIDINGRULE,ISDEFINED);
		AQLPriceDataSlidingRule& paySlr_att_ = dynamic_cast<AQLPriceDataSlidingRule& >(dh->get());
		paySlr_att_ = paySlr_att;

        dh = &objHolder.getData(CALIBRATION_DATA_CALENDAR,ISDEFINED);
		AQLPriceDataCalendar& payCal_att_ = dynamic_cast<AQLPriceDataCalendar& >(dh->get());
		payCal_att_ = payCal_att;

        dh = &objHolder.getData(PRICING_DATA_FIXINGCALENDAR,ISDEFINED);
        AQLPriceDataCalendar& fixCal_att_ = dynamic_cast<AQLPriceDataCalendar& >(dh->get());
		fixCal_att_ = fixCal_att;

        dh = &objHolder.getData("CAPLETVOLMATRIX",ISDEFINED);
		AQLDataDoubleMatrix& capletVol_ = dynamic_cast<AQLDataDoubleMatrix &>(dh->get());
		capletVol_.set(capletVol);

        dh = &objHolder.getData("CAPVOLMATRIX",ISDEFINED);
		AQLDataDoubleMatrix& capVol_ = dynamic_cast<AQLDataDoubleMatrix &>(dh->get());
		capVol_.set(capVol);

        dh = &objHolder.getData("SABRPARAM",ISDEFINED);
		AQLDataDoubleMatrix& sabr_result_ = dynamic_cast<AQLDataDoubleMatrix &>(dh->get());
		sabr_result_.set(sabr_result);

        dh = &objHolder.getData(CALIBRATION_DATA_INTERPOLATION,ISDEFINED);
		AQLPriceDataInterpolation& interpo_att = dynamic_cast<AQLPriceDataInterpolation &>(dh->get());
		interpo_att.convertFromString(interpolation);
	}
}

void
LAMathCapFloorVolUtility::outPutCapFloor( AQLDataInstance* dataInstance, const AQLString& capFloorID, AQLString capFloorFlag, 
        DoubleVector& ret, size_t& row, size_t& colum )
{
    ret.clear();

    AQLObject object = dataInstance->getObjectPool().getObject( capFloorID, ENCHKTYPE_ISDEFINED ).get();
    const AQLStringVector& expiryDate = 
        dynamic_cast<const AQLDataStrings& >(object.getData("EXPIRYDATE",ISDEFINED).get()).get();
    const DoubleVector& strike = 
        dynamic_cast<const AQLDataDoubles& >(object.getData("STRIKE",ISDEFINED).get()).get();
    size_t expirySize = expiryDate.size();
    size_t strikeSize = strike.size();
    upper(capFloorFlag);
    if( capFloorFlag == "CAPFLOORVOL" )
    {
        const DoubleMatrix& capletVol = 
            dynamic_cast<const AQLDataDoubleMatrix& >(object.getData("CAPVOLMATRIX",ISDEFINED).get()).get();
		if(capletVol.size() != expirySize) throw AQLCoreInvalidData("caplet vol matrix and expiry date are inconsistent!",	__FILE__,__LINE__);
		if(capletVol[0].size() != strikeSize) throw AQLCoreInvalidData("caplet vol matrix and strike are inconsistent!",	__FILE__,__LINE__);

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
            dynamic_cast<const AQLDataDoubleMatrix& >(object.getData("CAPLETVOLMATRIX",ISDEFINED).get()).get();
		if(capVol.size() != expirySize) throw AQLCoreInvalidData("cap vol matrix and expiry date are inconsistent!",	__FILE__,__LINE__);
		if(capVol[0].size() != strikeSize) throw AQLCoreInvalidData("cap vol matrix and strike are inconsistent!",	__FILE__,__LINE__);

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
            dynamic_cast<const AQLDataDoubleMatrix& >(object.getData("SABRPARAM",ISDEFINED).get()).get();
		if(sabrPARAM.size() != expirySize) throw AQLCoreInvalidData("sabr param number and expiry date are inconsistent!",	__FILE__,__LINE__);
		if(sabrPARAM[0].size() != 4) throw AQLCoreInvalidData("sabr param number and strike are inconsistent!",	__FILE__,__LINE__);

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
        throw AQLCoreInvalidData("cap floor flag is not supported.",	__FILE__,__LINE__);
    }
        
}

double 
LAMathCapFloorVolUtility::lookUpCapFloorVol
( AQLDataInstance* dataInstance, const AQLString& capFloorID, double strike, const AQLDate& fixingDate )
{
    AQLObject object = dataInstance->getObjectPool().getObject( capFloorID, ENCHKTYPE_ISDEFINED ).get();
    AQLString curveID = 
        dynamic_cast<const AQLDataString& >(object.getData("CurveID",ISDEFINED).get()).get();
    AQLString freq = 
        dynamic_cast<const AQLDataString& >(object.getData(IR_CALIBRATION_DATA_FREQUENCY,ISDEFINED).get()).get();
    AQLString spotLag = 
        dynamic_cast<const AQLDataString& >(object.getData(CURVEINPUT_SPOTLAG,ISDEFINED).get()).get();
    AQLString daycount =
        dynamic_cast<const AQLDataString& >(object.getData(IR_CALIBRATION_DATA_DAYCOUNT,ISDEFINED).get()).get();
    AQLPriceDataSlidingRule paySlr = 
        dynamic_cast<const AQLPriceDataSlidingRule& >(object.getData(CALIBRATION_DATA_SLIDINGRULE,ISDEFINED).get());
    AQLPriceDataCalendar payCal =
        dynamic_cast<const AQLPriceDataCalendar& >(object.getData(CALIBRATION_DATA_CALENDAR,ISDEFINED).get());
    AQLPriceDataCalendar fixCal =
        dynamic_cast<const AQLPriceDataCalendar& >(object.getData(PRICING_DATA_FIXINGCALENDAR,ISDEFINED).get());
    LAMathCapFloorConvention conv(freq, spotLag, daycount, paySlr, payCal, fixCal);

    const IntVector& expiryMonth = 
        dynamic_cast<const AQLDataInts& >(object.getData("EXPIRYMONTH",ISDEFINED).get()).get();

    const DoubleMatrix& sabrParamMat = 
            dynamic_cast<const AQLDataDoubleMatrix& >(object.getData("SABRPARAM",ISDEFINED).get()).get();
    
    AQLInterpolationBase* pInter = dynamic_cast<AQLInterpolationBase*>
        (dynamic_cast<AQLPriceDataInterpolation& >(object.getData(CALIBRATION_DATA_INTERPOLATION,ISDEFINED).get()).getMethod().clone());
    AQLCoreFunctionHolder fh(pInter, true);

    vector<LAMathSABR_Hagan> sabrParam(expiryMonth.size());
    for(size_t i=0; i<expiryMonth.size(); i++)   
    {
        sabrParam[i].setSABRParam(sabrParamMat[i][0], sabrParamMat[i][1], sabrParamMat[i][2], sabrParamMat[i][3]);
    }
    LAMathCapFloorSABR sabr(expiryMonth, sabrParam, pInter, conv, curveID);

    const AQLDate asOfDate = 
        dynamic_cast<const AQLDataDate& >(dataInstance->getObjectPool().getObject(curveID,ENCHKTYPE_ISDEFINED).
                                get().getData(CALIBRATION_DATA_ASOFDATE,ISNOTNULL).get()).get();
    double fixingTerm =  LAMathDateUtilities::getTerm(asOfDate,fixingDate,conv.daycount,true); 

    if( fixingTerm<0. ) throw AQLCoreInvalidData("fixing date is before as of date.!",__FILE__,__LINE__);

    return sabr.getVol(dataInstance, fixingTerm, strike);
}

//double 
//LAMathCapFloorVolUtility::lookUpCapFloorPrem( AQLDataInstance* dataInstance, const AQLString& capFloorID, const AQLDate& firstFixDate, 
//        double strike, const AQLString& capTerm, const AQLString& capOrFloor )
//{
//    AQLObject object = dataInstance->getObjectPool().getObject( capFloorID, ENCHKTYPE_ISDEFINED ).get();
//    AQLString curveID = 
//        dynamic_cast<const AQLDataString& >(object.getData("CurveID",ISDEFINED).get()).get();
//    AQLString freq = 
//        dynamic_cast<const AQLDataString& >(object.getData(IR_CALIBRATION_DATA_FREQUENCY,ISDEFINED).get()).get();
//    AQLString spotLag = 
//        dynamic_cast<const AQLDataString& >(object.getData(CURVEINPUT_SPOTLAG,ISDEFINED).get()).get();
//    AQLString daycount =
//        dynamic_cast<const AQLDataString& >(object.getData(IR_CALIBRATION_DATA_DAYCOUNT,ISDEFINED).get()).get();
//    AQLPriceDataSlidingRule paySlr = 
//        dynamic_cast<const AQLPriceDataSlidingRule& >(object.getData(CALIBRATION_DATA_SLIDINGRULE,ISDEFINED).get()).get();
//    AQLPriceDataCalendar payCal =
//        dynamic_cast<const AQLPriceDataCalendar& >(object.getData(CALIBRATION_DATA_CALENDAR,ISDEFINED).get()).get();
//    AQLPriceDataCalendar fixCal =
//        dynamic_cast<const AQLPriceDataCalendar& >(object.getData(PRICING_DATA_FIXINGCALENDAR,ISDEFINED).get()).get();
//    LAMathCapFloorConvention conv(curveID, freq, spotLag, daycount, paySlr, payCal, fixCal);
//
//    const IntVector& expiryMonth = 
//        dynamic_cast<const AQLDataInts& >(object.getData("EXPIRYMONTH",ISDEFINED).get()).get();
//
//    const DoubleMatrix& sabrPARAM = 
//            dynamic_cast<const AQLDataDoubleMatrix& >(object.getData("SABRPARAM",ISDEFINED).get()).get();
//    
//    const AQLInterpolationBase& inter 
//        dynamic_cast<const AQLPriceDataInterpolation& >(object.getData(CALIBRATION_DATA_INTERPOLATION,ISDEFINED).get()).getMethod();
//
//    LAMathCapFloorSABR sabr(expiryMonth, sabrPARAM, &inter, conv);
//
//    int y,m,d;
//    int capTerm_m;
//    LAMathDateCalculations::termStrtoYMD(capTerm, y, m, d);
//    capTerm_m = 12 * y + m;
//
//    if( capTerm_m % span != 0 ) throw AQLCoreInvalidData("cap term is inconsistent with frequency.",	__FILE__,__LINE__);
//
//    map<AQLString, LABlackScholesBase*> &var = AQLCoreComponentManager::getBlackComponentMap();
//    AQLString bscomponent;
//    if(capOrFloor == CAP) bscomponent = AQLString(BK)+AQLString(PREM)+AQLString(CALL);
//    else if(capOrFloor == FLOOR) bscomponent = AQLString(BK)+AQLString(PREM)+AQLString(PUT);
//    else throw AQLCoreInvalidData("option type is not supported.",	__FILE__,__LINE__)
//    map<AQLString, LABlackScholesBase*>::iterator it = var.find(bscomponent);
//    if(it==var.end()) throw AQLCoreInvalidData("Product Type is not supported",__FILE__,__LINE__);
//	LABlackScholesBase* p = it->second;
//    double ret=0.;
//    AQLPriceDataSlidingRule slr_Pre; slr_Pre.convertFromString(PRE);
//    AQLPriceDataSlidingRule slr_Fol; slr_Fol.convertFromString(FOL);
//    const AQLDate asOfDate = dynamic_cast<const AQLDataDate& >(dataInstance->getObjectPool().getObject(conv.curveID,ENCHKTYPE_ISDEFINED).
//                                get().getData(CALIBRATION_DATA_ASOFDATE,ISNOTNULL).get()).get();
//    //LAMathYieldCurve
//	LAMathYieldCurve& yc = LAMathCurveFuncUtility::getYieldCurveForCurveID(dataInstance,conv.curveID);
//	yc.setInterpolation(FN_SPLINEINTERPOLATION_STR);
//	yc.getDayCount().setDayCount(conv.daycount);
//    yc.getSlidingRule().convertFromString(NO_CH);
//    yc.getCalendar() = conv.payCal;
//	yc.getFrequency().convertFromString(conv.freq);
//
//    AQLPriceDataDayCount dc_act365(ACT_365_ISDA);
//    AQLPriceDataDayCount daycount; daycount.convertFromString(conv.daycount);
//
//    AnalyticBKParam param; param.K = strike; 
//    for(size_t i=0; span*(i+1)<=capTerm_m; i++)
//    {
//        AQLDate fixingDate = 
//        param.Te = ;
//        param.F = F[i]; param.Nu = Nu[i]; 
//        ret += p->calc(param);
//    }
//    return ret;
//}

IntVector 
LAMathCapFloorVolUtility::changeDateFromStringToMonth(const AQLStringVector& expiryDate)
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

AQLStringVector
LAMathCapFloorVolUtility::changeDateFromMonthToString(const IntVector& expiryMonth)
{
    AQLStringVector expiryDate;
    int y,m;
    for(size_t i=0; i<expiryMonth.size(); i++)
    {
        m = expiryMonth[i] % 12;
        y = (expiryMonth[i] - m) / 12;
        if( m != 0)
        {
            expiryDate.push_back( AQLString(y) + AQLString("Y") + AQLString(m) + AQLString("M") );
        }
        else
        {
            expiryDate.push_back( AQLString(y) + AQLString("Y") );
        }
    }
    return expiryDate;
}