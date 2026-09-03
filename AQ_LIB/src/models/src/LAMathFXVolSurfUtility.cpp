//  2009, AlgoQuantHub.

//#ifdef VISUAL_STUDIO_2010_ANALYTICS
// this class is only for vanilla addin

//#ifdef __GNUG__
//#pragma implementation
//#else
//#pragma warning(disable:4786)
//#endif
//
//#include <vector>
//
//#include <LAMathDefine.h>
//#include <LAPriceDataCalendar.h>
//#include <LAPriceDataSlidingRule.h>
//#include <LADataReference.h>
//#include <LADataMatrix.h>
//#include "LABasic.h"
//
//#include "LAMathFXVolSurfUtility.h"
//#include "LAMathFXVolatilitySurfaceGenerate.h"
//#include "LACoreUtility.h"
//#include "LAFunctionUtilities.h"
//#include <LAMathDateCalculations.h>
//#include "LAMathDateUtilities.h"
//#include <LAMathYieldCurve.h>
//
//
//#if !defined(WIN32) !defined(WIN64) && !defined(_isnan)
//#include <cmath>
//#define _isnan isnan
//#endif
//
//using namespace std;
//
//double 
//LAMathFXVolSurfUtility::FindStrikeFromDelta( LADataInstance* dataInstance, double delta, double vol, int sgn, 
//                                         LAStringMatrix volSurfaceInfo, const LADate& expiryDate, const LAString& deltaType )
//{
//    LAString dCurveID = searchbyrow(volSurfaceInfo,"DomesticCurveID",1,true);
//    LAString fCurveID = searchbyrow(volSurfaceInfo,"ForeignCurveID",1,true);
//    LAString volSurfaceID = searchbyrow(volSurfaceInfo,"VolSurfaceID",1,true);
//    uppermat(volSurfaceInfo);
//    double asOfFX = dynamic_cast<const LADataDouble& >(dataInstance->getObjectPool().getObject( volSurfaceID, ENCHKTYPE_ISDEFINED ).get()
//        .getData("ASOFFX",ISDEFINED).get()).get();
//    LAString spotLag = dynamic_cast<const LADataString& >(dataInstance->getObjectPool().getObject( volSurfaceID, ENCHKTYPE_ISDEFINED ).get()
//        .getData("SPOTLAG",ISDEFINED).get()).get();
//    LAPriceDataCalendar cal = dynamic_cast<const LAPriceDataCalendar& >(dataInstance->getObjectPool().getObject( volSurfaceID, ENCHKTYPE_ISDEFINED ).get()
//        .getData("CALENDAR",ISDEFINED).get());
//    //Set FXOptionParam
//    LAPriceDataSlidingRule fol; fol.convertFromString(FOL);
//    LADate deliveryDate = LAMathDateCalculations::getDate(expiryDate,spotLag,fol,&cal,true);
//    FXOptionData fxParam = LAMathFXVolatilitySurfaceGenerate::SetFXOptionParam(*dataInstance,dCurveID,fCurveID,expiryDate,deliveryDate,
//        asOfFX,deltaType,"FORWARDATM",cal);
//
//    double logStrike = LAMathFXVolatilitySurfaceGenerate::FindLogStrikeFromDelta(delta, vol, sgn, fxParam);
//    return exp( logStrike ) * fxParam.F;
//}
//
//void LAMathFXVolSurfUtility::SetDeltaType( const LAString& deltaType_str, FXDeltaType& deltaType )
//{
//    if( deltaType_str == "FWDNONPRE" )  { deltaType = FWD_NONPRE;}
//    else if( deltaType_str == "FWDPRE" )  { deltaType = FWD_PRE;}
//    else if( deltaType_str == "SPOTNONPRE" )  { deltaType = SPOT_NONPRE;}
//    else if( deltaType_str == "SPOTPRE" )  { deltaType = SPOT_PRE;}
//    else 
//    {
//        LAString msg("DeltaType is not supported!");
//	    throw LACoreInvalidData(msg.getCString(),__FILE__,__LINE__);
//    };
//}
//
//LAString LAMathFXVolSurfUtility::SetUpFXVolatilitySurface( LADataInstance* dataInstance, LAStringMatrix& volSurfaceInfo, 
//    LAStringMatrix& fxOptionData, LAStringMatrix& smParams_str, LAStringMatrix& wingFactor )
//{
//    //warning message
//    LAString warningMSG = "warning:";
//
//    LAString dCurveID = searchbyrow(volSurfaceInfo,"DomesticCurveID",1,true);
//    LAString fCurveID = searchbyrow(volSurfaceInfo,"ForeignCurveID",1,true);
//    LAString volSurfaceID = searchbyrow(volSurfaceInfo,"VolSurfaceID",1,true);
//    uppermat(volSurfaceInfo);
//    LAString spotLag = searchbyrow(volSurfaceInfo,"SPOTLAG",1,true);
//    LAString calStr = searchbyrow(volSurfaceInfo,"SPOTCALENDAR",1,true);
//    LAPriceDataCalendar cal;
//    cal.convertFromString(calStr); 
//    double spotFX = searchbyrow(volSurfaceInfo,"SPOTFX",1,true).getDoubleValue();
//    //DOMESTICCURVEID
//	LAMathYieldCurve dcurve(dataInstance);
//	dcurve.getYieldData().convertFromString(dCurveID);
//    dcurve.setInterpolation("fn_splineinterpolation");
//    //FOREIGNCURVEID
//	LAMathYieldCurve fcurve(dataInstance);
//	fcurve.getYieldData().convertFromString(fCurveID);	
//	fcurve.setInterpolation("fn_splineinterpolation");
//    const LADate asOfDate = dynamic_cast<const LADataDate& >(dataInstance->getObjectPool().getObject(dCurveID,ENCHKTYPE_ISDEFINED).
//                            get().getData(CALIBRATION_DATA_ASOFDATE,ISNOTNULL).get()).get();
//    LAPriceDataSlidingRule fol; fol.convertFromString(FOL);
//    LAString daycount(AC_365I);
//    double spotTerm = LAMathDateUtilities::getTerm(asOfDate, LAMathDateCalculations::getDate(asOfDate,spotLag,fol,&cal,true), daycount, true);
//    double asOfFX = spotFX * dcurve.getBasisDF( spotTerm ) / fcurve.getBasisDF( spotTerm );
//    
//    LAString isWingFlag_str = searchbyrow(volSurfaceInfo,"ISWINGFLAG",1,true);
//    upper(isWingFlag_str);
//    bool isWingFlag;
//    if( isWingFlag_str == "TRUE" ) isWingFlag = true;
//    else if( isWingFlag_str == "FALSE" ) isWingFlag = false;
//
//    bool isApproximationFlag;
//
//    LAString isApproximationFlag_str = searchbyrow(volSurfaceInfo,"APPROXIMATION",1,true);
//    upper(isApproximationFlag_str);
//    if( isApproximationFlag_str == "TRUE" ) isApproximationFlag = true;
//    else if( isApproximationFlag_str == "FALSE" ) isApproximationFlag = false;
//
//    LAString method_str = searchbyrow(volSurfaceInfo,"INTERPOLATIONMETHOD",1,true);
//    upper(method_str);
//    InterpolationMethod method;
//    LAMathFXVolatilitySurfaceGenerate::SetInterpolationMethod( method_str, method );
//
//    LAString interpoVariable = searchbyrow(volSurfaceInfo,"INTERPOLATIONVARIABLE",1,true);
//    upper(interpoVariable);
//    InterpolationTarget target;
//    LAMathFXVolatilitySurfaceGenerate::SetInterpolationTarget( interpoVariable, target );
//    
//    DoubleVector wfs = LACoreUtility::changeDoubleFromString(searchvecbycol(wingFactor,"WINGFACTOR",true));
//    size_t dataNum = fxOptionData.size()-1;
//    if(fxOptionData.empty() || smParams_str.empty() || wingFactor.empty())
//		throw LACoreInvalidData("Input Matrix is empty",__FILE__,__LINE__);
//    if(smParams_str.size() != fxOptionData.size() || wingFactor.size() != fxOptionData.size())
//		throw LACoreInvalidData("sizes are mismatched ",__FILE__,__LINE__);      
//
//    vector<FXOptionData > fxParams(dataNum);
//    vector<SmileParam > smParams(dataNum);
//    vector<SmileData > smDatas(dataNum);
//    LAStringVector deltaType	= searchvecbycol(fxOptionData,"DELTATYPE",true);
//    LAStringVector atmType	= searchvecbycol(fxOptionData,"ATMTYPE",true);
//    LAStringVector maturityDate_str	= searchvecbycol(fxOptionData,"MATURITYDATE",true);
//    DoubleVector smDatas_att(dataNum * 28 , 0.);
//	DoubleVector fxDatas_att(dataNum * 6 , 0.);
//
//    LAObjectPool &objPool = dataInstance->getObjectPool();
//    LAObjectHolder objHolder = objPool.getObject(volSurfaceID ,ENCHKTYPE_NOCHECK);
//	if(!objHolder.isDefined())
//	{
//		LAObject* e = new LAObject();
//		e->add( "NAME", new LADataString() ).convertFromString(volSurfaceID);
//        e->add( "DomesticCurveID", new LADataString() ).convertFromString(dCurveID);
//        e->add( "ForeignCurveID", new LADataString() ).convertFromString(fCurveID);
//		e->add( "SPOTFX",	new LADataDouble(spotFX));
//        e->add( "ASOFFX",	new LADataDouble(asOfFX));
//        e->add( "MATURITYDATE", new LADataStrings(maturityDate_str));
//        e->add( "DELTATYPE", new LADataStrings(deltaType));
//        e->add( "ATMTYPE", new LADataStrings(atmType));
//        e->add( "INTERPOLATIONMETHOD", new LADataString(method_str));
//        e->add( "INTERPOLATIONVARIABLE", new LADataString(interpoVariable));
//        e->add( "SMILEDATAS", new LADataDoubles(smDatas_att));
//		e->add( "FXOPTIONDATAS", new LADataDoubles(fxDatas_att));
//        e->add( "ISWINGFLAG", new LADataBool(isWingFlag));
//        e->add( "SPOTLAG", new LADataString(spotLag));
//        e->add( "CALENDAR", new LAPriceDataCalendar(cal));
//		objPool.set( volSurfaceID , e );
//	}
//	else if(objHolder.isDefined())
//	{
//        LADataHolder* dh;
//		dh = &objHolder.getData("DomesticCurveID",ISDEFINED);
//		LADataString& dCurveid_att = dynamic_cast<LADataString &>(dh->get());
//		dCurveid_att.set(dCurveID);
//
//		dh = &objHolder.getData("ForeignCurveID",ISDEFINED);
//		LADataString& fCurveid_att = dynamic_cast<LADataString &>(dh->get());
//		fCurveid_att.set(fCurveID);
//
//		dh = &objHolder.getData("SPOTFX",ISDEFINED);
//		LADataDouble& spot_ = dynamic_cast<LADataDouble &>(dh->get());
//		spot_.set(spotFX);
//
//        dh = &objHolder.getData("ASOFFX",ISDEFINED);
//		LADataDouble& asOfFX_att = dynamic_cast<LADataDouble &>(dh->get());
//		asOfFX_att.set(asOfFX);
//
//		dh = &objHolder.getData("MATURITYDATE",ISDEFINED);
//		LADataStrings& maturityDate_att = dynamic_cast<LADataStrings& >(dh->get());
//		maturityDate_att.set(maturityDate_str);
//
//        dh = &objHolder.getData("DELTATYPE",ISDEFINED);
//		LADataStrings& deltaType_att = dynamic_cast<LADataStrings& >(dh->get());
//		deltaType_att.set(deltaType);
//
//        dh = &objHolder.getData("ATMTYPE",ISDEFINED);
//		LADataStrings& atmType_att = dynamic_cast<LADataStrings& >(dh->get());
//		atmType_att.set(atmType);            
//
//        dh = &objHolder.getData("INTERPOLATIONMETHOD",ISDEFINED);
//		LADataString& interpolationMethod_att = dynamic_cast<LADataString& >(dh->get());
//		interpolationMethod_att.set(method_str);
//
//        dh = &objHolder.getData("INTERPOLATIONVARIABLE",ISDEFINED);
//		LADataString& interpoVariable_att = dynamic_cast<LADataString& >(dh->get());
//		interpoVariable_att.set(interpoVariable);
//
//        dh = &objHolder.getData("SMILEDATAS",ISDEFINED);
//		LADataDoubles& smDatas_att_ = dynamic_cast<LADataDoubles& >(dh->get());
//		smDatas_att_.set(smDatas_att);
//
//		dh = &objHolder.getData("FXOPTIONDATAS",ISDEFINED);
//		LADataDoubles& fxDatas_att_ = dynamic_cast<LADataDoubles& >(dh->get());
//		fxDatas_att_.set(fxDatas_att);
//
//        dh = &objHolder.getData("ISWINGFLAG",ISDEFINED);
//		LADataBool& isWingFlag_att_ = dynamic_cast<LADataBool& >(dh->get());
//		isWingFlag_att_.set(isWingFlag);
//
//        dh = &objHolder.getData("SPOTLAG",ISDEFINED);
//		LADataString& spotLag_att = dynamic_cast<LADataString& >(dh->get());
//		spotLag_att.set(spotLag);
//
//        dh = &objHolder.getData("CALENDAR",ISDEFINED);
//		LAPriceDataCalendar& spotCal_att = dynamic_cast<LAPriceDataCalendar& >(dh->get());
//		spotCal_att = cal;
//	}
//    objHolder = objPool.getObject(volSurfaceID ,ENCHKTYPE_NOCHECK);
//
//	LADate maturityDate,deliveryDate;
//    size_t i,j;
//	
//	for(i=0;i<dataNum;i++)
//    {
//		//Set SmileParam
//        smParams[i].atmVol = searchbycol(smParams_str,"ATM",i+1,true).getDoubleValue()/100;
//        smParams[i].lowRR = searchbycol(smParams_str,"10RR",i+1,true).getDoubleValue()/100;
//        smParams[i].lowBF = searchbycol(smParams_str,"10FLY",i+1,true).getDoubleValue()/100;
//        smParams[i].highRR = searchbycol(smParams_str,"25RR",i+1,true).getDoubleValue()/100;
//        smParams[i].highBF = searchbycol(smParams_str,"25FLY",i+1,true).getDoubleValue()/100;
//        
//		//Set FXOptionParam
//        maturityDate = LAMathDateUtilities::getLADate(maturityDate_str[i]);
//        deliveryDate = LAMathDateCalculations::getDate(maturityDate,spotLag,fol,&cal,true);
//        upper(deltaType[i]);
//        upper(atmType[i]);
//        fxParams[i] = LAMathFXVolatilitySurfaceGenerate::SetFXOptionParam(*dataInstance,dCurveID,fCurveID,maturityDate,
//            deliveryDate,asOfFX,deltaType[i],atmType[i],cal);
//
//        if( isApproximationFlag == true && isWingFlag == true )
//        {
//            //smile param adjust
//            if( LAMath::abs(smParams[i].lowBF)<EPS_Vol1 )
//            {
//                smParams[i].lowBF = LAMath::sign(EPS_Vol1, smParams[i].lowBF);
//                warningMSG  += LAString( fxParams[i].T, 3 ) + " low BF too small,";
//            }
//            if( LAMath::abs(smParams[i].highBF)<EPS_Vol1 )
//            {
//                smParams[i].highBF = LAMath::sign(EPS_Vol1, smParams[i].highBF);
//                warningMSG  += LAString( fxParams[i].T, 3 ) + " high BF too small,";
//            }
//
//            if( smParams[i].lowBF>0.0 && smParams[i].highBF<0.0 && smParams[i].lowBF>=-smParams[i].highBF )
//            {
//                smParams[i].highBF = EPS_Vol1;
//                warningMSG  += LAString( fxParams[i].T, 3 ) + " BF signs not same,";
//            }
//            else if( smParams[i].lowBF>0.0 && smParams[i].highBF<0.0 && smParams[i].lowBF<=-smParams[i].highBF )
//            {
//                smParams[i].lowBF = -EPS_Vol1;
//                warningMSG  += LAString( fxParams[i].T, 3 ) + " BF signs not same,";
//            }
//            else if( smParams[i].lowBF<0.0 && smParams[i].highBF>0.0 && -smParams[i].lowBF>=smParams[i].highBF )
//            {
//                smParams[i].highBF = -EPS_Vol1;
//                warningMSG  += LAString( fxParams[i].T, 3 ) + " BF signs not same,";
//            }
//            else if( smParams[i].lowBF<0.0 && smParams[i].highBF>0.0 && -smParams[i].lowBF<=smParams[i].highBF )
//            {
//                smParams[i].lowBF = EPS_Vol1;
//                warningMSG  += LAString( fxParams[i].T, 3 ) + " BF signs not same,";
//            }                
//
//            //smile param adjust
//            if( LAMath::abs(smParams[i].lowRR)<EPS_Vol1 )
//            {
//                smParams[i].lowRR = LAMath::sign(EPS_Vol1, smParams[i].lowRR);
//                warningMSG  += LAString( fxParams[i].T, 3 ) + " low RR too small,";
//            }
//            if( LAMath::abs(smParams[i].highRR)<EPS_Vol1 )
//            {
//                smParams[i].highRR = LAMath::sign(EPS_Vol1, smParams[i].highRR);
//                warningMSG  += LAString( fxParams[i].T, 3 ) + " hihg RR too small,";
//            }
//
//            if( smParams[i].lowRR>0.0 && smParams[i].highRR<0.0 && smParams[i].lowRR>=-smParams[i].highRR )
//            {
//                smParams[i].highRR = EPS_Vol1;
//                warningMSG  += LAString( fxParams[i].T, 3 ) + " RR signs not same,";
//            }
//            else if( smParams[i].lowRR>0.0 && smParams[i].highRR<0.0 && smParams[i].lowRR<=-smParams[i].highRR )
//            {
//                smParams[i].lowRR = -EPS_Vol1;
//                warningMSG  += LAString( fxParams[i].T, 3 ) + " RR signs not same,";
//            }
//            else if( smParams[i].lowRR<0.0 && smParams[i].highRR>0.0 && -smParams[i].lowRR>=smParams[i].highRR )
//            {
//                smParams[i].highRR = -EPS_Vol1;
//                warningMSG  += LAString( fxParams[i].T, 3 ) + " RR signs not same,";
//            }
//            else if( smParams[i].lowRR<0.0 && smParams[i].highRR>0.0 && -smParams[i].lowRR<=smParams[i].highRR )
//            {
//                smParams[i].lowRR = EPS_Vol1;
//                warningMSG  += LAString( fxParams[i].T, 3 ) + " RR signs not same,";
//            }                
//        }
//	}
//    
//	if( isApproximationFlag )
//    {
//		for(i=0;i<dataNum;i++)
//		{
//			smDatas[i] = LAMathFXVolatilitySurfaceGenerate::BuildSmile( smParams[i], fxParams[i], false, 1., LAString("") );
//
//			for(j=1;j<6;j++)
//			{
//				smDatas_att[i*28+j] = smDatas[i].vols[j-1];
//                smDatas_att[i*28+j+7] = smDatas[i].strikes[j-1];
//				smDatas_att[i*28+j+14] = smDatas[i].logStrikes[j-1];
//				smDatas_att[i*28+j+21] = smDatas[i].deltaPuts[j-1];
//			}
//
//			LADataHolder* dh;
//			dh = &objHolder.getData("SMILEDATAS",ISDEFINED);
//			LADataDoubles& smDatas_att_ = dynamic_cast<LADataDoubles& >(dh->get());
//			smDatas_att_.set(smDatas_att);
//		}
//    }
//
//    for(i=0;i<dataNum;i++)
//    {
//        if( isApproximationFlag )
//        {
//            smDatas[i] = LAMathFXVolatilitySurfaceGenerate::BuildSmile( smParams[i], fxParams[i], isWingFlag, wfs[i], warningMSG );
//        }
//        else
//        {
//            smDatas[i] = LAMathFXVolatilitySurfaceGenerate::FindStrangleVol( fxParams[i], smParams[i], method, target, 
//                                isWingFlag, wfs[i] );
//        }
//
//        if( isWingFlag )
//        {
//            for(j=0;j<7;j++)
//            {
//                smDatas_att[i*28+j] = smDatas[i].vols[j];
//                smDatas_att[i*28+j+7] = smDatas[i].strikes[j];
//                smDatas_att[i*28+j+14] = smDatas[i].logStrikes[j];
//                smDatas_att[i*28+j+21] = smDatas[i].deltaPuts[j];
//            }
//        }
//        else
//        {
//            for(j=0;j<5;j++)
//			{
//				smDatas_att[i*28+j+1] = smDatas[i].vols[j];
//                smDatas_att[i*28+j+1+7] = smDatas[i].strikes[j];
//				smDatas_att[i*28+j+1+14] = smDatas[i].logStrikes[j];
//				smDatas_att[i*28+j+1+21] = smDatas[i].deltaPuts[j];
//			}
//        }
//
//        LADataHolder* dh;
//        dh = &objHolder.getData("SMILEDATAS",ISDEFINED);
//		LADataDoubles& smDatas_att_ = dynamic_cast<LADataDoubles& >(dh->get());
//		smDatas_att_.set(smDatas_att);
//		LAMathFXVolatilitySurfaceGenerate::SmileDataCheck( smDatas[i], isWingFlag );
//
//		fxDatas_att[i*6] = fxParams[i].T;
//		fxDatas_att[i*6+1] = fxParams[i].Pd;
//		fxDatas_att[i*6+2] = fxParams[i].Pf;
//		fxDatas_att[i*6+3] = fxParams[i].F;
//		fxDatas_att[i*6+4] = fxParams[i].spotFX;
//		fxDatas_att[i*6+5] = fxParams[i].Days;
//
//		dh = &objHolder.getData("FXOPTIONDATAS",ISDEFINED);
//		LADataDoubles& fxDatas_att_ = dynamic_cast<LADataDoubles& >(dh->get());
//		fxDatas_att_.set(fxDatas_att);
//    }
//	
//    LAString ans;
//    if( warningMSG == "warning:" || isApproximationFlag == false ){ ans = "VolSurface has been set"; }
//    else { ans = warningMSG; }
//
//    return ans;
//}
//
//void 
//LAMathFXVolSurfUtility::OutPutVolatilitySurface( LADataInstance* dataInstance, const LAString& volSurfaceID, DoubleVector& ret, 
//                                             size_t& maturitySize, size_t& strikeSize )
//{
//    LAObjectPool& objPool = dataInstance->getObjectPool();
//	const LAObject& e = objPool.getObject( volSurfaceID, ENCHKTYPE_ISDEFINED ).get();
//	DoubleVector smDatas = dynamic_cast<const LADataDoubles& >(e.getData("SMILEDATAS",ISDEFINED).get()).get();
//    LAStringVector maturityDate_str = dynamic_cast<const LADataStrings& >(e.getData("MATURITYDATE",ISDEFINED).get()).get();
//    maturitySize = maturityDate_str.size();
//    strikeSize = smDatas.size()/maturitySize;
//    ret.resize( (strikeSize+1)*maturitySize );
//    for(size_t i=0;i<maturitySize;i++)
//    {
//        ret[i*(strikeSize+1)]=static_cast<double >(LAMathDateUtilities::getExcelDate(LAMathDateUtilities::getLADate(maturityDate_str[i])));
//        for(size_t j=1;j<(strikeSize+1);j++)
//        {
//            ret[i*(strikeSize+1)+j] = smDatas[i*strikeSize+j-1];
//        }
//    }
//}
//void 
//LAMathFXVolSurfUtility::OutPutFXOptionData( LADataInstance* dataInstance, const LAString& volSurfaceID, DoubleVector& ret, size_t& dataNum )
//{
//    LAObjectPool& objPool = dataInstance->getObjectPool();
//	const LAObject& e = objPool.getObject( volSurfaceID, ENCHKTYPE_ISDEFINED ).get();
//	LAString dCurveID = dynamic_cast<const LADataString& >(e.getData("DomesticCurveID",ISDEFINED).get()).get();
//    LAString fCurveID = dynamic_cast<const LADataString& >(e.getData("ForeignCurveID",ISDEFINED).get()).get();
//    double asOfFX = dynamic_cast<const LADataDouble& >(e.getData("ASOFFX",ISDEFINED).get()).get();
//    LAStringVector maturityDate_str = dynamic_cast<const LADataStrings& >(e.getData("MATURITYDATE",ISDEFINED).get()).get();
//    LAStringVector deltaType = dynamic_cast<const LADataStrings& >(e.getData("DELTATYPE",ISDEFINED).get()).get();
//    LAStringVector atmType = dynamic_cast<const LADataStrings& >(e.getData("ATMTYPE",ISDEFINED).get()).get();
//    LAString spotLag = dynamic_cast<const LADataString& >(e.getData("SPOTLAG",ISDEFINED).get()).get();
//    LAPriceDataCalendar cal = dynamic_cast<const LAPriceDataCalendar& >(e.getData("CALENDAR",ISDEFINED).get());
//    LAPriceDataSlidingRule fol; fol.convertFromString(FOL);
//    
//    dataNum=deltaType.size();
//    ret.resize( dataNum*4 );
//    vector<FXOptionData > fxParams(dataNum);
//    LADate maturityDate,deliveryDate;
//
//    for(size_t i=0;i<dataNum;i++)
//    {
//        //Set FXOptionParam
//        maturityDate = LAMathDateUtilities::getLADate(maturityDate_str[i]);
//        deliveryDate = LAMathDateCalculations::getDate(maturityDate,spotLag,fol,&cal,true);
//        upper(deltaType[i]);
//        upper(atmType[i]);
//        fxParams[i] = LAMathFXVolatilitySurfaceGenerate::SetFXOptionParam(*dataInstance,dCurveID,fCurveID,maturityDate,
//            deliveryDate,asOfFX,deltaType[i],atmType[i],cal);
//        ret[i*4]=fxParams[i].T;
//        ret[i*4+1]=fxParams[i].Pd;
//        ret[i*4+2]=fxParams[i].Pf;
//        ret[i*4+3]=fxParams[i].F;
//    }
//}
//
//double
//LAMathFXVolSurfUtility::GetVolatility( LADataInstance* dataInstance, const LAString& volSurfaceID, double point, const LADate& expiryPoint, 
//        const LAString& variable_str, const LAString& maturityMethod_str)
//{
//    LAObjectPool& objPool = dataInstance->getObjectPool();
//	const LAObject& e = objPool.getObject( volSurfaceID, ENCHKTYPE_ISDEFINED ).get();
//    bool isWingFlag = dynamic_cast<const LADataBool& >(e.getData("ISWINGFLAG",ISDEFINED).get()).get();
//	const LADataDoubles& smDatas_att = dynamic_cast<const LADataDoubles& >(e.getData("SMILEDATAS",ISDEFINED).get());
//    size_t dataNum = smDatas_att.get().size() / 28;
//    vector<SmileData > smDatas(dataNum);
//    size_t i,j;
//    if( isWingFlag )
//    {
//        for(i=0;i<dataNum;i++)
//        {
//            for(j=0; j<7; j++)
//            {
//                smDatas[i].vols.resize(7);
//                smDatas[i].strikes.resize(7);
//                smDatas[i].logStrikes.resize(7);
//                smDatas[i].deltaPuts.resize(7);
//                smDatas[i].vols[j] = smDatas_att.get()[ j + i * 28 ];
//                smDatas[i].strikes[j] = smDatas_att.get()[ 7 + j + i * 28 ];
//                smDatas[i].logStrikes[j] = smDatas_att.get()[ 14 + j + i * 28 ];
//                smDatas[i].deltaPuts[j] = smDatas_att.get()[ 21 + j + i * 28 ];
//            }
//        }
//    }
//    else
//    {
//        for(i=0; i<dataNum; i++)
//        {
//            for(j=0; j<5; j++)
//            {
//                smDatas[i].vols.resize(5);
//                smDatas[i].strikes.resize(5);
//                smDatas[i].logStrikes.resize(5);
//                smDatas[i].deltaPuts.resize(5);
//                smDatas[i].vols[j] = smDatas_att.get()[ i * 28 + j + 1  ];
//                smDatas[i].strikes[j] = smDatas_att.get()[ i * 28 + j + 1 + 7 ];
//                smDatas[i].logStrikes[j] = smDatas_att.get()[ i * 28 + j + 1 + 14 ];
//                smDatas[i].deltaPuts[j] = smDatas_att.get()[ i * 28 + j + 1 + 21 ];
//            }
//        }
//    }
//    LAString dCurveID = dynamic_cast<const LADataString& >(e.getData("DomesticCurveID",ISDEFINED).get()).get();
//    LAString fCurveID = dynamic_cast<const LADataString& >(e.getData("ForeignCurveID",ISDEFINED).get()).get();
//    double asOfFX = dynamic_cast<const LADataDouble& >(e.getData("ASOFFX",ISDEFINED).get()).get();
//    LAStringVector maturityDate_str = dynamic_cast<const LADataStrings& >(e.getData("MATURITYDATE",ISDEFINED).get()).get();
//    LAStringVector deltaType = dynamic_cast<const LADataStrings& >(e.getData("DELTATYPE",ISDEFINED).get()).get();
//    LAStringVector atmType = dynamic_cast<const LADataStrings& >(e.getData("ATMTYPE",ISDEFINED).get()).get();
//    LAString method_str = dynamic_cast<const LADataString& >(e.getData("INTERPOLATIONMETHOD",ISDEFINED).get()).get();
//    LAString interpoVariable = dynamic_cast<const LADataString& >(e.getData("INTERPOLATIONVARIABLE",ISDEFINED).get()).get();
//    LAString spotLag = dynamic_cast<const LADataString& >(e.getData("SPOTLAG",ISDEFINED).get()).get();
//    LAPriceDataCalendar cal = dynamic_cast<const LAPriceDataCalendar& >(e.getData("CALENDAR",ISDEFINED).get());
//    LAPriceDataSlidingRule fol; fol.convertFromString(FOL);
//     //set interpolation method
//    InterpolationMethod method;
//    upper(method_str);
//    LAMathFXVolatilitySurfaceGenerate::SetInterpolationMethod( method_str, method );
//    //set interpolation target
//    InterpolationTarget target;
//    upper(interpoVariable);
//    LAMathFXVolatilitySurfaceGenerate::SetInterpolationTarget( interpoVariable, target );
//    //set interpolation maturity method
//    ATMInterpolationMethod maturityMethod;
//    LAMathFXVolatilitySurfaceGenerate::SetATMInterpolationMethod( maturityMethod_str, maturityMethod );
//    //set variable
//    InterpolationVariable variable;
//    LAMathFXVolatilitySurfaceGenerate::SetInterpolationVariable( variable_str, variable );
//    vector<FXOptionData > fxParams(dataNum);
//    LADate maturityDate,deliveryDate;
//    for(i=0; i<dataNum;i ++)
//    {
//        //Set FXOptionParam
//        maturityDate = LAMathDateUtilities::getLADate(maturityDate_str[i]);
//        deliveryDate = LAMathDateCalculations::getDate(maturityDate,spotLag,fol,&cal,true);
//        upper(deltaType[i]);
//        upper(atmType[i]);
//        fxParams[i] = LAMathFXVolatilitySurfaceGenerate::SetFXOptionParam(*dataInstance,dCurveID,fCurveID,maturityDate,
//            deliveryDate,asOfFX,deltaType[i],atmType[i],cal);
//    }
//    
//    LADate deliveryPoint = LAMathDateCalculations::getDate(expiryPoint,spotLag,fol,&cal,true);
//    FXOptionData x = LAMathFXVolatilitySurfaceGenerate::SetFXOptionParam(*dataInstance,dCurveID,fCurveID,expiryPoint,
//                deliveryPoint,asOfFX,"FWDPRE","FORWARDATM",cal);
//
//    double ret;
//    if( variable_str == "STRIKE" )
//    {
//        if( maturityMethod == TermWeighted || maturityMethod == TermNoWeighted )
//        {
//            ret = LAMathFXVolatilitySurfaceGenerate::GetSurfaceInterpolation( LAMath::log(point/x.F), x.T, method, target, variable, 
//                        maturityMethod, fxParams, smDatas, isWingFlag );
//        }
//        else
//        {
//            ret = LAMathFXVolatilitySurfaceGenerate::GetSurfaceInterpolation( LAMath::log(point/x.F), x.Days, method, target, variable, 
//                        maturityMethod, fxParams, smDatas, isWingFlag );
//        }
//    }
//    else
//    {
//        if( maturityMethod == TermWeighted || maturityMethod == TermNoWeighted )
//        {
//            ret = LAMathFXVolatilitySurfaceGenerate::GetSurfaceInterpolation( point, x.T, method, target, variable, 
//                        maturityMethod, fxParams, smDatas, isWingFlag );
//        }
//        else
//        {
//            ret = LAMathFXVolatilitySurfaceGenerate::GetSurfaceInterpolation( point, x.Days, method, target, variable, 
//                        maturityMethod, fxParams, smDatas, isWingFlag );
//        }
//    }
//    return ret;
//}
//
//double
//LAMathFXVolSurfUtility::GetVolatility2( LADataInstance* dataInstance, const LAString& volSurfaceID, double point, double expiryPoint, 
//        double forwardFX, const LAString& variable_str, const LAString& maturityMethod_str)
//{
//	const LAObject& e = dataInstance->getObjectPool().getObject( volSurfaceID, ENCHKTYPE_ISDEFINED ).get();
//
//	//// get smile data
//	DoubleMatrix deltaPutMat = dynamic_cast<const LADataDoubleMatrix& >(e.getData(IR_CALIBRATION_DATA_SMILEDATA_DELTAPUT, ISNOTNULL).get()).get();
//	DoubleMatrix logStrikeMat = dynamic_cast<const LADataDoubleMatrix& >(e.getData(IR_CALIBRATION_DATA_SMILEDATA_LOGSTRIKE, ISNOTNULL).get()).get();
//	DoubleMatrix strikeMat = dynamic_cast<const LADataDoubleMatrix& >(e.getData(IR_CALIBRATION_DATA_SMILEDATA_STRIKE, ISNOTNULL).get()).get();
//	DoubleMatrix volMat = dynamic_cast<const LADataDoubleMatrix& >(e.getData(IR_CALIBRATION_DATA_SMILEDATA_VOL, ISNOTNULL).get()).get();
//	// size check
//	if(	deltaPutMat.size() == 0 || 
//		deltaPutMat.size() != logStrikeMat.size() || 
//		deltaPutMat.size() != strikeMat.size() || 
//		deltaPutMat.size() != volMat.size() )
//	{
//		throw LACoreInvalidData("Smile data sizes are wrong!",__FILE__,__LINE__);
//	}
//	if(	deltaPutMat[0].size() != logStrikeMat[0].size() || 
//		deltaPutMat[0].size() != strikeMat[0].size() || 
//		deltaPutMat[0].size() != volMat[0].size() )
//	{
//		throw LACoreInvalidData("Smile data sizes are wrong!",__FILE__,__LINE__);
//	}
//	// set data into struct
//    vector<SmileData > smDatas(deltaPutMat.size());
//    for (size_t i = 0; i < deltaPutMat.size(); ++i)
//	{
//		smDatas[i].deltaPuts = deltaPutMat[i];
//		smDatas[i].logStrikes = logStrikeMat[i];
//		smDatas[i].strikes = strikeMat[i];
//		smDatas[i].vols = volMat[i];
//	}
//
//	//// get fx option data
//	DoubleVector T_fxOptData = dynamic_cast<const LADataDoubles& >(e.getData(IR_CALIBRATION_DATA_FXOPTDATA_T, ISNOTNULL).get()).get();
//	DoubleVector Pd_fxOptData = dynamic_cast<const LADataDoubles& >(e.getData(IR_CALIBRATION_DATA_FXOPTDATA_PD, ISNOTNULL).get()).get();
//	DoubleVector Pf_fxOptData = dynamic_cast<const LADataDoubles& >(e.getData(IR_CALIBRATION_DATA_FXOPTDATA_PF, ISNOTNULL).get()).get();
//	DoubleVector F_fxOptData = dynamic_cast<const LADataDoubles& >(e.getData(IR_CALIBRATION_DATA_FXOPTDATA_F, ISNOTNULL).get()).get();
//	DoubleVector spotFX_fxOptData = dynamic_cast<const LADataDoubles& >(e.getData(IR_CALIBRATION_DATA_FXOPTDATA_SPOTFX, ISNOTNULL).get()).get();
//	DoubleVector Days_fxOptData = dynamic_cast<const LADataDoubles& >(e.getData(IR_CALIBRATION_DATA_FXOPTDATA_DAYS, ISNOTNULL).get()).get();
//	IntVector deltaType_fxOptData = dynamic_cast<const LADataInts& >(e.getData(IR_CALIBRATION_DATA_FXOPTDATA_DELTATYPE, ISNOTNULL).get()).get();
//	IntVector atmType_fxOptData = dynamic_cast<const LADataInts& >(e.getData(IR_CALIBRATION_DATA_FXOPTDATA_ATMTYPE, ISNOTNULL).get()).get();
//	// size check
//	if(	deltaPutMat.size() != T_fxOptData.size() || 
//		deltaPutMat.size() != Pd_fxOptData.size() || 
//		deltaPutMat.size() != Pf_fxOptData.size() || 
//		deltaPutMat.size() != F_fxOptData.size() ||
//		deltaPutMat.size() != spotFX_fxOptData.size() ||
//		deltaPutMat.size() != Days_fxOptData.size() ||
//		deltaPutMat.size() != deltaType_fxOptData.size() ||
//		deltaPutMat.size() != atmType_fxOptData.size() )
//	{
//		throw LACoreInvalidData("FX option data sizes are wrong!",__FILE__,__LINE__);
//	}
//	// set data into struct
//    vector<FXOptionData > fxOptData(deltaPutMat.size());
//    for (size_t i = 0; i < T_fxOptData.size(); ++i)
//	{
//		fxOptData[i].T = T_fxOptData[i];
//		fxOptData[i].Pd = Pd_fxOptData[i];
//		fxOptData[i].Pf = Pf_fxOptData[i];
//		fxOptData[i].F = F_fxOptData[i];
//		fxOptData[i].spotFX = spotFX_fxOptData[i];
//		fxOptData[i].Days = Days_fxOptData[i];
//		fxOptData[i].deltaType = static_cast<FXDeltaType > (deltaType_fxOptData[i]);
//		fxOptData[i].atmType = static_cast<FXATMStrikeType > (atmType_fxOptData[i]);
//	}
//
//	// set properties
//    bool isWingFlag = dynamic_cast<const LADataBool& >(e.getData(IR_CALIBRATION_DATA_ISWINGFLAG, ISNOTNULL).get()).get();
//    InterpolationMethod method = static_cast<InterpolationMethod > (dynamic_cast<const LADataInt& >(e.getData(IR_CALIBRATION_DATA_INTERPOLATIONMETHOD, ISNOTNULL).get()).get());
//    InterpolationTarget target = static_cast<InterpolationTarget > (dynamic_cast<const LADataInt& >(e.getData(IR_CALIBRATION_DATA_INTERPOLATIONTARGET, ISNOTNULL).get()).get());
//	// set interpolation maturity method
//    ATMInterpolationMethod maturityMethod;
//    LAMathFXVolatilitySurfaceGenerate::SetATMInterpolationMethod( maturityMethod_str, maturityMethod );
//    // set variable
//    InterpolationVariable variable;
//    LAMathFXVolatilitySurfaceGenerate::SetInterpolationVariable( variable_str, variable );
//
//    double ret;
//    if( variable_str == "STRIKE" )
//    {
//        if( maturityMethod == TermWeighted || maturityMethod == TermNoWeighted )
//        {
//            ret = LAMathFXVolatilitySurfaceGenerate::GetSurfaceInterpolation( LAMath::log(point / forwardFX), expiryPoint, method, target, variable, 
//                        maturityMethod, fxOptData, smDatas, isWingFlag );
//        }
//        else
//        {
//            ret = LAMathFXVolatilitySurfaceGenerate::GetSurfaceInterpolation( LAMath::log(point / forwardFX), expiryPoint, method, target, variable, 
//                        maturityMethod, fxOptData, smDatas, isWingFlag );
//        }
//    }
//    else
//    {
//        if( maturityMethod == TermWeighted || maturityMethod == TermNoWeighted )
//        {
//            ret = LAMathFXVolatilitySurfaceGenerate::GetSurfaceInterpolation( point, expiryPoint, method, target, variable, 
//                        maturityMethod, fxOptData, smDatas, isWingFlag );
//        }
//        else
//        {
//            ret = LAMathFXVolatilitySurfaceGenerate::GetSurfaceInterpolation( point, expiryPoint, method, target, variable, 
//                        maturityMethod, fxOptData, smDatas, isWingFlag );
//        }
//    }
//
//    return ret;
//}
//
//double
//LAMathFXVolSurfUtility::GetATMVolatility( LADataInstance* dataInstance, const LAString& volSurfaceID, 
//                                      const LADate& expiryPoint, const LAString& maturityMethod_str )
//{
//    LAString daycount(AC_365I);
//
//    LAObjectPool& objPool = dataInstance->getObjectPool();
//	const LAObject& e = objPool.getObject( volSurfaceID, ENCHKTYPE_ISDEFINED ).get();
//    bool isWingFlag = dynamic_cast<const LADataBool& >(e.getData("ISWINGFLAG",ISDEFINED).get()).get();
//	const LADataDoubles& smDatas_att = dynamic_cast<const LADataDoubles& >(e.getData("SMILEDATAS",ISDEFINED).get());
//    size_t dataNum = smDatas_att.get().size() / 28;
//    vector<SmileData > smDatas(dataNum);
//    if( isWingFlag )
//        for( size_t i=0;i<dataNum;i++)
//        {
//            for(size_t j=0;j<7;j++)
//            {
//                smDatas[i].vols.resize(7);
//                smDatas[i].strikes.resize(7);
//                smDatas[i].logStrikes.resize(7);
//                smDatas[i].deltaPuts.resize(7);
//                smDatas[i].vols[j] = smDatas_att.get()[ j + i * 28 ];
//                smDatas[i].strikes[j] = smDatas_att.get()[ 7 + j + i * 28 ];
//                smDatas[i].logStrikes[j] = smDatas_att.get()[ 14 + j + i * 28 ];
//                smDatas[i].deltaPuts[j] = smDatas_att.get()[ 21 + j + i * 28 ];
//            }
//    }
//    else
//    {
//        for(size_t i=0; i<dataNum; i++)
//        {
//            for(size_t j=0; j<5; j++)
//            {
//                smDatas[i].vols.resize(5);
//                smDatas[i].strikes.resize(5);
//                smDatas[i].logStrikes.resize(5);
//                smDatas[i].deltaPuts.resize(5);
//                smDatas[i].vols[j] = smDatas_att.get()[ i * 28 + j + 1  ];
//                smDatas[i].strikes[j] = smDatas_att.get()[ i * 28 + j + 1 + 7 ];
//                smDatas[i].logStrikes[j] = smDatas_att.get()[ i * 28 + j + 1 + 14 ];
//                smDatas[i].deltaPuts[j] = smDatas_att.get()[ i * 28 + j + 1 + 21 ];
//            }
//        }
//    }
//
//    LAString dCurveID = dynamic_cast<const LADataString& >(e.getData("DomesticCurveID",ISDEFINED).get()).get();
//    LAString fCurveID = dynamic_cast<const LADataString& >(e.getData("ForeignCurveID",ISDEFINED).get()).get();
//    double asOfFX = dynamic_cast<const LADataDouble& >(e.getData("ASOFFX",ISDEFINED).get()).get();
//    LAStringVector maturityDate_str = dynamic_cast<const LADataStrings& >(e.getData("MATURITYDATE",ISDEFINED).get()).get();
//    LAStringVector deltaType = dynamic_cast<const LADataStrings& >(e.getData("DELTATYPE",ISDEFINED).get()).get();
//    LAStringVector atmType = dynamic_cast<const LADataStrings& >(e.getData("ATMTYPE",ISDEFINED).get()).get();
//    LAString method_str = dynamic_cast<const LADataString& >(e.getData("INTERPOLATIONMETHOD",ISDEFINED).get()).get();
//    LAString interpoVariable = dynamic_cast<const LADataString& >(e.getData("INTERPOLATIONVARIABLE",ISDEFINED).get()).get();
//    LAString spotLag = dynamic_cast<const LADataString& >(e.getData("SPOTLAG",ISDEFINED).get()).get();
//    LAPriceDataCalendar cal = dynamic_cast<const LAPriceDataCalendar& >(e.getData("CALENDAR",ISDEFINED).get());
//    LAPriceDataSlidingRule fol; fol.convertFromString(FOL);
//
//    ATMInterpolationMethod maturityMethod;
//    LAMathFXVolatilitySurfaceGenerate::SetATMInterpolationMethod( maturityMethod_str, maturityMethod );
//
//    vector<FXOptionData > fxParams(dataNum);
//    LADate maturityDate,deliveryDate;
//    for(size_t i=0;i<dataNum;i++)
//    {
//        //Set FXOptionParam
//        maturityDate = LAMathDateUtilities::getLADate(maturityDate_str[i]);
//        deliveryDate = LAMathDateCalculations::getDate(maturityDate,spotLag,fol,&cal,true);
//        upper(deltaType[i]);
//        upper(atmType[i]);
//        fxParams[i] = LAMathFXVolatilitySurfaceGenerate::SetFXOptionParam(*dataInstance,dCurveID,fCurveID,maturityDate,
//            deliveryDate,asOfFX,deltaType[i],atmType[i],cal);
//    };
//
//    const LADate& asOfDate = dynamic_cast<const LADataDate& >(dataInstance->getObjectPool().getObject(dCurveID,ENCHKTYPE_ISDEFINED).
//                            get().getData(CALIBRATION_DATA_ASOFDATE,ISNOTNULL).get()).get();
//    double termPoint = LAMathDateUtilities::getTerm(asOfDate,expiryPoint,daycount,true);
//    return LAMathFXVolatilitySurfaceGenerate::GetATMVolatility( termPoint, maturityMethod, fxParams, smDatas );
//}
//
//double 
//LAMathFXVolSurfUtility::GetFXPremiumFromSurface( LADataInstance* dataInstance, const LAString& volSurfaceID, double point, const LADate& expiryPoint, 
//        const LAString& variable_str, const LAString& maturityMethod_str, const LAString& callput, const LAString& deltaType_str )
//{
//    LAObjectPool& objPool = dataInstance->getObjectPool();
//	const LAObject& e = objPool.getObject( volSurfaceID, ENCHKTYPE_ISDEFINED ).get();
//    bool isWingFlag = dynamic_cast<const LADataBool& >(e.getData("ISWINGFLAG",ISDEFINED).get()).get();
//	const LADataDoubles& smDatas_att = dynamic_cast<const LADataDoubles& >(e.getData("SMILEDATAS",ISDEFINED).get());
//    size_t dataNum = smDatas_att.get().size() / 28;
//    vector<SmileData > smDatas(dataNum);
//    size_t i,j;
//    if( isWingFlag )
//    {
//        for(i=0;i<dataNum;i++)
//        {
//            for(j=0; j<7; j++)
//            {
//                smDatas[i].vols.resize(7);
//                smDatas[i].strikes.resize(7);
//                smDatas[i].logStrikes.resize(7);
//                smDatas[i].deltaPuts.resize(7);
//                smDatas[i].vols[j] = smDatas_att.get()[ j + i * 28 ];
//                smDatas[i].strikes[j] = smDatas_att.get()[ 7 + j + i * 28 ];
//                smDatas[i].logStrikes[j] = smDatas_att.get()[ 14 + j + i * 28 ];
//                smDatas[i].deltaPuts[j] = smDatas_att.get()[ 21 + j + i * 28 ];
//            }
//        }
//    }
//    else
//    {
//        for(i=0; i<dataNum; i++)
//        {
//            for(j=0; j<5; j++)
//            {
//                smDatas[i].vols.resize(5);
//                smDatas[i].strikes.resize(5);
//                smDatas[i].logStrikes.resize(5);
//                smDatas[i].deltaPuts.resize(5);
//                smDatas[i].vols[j] = smDatas_att.get()[ i * 28 + j + 1  ];
//                smDatas[i].strikes[j] = smDatas_att.get()[ i * 28 + j + 1 + 7 ];
//                smDatas[i].logStrikes[j] = smDatas_att.get()[ i * 28 + j + 1 + 14 ];
//                smDatas[i].deltaPuts[j] = smDatas_att.get()[ i * 28 + j + 1 + 21 ];
//            }
//        }
//    }
//    LAString dCurveID = dynamic_cast<const LADataString& >(e.getData("DomesticCurveID",ISDEFINED).get()).get();
//    LAString fCurveID = dynamic_cast<const LADataString& >(e.getData("ForeignCurveID",ISDEFINED).get()).get();
//    double asOfFX = dynamic_cast<const LADataDouble& >(e.getData("ASOFFX",ISDEFINED).get()).get();
//    LAStringVector maturityDate_str = dynamic_cast<const LADataStrings& >(e.getData("MATURITYDATE",ISDEFINED).get()).get();
//    LAStringVector deltaType = dynamic_cast<const LADataStrings& >(e.getData("DELTATYPE",ISDEFINED).get()).get();
//    LAStringVector atmType = dynamic_cast<const LADataStrings& >(e.getData("ATMTYPE",ISDEFINED).get()).get();
//    LAString method_str = dynamic_cast<const LADataString& >(e.getData("INTERPOLATIONMETHOD",ISDEFINED).get()).get();
//    LAString interpoVariable = dynamic_cast<const LADataString& >(e.getData("INTERPOLATIONVARIABLE",ISDEFINED).get()).get();
//    LAString spotLag = dynamic_cast<const LADataString& >(e.getData("SPOTLAG",ISDEFINED).get()).get();
//    LAPriceDataCalendar cal = dynamic_cast<const LAPriceDataCalendar& >(e.getData("CALENDAR",ISDEFINED).get());
//    LAPriceDataSlidingRule fol; fol.convertFromString(FOL);
//     //set interpolation method
//    InterpolationMethod method;
//    upper(method_str);
//    LAMathFXVolatilitySurfaceGenerate::SetInterpolationMethod( method_str, method );
//    //set interpolation target
//    InterpolationTarget target;
//    upper(interpoVariable);
//    LAMathFXVolatilitySurfaceGenerate::SetInterpolationTarget( interpoVariable, target );
//    //set interpolation maturity method
//    ATMInterpolationMethod maturityMethod;
//    LAMathFXVolatilitySurfaceGenerate::SetATMInterpolationMethod( maturityMethod_str, maturityMethod );
//    //set variable
//    InterpolationVariable variable;
//    LAMathFXVolatilitySurfaceGenerate::SetInterpolationVariable( variable_str, variable );
//    vector<FXOptionData > fxParams(dataNum);
//    LADate maturityDate,deliveryDate;
//    for(i=0; i<dataNum;i ++)
//    {
//        //Set FXOptionParam
//        maturityDate = LAMathDateUtilities::getLADate(maturityDate_str[i]);
//        deliveryDate = LAMathDateCalculations::getDate(maturityDate,spotLag,fol,&cal,true);
//        upper(deltaType[i]);
//        upper(atmType[i]);
//        fxParams[i] = LAMathFXVolatilitySurfaceGenerate::SetFXOptionParam(*dataInstance,dCurveID,fCurveID,maturityDate,
//            deliveryDate,asOfFX,deltaType[i],atmType[i],cal);
//    }
//    
//    LADate deliveryPoint = LAMathDateCalculations::getDate(expiryPoint,spotLag,fol,&cal,true);
//    FXOptionData x = LAMathFXVolatilitySurfaceGenerate::SetFXOptionParam(*dataInstance,dCurveID,fCurveID,expiryPoint,
//                deliveryPoint,asOfFX,"FWDPRE","FORWARDATM",cal);
//
//    double V;
//    if( variable_str == "STRIKE" )
//    {
//        if( maturityMethod == TermWeighted || maturityMethod == TermNoWeighted )
//        {
//            V = LAMathFXVolatilitySurfaceGenerate::GetSurfaceInterpolation( LAMath::log(point/x.F), x.T, method, target, variable, 
//                        maturityMethod, fxParams, smDatas, isWingFlag );
//        }
//        else
//        {
//            V = LAMathFXVolatilitySurfaceGenerate::GetSurfaceInterpolation( LAMath::log(point/x.F), x.Days, method, target, variable, 
//                        maturityMethod, fxParams, smDatas, isWingFlag );
//        }
//    }
//    else
//    {
//        if( maturityMethod == TermWeighted || maturityMethod == TermNoWeighted )
//        {
//            V = LAMathFXVolatilitySurfaceGenerate::GetSurfaceInterpolation( point, x.T, method, target, variable, 
//                        maturityMethod, fxParams, smDatas, isWingFlag );
//        }
//        else
//        {
//            V = LAMathFXVolatilitySurfaceGenerate::GetSurfaceInterpolation( point, x.Days, method, target, variable, 
//                        maturityMethod, fxParams, smDatas, isWingFlag );
//        }
//    }
//
//    if( variable_str=="DELTAPUT"||variable_str == "DELTACALL" )
//    {
//        if( deltaType_str == "FWDNONPRE" )  { x.deltaType = FWD_NONPRE;}
//        else if( deltaType_str == "FWDPRE" )  { x.deltaType = FWD_PRE;}
//        else if( deltaType_str == "SPOTNONPRE" )  { x.deltaType = SPOT_NONPRE;}
//        else if( deltaType_str == "SPOTPRE" )  { x.deltaType = SPOT_PRE;}
//        else 
//        {
//            LAString msg("DeltaType is not supported!");
//	        throw LACoreInvalidData(msg.getCString(),__FILE__,__LINE__);
//        };
//    }
//
//    double K;
//    if( variable_str == "LOGSTRIKE" ) { K = LAMath::exp( point ) * x.F ; }
//    else if( variable_str == "STRIKE" ) { K = point; }
//    else if( variable_str == "DELTAPUT" ) { K = LAMath::exp(  LAMathFXVolatilitySurfaceGenerate::FindLogStrikeFromDelta(point,V,-1,x) ) * x.F; }
//    else if( variable_str == "DELTACALL" ) { K = LAMath::exp(  LAMathFXVolatilitySurfaceGenerate::FindLogStrikeFromDelta(point,V,1,x) ) * x.F; }
//    
//    int sgn;
//    if( callput == "CALL" ) { sgn = 1; }
//    else if( callput == "PUT" ) { sgn = -1; }
//    else
//    {
//        LAString msg = "Option type is not supported";
//        throw LACoreInvalidData(msg.getCString(),__FILE__,__LINE__);
//    }
//
//    return LAMathFXVolatilitySurfaceGenerate::GetFXOptionPremium( K, V, sgn, x );
//}
//
//void 
//LAMathFXVolSurfUtility::GetWingFactorFromHeston( LADataInstance* dataInstance, LAStringMatrix& volSurfaceInfo, LAStringMatrix& fxOptionData, 
//                                             LAStringMatrix& smParams_str,const HestonParams_TDP& hestonParam, DoubleVector& wfs )
//{
//    //VolSurfaceInfo
//    LAString dCurveID = searchbyrow(volSurfaceInfo,"DomesticCurveID",1,true);
//    LAString fCurveID = searchbyrow(volSurfaceInfo,"ForeignCurveID",1,true);
//    uppermat(volSurfaceInfo);
//    LAString volSurfaceID = searchbyrow(volSurfaceInfo,"VOLSURFACEID",1,true);
//    LAString spotLag = searchbyrow(volSurfaceInfo,"SPOTLAG",1,true);
//    LAString calStr = searchbyrow(volSurfaceInfo,"SPOTCALENDAR",1,true);
//    LAPriceDataCalendar cal;
//    cal.convertFromString(calStr); 
//    double spotFX = searchbyrow(volSurfaceInfo,"SPOTFX",1,true).getDoubleValue();
//    LAPriceDataSlidingRule fol; fol.convertFromString(FOL);
//    //DOMESTICCURVEID
//	LAMathYieldCurve dcurve(dataInstance);
//	dcurve.getYieldData().convertFromString(dCurveID);
//    dcurve.setInterpolation("fn_splineinterpolation");
//    //FOREIGNCURVEID
//	LAMathYieldCurve fcurve(dataInstance);
//	fcurve.getYieldData().convertFromString(fCurveID);	
//	fcurve.setInterpolation("fn_splineinterpolation");
//    const LADate asOfDate = dynamic_cast<const LADataDate& >(dataInstance->getObjectPool().getObject(dCurveID,ENCHKTYPE_ISDEFINED).
//                            get().getData(CALIBRATION_DATA_ASOFDATE,ISNOTNULL).get()).get();
//    LAString daycount(AC_365I);
//    double spotTerm = LAMathDateUtilities::getTerm(asOfDate, LAMathDateCalculations::getDate(asOfDate,spotLag,fol,&cal,true), daycount, true);
//    double asOfFX = spotFX * dcurve.getBasisDF( spotTerm ) / fcurve.getBasisDF( spotTerm );
//    //FXOptionData
//    LAStringVector deltaType	= searchvecbycol(fxOptionData,"DELTATYPE",true);
//    LAStringVector atmType	= searchvecbycol(fxOptionData,"ATMTYPE",true);
//    LAStringVector maturityDate_str	= searchvecbycol(fxOptionData,"MATURITYDATE",true);
//    FXOptionData fxParams;
//    SmileParam smParams;
//    SmileData smDatas;
//    DoubleMatrix strikes(1,DoubleArray(5));
//    LADate maturityDate,deliveryDate;
//    size_t i,j;
//    size_t dataNum = wfs.size();
//    for(i=0;i<dataNum;i++)
//    {
//        //Set FXOptionParam
//        maturityDate = LAMathDateUtilities::getLADate(maturityDate_str[i]);
//        deliveryDate = LAMathDateCalculations::getDate(maturityDate,spotLag,fol,&cal,true);
//        upper(deltaType[i]);
//        upper(atmType[i]);
//        fxParams = LAMathFXVolatilitySurfaceGenerate::SetFXOptionParam(*dataInstance,dCurveID,fCurveID,maturityDate,
//            deliveryDate,asOfFX,deltaType[i],atmType[i],cal);
//        //Set SmileParam
//        smParams.atmVol = searchbycol(smParams_str,"ATM",i+1,true).getDoubleValue()/100;
//        smParams.lowRR = searchbycol(smParams_str,"10RR",i+1,true).getDoubleValue()/100;
//        smParams.lowBF = searchbycol(smParams_str,"10FLY",i+1,true).getDoubleValue()/100;
//        smParams.highRR = searchbycol(smParams_str,"25RR",i+1,true).getDoubleValue()/100;
//        smParams.highBF = searchbycol(smParams_str,"25FLY",i+1,true).getDoubleValue()/100;
//
//        //BuildSmile
//        smDatas = LAMathFXVolatilitySurfaceGenerate::BuildSmile( smParams, fxParams, false, 1., LAString("") );
//        for(j=0;j<5;j++)
//        {
//            strikes[0][j] = LAMath::exp( smDatas.logStrikes[j+1] ) * fxParams.F;
//        }
//
//        wfs[i] = LAMathDisplacedHestonTDP::GetWingFactorFromHeston(fxParams,smParams,smDatas.deltaPuts[3],
//                        hestonParam,smDatas.logStrikes[0],smDatas.logStrikes[1]);
//    }
//}
//
//void 
//LAMathFXVolSurfUtility::GetMarketDataFromSmileData( LADataInstance* dataInstance, const LAString& volSurfaceID, 
//                                                DoubleVector& mkDatas, size_t& dataNum )
//{
//    LAObjectPool& objPool = dataInstance->getObjectPool();
//    const LAObject& e = objPool.getObject( volSurfaceID, ENCHKTYPE_ISDEFINED ).get();
//    bool isWingFlag = dynamic_cast<const LADataBool& >(e.getData("ISWINGFLAG",ISDEFINED).get()).get();
//    const LADataDoubles& smDatas_att = dynamic_cast<const LADataDoubles& >(e.getData("SMILEDATAS",ISDEFINED).get());
//    dataNum = smDatas_att.get().size() / 28;
//    vector<SmileData > smDatas(dataNum);
//    if( isWingFlag )
//    {
//        for(size_t i=0;i<dataNum;i++)
//        {
//            for(size_t j=0; j<7; j++)
//            {
//                smDatas[i].vols.resize(7);
//                smDatas[i].strikes.resize(7);
//                smDatas[i].logStrikes.resize(7);
//                smDatas[i].deltaPuts.resize(7);
//                smDatas[i].vols[j] = smDatas_att.get()[ j + i * 28 ];
//                smDatas[i].strikes[j] = smDatas_att.get()[ 7 + j + i * 28 ];
//                smDatas[i].logStrikes[j] = smDatas_att.get()[ 14 + j + i * 28 ];
//                smDatas[i].deltaPuts[j] = smDatas_att.get()[ 21 + j + i * 28 ];
//            }
//        }
//    }
//    else
//    {
//        for(size_t i=0; i<dataNum; i++)
//        {
//            for(size_t j=0; j<5; j++)
//            {
//                smDatas[i].vols.resize(5);
//                smDatas[i].strikes.resize(5);
//                smDatas[i].logStrikes.resize(5);
//                smDatas[i].deltaPuts.resize(5);
//                smDatas[i].vols[j] = smDatas_att.get()[ i * 28 + j + 1  ];
//                smDatas[i].strikes[j] = smDatas_att.get()[ i * 28 + j + 1 + 7 ];
//                smDatas[i].logStrikes[j] = smDatas_att.get()[ i * 28 + j + 1 + 14 ];
//                smDatas[i].deltaPuts[j] = smDatas_att.get()[ i * 28 + j + 1 + 21 ];
//            }
//        }
//    }
//    LAString dCurveID = dynamic_cast<const LADataString& >(e.getData("DomesticCurveID",ISDEFINED).get()).get();
//    LAString fCurveID = dynamic_cast<const LADataString& >(e.getData("ForeignCurveID",ISDEFINED).get()).get();
//    double asOfFX = dynamic_cast<const LADataDouble& >(e.getData("ASOFFX",ISDEFINED).get()).get();
//    LAStringVector maturityDate_str = dynamic_cast<const LADataStrings& >(e.getData("MATURITYDATE",ISDEFINED).get()).get();
//    LAStringVector deltaType = dynamic_cast<const LADataStrings& >(e.getData("DELTATYPE",ISDEFINED).get()).get();
//    LAStringVector atmType = dynamic_cast<const LADataStrings& >(e.getData("ATMTYPE",ISDEFINED).get()).get();
//    LAString method_str = dynamic_cast<const LADataString& >(e.getData("INTERPOLATIONMETHOD",ISDEFINED).get()).get();
//    LAString interpoVariable = dynamic_cast<const LADataString& >(e.getData("INTERPOLATIONVARIABLE",ISDEFINED).get()).get();
//    LAString spotLag = dynamic_cast<const LADataString& >(e.getData("SPOTLAG",ISDEFINED).get()).get();
//    LAPriceDataCalendar cal = dynamic_cast<const LAPriceDataCalendar& >(e.getData("CALENDAR",ISDEFINED).get());
//    LAPriceDataSlidingRule fol; fol.convertFromString(FOL);
//
//    InterpolationMethod method;
//    upper(method_str);
//    LAMathFXVolatilitySurfaceGenerate::SetInterpolationMethod( method_str, method );
//    InterpolationTarget target;
//    upper(interpoVariable);
//    LAMathFXVolatilitySurfaceGenerate::SetInterpolationTarget( interpoVariable, target );
//
//    vector<FXOptionData > fxParams(dataNum);
//    mkDatas.resize( dataNum*5 );
//    LADate maturityDate,deliveryDate;
//    for(size_t i=0;i<dataNum;i++)
//    {
//        //Set FXOptionParam
//        maturityDate = LAMathDateUtilities::getLADate(maturityDate_str[i]);
//        deliveryDate = LAMathDateCalculations::getDate(maturityDate,spotLag,fol,&cal,true);
//        upper(deltaType[i]);
//        upper(atmType[i]);
//        fxParams[i] = LAMathFXVolatilitySurfaceGenerate::SetFXOptionParam(*dataInstance,dCurveID,fCurveID,maturityDate,
//            deliveryDate,asOfFX,deltaType[i],atmType[i],cal);
//        if( isWingFlag )
//        {
//            //handle error
//            if( _isnan( smDatas[i].vols[0] ) ) smDatas[i].vols[0] = smDatas[i].vols[1];
//            if( _isnan( smDatas[i].vols[6] ) ) smDatas[i].vols[6] = smDatas[i].vols[5];
//            
//		    mkDatas[i*5] = smDatas[i].vols[3]*100;
//		    mkDatas[i*5+1] = (smDatas[i].vols[4] - smDatas[i].vols[2])*100;
//		    mkDatas[i*5+2] = (LAMathFXVolatilitySurfaceGenerate::FindHighButterfly(method,target,fxParams[i],smDatas[i],isWingFlag))*100;
//		    mkDatas[i*5+3] = (smDatas[i].vols[5] - smDatas[i].vols[1])*100;
//		    mkDatas[i*5+4] = (LAMathFXVolatilitySurfaceGenerate::FindLowButterfly(method,target,fxParams[i],smDatas[i],isWingFlag))*100;
//        }
//        else
//        {
//            mkDatas[i*5] = smDatas[i].vols[2]*100;
//		    mkDatas[i*5+1] = (smDatas[i].vols[3] - smDatas[i].vols[1])*100;
//		    mkDatas[i*5+2] = (LAMathFXVolatilitySurfaceGenerate::FindHighButterfly(method,target,fxParams[i],smDatas[i],isWingFlag))*100;
//		    mkDatas[i*5+3] = (smDatas[i].vols[4] - smDatas[i].vols[0])*100;
//		    mkDatas[i*5+4] = (LAMathFXVolatilitySurfaceGenerate::FindLowButterfly(method,target,fxParams[i],smDatas[i],isWingFlag))*100;
//        }
//    }
//}
//
////#endif
