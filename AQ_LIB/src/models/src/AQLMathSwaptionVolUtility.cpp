#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include <vector>

#include <AQLDataInstance.h>
#include <AQLFunctionUtilities.h>
#include <AQLPriceDataCalendar.h>
#include <AQLPriceDataSlidingRule.h>
#include <AQLPriceDataCalendar.h>
#include <AQLDataMatrix.h>
#include <AQLMathDefine.h>
#include "AQLNl2sol.h"

#include <AQLMathYieldCurve.h>
#include <AQLMathDateCalculations.h>
#include <AQLPriceCashFlowGenerator.h>

#include "AQLMathSwaptionVolUtility.h"
#include "AQLMathDateUtilities.h"
#include "AQLMathCurveFuncUtility.h"
#include "AQLMathInterpolationUtilities.h"
#include "AQLMathSABR.h"
#include "AQLMathLeastSquareSABR.h"
#include "AQLMathLeastSquareSABRQlib.h"
#include "AQLMathSwaptionSABR.h"
#include "AQLMathCapFloorSABR.h"
#include "AQLMathSABR.h"
#include "AQLModelUtilities.h"
#include "AQLQuantLibConstraint.h"
#include "AQLQuantLibProblem.h"
#include "AQLQuantLibOptimizationMethod.h"
#include "AQLQuantLibEndCriteria.h"
#include "AQLQuantLibConjugateGradient.h"
#include "AQLQuantLibSteepestDescent.h"
#include "AQLQuantLibSimplex.h"
#include "AQLQuantLibLevenbergMarquardt.h"
#include "AQLAlgorithm.h"

using namespace std;

// (copied by AQLDate.cpp) 
static const unsigned short LOOKUP_TABLE_NUMBER_OF_DAYS_IN_A_MONTH_OR_YEAR[2][2][12] =
{ { { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
{ 0, 31, 59, 90,120,151,181,212,243,273,304,334 } },
{ { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
{ 0, 31, 60, 91,121,152,182,213,244,274,305,335 } } };

static const double OPTION_SIGN_DUMMY = -9999.;

void
AQLMathSwaptionVolUtility::setUpSABRGrid( AQLDataInstance* dataInstance, const AQLString& matID, const AQLString& convID, 
									   AQLStringMatrix& mat )
{
    AQLString daycount(AC_365I);
    const AQLObject& object = dataInstance->getObjectPool().getObject( convID, ENCHKTYPE_ISDEFINED ).get();
    const AQLPriceDataCalendar& cal =
        dynamic_cast<const AQLPriceDataCalendar& >(object.getData(PRICING_DATA_FIXINGCALENDAR,ISDEFINED).get());
    const AQLPriceDataSlidingRule& sr = 
        dynamic_cast<const AQLPriceDataSlidingRule& >(object.getData(CALIBRATION_DATA_SLIDINGRULE,ISDEFINED).get());
    const AQLDate asOfDate = 
        dynamic_cast<const AQLDataDate& >(object.getData(CALIBRATION_DATA_ASOFDATE,ISNOTNULL).get()).get();
	if(mat.size()<=1 || mat[0].size()<=1) throw AQLCoreInvalidData("the size of matrix is small!", __FILE__, __LINE__);

    DoubleVector tenorVec,expiryVec;
    AQLStringVector tenorStr,expiryStr;
    DateVector expiDateVec;
    for(size_t i=1; i<mat.size(); i++)
    {
        expiryStr.push_back(mat[i][0]);
        expiDateVec.push_back( AQLMathDateCalculations::getDate(asOfDate,mat[i][0],sr,&cal,true) );
        expiryVec.push_back( AQLMathDateUtilities::getTerm(asOfDate, expiDateVec[i-1], daycount, true) );
    }

    int y,m,d,w;
    for(size_t i=1; i<mat[0].size(); i++)
    {
        tenorStr.push_back(mat[0][i]);
        AQLMathDateCalculations::termStrtoYMDW(mat[0][i], y, m, d, w);
        tenorVec.push_back( static_cast<double > (y) + static_cast<double > (m) / 12 );
    }

    mat.erase( mat.begin() );
    for(size_t i=0; i<mat.size(); i++)
    {
        mat[i].erase( mat[i].begin() );
    }

    DoubleMatrix mat_per(mat.size(), DoubleVector(mat[0].size()));
    for(size_t i=0; i<mat.size(); i++)
        for(size_t j=0; j<mat[0].size(); j++)
            mat_per[i][j] = mat[i][j].getDoubleValue();

	uppervec(tenorStr); uppervec(expiryStr);
    AQLObjectPool &objPool = dataInstance->getObjectPool();
    AQLObjectHolder objHolder = objPool.getObject(matID ,ENCHKTYPE_NOCHECK);
	if(!objHolder.isDefined())
	{
		AQLObject* e = new AQLObject();
		e->add( CALIBRATION_DATA_NAME, new AQLDataString() ).convertFromString(matID);
		e->add( CALIBRATION_DATA_ASOFDATE, new AQLDataDate(asOfDate));
		e->add( CALIBRATION_DATA_CALENDAR, new AQLPriceDataCalendar(cal));
		e->add( CALIBRATION_DATA_SLIDINGRULE, new AQLPriceDataSlidingRule(sr));
        e->add( PRICING_DATA_SWAPTIONMATRIX, new AQLDataDoubleMatrix( mat_per ));
		e->add( PRICING_DATA_EXPIRYDATEVECTOR,	new AQLDataDates(expiDateVec));
		e->add( PRICING_DATA_EXPIRYVECTOR,	new AQLDataDoubles(expiryVec));
        e->add( PRICING_DATA_TENORVECTOR,	new AQLDataDoubles(tenorVec));
        e->add( PRICING_DATA_EXPIRYSTRING,	new AQLDataStrings(expiryStr));
        e->add( PRICING_DATA_TENORSTRING,	new AQLDataStrings(tenorStr));
		objPool.set( matID , e );
	}
	else if(objHolder.isDefined())
	{
        AQLDataHolder* dh;
		dh = &objHolder.getData(CALIBRATION_DATA_ASOFDATE,ISDEFINED);
		AQLDataDate& asOfDate_att = dynamic_cast<AQLDataDate &>(dh->get());
        asOfDate_att.set( asOfDate );	

		dh = &objHolder.getData(CALIBRATION_DATA_CALENDAR,ISDEFINED);
		AQLPriceDataCalendar& cal_att = dynamic_cast<AQLPriceDataCalendar &>(dh->get());
        cal_att = cal;

		dh = &objHolder.getData(CALIBRATION_DATA_SLIDINGRULE,ISDEFINED);
		AQLPriceDataSlidingRule& sr_att = dynamic_cast<AQLPriceDataSlidingRule &>(dh->get());
        sr_att = sr;

		dh = &objHolder.getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED);
		AQLDataDoubleMatrix& mat_att = dynamic_cast<AQLDataDoubleMatrix &>(dh->get());
        mat_att.set( mat_per );		

		dh = &objHolder.getData(PRICING_DATA_EXPIRYDATEVECTOR,ISDEFINED);
		AQLDataDates& expiryDateVec_att = dynamic_cast<AQLDataDates &>(dh->get());
		expiryDateVec_att.set(expiDateVec);

		dh = &objHolder.getData(PRICING_DATA_EXPIRYVECTOR,ISDEFINED);
		AQLDataDoubles& expiryVec_att = dynamic_cast<AQLDataDoubles &>(dh->get());
		expiryVec_att.set(expiryVec);

        dh = &objHolder.getData(PRICING_DATA_TENORVECTOR,ISDEFINED);
		AQLDataDoubles& tenorVec_att = dynamic_cast<AQLDataDoubles &>(dh->get());
		tenorVec_att.set(tenorVec);

        dh = &objHolder.getData(PRICING_DATA_EXPIRYSTRING,ISDEFINED);
		AQLDataStrings& expiryStr_att = dynamic_cast<AQLDataStrings &>(dh->get());
		expiryStr_att.set(expiryStr);

        dh = &objHolder.getData(PRICING_DATA_TENORSTRING,ISDEFINED);
		AQLDataStrings& tenorStr_att = dynamic_cast<AQLDataStrings &>(dh->get());
		tenorStr_att.set(tenorStr);        
	}
}

void
AQLMathSwaptionVolUtility::setUpSwaptionVol( AQLDataInstance* dataInstance, const AQLString& matID, AQLStringMatrix& volMat, AQLStringMatrix& strikeMat, AQLStringMatrix& signMat)
{
	if(strikeMat.size()==0 || strikeMat[0].size()==0) throw AQLCoreInvalidData("the size of matrix is zero!", __FILE__, __LINE__);
	if(volMat.size()==0 || volMat[0].size()==0) throw AQLCoreInvalidData("the size of matrix is zero!", __FILE__, __LINE__);

    //vol matrix
    AQLStringVector tenorStr_vol,expiryStr_vol;
    for(size_t i=1; i<volMat.size(); i++)
    {
        expiryStr_vol.push_back(volMat[i][0]);
    }

    //int y,m,d;
    for(size_t i=1; i<volMat[0].size(); i++)
    {
        tenorStr_vol.push_back(volMat[0][i]);
    }

    volMat.erase( volMat.begin() );
    for(size_t i=0; i<volMat.size(); i++)
    {
        volMat[i].erase( volMat[i].begin() );
    }

    DoubleMatrix volMat_per(volMat.size(), DoubleVector(volMat[0].size()));
    for(size_t i=0; i<volMat.size(); i++)
        for(size_t j=0; j<volMat[0].size(); j++)
            volMat_per[i][j] = volMat[i][j].getDoubleValue();

    //strike matrix
    AQLStringVector tenorStr_K,expiryStr_K;
    for(size_t i=1; i<strikeMat.size(); i++)
    {
        expiryStr_K.push_back(strikeMat[i][0]);
    }
    for(size_t i=1; i<strikeMat[0].size(); i++)
    {
        tenorStr_K.push_back(strikeMat[0][i]);
    }
    if(tenorStr_vol != tenorStr_K ||  expiryStr_vol != expiryStr_K)
        throw AQLCoreInvalidData("strike matrix and vol matrix are inconsistent!",	__FILE__,__LINE__);

    strikeMat.erase( strikeMat.begin() );
    for(size_t i=0; i<strikeMat.size(); i++)
    {
        strikeMat[i].erase( strikeMat[i].begin() );
    }

    DoubleMatrix strikeMat_per(strikeMat.size(), DoubleVector(strikeMat[0].size()));
    for(size_t i=0; i<strikeMat.size(); i++)
        for(size_t j=0; j<strikeMat[0].size(); j++)
            strikeMat_per[i][j] = strikeMat[i][j].getDoubleValue();

	//sign matrix
	DoubleMatrix signMat_per(volMat_per.size(), DoubleVector(volMat_per[0].size(), OPTION_SIGN_DUMMY));
	if (signMat.size() > 0)
	{
		AQLStringVector tenorStr_Sign, expiryStr_Sign;
		for (size_t i = 1; i < signMat.size(); i++)
		{
			expiryStr_Sign.push_back(signMat[i][0]);
		}
		for (size_t i = 1; i < signMat[0].size(); i++)
		{
			tenorStr_Sign.push_back(signMat[0][i]);
		}
		if (tenorStr_vol != tenorStr_Sign || expiryStr_vol != expiryStr_Sign)
			throw AQLCoreInvalidData("sign matrix and vol matrix are inconsistent!", __FILE__, __LINE__);

		signMat.erase(signMat.begin());
		for (size_t i = 0; i < signMat.size(); i++)
		{
			signMat[i].erase(signMat[i].begin());
		}

		signMat_per = DoubleMatrix(signMat.size(), DoubleVector(signMat[0].size()));
		for (size_t i = 0; i < signMat.size(); i++)
			for (size_t j = 0; j < signMat[0].size(); j++)
				signMat_per[i][j] = signMat[i][j].getDoubleValue();
	}

	uppervec(tenorStr_vol); uppervec(expiryStr_vol);
    AQLObjectPool &objPool = dataInstance->getObjectPool();
    AQLObjectHolder objHolder = objPool.getObject(matID ,ENCHKTYPE_NOCHECK);
	if(!objHolder.isDefined())
	{
		AQLObject* e = new AQLObject();
		e->add( CALIBRATION_DATA_NAME, new AQLDataString() ).convertFromString(matID);
        e->add( PRICING_DATA_SWAPTIONMATRIX, new AQLDataDoubleMatrix( volMat_per ));
        e->add( PRICING_DATA_SWAPTIONSTRIKEMATRIX, new AQLDataDoubleMatrix( strikeMat_per ));
		e->add( PRICING_DATA_SWAPTIONSIGNMATRIX, new AQLDataDoubleMatrix( signMat_per ));
        e->add( PRICING_DATA_EXPIRYSTRING,	new AQLDataStrings(expiryStr_vol));
        e->add( PRICING_DATA_TENORSTRING,	new AQLDataStrings(tenorStr_vol));
		objPool.set( matID , e );
	}
	else if(objHolder.isDefined())
	{
        AQLDataHolder* dh;

        dh = &objHolder.getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED);
		AQLDataDoubleMatrix& matVol_att = dynamic_cast<AQLDataDoubleMatrix &>(dh->get());
        matVol_att.set( volMat_per );

        dh = &objHolder.getData(PRICING_DATA_SWAPTIONSTRIKEMATRIX,ISDEFINED);
		AQLDataDoubleMatrix& matStri_att = dynamic_cast<AQLDataDoubleMatrix &>(dh->get());
        matStri_att.set( strikeMat_per );

		dh = &objHolder.getData(PRICING_DATA_SWAPTIONSIGNMATRIX, ISDEFINED);
		AQLDataDoubleMatrix& matSign_att = dynamic_cast<AQLDataDoubleMatrix &>(dh->get());
		matSign_att.set( signMat_per );

        dh = &objHolder.getData(PRICING_DATA_EXPIRYSTRING,ISDEFINED);
		AQLDataStrings& expiryStr_att = dynamic_cast<AQLDataStrings &>(dh->get());
		expiryStr_att.set(expiryStr_vol);

        dh = &objHolder.getData(PRICING_DATA_TENORSTRING,ISDEFINED);
		AQLDataStrings& tenorStr_att = dynamic_cast<AQLDataStrings &>(dh->get());
		tenorStr_att.set(tenorStr_vol);        
	}
}

double
AQLMathSwaptionVolUtility::lookUpSwapGrid( AQLDataInstance* dataInstance, const AQLString& matID, 
                                        AQLString expPoint, AQLString tenorPoint )
{
	AQLPriceDataDayCount dc_act365(ACT_365_ISDA);

    AQLObjectPool& objPool = dataInstance->getObjectPool();
    const DoubleMatrix& mat = dynamic_cast<AQLDataDoubleMatrix& >
        (objPool.getObject( matID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
    const DoubleVector& tenorVec = dynamic_cast<const AQLDataDoubles& >
        (objPool.getObject( matID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_TENORVECTOR,ISDEFINED).get()).get();
    const DoubleVector& expiryVec = dynamic_cast<const AQLDataDoubles& >
        (objPool.getObject( matID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_EXPIRYVECTOR,ISDEFINED).get()).get();
    const AQLPriceDataSlidingRule& paySlr = dynamic_cast<const AQLPriceDataSlidingRule& >(objPool.getObject( matID, ENCHKTYPE_ISDEFINED ).
		get().getData(CALIBRATION_DATA_SLIDINGRULE,ISDEFINED).get());
    const AQLPriceDataCalendar& fixCal = dynamic_cast<const AQLPriceDataCalendar& >(objPool.getObject( matID, ENCHKTYPE_ISDEFINED ).get().
		getData(CALIBRATION_DATA_CALENDAR,ISDEFINED).get());
    const AQLDate& asOfDate = dynamic_cast<const AQLDataDate& >(dataInstance->getObjectPool().getObject(matID,ENCHKTYPE_ISDEFINED).
		get().getData(CALIBRATION_DATA_ASOFDATE,ISNOTNULL).get()).get();

	double expPoint_d = getExpiryPoint(expPoint, asOfDate, paySlr, fixCal);
	double tenorPoint_d = getTenorPoint(tenorPoint);

    AQLMathSwaptionMatrix swaptionMat(mat, expiryVec, tenorVec);
    
    return swaptionMat.lookUpMatrix(expPoint_d, tenorPoint_d);
}

double
AQLMathSwaptionVolUtility::lookUpSwapGrid(AQLDataInstance* dataInstance, const AQLString& matID, AQLDate expDate, AQLString tenorPoint)
{
    AQLPriceDataDayCount dc_act365(ACT_365_ISDA);

    AQLObjectPool& objPool = dataInstance->getObjectPool();
    const DoubleMatrix& mat = dynamic_cast<AQLDataDoubleMatrix& >
        (objPool.getObject(matID, ENCHKTYPE_ISDEFINED).get().getData(PRICING_DATA_SWAPTIONMATRIX, ISDEFINED).get()).get();
    const DoubleVector& tenorVec = dynamic_cast<const AQLDataDoubles& >
        (objPool.getObject(matID, ENCHKTYPE_ISDEFINED).get().getData(PRICING_DATA_TENORVECTOR, ISDEFINED).get()).get();
    const DoubleVector& expiryVec = dynamic_cast<const AQLDataDoubles& >
        (objPool.getObject(matID, ENCHKTYPE_ISDEFINED).get().getData(PRICING_DATA_EXPIRYVECTOR, ISDEFINED).get()).get();
    const AQLPriceDataSlidingRule& paySlr = dynamic_cast<const AQLPriceDataSlidingRule& >(objPool.getObject(matID, ENCHKTYPE_ISDEFINED).
        get().getData(CALIBRATION_DATA_SLIDINGRULE, ISDEFINED).get());
    const AQLPriceDataCalendar& fixCal = dynamic_cast<const AQLPriceDataCalendar& >(objPool.getObject(matID, ENCHKTYPE_ISDEFINED).get().
        getData(CALIBRATION_DATA_CALENDAR, ISDEFINED).get());
    const AQLDate& asOfDate = dynamic_cast<const AQLDataDate& >(dataInstance->getObjectPool().getObject(matID, ENCHKTYPE_ISDEFINED).
        get().getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL).get()).get();

    double expPoint_d = ModelTime(asOfDate, expDate);
    double tenorPoint_d = getTenorPoint(tenorPoint);

    AQLMathSwaptionMatrix swaptionMat(mat, expiryVec, tenorVec);

    return swaptionMat.lookUpMatrix(expPoint_d, tenorPoint_d);
}

void 
AQLMathSwaptionVolUtility::outPutSABRGrid( AQLDataInstance* dataInstance, const AQLString& matID, 
                                        DoubleVector& ret, size_t& row, size_t& colum )
{
    AQLObjectPool& objPool = dataInstance->getObjectPool();
    const AQLObject& object = objPool.getObject( matID, ENCHKTYPE_ISDEFINED ).get();

    const DoubleMatrix& mat = dynamic_cast<const AQLDataDoubleMatrix &>
        (object.getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();

    row = mat.size();
    colum = mat[0].size();
    ret.clear();
    for(size_t i=0; i<row; i++)
    {
        for(size_t j=0; j<colum; j++)
        {
            ret.push_back( mat[i][j] );
        }
    }
}

void
AQLMathSwaptionVolUtility::setUpConvention( AQLDataInstance* dataInstance, const AQLString& convID, AQLStringMatrix& convMat )
{
	AQLString curveID = searchbyrow(convMat, "CurveID",1,false);
	AQLString oneMCurveName = searchbyrow(convMat, "1MLName",1,false);
	AQLString threeMCurveName = searchbyrow(convMat, "3MLName",1,false);
	AQLString sixMCurveName = searchbyrow(convMat, "6MLName",1,false);
	AQLString swapCurveName = searchbyrow(convMat, "SwapRateName",1,false);
	AQLString discountCurveName = searchbyrow(convMat, "DFName",1,false);

	uppermat(convMat);
	AQLString asOfDate_str = searchbyrow(convMat,CURVEINPUT_ASOFDATE,1,false);
    AQLString freq = searchbyrow(convMat, CURVEINPUT_FREQUENCY,1,false);
    AQLString daycount = searchbyrow(convMat, CURVEINPUT_DAYCOUNT,1,false);
    AQLString paySlr = searchbyrow(convMat, CURVEINPUT_SLIDINGRULE,1,false);
    AQLString spotLag = searchbyrow(convMat, CURVEINPUT_SPOTLAG,1,false);
    AQLString payCal = searchbyrow(convMat, "PAYMENTCALENDAR",1,false);
    AQLString fixCal = searchbyrow(convMat, "FIXINGCALENDAR",1,false);

	AQLPriceDataSlidingRule paySlr_att; AQLPriceDataCalendar payCal_att, fixCal_att;  AQLPriceDataDayCount daycount_att;
	if(paySlr != AQLString("")) { paySlr_att.convertFromString(paySlr); }
	if(payCal != AQLString("")) { payCal_att.convertFromString(payCal); }
	if(fixCal != AQLString("")) { fixCal_att.convertFromString(fixCal); }
	if(daycount != AQLString("")) { daycount_att.convertFromString(daycount); }
	AQLDate asOfDate;
	if(asOfDate_str != AQLString("")) 
	{
		asOfDate = AQLMathDateUtilities::getAQLDate(asOfDate_str);
	}
	else if(curveID != AQLString(""))
	{
		const AQLObject& object = dataInstance->getObjectPool().getObject( curveID, ENCHKTYPE_ISDEFINED ).get();
		AQLDate asOfDate_curve = dynamic_cast<const AQLDataDate &>(object.getData(CALIBRATION_DATA_ASOFDATE,ISDEFINED).get()).get();
		if(asOfDate_str != AQLString("") && asOfDate != asOfDate_curve) throw AQLCoreInvalidData("asOfDates are inconsistent!",	__FILE__,__LINE__);
		else asOfDate = asOfDate_curve;
	}
	else
	{
		throw AQLCoreInvalidData("input asOfDate!",	__FILE__,__LINE__);
	}

    AQLObjectPool &objPool = dataInstance->getObjectPool();
    AQLObjectHolder objHolder = objPool.getObject(convID, ENCHKTYPE_NOCHECK);
    if(!objHolder.isDefined())
	{
		AQLObject* e = new AQLObject();
		e->add( CALIBRATION_DATA_NAME, new AQLDataString()).convertFromString(convID);
		if(asOfDate_str != AQLString("") || curveID != AQLString("")) { e->add( CALIBRATION_DATA_ASOFDATE, new AQLDataDate(asOfDate) ); }
		if(curveID != AQLString("")) { e->add( CALIBRATION_DATA_CURVEID, new AQLDataString()).convertFromString(curveID); }
		if(oneMCurveName != AQLString("")) { e->add( CALIBRATION_DATA_1MLCURVENAME, new AQLDataString()).convertFromString(oneMCurveName); }
		if(threeMCurveName != AQLString("")) { e->add( CALIBRATION_DATA_3MLCURVENAME, new AQLDataString()).convertFromString(threeMCurveName); }
		if(sixMCurveName != AQLString("")) { e->add( CALIBRATION_DATA_6MLCURVENAME, new AQLDataString()).convertFromString(sixMCurveName); }
		if(swapCurveName != AQLString("")) { e->add( CALIBRATION_DATA_SWAPRATELCURVENAME, new AQLDataString()).convertFromString(swapCurveName); }
		if(discountCurveName != AQLString("")) { e->add( CALIBRATION_DATA_DISCOUNTCURVENAME, new AQLDataString()).convertFromString(discountCurveName); }
        if(freq != AQLString("")) { e->add( IR_CALIBRATION_DATA_FREQUENCY, new AQLDataString(freq)); }
        if(daycount != AQLString("")) { e->add( IR_CALIBRATION_DATA_DAYCOUNT, new AQLPriceDataDayCount(daycount_att)); }
        if(spotLag != AQLString("")) { e->add( CURVEINPUT_SPOTLAG, new AQLDataString(spotLag)); }
        if(paySlr != AQLString("")) { e->add( CALIBRATION_DATA_SLIDINGRULE, new AQLPriceDataSlidingRule(paySlr_att)); }
        if(payCal != AQLString("")) { e->add( CALIBRATION_DATA_CALENDAR	, new AQLPriceDataCalendar(payCal_att)); }
        if(fixCal != AQLString("")) { e->add( PRICING_DATA_FIXINGCALENDAR, new AQLPriceDataCalendar(fixCal_att)); }

		objPool.set( convID , e );
	}
	else if(objHolder.isDefined())
	{
        AQLDataHolder* dh;

		if(asOfDate_str != AQLString("") || curveID != AQLString(""))
		{
			dh = &objHolder.getData(CALIBRATION_DATA_ASOFDATE,ISDEFINED);
			AQLDataDate& asOfDate_att = dynamic_cast<AQLDataDate &>(dh->get());
			asOfDate_att.set(asOfDate);
		}

		if(curveID != AQLString(""))
		{
			dh = &objHolder.getData(CALIBRATION_DATA_CURVEID,ISDEFINED);
			AQLDataString& curveid_att = dynamic_cast<AQLDataString &>(dh->get());
			curveid_att.set(curveID);
		}

		if(oneMCurveName != AQLString(""))
		{
			dh = &objHolder.getData(CALIBRATION_DATA_1MLCURVENAME,ISDEFINED);
			AQLDataString& curveid_att = dynamic_cast<AQLDataString &>(dh->get());
			curveid_att.set(oneMCurveName);
		}

		if(threeMCurveName != AQLString(""))
		{
			dh = &objHolder.getData(CALIBRATION_DATA_3MLCURVENAME,ISDEFINED);
			AQLDataString& curveid_att = dynamic_cast<AQLDataString &>(dh->get());
			curveid_att.set(threeMCurveName);
		}

		if(sixMCurveName != AQLString(""))
		{
			dh = &objHolder.getData(CALIBRATION_DATA_6MLCURVENAME,ISDEFINED);
			AQLDataString& curveid_att = dynamic_cast<AQLDataString &>(dh->get());
			curveid_att.set(sixMCurveName);
		}

		if(swapCurveName != AQLString(""))
		{
			dh = &objHolder.getData(CALIBRATION_DATA_SWAPRATELCURVENAME,ISDEFINED);
			AQLDataString& curveid_att = dynamic_cast<AQLDataString &>(dh->get());
			curveid_att.set(swapCurveName);
		}

		if(discountCurveName != AQLString(""))
		{
			dh = &objHolder.getData(CALIBRATION_DATA_DISCOUNTCURVENAME,ISDEFINED);
			AQLDataString& curveid_att = dynamic_cast<AQLDataString &>(dh->get());
			curveid_att.set(discountCurveName);
		}

		if(freq != AQLString(""))
		{
			dh = &objHolder.getData(IR_CALIBRATION_DATA_FREQUENCY,ISDEFINED);
			AQLDataString& freq_ = dynamic_cast<AQLDataString &>(dh->get());
			freq_.set(freq);
		}

		if(daycount != AQLString(""))
		{
			dh = &objHolder.getData(IR_CALIBRATION_DATA_DAYCOUNT,ISDEFINED);
			AQLPriceDataDayCount& daycount_att_ = dynamic_cast<AQLPriceDataDayCount &>(dh->get());
			daycount_att_ = daycount_att;      
		}

		if(spotLag != AQLString(""))
		{
			dh = &objHolder.getData(CURVEINPUT_SPOTLAG,ISDEFINED);
			AQLDataString& spotLag_ = dynamic_cast<AQLDataString& >(dh->get());
			spotLag_.set(spotLag);
		}

		if(paySlr != AQLString("")) 
		{
			dh = &objHolder.getData(CALIBRATION_DATA_SLIDINGRULE,ISDEFINED);
			AQLPriceDataSlidingRule& paySlr_att_ = dynamic_cast<AQLPriceDataSlidingRule& >(dh->get());
			paySlr_att_ = paySlr_att;
		}

		if(payCal != AQLString(""))
		{
			dh = &objHolder.getData(CALIBRATION_DATA_CALENDAR,ISDEFINED);
			AQLPriceDataCalendar& payCal_att_ = dynamic_cast<AQLPriceDataCalendar& >(dh->get());
			payCal_att_ = payCal_att;
		}

		if(fixCal != AQLString(""))
		{
			dh = &objHolder.getData(PRICING_DATA_FIXINGCALENDAR,ISDEFINED);
			AQLPriceDataCalendar& fixCal_att_ = dynamic_cast<AQLPriceDataCalendar& >(dh->get());
			fixCal_att_ = fixCal_att;
		}
	}
}

void 
AQLMathSwaptionVolUtility::setCurveID(AQLDataInstance* dataInstance, const AQLString& name, const AQLString& swapConvID, const AQLString& capConvID)
{
	const AQLObject& object = dataInstance->getObjectPool().getObject( swapConvID, ENCHKTYPE_ISDEFINED ).get();
	AQLString curveID = dynamic_cast<const AQLDataString& >(object.getData(CALIBRATION_DATA_CURVEID,ISDEFINED).get()).get();
	AQLString oneMCurveName(""),threeMCurveName(""),sixMCurveName(""),swapCurveName(""),discountCurveName("");
	if(capConvID != AQLString(""))
	{
		const AQLObject& entity_cap = dataInstance->getObjectPool().getObject(capConvID, ENCHKTYPE_ISDEFINED).get();
		oneMCurveName = dynamic_cast<const AQLDataString& >(entity_cap.getData(CALIBRATION_DATA_1MLCURVENAME,ISDEFINED).get()).get();
		threeMCurveName = dynamic_cast<const AQLDataString& >(entity_cap.getData(CALIBRATION_DATA_3MLCURVENAME,ISDEFINED).get()).get();
		sixMCurveName = dynamic_cast<const AQLDataString& >(entity_cap.getData(CALIBRATION_DATA_6MLCURVENAME,ISDEFINED).get()).get();
		swapCurveName = dynamic_cast<const AQLDataString& >(entity_cap.getData(CALIBRATION_DATA_SWAPRATELCURVENAME,ISDEFINED).get()).get();
		discountCurveName = dynamic_cast<const AQLDataString& >(entity_cap.getData(CALIBRATION_DATA_DISCOUNTCURVENAME,ISDEFINED).get()).get();
	}
	AQLObjectPool &objPool = dataInstance->getObjectPool();
    AQLObjectHolder objHolder = objPool.getObject(name ,ENCHKTYPE_NOCHECK);
    if(!objHolder.isDefined())
	{
		AQLObject* e = new AQLObject();
		e->add( CALIBRATION_DATA_NAME, new AQLDataString()).convertFromString(name);
		e->add( CALIBRATION_DATA_CURVEID, new AQLDataString()).convertFromString(curveID); 

		if(oneMCurveName != AQLString("")) 
		{ 
			e->add( CALIBRATION_DATA_1MLCURVENAME, new AQLDataString()).convertFromString(oneMCurveName); 
		}
		else
		{
			e->add( CALIBRATION_DATA_1MLCURVENAME, new AQLDataString()).convertFromString(STD); 
		}

		if(threeMCurveName != AQLString("")) 
		{
			e->add( CALIBRATION_DATA_3MLCURVENAME, new AQLDataString()).convertFromString(threeMCurveName); 
		}
		else
		{
			e->add( CALIBRATION_DATA_3MLCURVENAME, new AQLDataString()).convertFromString(STD); 
		}

		if(sixMCurveName != AQLString("")) 
		{ 
			e->add( CALIBRATION_DATA_6MLCURVENAME, new AQLDataString()).convertFromString(sixMCurveName); 
		}
		else
		{
			e->add( CALIBRATION_DATA_6MLCURVENAME, new AQLDataString()).convertFromString(STD); 
		}

		if(swapCurveName != AQLString("")) 
		{ 
			e->add( CALIBRATION_DATA_SWAPRATELCURVENAME	, new AQLDataString()).convertFromString(swapCurveName); 
		}
		else
		{
			e->add( CALIBRATION_DATA_SWAPRATELCURVENAME	, new AQLDataString()).convertFromString(STD); 
		}

		if(discountCurveName != AQLString("")) 
		{ 
			e->add( CALIBRATION_DATA_DISCOUNTCURVENAME	, new AQLDataString()).convertFromString(discountCurveName); 
		}
		else
		{
			e->add( CALIBRATION_DATA_DISCOUNTCURVENAME	, new AQLDataString()).convertFromString(STD); 
		}
      
		objPool.set( name , e );
	}
	else if(objHolder.isDefined())
	{
        AQLDataHolder* dh;

		dh = &objHolder.getData(CALIBRATION_DATA_CURVEID,ISDEFINED);
		AQLDataString& curveid_att = dynamic_cast<AQLDataString &>(dh->get());
		curveid_att.set(curveID);

		if(oneMCurveName != AQLString(""))
		{
			dh = &objHolder.getData(CALIBRATION_DATA_1MLCURVENAME,ISDEFINED);
			AQLDataString& curveid_att = dynamic_cast<AQLDataString &>(dh->get());
			curveid_att.set(oneMCurveName);
		}
		else
		{
			dh = &objHolder.getData(CALIBRATION_DATA_1MLCURVENAME,ISDEFINED);
			AQLDataString& curveid_att = dynamic_cast<AQLDataString &>(dh->get());
			curveid_att.set(STD);
		}

		if(threeMCurveName != AQLString(""))
		{
			dh = &objHolder.getData(CALIBRATION_DATA_3MLCURVENAME,ISDEFINED);
			AQLDataString& curveid_att = dynamic_cast<AQLDataString &>(dh->get());
			curveid_att.set(threeMCurveName);
		}
		else
		{
			dh = &objHolder.getData(CALIBRATION_DATA_3MLCURVENAME,ISDEFINED);
			AQLDataString& curveid_att = dynamic_cast<AQLDataString &>(dh->get());
			curveid_att.set(STD);
		}

		if(sixMCurveName != AQLString(""))
		{
			dh = &objHolder.getData(CALIBRATION_DATA_6MLCURVENAME,ISDEFINED);
			AQLDataString& curveid_att = dynamic_cast<AQLDataString &>(dh->get());
			curveid_att.set(sixMCurveName);
		}
		else
		{
			dh = &objHolder.getData(CALIBRATION_DATA_6MLCURVENAME,ISDEFINED);
			AQLDataString& curveid_att = dynamic_cast<AQLDataString &>(dh->get());
			curveid_att.set(STD);
		}

		if(swapCurveName != AQLString(""))
		{
			dh = &objHolder.getData(CALIBRATION_DATA_SWAPRATELCURVENAME	,NOCHECK);
			AQLDataString& curveid_att = dynamic_cast<AQLDataString &>(dh->get());
			curveid_att.set(swapCurveName);
		}
		else
		{
			dh = &objHolder.getData(CALIBRATION_DATA_SWAPRATELCURVENAME	,NOCHECK);
			AQLDataString& curveid_att = dynamic_cast<AQLDataString &>(dh->get());
			curveid_att.set(STD);
		}

		if(discountCurveName != AQLString(""))
		{
			dh = &objHolder.getData(CALIBRATION_DATA_DISCOUNTCURVENAME	,NOCHECK);
			AQLDataString& curveid_att = dynamic_cast<AQLDataString &>(dh->get());
			curveid_att.set(discountCurveName);
		}
		else
		{
			dh = &objHolder.getData(CALIBRATION_DATA_DISCOUNTCURVENAME	,NOCHECK);
			AQLDataString& curveid_att = dynamic_cast<AQLDataString &>(dh->get());
			curveid_att.set(STD);
		}
	}
}

void 
AQLMathSwaptionVolUtility::setCurveID2(AQLDataInstance* dataInstance, const AQLString& name, AQLStringMatrix& curveMat)
{
	AQLString curveID = searchbyrow(curveMat, "CurveID",1,true);
	AQLString oneMCurveName = searchbyrow(curveMat, "1MLName",1,true);
	AQLString threeMCurveName = searchbyrow(curveMat, "3MLName",1,true);
	AQLString sixMCurveName = searchbyrow(curveMat, "6MLName",1,true);
	AQLString swapCurveName = searchbyrow(curveMat, "SwapRateName",1,true);
	AQLString discountCurveName = searchbyrow(curveMat, "DFName",1,true);

	AQLObjectPool &objPool = dataInstance->getObjectPool();
    AQLObjectHolder objHolder = objPool.getObject(name ,ENCHKTYPE_NOCHECK);
    if(!objHolder.isDefined())
	{
		AQLObject* e = new AQLObject();
		e->add( CALIBRATION_DATA_NAME, new AQLDataString()).convertFromString(name);
		if(curveID != AQLString("")) 
		{ 
			e->add( CALIBRATION_DATA_CURVEID, new AQLDataString()).convertFromString(curveID); 
		}
		else
		{
			throw AQLCoreInvalidData("input curve ID!", __FILE__, __LINE__);
		}

		if(oneMCurveName != AQLString("")) 
		{ 
			e->add( CALIBRATION_DATA_1MLCURVENAME, new AQLDataString()).convertFromString(oneMCurveName); 
		}
		else
		{
			e->add( CALIBRATION_DATA_1MLCURVENAME, new AQLDataString()).convertFromString(STD); 
		}

		if(threeMCurveName != AQLString("")) 
		{
			e->add( CALIBRATION_DATA_3MLCURVENAME, new AQLDataString()).convertFromString(threeMCurveName); 
		}
		else
		{
			e->add( CALIBRATION_DATA_3MLCURVENAME, new AQLDataString()).convertFromString(STD); 
		}

		if(sixMCurveName != AQLString("")) 
		{ 
			e->add( CALIBRATION_DATA_6MLCURVENAME, new AQLDataString()).convertFromString(sixMCurveName); 
		}
		else
		{
			e->add( CALIBRATION_DATA_6MLCURVENAME, new AQLDataString()).convertFromString(STD); 
		}

		if(swapCurveName != AQLString("")) 
		{ 
			e->add( CALIBRATION_DATA_SWAPRATELCURVENAME	, new AQLDataString()).convertFromString(swapCurveName); 
		}
		else
		{
			e->add( CALIBRATION_DATA_SWAPRATELCURVENAME	, new AQLDataString()).convertFromString(STD); 
		}

		if(discountCurveName != AQLString("")) 
		{ 
			e->add( CALIBRATION_DATA_DISCOUNTCURVENAME	, new AQLDataString()).convertFromString(discountCurveName); 
		}
		else
		{
			e->add( CALIBRATION_DATA_DISCOUNTCURVENAME	, new AQLDataString()).convertFromString(STD); 
		}
      
		objPool.set( name , e );
	}
	else if(objHolder.isDefined())
	{
        AQLDataHolder* dh;

		if(curveID != AQLString(""))
		{
			dh = &objHolder.getData(CALIBRATION_DATA_CURVEID,NOCHECK);
			AQLDataString& curveid_att = dynamic_cast<AQLDataString &>(dh->get());
			curveid_att.set(curveID);
		}
		else
		{
			throw AQLCoreInvalidData("input curve ID!", __FILE__, __LINE__);
		}

		if(oneMCurveName != AQLString(""))
		{
			dh = &objHolder.getData(CALIBRATION_DATA_1MLCURVENAME,ISDEFINED);
			AQLDataString& curveid_att = dynamic_cast<AQLDataString &>(dh->get());
			curveid_att.set(oneMCurveName);
		}
		else
		{
			dh = &objHolder.getData(CALIBRATION_DATA_1MLCURVENAME,ISDEFINED);
			AQLDataString& curveid_att = dynamic_cast<AQLDataString &>(dh->get());
			curveid_att.set(STD);
		}

		if(threeMCurveName != AQLString(""))
		{
			dh = &objHolder.getData(CALIBRATION_DATA_3MLCURVENAME,ISDEFINED);
			AQLDataString& curveid_att = dynamic_cast<AQLDataString &>(dh->get());
			curveid_att.set(threeMCurveName);
		}
		else
		{
			dh = &objHolder.getData(CALIBRATION_DATA_3MLCURVENAME,ISDEFINED);
			AQLDataString& curveid_att = dynamic_cast<AQLDataString &>(dh->get());
			curveid_att.set(STD);
		}

		if(sixMCurveName != AQLString(""))
		{
			dh = &objHolder.getData(CALIBRATION_DATA_6MLCURVENAME,ISDEFINED);
			AQLDataString& curveid_att = dynamic_cast<AQLDataString &>(dh->get());
			curveid_att.set(sixMCurveName);
		}
		else
		{
			dh = &objHolder.getData(CALIBRATION_DATA_6MLCURVENAME,ISDEFINED);
			AQLDataString& curveid_att = dynamic_cast<AQLDataString &>(dh->get());
			curveid_att.set(STD);
		}

		if(swapCurveName != AQLString(""))
		{
			dh = &objHolder.getData(CALIBRATION_DATA_SWAPRATELCURVENAME	,NOCHECK);
			AQLDataString& curveid_att = dynamic_cast<AQLDataString &>(dh->get());
			curveid_att.set(swapCurveName);
		}
		else
		{
			dh = &objHolder.getData(CALIBRATION_DATA_SWAPRATELCURVENAME	,NOCHECK);
			AQLDataString& curveid_att = dynamic_cast<AQLDataString &>(dh->get());
			curveid_att.set(STD);
		}

		if(discountCurveName != AQLString(""))
		{
			dh = &objHolder.getData(CALIBRATION_DATA_DISCOUNTCURVENAME	,NOCHECK);
			AQLDataString& curveid_att = dynamic_cast<AQLDataString &>(dh->get());
			curveid_att.set(discountCurveName);
		}
		else
		{
			dh = &objHolder.getData(CALIBRATION_DATA_DISCOUNTCURVENAME	,NOCHECK);
			AQLDataString& curveid_att = dynamic_cast<AQLDataString &>(dh->get());
			curveid_att.set(STD);
		}
	}
}

void 
AQLMathSwaptionVolUtility::setCurveID2(AQLDataInstance* dataInstance, const AQLString& name, const AQLStringVector& sTenor, AQLStringMatrix& curveMat)
{
	AQLMathSwaptionVolUtility::setCurveID2(dataInstance, name, curveMat);

	AQLObjectPool &objPool = dataInstance->getObjectPool();
    AQLObjectHolder objHolder = objPool.getObject(name,ENCHKTYPE_NOCHECK);

	for(size_t i = 0; i < sTenor.size(); i++)
	{
		AQLString swapCurveName = searchbyrow(curveMat, AQLString("SwapRateName_") + sTenor[i],1,false);

		AQLDataHolder* dh;
		dh = &objHolder.getData(CALIBRATION_DATA_SWAPRATELCURVENAME + AQLString("_") + sTenor[i], NOCHECK);		
		if(dh->isDefined() && !dh->isNull())
		{
			if(swapCurveName != AQLString(""))
			{
				AQLDataString& curveid_att = dynamic_cast<AQLDataString &>(dh->get());
				curveid_att.set(swapCurveName);
			}
		}
		else
		{
			if(swapCurveName != AQLString(""))
				objHolder.get().add( CALIBRATION_DATA_SWAPRATELCURVENAME + AQLString("_") + sTenor[i], new AQLDataString()).convertFromString(swapCurveName); 
		}
	}
}

void 
AQLMathSwaptionVolUtility::calibrateSABRMatrix
( AQLDataInstance* dataInstance, const AQLString& approxMethod, const std::vector<bool>& calibFlg, const AQLString& calibMethod, const AQLString& curveSetID, const AQLString& alphaID, const AQLString& betaID, 
  const AQLString& nuID, const AQLString& rhoID, const AQLString& swapConvID, 
  const AQLString& capConvID, const AQLStringVector& swapVolID, /*AQLStringMatrix sabrLimiter,*/
  const AQLString& target, const DoubleVector& weight, const IntVector& sgn, const AQLString& forwardID, const double forwardShiftValue, 
  const AQLString& numeraireID, AQLString& msg, const BoolMatrix *calibFlgMtx, bool isLognormal)
{
    AQLPriceDataDayCount dc_act365(ACT_365_ISDA);
	AQLString tmp_target = target;
	tmp_target.toUpper();
    //upper(sabrLimiter);
    //const double eps = 0.000000001;

    //AQLMathSABRLimiter limiter(sabrLimiter);

    if( swapVolID.size() != weight.size() )
        throw AQLCoreInvalidData("sizes are inconsistent!",	__FILE__,__LINE__);

	if( swapVolID.size() != sgn.size() )
        throw AQLCoreInvalidData("sizes are inconsistent!",	__FILE__,__LINE__);

	matirixCheck(dataInstance, alphaID, betaID);
	matirixCheck(dataInstance, alphaID, nuID);
	matirixCheck(dataInstance, alphaID, rhoID);

    AQLObjectPool& objPool = dataInstance->getObjectPool();
    DoubleMatrix& alphaMat = dynamic_cast<AQLDataDoubleMatrix &>
        (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
    const AQLStringVector& expiry = dynamic_cast<const AQLDataStrings &>
        (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_EXPIRYSTRING,ISDEFINED).get()).get();
    const AQLStringVector& tenor = dynamic_cast<const AQLDataStrings &>
        (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_TENORSTRING,ISDEFINED).get()).get();
	const DoubleVector& expiryTerms = dynamic_cast<const AQLDataDoubles &>
        (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_EXPIRYVECTOR,ISDEFINED).get()).get();
	const DateVector& expiryDates = dynamic_cast<const AQLDataDates &>
        (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_EXPIRYDATEVECTOR,ISDEFINED).get()).get();
    DoubleMatrix& betaMat = dynamic_cast<AQLDataDoubleMatrix &>
        (objPool.getObject( betaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
    DoubleMatrix& nuMat = dynamic_cast<AQLDataDoubleMatrix &>
        (objPool.getObject( nuID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
    DoubleMatrix& rhoMat = dynamic_cast<AQLDataDoubleMatrix &>
        (objPool.getObject( rhoID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();

    AQLDataHolder* dh = &((objPool.getObject( alphaID, ENCHKTYPE_NOCHECK ).get()).getData(PRICING_DATA_GRIDAPPROXMETHOD,NOCHECK));
	AQLString tmp_approxMethod = approxMethod;
	AQLStringVector stmpvec(tenor.size(),tmp_approxMethod.toUpper());
	AQLStringMatrix gridApproxMethodMtx(expiry.size(),stmpvec);

	if(dh->isDefined() && !dh->isNull())
	{
		gridApproxMethodMtx = dynamic_cast<const AQLDataStringMatrix&>(dh->get()).get();
		if (gridApproxMethodMtx.size() != expiry.size() || (gridApproxMethodMtx)[0].size() != tenor.size())
		{
			throw AQLCoreInvalidData("Grid Approximation Method matrix does not match swaption volatility matrix!",	__FILE__,__LINE__);
		}
	}
	
	if(calibFlgMtx)
	{
		if (calibFlgMtx->size() != expiry.size() || (*calibFlgMtx)[0].size() != tenor.size())
		{
			throw AQLCoreInvalidData("Calibration flag matrix does not match swaption volatility matrix!",	__FILE__,__LINE__);
		}
	}

	DoubleMatrix forwardMat, numeraireMat;
	if(forwardID != AQLString("")) 
	{
		matirixCheck(dataInstance, alphaID, forwardID);
		forwardMat = dynamic_cast<AQLDataDoubleMatrix &>
			(objPool.getObject( forwardID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();

		numeraireMat = dynamic_cast<AQLDataDoubleMatrix &>
			(objPool.getObject( numeraireID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
	}

    vector<DoubleMatrix > marketVol, marketStk;
    for(size_t i=0; i<swapVolID.size(); i++)
    {
        matirixCheck(dataInstance, alphaID, swapVolID[i], true);
        marketVol.push_back(
        dynamic_cast<const AQLDataDoubleMatrix &> (objPool.getObject( swapVolID[i], ENCHKTYPE_ISDEFINED ).get().
        getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get() );

        marketStk.push_back(
        dynamic_cast<const AQLDataDoubleMatrix &> (objPool.getObject( swapVolID[i], ENCHKTYPE_ISDEFINED ).get().
        getData(PRICING_DATA_SWAPTIONSTRIKEMATRIX,ISDEFINED).get()).get() );
    }

	//
	double max_alpha = 3.;
	double min_alpha = 0.0001;
	double max_beta = 1.;
	double min_beta = 0.;
	double max_nu = 5.;
	double min_nu    = 0.0001;
	double max_rho = 0.98;
	double min_rho = -0.98;

    double forward, numeraire;
    DoubleArray forward_calib, numeraire_calib, strike_calib, vol_calib, expiry_calib;//, x(limiter.getParamNum());
    DateVector dates;
//    AQLMathSABR_Hagan sabr;
	AQLString curveID,convID,foreName,dfName;
	AQLObject curveEntity;
	
	if(forwardID == AQLString(""))
	{
		curveEntity = dataInstance->getObjectPool().getObject( curveSetID, ENCHKTYPE_ISDEFINED ).get();
		curveID = dynamic_cast<const AQLDataString& >(curveEntity.getData(CALIBRATION_DATA_CURVEID,ISDEFINED).get()).get();
		bool isMap = false;
	}

	const double ATM_VOL = std::numeric_limits<double>::quiet_NaN();

	for(size_t j=0; j<tenor.size(); j++)
    {
		for(size_t i=0; i<expiry.size(); i++)
		{
			if (calibFlgMtx && !(*calibFlgMtx)[i][j])
			{
				continue;
			}
			if(forwardID == AQLString(""))
			{
				if( tenor[j]==AQLString("1M") || tenor[j]==AQLString("3M") || tenor[j]==AQLString("6M") )
				{
					convID = capConvID;
					foreName = dynamic_cast<const AQLDataString& >(curveEntity.getData(tenor[j]+AQLString("LiborCurveName"),ISDEFINED).get()).get();
					dfName = dynamic_cast<const AQLDataString& >(curveEntity.getData(CALIBRATION_DATA_DISCOUNTCURVENAME,ISDEFINED).get()).get();
				}
				else
				{
					convID = swapConvID;
					foreName = dynamic_cast<const AQLDataString& >(curveEntity.getData(AQLString("SwapRateCurveName"),ISDEFINED).get()).get();
					dfName = dynamic_cast<const AQLDataString& >(curveEntity.getData(CALIBRATION_DATA_DISCOUNTCURVENAME,ISDEFINED).get()).get();
				}
				//forward = getForward(dataInstance, expiryDates[i], tenor[j], curveID, convID, foreName, dfName); forward = (forward > 0.0) ? forward : eps_SABR;
				forward = getForward(dataInstance, expiryDates[i], tenor[j], curveID, convID, foreName, dfName);
				numeraire = getNumeraire(dataInstance, expiryDates[i], tenor[j], curveID, convID, dfName);
			}
			else
			{
				forward = forwardMat[i][j];
				numeraire = numeraireMat[i][j];
			}
			forward += forwardShiftValue;

			forward = (std::fabs(forward) < eps_SABR) ? eps_SABR : forward;
			double max_alpha = fabs(forward) < 0.01 ? 0.01 / fabs(forward) * 5. : 5.;
			forward_calib = getConstantVector(forward,swapVolID.size());
            numeraire_calib = getConstantVector(numeraire,swapVolID.size());
			expiry_calib = getConstantVector(expiryTerms[i],swapVolID.size());

            strike_calib.clear();
            vol_calib.clear();
            for(size_t k=0; k<swapVolID.size(); k++)
            {                
                vol_calib.push_back(marketVol[k][i][j]);
                strike_calib.push_back(marketStk[k][i][j] + forwardShiftValue);
            }

			vector<double> initValue(4);
			initValue[0] = alphaMat[i][j];
			initValue[1] = betaMat[i][j];
			initValue[2] = nuMat[i][j];
			initValue[3] = rhoMat[i][j];


			msg = AQLString("alpha is out of range.");
			if(initValue[0] < min_alpha || initValue[0] > max_alpha) throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);

			msg = AQLString("beta is out of range.");
			if(initValue[1] < min_beta || initValue[1] > max_beta) throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);

			msg = AQLString("nu is out of range.");
			if(initValue[2] < min_nu || initValue[2] > max_nu) throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);

			msg = AQLString("rho is out of range.");
			if(initValue[3] < min_rho || initValue[3] > max_rho) throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);

            //calibration
			AQLMathSABR* sabr = createSABR(gridApproxMethodMtx[i][j], alphaMat[i][j], betaMat[i][j], nuMat[i][j], rhoMat[i][j], isLognormal);

			AQLMathLeastSquareSABRCostFunc* costfunc = new AQLMathLeastSquareSABRCostFunc(*sabr,
																				    expiryTerms[i],
																					forward,
																					numeraire,
																					ATM_VOL,
																					strike_calib,
																			        weight,
																					sgn,
																			        initValue,
																					calibFlg,
																					vol_calib,
																					tmp_target);
	
			// Create constraint
			AQLQuantLibConstraint* constraint = 0;
			AQLString tmp_approxMethod = gridApproxMethodMtx[i][j];
			tmp_approxMethod.toUpper();
			if(tmp_approxMethod == APPROXIMATION_ANTONOV)
			{
				constraint = new AQLMathLeastSquareSABRConstraint_Antonov(forward,
																	   initValue,
																	   calibFlg,
																	   max_alpha,
																	   min_alpha,
																	   max_beta,
																	   min_beta,
																	   max_nu,
																	   min_nu,
																	   max_rho,
																	   min_rho);
			}	
			else
			{
				constraint = new AQLMathLeastSquareSABRConstraint(initValue,
										 					   calibFlg,
															   max_alpha,
										                       min_alpha,
										                       max_beta,
										                       min_beta,
										                       max_nu,
										                       min_nu,
										                       max_rho,
											                   min_rho);
			}

			//
			vector<double> x_;
			if(!calibFlg[0])
			{
				if(!calibFlg[1])
				{
					if(!calibFlg[2])
					{
						if(!calibFlg[3])
						{
							throw AQLCoreInvalidData("All element of flg is false : AQLMathLeastSquareSABRCostFunc::set_params", __FILE__, __LINE__);
						}
						else x_.push_back(rhoMat[i][j]);
					}
					else
					{
						x_.push_back(nuMat[i][j]);
						if(calibFlg[3]) x_.push_back(rhoMat[i][j]);
					}
				}
				else
				{
					x_.push_back(betaMat[i][j]);
					if(calibFlg[2])
					{
						x_.push_back(nuMat[i][j]);
						if(calibFlg[3]) x_.push_back(rhoMat[i][j]);
					}
					else
					{
						if(calibFlg[3]) x_.push_back(rhoMat[i][j]);
					}
				}
			}
			else
			{
				x_.push_back(alphaMat[i][j]);
				if(!calibFlg[1])
				{
					if(calibFlg[2])
					{
						x_.push_back(nuMat[i][j]);
						if(calibFlg[3]) x_.push_back(rhoMat[i][j]);
					}
					else
					{
						if(calibFlg[3])  x_.push_back(rhoMat[i][j]);
					}
				}
				else
				{
					 x_.push_back(betaMat[i][j]);
					if(calibFlg[2])
					{
						x_.push_back(nuMat[i][j]);
						if(calibFlg[3]) x_.push_back(rhoMat[i][j]);
					}
					else
					{
						if(calibFlg[3]) x_.push_back(rhoMat[i][j]);
					}
				}
			}

			AQLQuantLibArray x(x_.size());
			for(size_t k = 0; k < x_.size(); ++k) x[k] = x_[k];
			AQLQuantLibProblem* opt_problem = new AQLQuantLibProblem(*costfunc, *constraint, x);


			// Set Optimization Method
			AQLQuantLibOptimizationMethod* optMethod;
			AQLString tmp_calibMethod = calibMethod;
			tmp_calibMethod.toUpper();
			if (tmp_calibMethod == CALIB_NON_LINEAR_CONJUGATE_GRADIENT_METHOD)
			{
				optMethod = new AQLQuantLibConjugateGradient();
			}
			else if (tmp_calibMethod == CALIB_STEEPEST_DESCENT_METHOD)
			{
				optMethod = new AQLQuantLibSteepestDescent();
			}
			else if (tmp_calibMethod == CALIB_SIMPLEX_METHOD)
			{
				if (tmp_target == CALIB_TARGET_VOLATILITY)
				{
					optMethod = new AQLQuantLibSimplex(0.1);
				}
				else
				{
					optMethod = new AQLQuantLibSimplex(max_alpha);
				}
			}
			else if (tmp_calibMethod == CALIB_LEVENBERG_MARQUARDT_METHOD)
			{
				double levenbergMarquardtEpsfcn = 0.0000001;       // parameters specific for Levenberg-Marquardt
				double levenbergMarquardtXtol   = 0.0000001;	   //
				double levenbergMarquardtGtol   = 0.0000001;       //

				optMethod = new AQLQuantLibLevenbergMarquardt(levenbergMarquardtEpsfcn,
					                                         levenbergMarquardtXtol, 
															 levenbergMarquardtGtol);

			}
			else
			{
				delete optMethod;
				delete opt_problem;
				delete constraint;
				delete costfunc;
				delete sabr;


				AQLString msg = AQLString("Optimization Method: ") + calibMethod + " not support";
				throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
			}



			// Set EndCriteria
			unsigned int maxIteration                = static_cast<unsigned int>(5000); 
			unsigned int maxStationaryStateIteration = static_cast<unsigned int>(2500);
			double rootEpsilon         = 0.0000001; 
			double functionEpsilon     = 0.0000001; 
			double gradientNormEpsilon = 0.0000001;

			if (maxStationaryStateIteration >= maxIteration)
			{
				delete optMethod;
				delete opt_problem;
				delete constraint;
				delete costfunc;
				delete sabr;

				AQLString msg = AQLString("maxStationaryStateIteration must be smaller than maxIterration.");
				throw AQLCoreInvalidData(msg.getCString(),__FILE__,__LINE__);
			}

			AQLQuantLibEndCriteria* endCriteria = new AQLQuantLibEndCriteria(maxIteration, 
					                                                           maxStationaryStateIteration,
														                       rootEpsilon, 
														                       functionEpsilon, 
														                       gradientNormEpsilon);

			// Optimization
			msg = "";
			try
			{
				try
				{
					AQLQuantLibEndCriteria::Type endCriteriaResult = optMethod->minimize(*opt_problem, *endCriteria);
				}
				catch(std::exception& e)
				{
					msg += AQLString(e.what());
					msg += AQLString("(");
					msg += AQLString(AQLModelUtilities::n2s(i).c_str());
					msg += AQLString(",");
					msg += AQLString(AQLModelUtilities::n2s(j).c_str());
					msg += AQLString(")");
					msg += AQLString(" : ");

				}
			}
			catch(AQLCoreError&e)
			{
				for(unsigned int k =0;k<e.getSize();k++) msg+=AQLString(e.getMsg(e.getSize()-k-1));
				msg += AQLString("(");
				msg += AQLString(AQLModelUtilities::n2s(i).c_str());
				msg += AQLString(",");
				msg += AQLString(AQLModelUtilities::n2s(j).c_str());
				msg += AQLString(")");
				msg += AQLString(" : ");
			}

			//QuantLib::Array xMinCalculated = opt_problem->currentValue();

			std::shared_ptr<AQLMathLeastSquareSABRCostFuncQlib> costfuncQlib = dynamic_pointer_cast<AQLMathLeastSquareSABRCostFuncQlib>(costfunc->getCostFunction());
			alphaMat[i][j] = costfuncQlib->getAlpha();
			betaMat[i][j]  = costfuncQlib->getBeta();
			nuMat[i][j]    = costfuncQlib->getNu();
			rhoMat[i][j]   = costfuncQlib->getRho();

			delete endCriteria;
			delete optMethod;
			delete opt_problem;
			delete constraint;
			delete costfunc;
			delete sabr;
        }
    }

    AQLDataDoubleMatrix mat_att;
    dh = &dataInstance->getObjectPool().getObject(alphaID ,ENCHKTYPE_NOCHECK).getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED);   
    mat_att = dynamic_cast<AQLDataDoubleMatrix &>(dh->get());
    mat_att.set( alphaMat );

    dh = &dataInstance->getObjectPool().getObject(betaID ,ENCHKTYPE_NOCHECK).getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED);   
    mat_att = dynamic_cast<AQLDataDoubleMatrix &>(dh->get());
    mat_att.set( betaMat );

    dh = &dataInstance->getObjectPool().getObject(nuID ,ENCHKTYPE_NOCHECK).getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED);   
    mat_att = dynamic_cast<AQLDataDoubleMatrix &>(dh->get());
    mat_att.set( nuMat );

    dh = &dataInstance->getObjectPool().getObject(rhoID ,ENCHKTYPE_NOCHECK).getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED);   
    mat_att = dynamic_cast<AQLDataDoubleMatrix &>(dh->get());
    mat_att.set( rhoMat );
}


void
AQLMathSwaptionVolUtility::calibrateSABRMatrix(DoubleMatrix& alphaMat, DoubleMatrix& betaMat, DoubleMatrix& nuMat, DoubleMatrix& rhoMat,
											const AQLString& approxMethod, const std::vector<bool>& calibFlg, const AQLString& calibMethod,
											const std::vector<DoubleMatrix >& marketVol, const std::vector<DoubleMatrix >& marketStrike, const DoubleVector& expiryTerm,
											const AQLString& target, const DoubleVector& weight, const IntVector& sgn, const double solverEpsilon,
											const DoubleMatrix& forwardMat, const double forwardShiftValue, const DoubleMatrix& numeraireMat,
											const DoubleMatrix& atmMarketVol, const bool alphaFromAtmVol,
											AQLString& msg, const BoolMatrix *calibFlgMtx, const bool isLognormal)
{

	AQLString tmp_target = target;
	tmp_target.toUpper();

	//vol size is the row size of marketVol
	auto swapVolSize = marketVol.size();

	//tenor size is the column size of marketVol
	AQ_REQUIRE(marketVol.size() > 0 && marketVol[0].size() > 0, "Market Vol Matrix cannot be empty");
	auto tenorSize = marketVol[0][0].size();

	auto expirySize = expiryTerm.size();

	AQ_REQUIRE(swapVolSize == weight.size(), "Inconsistent Market Data: The number of Swap Vol Objects and Weights must match")

	AQ_REQUIRE(swapVolSize == sgn.size(), "Inconsistent Market Data: The number of Swap Vol Objects and Call/Put Sign Indicators must match")
	
	AQLString tmp_approxMethod = approxMethod;
	AQLStringVector stmpvec(tenorSize, tmp_approxMethod.toUpper());
	AQLStringMatrix gridApproxMethodMtx(expiryTerm.size(), stmpvec);

	if (calibFlgMtx)
	{
		if (calibFlgMtx->size() != expirySize || (*calibFlgMtx)[0].size() != tenorSize)
		{
			AQ_THROW("Inconsistent Market Data: The dimension of the calibration and volatility matrices must match")
		}
	}

	//
	double max_alpha = 3.;
	double min_alpha = 0.0001;
	double max_beta = 1.;
	double min_beta = 0.;
	double max_nu = 5.;
	double min_nu = 0.0001;
	double max_rho = 0.98;
	double min_rho = -0.98;

	double forward, numeraire;
	DoubleArray forward_calib, numeraire_calib, strike_calib, vol_calib, expiry_calib;//, x(limiter.getParamNum());
	DateVector dates;

	for (size_t j = 0; j < tenorSize; j++)
	{
		for (size_t i = 0; i < expirySize; i++)
		{
			if (calibFlgMtx && !(*calibFlgMtx)[i][j])
			{
				continue;
			}

			forward = forwardMat[i][j];
			forward += forwardShiftValue;

			numeraire = numeraireMat[i][j];

			forward = (std::fabs(forward) < eps_SABR) ? eps_SABR : forward;
			double max_alpha = fabs(forward) < 0.01 ? 0.01 / fabs(forward) * 5. : 5.;
			forward_calib = getConstantVector(forward, swapVolSize);
			numeraire_calib = getConstantVector(numeraire, swapVolSize);
			expiry_calib = getConstantVector(expiryTerm[i], swapVolSize);

			strike_calib.clear();
			vol_calib.clear();
			for (size_t k = 0; k < swapVolSize; k++)
			{
				vol_calib.push_back(marketVol[k][i][j]);
				strike_calib.push_back(marketStrike[k][i][j] + forwardShiftValue);
			}

			vector<double> initValue(4);
			initValue[0] = alphaMat[i][j];
			initValue[1] = betaMat[i][j];
			initValue[2] = nuMat[i][j];
			initValue[3] = rhoMat[i][j];

			AQ_THROW_IF( initValue[0] < min_alpha || initValue[0] > max_alpha, "Invalid Alpha Parameter: Must be between " + AQ_TO_STRING_FROM_DOUBLE(min_alpha) +" and " +  AQ_TO_STRING_FROM_DOUBLE(max_alpha) )
			AQ_THROW_IF( initValue[1] < min_beta || initValue[1] > max_beta, "Invalid Beta Parameter: Must be between " + AQ_TO_STRING_FROM_DOUBLE(min_beta) +" and " +  AQ_TO_STRING_FROM_DOUBLE(max_beta) )
			AQ_THROW_IF( initValue[2] < min_nu || initValue[2] > max_nu,	"Invalid Alpha Parameter: Must be between " + AQ_TO_STRING_FROM_DOUBLE(min_nu) +" and " +  AQ_TO_STRING_FROM_DOUBLE(max_nu) )
			AQ_THROW_IF( initValue[3] < min_rho || initValue[3] > max_rho, "Invalid Alpha Parameter: Must be between " + AQ_TO_STRING_FROM_DOUBLE(min_rho) +" and " +  AQ_TO_STRING_FROM_DOUBLE(max_rho) )
				
			//calibration
			AQLMathSABR* sabr = createSABR(gridApproxMethodMtx[i][j], alphaMat[i][j], betaMat[i][j], nuMat[i][j], rhoMat[i][j], isLognormal);

			double atmVol = alphaFromAtmVol ? atmMarketVol[i][j] : std::numeric_limits<double>::quiet_NaN();

			AQLMathLeastSquareSABRCostFunc* costfunc = new AQLMathLeastSquareSABRCostFunc(*sabr,
																						expiryTerm[i],
																						forward,
																						numeraire,
																						atmVol,
																						strike_calib,
																						weight,
																						sgn,
																						initValue,
																						calibFlg,
																						vol_calib,
																						tmp_target);

			// Create constraint
			AQLQuantLibConstraint* constraint = 0;
			AQLString tmp_approxMethod = gridApproxMethodMtx[i][j];
			tmp_approxMethod.toUpper();
			if (tmp_approxMethod == APPROXIMATION_ANTONOV)
			{
				constraint = new AQLMathLeastSquareSABRConstraint_Antonov(forward,
																		 initValue,
																		 calibFlg,
																		 max_alpha,
																		 min_alpha,
																		 max_beta,
																		 min_beta,
																		 max_nu,
																		 min_nu,
																		 max_rho,
																		 min_rho);
			}
			else
			{
				constraint = new AQLMathLeastSquareSABRConstraint(initValue,
																 calibFlg,
																 max_alpha,
																 min_alpha,
																 max_beta,
																 min_beta,
																 max_nu,
																 min_nu,
																 max_rho,
																 min_rho);
			}

			//
			vector<double> x_;
			if (!calibFlg[0])
			{
				if (!calibFlg[1])
				{
					if (!calibFlg[2])
					{
						if (!calibFlg[3])
						{
							AQ_THROW("Invalid Market Data: All calibration flags are set to false")
						}
						else x_.push_back(rhoMat[i][j]);
					}
					else
					{
						x_.push_back(nuMat[i][j]);
						if (calibFlg[3]) x_.push_back(rhoMat[i][j]);
					}
				}
				else
				{
					x_.push_back(betaMat[i][j]);
					if (calibFlg[2])
					{
						x_.push_back(nuMat[i][j]);
						if (calibFlg[3]) x_.push_back(rhoMat[i][j]);
					}
					else
					{
						if (calibFlg[3]) x_.push_back(rhoMat[i][j]);
					}
				}
			}
			else
			{
				x_.push_back(alphaMat[i][j]);
				if (!calibFlg[1])
				{
					if (calibFlg[2])
					{
						x_.push_back(nuMat[i][j]);
						if (calibFlg[3]) x_.push_back(rhoMat[i][j]);
					}
					else
					{
						if (calibFlg[3])  x_.push_back(rhoMat[i][j]);
					}
				}
				else
				{
					x_.push_back(betaMat[i][j]);
					if (calibFlg[2])
					{
						x_.push_back(nuMat[i][j]);
						if (calibFlg[3]) x_.push_back(rhoMat[i][j]);
					}
					else
					{
						if (calibFlg[3]) x_.push_back(rhoMat[i][j]);
					}
				}
			}

			AQLQuantLibArray x(x_.size());
			for (size_t k = 0; k < x_.size(); ++k) x[k] = x_[k];
			AQLQuantLibProblem* opt_problem = new AQLQuantLibProblem(*costfunc, *constraint, x);


			// Set Optimization Method
			AQLQuantLibOptimizationMethod* optMethod;
			AQLString tmp_calibMethod = calibMethod;
			tmp_calibMethod.toUpper();
			if (tmp_calibMethod == CALIB_NON_LINEAR_CONJUGATE_GRADIENT_METHOD)
			{
				optMethod = new AQLQuantLibConjugateGradient();
			}
			else if (tmp_calibMethod == CALIB_STEEPEST_DESCENT_METHOD)
			{
				optMethod = new AQLQuantLibSteepestDescent();
			}
			else if (tmp_calibMethod == CALIB_SIMPLEX_METHOD)
			{
				if (tmp_target == CALIB_TARGET_VOLATILITY)
				{
					optMethod = new AQLQuantLibSimplex(0.1);
				}
				else
				{
					optMethod = new AQLQuantLibSimplex(max_alpha);
				}
			}
			else if (tmp_calibMethod == CALIB_LEVENBERG_MARQUARDT_METHOD)
			{
				double levenbergMarquardtEpsfcn = 0.0000001;       // parameters specific for Levenberg-Marquardt
				double levenbergMarquardtXtol = 0.0000001;	   //
				double levenbergMarquardtGtol = 0.0000001;       //

				optMethod = new AQLQuantLibLevenbergMarquardt(levenbergMarquardtEpsfcn,
					levenbergMarquardtXtol,
					levenbergMarquardtGtol);

			}
			else
			{
				delete optMethod;
				delete opt_problem;
				delete constraint;
				delete costfunc;
				delete sabr;

				AQ_THROW("Invalid Optimization Method: " + calibMethod + " is not supported. Must be non-linear conjugate gradient, steepest decent, simplex or levenberg-marquardt method")
			}

			// Set EndCriteria
			unsigned int maxIteration = static_cast<unsigned int>(5000);
			unsigned int maxStationaryStateIteration = static_cast<unsigned int>(2500);


			double rootEpsilon = solverEpsilon;
			double functionEpsilon = solverEpsilon;
			double gradientNormEpsilon = solverEpsilon;

			if (maxStationaryStateIteration >= maxIteration)
			{
				delete optMethod;
				delete opt_problem;
				delete constraint;
				delete costfunc;
				delete sabr;

				AQ_THROW ("Solver Failed to Converge to a Solution")
			}

			AQLQuantLibEndCriteria* endCriteria = new AQLQuantLibEndCriteria(maxIteration,
																		   maxStationaryStateIteration,
																		   rootEpsilon,
																		   functionEpsilon,
																		   gradientNormEpsilon);

			// Optimization
			msg = "";
			try
			{
				try
				{
					AQLQuantLibEndCriteria::Type endCriteriaResult = optMethod->minimize(*opt_problem, *endCriteria);
				}
				catch (std::exception& e)
				{
					msg += AQLString(e.what());
					msg += AQLString("(");
					msg += AQLString(AQLModelUtilities::n2s(i).c_str());
					msg += AQLString(",");
					msg += AQLString(AQLModelUtilities::n2s(j).c_str());
					msg += AQLString(")");
					msg += AQLString(" : ");

				}
			}
			catch (AQLCoreError&e)
			{
				for (unsigned int k = 0; k < e.getSize(); k++) msg += AQLString(e.getMsg(e.getSize() - k - 1));
				msg += AQLString("(");
				msg += AQLString(AQLModelUtilities::n2s(i).c_str());
				msg += AQLString(",");
				msg += AQLString(AQLModelUtilities::n2s(j).c_str());
				msg += AQLString(")");
				msg += AQLString(" : ");
			}

			//QuantLib::Array xMinCalculated = opt_problem->currentValue();

			std::shared_ptr<AQLMathLeastSquareSABRCostFuncQlib> costfuncQlib = dynamic_pointer_cast<AQLMathLeastSquareSABRCostFuncQlib>(costfunc->getCostFunction());
			alphaMat[i][j] = costfuncQlib->getAlpha();
			betaMat[i][j] = costfuncQlib->getBeta();
			nuMat[i][j] = costfuncQlib->getNu();
			rhoMat[i][j] = costfuncQlib->getRho();

			delete endCriteria;
			delete optMethod;
			delete opt_problem;
			delete constraint;
			delete costfunc;
			delete sabr;
		}
	}

}


void 
AQLMathSwaptionVolUtility::calibrateSABRMatrix
( AQLDataInstance* dataInstance, const AQLString& curveSetID, const AQLString& alphaID, const AQLString& betaID, 
  const AQLString& nuID, const AQLString& rhoID, const AQLString& swapConvID, 
  const AQLString& capConvID, const AQLStringVector& swapVolID, AQLStringMatrix sabrLimiter, 
  AQLString target, const DoubleVector& weight, const AQLString& forwardID, const double forwardShiftValue )
{
    AQLPriceDataDayCount dc_act365(ACT_365_ISDA);
    upper(target);
    upper(sabrLimiter);
    const double eps = 0.000000001;

    AQLMathSABRLimiter limiter(sabrLimiter);

    AQ_THROW_IF(swapVolID.size() != weight.size(), "Invalid Market Data: Number of Swap Vol Objects and Weights does not match")

	matirixCheck(dataInstance, alphaID, betaID);
	matirixCheck(dataInstance, alphaID, nuID);
	matirixCheck(dataInstance, alphaID, rhoID);

    AQLObjectPool& objPool = dataInstance->getObjectPool();
    DoubleMatrix& alphaMat = dynamic_cast<AQLDataDoubleMatrix &>
        (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
    const AQLStringVector& expiry = dynamic_cast<const AQLDataStrings &>
        (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_EXPIRYSTRING,ISDEFINED).get()).get();
    const AQLStringVector& tenor = dynamic_cast<const AQLDataStrings &>
        (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_TENORSTRING,ISDEFINED).get()).get();
	const DoubleVector& expiryTerms = dynamic_cast<const AQLDataDoubles &>
        (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_EXPIRYVECTOR,ISDEFINED).get()).get();
	const DateVector& expiryDates = dynamic_cast<const AQLDataDates &>
        (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_EXPIRYDATEVECTOR,ISDEFINED).get()).get();
    DoubleMatrix& betaMat = dynamic_cast<AQLDataDoubleMatrix &>
        (objPool.getObject( betaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
    DoubleMatrix& nuMat = dynamic_cast<AQLDataDoubleMatrix &>
        (objPool.getObject( nuID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
    DoubleMatrix& rhoMat = dynamic_cast<AQLDataDoubleMatrix &>
        (objPool.getObject( rhoID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
	
	DoubleMatrix forwardMat;
	if(forwardID != AQLString("")) 
	{
		matirixCheck(dataInstance, alphaID, forwardID);
		forwardMat = dynamic_cast<AQLDataDoubleMatrix &>
			(objPool.getObject( forwardID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
	}

    vector<DoubleMatrix > marketVol, marketStk;
    for(size_t i=0; i<swapVolID.size(); i++)
    {
        matirixCheck(dataInstance, alphaID, swapVolID[i], true);
        marketVol.push_back(
        dynamic_cast<const AQLDataDoubleMatrix &> (objPool.getObject( swapVolID[i], ENCHKTYPE_ISDEFINED ).get().
        getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get() );

        marketStk.push_back(
        dynamic_cast<const AQLDataDoubleMatrix &> (objPool.getObject( swapVolID[i], ENCHKTYPE_ISDEFINED ).get().
        getData("SwaptionStrikeMatrix",ISDEFINED).get()).get() );
    }

    double forward, numeraire;
    DoubleArray forward_calib, numeraire_calib, strike_calib, vol_calib, expiry_calib, x(limiter.getParamNum());
    DateVector dates;
    AQLMathSABR_Hagan sabr;
	AQLObject curveEntity = dataInstance->getObjectPool().getObject( curveSetID, ENCHKTYPE_ISDEFINED ).get();
	AQLString curveID,convID,foreName,dfName;
	curveID = dynamic_cast<const AQLDataString& >(curveEntity.getData(CALIBRATION_DATA_CURVEID,ISDEFINED).get()).get();
    bool isMap = false;
	for(size_t j=0; j<tenor.size(); j++)
    {
		for(size_t i=0; i<expiry.size(); i++)
		{  
			if(forwardID == AQLString(""))
			{
				if( tenor[j]==AQLString("1M") || tenor[j]==AQLString("3M") || tenor[j]==AQLString("6M") )
				{
					convID = capConvID;
					foreName = dynamic_cast<const AQLDataString& >(curveEntity.getData(tenor[j]+AQLString("LiborCurveName"),ISDEFINED).get()).get();
					dfName = dynamic_cast<const AQLDataString& >(curveEntity.getData(CALIBRATION_DATA_DISCOUNTCURVENAME,ISDEFINED).get()).get();
				}
				else
				{
					convID = swapConvID;
					foreName = dynamic_cast<const AQLDataString& >(curveEntity.getData(AQLString("SwapRateCurveName"),ISDEFINED).get()).get();
					dfName = dynamic_cast<const AQLDataString& >(curveEntity.getData(CALIBRATION_DATA_DISCOUNTCURVENAME,ISDEFINED).get()).get();
				}
				forward = getForward(dataInstance, expiryDates[i], tenor[j], curveID, convID, foreName, dfName); forward = (forward > 0.0) ? forward : eps_SABR;
				numeraire = getNumeraire(dataInstance, expiryDates[i], tenor[j], curveID, convID, dfName);
			}
			else
			{
				forward = forwardMat[i][j];
				numeraire = 1.;
			}
			forward += forwardShiftValue;

            forward_calib = getConstantVector(forward,swapVolID.size());
            numeraire_calib = getConstantVector(numeraire,swapVolID.size());
			expiry_calib = getConstantVector(expiryTerms[i],swapVolID.size());

            strike_calib.clear();
            vol_calib.clear();
            for(size_t k=0; k<swapVolID.size(); k++)
            {                
                if( marketVol[k][i][j] >= eps )
                {
                    vol_calib.push_back(marketVol[k][i][j]);
                    strike_calib.push_back(marketStk[k][i][j] + forwardShiftValue);
                }
            }

            if( vol_calib.size() < limiter.getParamNum() ) break;
			
            //calibration
			sabr.setSABRParam(alphaMat[i][j],betaMat[i][j],nuMat[i][j],rhoMat[i][j]);
            AQLMathSABRCalibrator calibrator(sabr, strike_calib, vol_calib, forward_calib, expiry_calib, 
                numeraire_calib, weight, target, isMap);
            calibrator.setLimiter( limiter );
            limiter.getArgument( sabr, x, isMap );
            NL2SOL solver( calibrator );
            solver.tryToSolve( x );
            //set param
            sabr = limiter.getSABR( sabr, x, isMap );
            alphaMat[i][j] = sabr.getAlpha();
            betaMat[i][j] = sabr.getBeta();
            nuMat[i][j] = sabr.getNu();
            rhoMat[i][j] = sabr.getRho();
        }
    }

    AQLDataHolder* dh;
    AQLDataDoubleMatrix mat_att;
    dh = &dataInstance->getObjectPool().getObject(alphaID ,ENCHKTYPE_NOCHECK).getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED);   
    mat_att = dynamic_cast<AQLDataDoubleMatrix &>(dh->get());
    mat_att.set( alphaMat );

    dh = &dataInstance->getObjectPool().getObject(betaID ,ENCHKTYPE_NOCHECK).getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED);   
    mat_att = dynamic_cast<AQLDataDoubleMatrix &>(dh->get());
    mat_att.set( betaMat );

    dh = &dataInstance->getObjectPool().getObject(nuID ,ENCHKTYPE_NOCHECK).getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED);   
    mat_att = dynamic_cast<AQLDataDoubleMatrix &>(dh->get());
    mat_att.set( nuMat );

    dh = &dataInstance->getObjectPool().getObject(rhoID ,ENCHKTYPE_NOCHECK).getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED);   
    mat_att = dynamic_cast<AQLDataDoubleMatrix &>(dh->get());
    mat_att.set( rhoMat );
}

void
AQLMathSwaptionVolUtility::calibrateSABRMatrixCapFloor
(AQLDataInstance* dataInstance, const AQLString& approxMethod, const std::vector<bool>& calibFlg, const AQLString& calibMethod, const AQLString& curveSetID, const AQLString& alphaID, const AQLString& betaID,
	const AQLString& nuID, const AQLString& rhoID, const AQLString& swapConvID,
	const AQLString& capConvID, const AQLStringVector& swapVolID, /*AQLStringMatrix sabrLimiter,*/
	const AQLString& target, const DoubleVector& weight, const IntVector& sgn_in, const double forwardShiftValue,
    AQLString& msg, const BoolMatrix *calibFlgMtx)
{
	AQLString tmp_target = target;
	tmp_target.toUpper();
	AQ_THROW_IF(tmp_target != "PREMIUM", "Only premium is allowed as a calibration target.")
	
	AQ_THROW_IF(swapVolID.size() != weight.size(), "Invalid Market Data: Number of swap volobjects and weights does not match")

	IntVector sgn(swapVolID.size(), static_cast<int>(OPTION_SIGN_DUMMY));
	if (sgn_in.size() > 0)
	{
		sgn = sgn_in;
		AQ_THROW_IF(swapVolID.size() != sgn.size() ,"Invalid Market Data: Number of swap volobjects and call/put sign indicators do not match")
	}

	matirixCheck(dataInstance, alphaID, betaID);
	matirixCheck(dataInstance, alphaID, nuID);
	matirixCheck(dataInstance, alphaID, rhoID);

	AQLObjectPool& objPool = dataInstance->getObjectPool();
	DoubleMatrix& alphaMat = dynamic_cast<AQLDataDoubleMatrix &>
		(objPool.getObject(alphaID, ENCHKTYPE_ISDEFINED).get().getData(PRICING_DATA_SWAPTIONMATRIX, ISDEFINED).get()).get();
	const AQLStringVector& expiry = dynamic_cast<const AQLDataStrings &>
		(objPool.getObject(alphaID, ENCHKTYPE_ISDEFINED).get().getData(PRICING_DATA_EXPIRYSTRING, ISDEFINED).get()).get();
	const AQLStringVector& tenor = dynamic_cast<const AQLDataStrings &>
		(objPool.getObject(alphaID, ENCHKTYPE_ISDEFINED).get().getData(PRICING_DATA_TENORSTRING, ISDEFINED).get()).get();
	DoubleMatrix& betaMat = dynamic_cast<AQLDataDoubleMatrix &>
		(objPool.getObject(betaID, ENCHKTYPE_ISDEFINED).get().getData(PRICING_DATA_SWAPTIONMATRIX, ISDEFINED).get()).get();
	DoubleMatrix& nuMat = dynamic_cast<AQLDataDoubleMatrix &>
		(objPool.getObject(nuID, ENCHKTYPE_ISDEFINED).get().getData(PRICING_DATA_SWAPTIONMATRIX, ISDEFINED).get()).get();
	DoubleMatrix& rhoMat = dynamic_cast<AQLDataDoubleMatrix &>
		(objPool.getObject(rhoID, ENCHKTYPE_ISDEFINED).get().getData(PRICING_DATA_SWAPTIONMATRIX, ISDEFINED).get()).get();
	const AQLObject& object = dataInstance->getObjectPool().getObject(capConvID, ENCHKTYPE_ISDEFINED).get();
	const AQLPriceDataCalendar& cal =	dynamic_cast<const AQLPriceDataCalendar& >(object.getData(CALIBRATION_DATA_CALENDAR, ISDEFINED).get());
	const AQLPriceDataCalendar& cal2 = dynamic_cast<const AQLPriceDataCalendar& >(object.getData(PRICING_DATA_FIXINGCALENDAR, ISDEFINED).get());
	const AQLPriceDataSlidingRule& sr = dynamic_cast<const AQLPriceDataSlidingRule& >(object.getData(CALIBRATION_DATA_SLIDINGRULE, ISDEFINED).get());
	const AQLDate& asOfDate =	dynamic_cast<const AQLDataDate& >(object.getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL).get()).get();
	const AQLString& spotlag = dynamic_cast<const AQLDataString& >(object.getData(CURVEINPUT_SPOTLAG, ISDEFINED).get()).get();
	const AQLPriceDataDayCount& daycount = dynamic_cast<const AQLPriceDataDayCount& >(object.getData(IR_CALIBRATION_DATA_DAYCOUNT, ISDEFINED).get());

	if (calibFlgMtx)
	{
		if (calibFlgMtx->size() != expiry.size() || (*calibFlgMtx)[0].size() != tenor.size())
		{
			throw AQLCoreInvalidData("Calibration flag matrix does not match capfloor premium matrix!", __FILE__, __LINE__);
		}
	}

	vector<DoubleMatrix > marketVol, marketStk, marketSign;
	vector<AQLStringVector > premiumTerms;
	for (size_t i = 0; i<swapVolID.size(); i++)
	{
		marketVol.push_back(
			dynamic_cast<const AQLDataDoubleMatrix &> (objPool.getObject(swapVolID[i], ENCHKTYPE_ISDEFINED).get().
				getData(PRICING_DATA_SWAPTIONMATRIX, ISDEFINED).get()).get());
		marketStk.push_back(
			dynamic_cast<const AQLDataDoubleMatrix &> (objPool.getObject(swapVolID[i], ENCHKTYPE_ISDEFINED).get().
				getData(PRICING_DATA_SWAPTIONSTRIKEMATRIX, ISDEFINED).get()).get());
		marketSign.push_back(
			dynamic_cast<const AQLDataDoubleMatrix &> (objPool.getObject(swapVolID[i], ENCHKTYPE_ISDEFINED).get().
				getData(PRICING_DATA_SWAPTIONSIGNMATRIX, ISDEFINED).get()).get());
		premiumTerms.push_back(
			dynamic_cast<const AQLDataStrings &> (objPool.getObject(swapVolID[i], ENCHKTYPE_ISDEFINED).get().
				getData(PRICING_DATA_EXPIRYSTRING, ISDEFINED).get()).get());
	}

	double max_alpha = 3.;
	const double min_alpha = 0.0001, max_beta = 1., min_beta = 0., max_nu = 5., min_nu = 0.0001, max_rho = 0.98, min_rho = -0.98;

	DoubleMatrix alphaMat_calib(alphaMat), betaMat_calib(betaMat), nuMat_calib(nuMat), rhoMat_calib(rhoMat);

	int y, m, d, w;
	DoubleVector tenorVec;
	for (size_t j = 0; j < tenor.size(); ++j)
	{
		AQLMathDateCalculations::termStrtoYMDW(tenor[j], y, m, d, w);
		tenorVec.push_back(y + m / 12.);
	}
		
	for (size_t j = 0; j<tenor.size(); j++)
	{
		// get caplet/floorlet tenor
		const AQLString tenorPoint = tenor[j];
		const double tenorPoint_d = AQLMathSwaptionVolUtility::getTenorPoint(tenorPoint);
		AQLString CapFloorletFrequency;
		int tenor_num;
		if (tenorPoint == "3M")
		{
			CapFloorletFrequency = QUARTERLY;
			tenor_num = 3;
		}
		else if (tenorPoint == "6M")
		{
			CapFloorletFrequency = SEMI_ANNUAL;
			tenor_num = 6;
		}
		else
		{
			AQ_THROW("CapletFloorlet Frequency must be 3M (Quarterly) or 6M (Semi-Annual)")
		}

		// Set up AQLMathYieldCurve for fwdrate, numeraire
		AQLObject curveEntity;
		AQLString curveID, convID, foreName, dfName;
		curveEntity = dataInstance->getObjectPool().getObject(curveSetID, ENCHKTYPE_ISDEFINED).get();
		const AQLString curveid = dynamic_cast<const AQLDataString&>(curveEntity.getData(CALIBRATION_DATA_CURVEID, ISDEFINED).get()).get();
		AQLMathYieldCurve& yc = AQLMathCurveFuncUtility::getYieldCurveForCurveID(dataInstance, curveid);
		yc.setInterpolation(FN_SPLINEINTERPOLATION_STR);
		yc.getDayCount().setDayCount(daycount.convertToString());
		yc.getSlidingRule().convertFromString(NO_CH);
		AQLMathCurveFuncUtility::setCalendarForCurveID(yc, cal.convertToString());
		yc.getFrequency().convertFromString(CapFloorletFrequency);
		AQLPriceDataConvention conv(ACT_ACT, CONT), conv2(yc.getDayCount().getDayCount(), CONT);
		yc.getFrequency().set("SIMPLE");
		// set up slidingrule and daycount for numeraire
		AQLMathYieldCurve& yc_nu = yc;
		yc_nu.getSlidingRule() = sr;
		yc_nu.getDayCount().setDayCount(ACT_ACT);

		// get curvename
		if (tenor[j] == AQLString("1M") || tenor[j] == AQLString("3M") || tenor[j] == AQLString("6M"))
		{
			convID = capConvID;
			foreName = dynamic_cast<const AQLDataString&>(curveEntity.getData(tenor[j] + AQLString("LiborCurveName"), ISDEFINED).get()).get();
			dfName = dynamic_cast<const AQLDataString&>(curveEntity.getData(CALIBRATION_DATA_DISCOUNTCURVENAME, ISDEFINED).get()).get();
		}
		else
		{
			convID = swapConvID;
			foreName = dynamic_cast<const AQLDataString&>(curveEntity.getData(AQLString("SwapRateCurveName"), ISDEFINED).get()).get();
			dfName = dynamic_cast<const AQLDataString&>(curveEntity.getData(CALIBRATION_DATA_DISCOUNTCURVENAME, ISDEFINED).get()).get();
		}

		// check SABR parameters term
		const size_t num_diffterm = expiry.size() - premiumTerms[0].size();; // this parameter is used for calibrating some parameter grids simultaneously which do not exist in premium grid. ex) premium grid = 1Y, parameter grids = 3M, 6M, 1Y  
		AQLStringVector paramsterm_month, paramsterm_month_calib; //  the second parmeter is used for changing expiry of SABR parameters into "expiry - tenor";
		int paramsterm, paramsterm_calib;
		for (size_t i = 0; i < expiry.size(); i++)
		{
			AQLMathDateCalculations::termStrtoYMDW(expiry[i], y, m, d, w);
			paramsterm = 12 * y + m;
			paramsterm_month.push_back(AQLString(paramsterm) + "M");
			
			if (i < num_diffterm)
			{
				paramsterm_calib = 12 * y + m;
			}
			else
			{
				AQ_THROW_IF(expiry[i] != premiumTerms[0][i - num_diffterm], "Terms of SABR parameters must be same as terms of premiums after the minimum premium term.")
				AQ_THROW_IF(paramsterm % tenor_num != 0, "Terms of SABR parameters should be multiples of " + tenorPoint )
				paramsterm_calib = 12 * y + m - tenor_num;
			}
			paramsterm_month_calib.push_back(AQLString(paramsterm_calib) + "M");
		}

		// check whether expiry of SABR parameters include all of the "premiumTerms - tenor"
		// Also, we can input expiry of SABR parameters which is shorter than the first "premiumTerms - tenor".
		
		for (size_t i = 0; i < swapVolID.size(); i++)
		{
			for (size_t k = 0; k < premiumTerms[i].size() - 1; k++)
			{
				AQLMathDateCalculations::termStrtoYMDW(premiumTerms[i][k], y, m, d, w);
				int premiumterm = 12 * y + m;
				const AQLString premiumterm_month = AQLString(premiumterm) + "M";
				if (num_diffterm >= 0)
				{
					if (premiumterm_month != paramsterm_month[k + num_diffterm])
						throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
				}
				else
				{
					AQ_THROW("Inconsistent Data: SABR terms data and premium data are inconsistent")
				}
			}
		}

		// These objects are used for converting term. ex) 1Y, 2Y,... <-> 0.75Y, 1.75Y, ...
		AQLMathSwaptionMatrix alphaMat_temp({ {1} ,{1} }, {1}, tenorVec);
		AQLMathSwaptionMatrix betaMat_temp({ {1} ,{1} }, {1}, tenorVec);
		AQLMathSwaptionMatrix nuMat_temp({ {1} ,{1} }, {1}, tenorVec);
		AQLMathSwaptionMatrix rhoMat_temp({ {1} ,{1} }, {1}, tenorVec);

		// create expiry of calibration target
		AQLStringVector target_expiry;
		for (size_t i = num_diffterm; i < paramsterm_month_calib.size(); i++)
			target_expiry.push_back(paramsterm_month_calib[i]);

		double forward, numeraire;
		DoubleArray strike_calib, vol_calib;
		IntArray sign_calib;
		DoubleVector expiryterms_output, expiryterms_calib_output;
		for (size_t i = 0; i<target_expiry.size(); i++)
		{
			if (calibFlgMtx && !(*calibFlgMtx)[i][j])
			{
				continue;
			}

			// Set up information on expiry term, fwdrate, numeraire
			const AQLString roll_conv = "TRUE";
			const AQLDate tmpstart = AQLMathDateCalculations::getDate(asOfDate, spotlag, sr, &cal, true, &roll_conv);
			AQLDate endpay(tmpstart);
			int y, m, d, w;
			AQLMathDateCalculations::termStrtoYMDW(premiumTerms[0][i], y, m, d, w);
			endpay.addYears(y);
			endpay.addMonths(m);
			endpay.addDays(d);
			int	roll = tmpstart.dayOfMonth();
			//roll set
			if (roll > LOOKUP_TABLE_NUMBER_OF_DAYS_IN_A_MONTH_OR_YEAR[IS_LEAP_YEAR(endpay.yearOfEra())][0][endpay.monthOfYear() - 1])
				endpay.setDay(LOOKUP_TABLE_NUMBER_OF_DAYS_IN_A_MONTH_OR_YEAR[IS_LEAP_YEAR(endpay.yearOfEra())][0][endpay.monthOfYear() - 1]);
			else endpay.setDay(roll);
			//paymentvec;
			DateVector payVec;
			AQLMathDateCalculations::generateSchedule(tmpstart, endpay, CapFloorletFrequency, true, NULL, NULL, &roll, payVec, &sr, &cal);
			//slidingrule 
			AQLPriceDataSlidingRule sr2;
			sr2.convertFromString(PRE);
			//fixingVector
			const size_t N = payVec.size();
			DateVector fixVec(N);
			for (int k = 0; k < N; k++)
			{
				fixVec[k] = AQLMathDateCalculations::getDate(payVec[k], spotlag, sr2, &cal2, false);
			}

			unsigned int pos = 0;
			AQLAlgorithm::locate<DateVector, AQLDate>(fixVec, asOfDate, N - 1, pos);
			if (N - 1 == static_cast<int>(pos))
			{
				AQLString msg = "valuedate : " + asOfDate.stringWithFormat("YYYYMMDD") + " is not support for this option pricing";
				throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			}

			// create forward rate, numeraire, expiryterm for caplet/floorlet
			DoubleVector forwardMat_capfloorlet, numeraireMat_capfloorlet, expiryTerms_capfloorlet;
			for (size_t k = pos; k < N - 1; k++)
			{
				// forwardrate
				yc.setCurveType(foreName);
				forwardMat_capfloorlet.push_back(yc.getZeroRate(payVec[k], payVec[k + 1], true) + forwardShiftValue);
				forwardMat_capfloorlet[k] = (forwardMat_capfloorlet[k] < eps_SABR) ? eps_SABR : forwardMat_capfloorlet[k];
				// numeraire
				double del = conv2.getTerm(payVec[k], payVec[k + 1]);
				yc_nu.setCurveType(dfName);
				numeraireMat_capfloorlet.push_back(yc_nu.getDF(asOfDate, payVec[k + 1])*del);
				// expiryterm
				expiryTerms_capfloorlet.push_back(conv.getTerm(asOfDate, fixVec[k]));
			}

			// create expiry terms for cap/floor
			DoubleVector expiryterms_capfloor;
			AQLDate expiryDate;
			AQLString daycount_expiry(AC_365I);
			for (size_t k = 0; k < i + num_diffterm + 1; k++)
			{
				expiryDate = AQLMathDateCalculations::getDate(asOfDate, expiry[k], sr, &cal2, true);
				expiryterms_capfloor.push_back(AQLMathDateUtilities::getTerm(asOfDate, expiryDate, daycount_expiry, true));
			}
			// create expiry terms for calibration target cap/floor
			DoubleVector expiryterms_capfloor_calib;
			for (size_t k = 0; k < num_diffterm; k++)
			{
				expiryterms_capfloor_calib.push_back(expiryterms_capfloor[k]);
			}
			for (size_t k = num_diffterm; k < i + num_diffterm + 1; k++)
			{
				AQLMathDateCalculations::termStrtoYMDW(expiry[k], y, m, d, w);
				int term_pos = (12 * y + m) / tenor_num - 1;
				expiryterms_capfloor_calib.push_back(expiryTerms_capfloorlet[term_pos - 1]);
			}

			// get parameters at (expiry - tenor) ex) 1Y -> 0.75Y
			alphaMat_temp.mat = alphaMat;
			betaMat_temp.mat = betaMat;
			nuMat_temp.mat = nuMat;
			rhoMat_temp.mat = rhoMat;
			alphaMat_temp.expiryVec = betaMat_temp.expiryVec = nuMat_temp.expiryVec = rhoMat_temp.expiryVec = expiryterms_capfloor;
			if (i == 0 && num_diffterm > 0)
			{
				for (size_t k = 0; k < num_diffterm + 1; k++)
				{
					alphaMat_calib[k][j] = alphaMat_temp.lookUpMatrix(expiryterms_capfloor_calib[k], tenorPoint_d);
					betaMat_calib[k][j] = betaMat_temp.lookUpMatrix(expiryterms_capfloor_calib[k], tenorPoint_d);
					nuMat_calib[k][j] = nuMat_temp.lookUpMatrix(expiryterms_capfloor_calib[k], tenorPoint_d);
					rhoMat_calib[k][j] = rhoMat_temp.lookUpMatrix(expiryterms_capfloor_calib[k], tenorPoint_d);
					// expiry terms for SABR parametes output
					expiryterms_output.push_back(expiryterms_capfloor[k]);
					expiryterms_calib_output.push_back(expiryterms_capfloor_calib[k]);
				}
			}
			else
			{
				alphaMat_calib[i + num_diffterm][j] = alphaMat_temp.lookUpMatrix(expiryterms_capfloor_calib[i + num_diffterm], tenorPoint_d);
				betaMat_calib[i + num_diffterm][j] = betaMat_temp.lookUpMatrix(expiryterms_capfloor_calib[i + num_diffterm], tenorPoint_d);
				nuMat_calib[i + num_diffterm][j] = nuMat_temp.lookUpMatrix(expiryterms_capfloor_calib[i + num_diffterm], tenorPoint_d);
				rhoMat_calib[i + num_diffterm][j] = rhoMat_temp.lookUpMatrix(expiryterms_capfloor_calib[i + num_diffterm], tenorPoint_d);
				// expiry terms for SABR parametes output
				expiryterms_output.push_back(expiryterms_capfloor[i + num_diffterm]);
				expiryterms_calib_output.push_back(expiryterms_capfloor_calib[i + num_diffterm]);
			}

			AQLMathDateCalculations::termStrtoYMDW(target_expiry[i], y, m, d, w);
			size_t num_capfloorlet = (12 * y + m) / tenor_num;
			forward = forwardMat_capfloorlet[num_capfloorlet - 1];
			max_alpha = fabs(forward) < 0.01 ? 0.01 / fabs(forward) * 5. : 5.;
			
			// get premium target and strike
			strike_calib.clear();
			vol_calib.clear();
			sign_calib.clear();
			for (size_t k = 0; k<swapVolID.size(); k++)
			{
				vol_calib.push_back(marketVol[k][i][j]);
				strike_calib.push_back(marketStk[k][i][j] + forwardShiftValue);
				sign_calib.push_back(marketSign[k][i][j] != OPTION_SIGN_DUMMY ? static_cast<int>(round(marketSign[k][i][j])) : sgn[k]);
			}
			
			// set initial SABR parameters
			vector<double> initValue(4);
			initValue[0] = alphaMat_calib[i + num_diffterm][j];
			initValue[1] = betaMat_calib[i + num_diffterm][j];
			initValue[2] = nuMat_calib[i + num_diffterm][j];
			initValue[3] = rhoMat_calib[i + num_diffterm][j];
			msg = AQLString("alpha is out of range.");
			if (initValue[0] < min_alpha || initValue[0] > max_alpha) throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			msg = AQLString("beta is out of range.");
			if (initValue[1] < min_beta || initValue[1] > max_beta) throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			msg = AQLString("nu is out of range.");
			if (initValue[2] < min_nu || initValue[2] > max_nu) throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			msg = AQLString("rho is out of range.");
			if (initValue[3] < min_rho || initValue[3] > max_rho) throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);

			// Create CostFunc
			AQLString tmp_approxMethod = approxMethod;
			tmp_approxMethod.toUpper();
			IntVector param_pos(2);
			param_pos[0] = i + num_diffterm;
			param_pos[1] = j;
			AQLMathLeastSquareSABRCapFloorCostFunc* costfunc = new AQLMathLeastSquareSABRCapFloorCostFunc(
				alphaMat_calib,
				betaMat_calib,
				nuMat_calib,
				rhoMat_calib,
				param_pos,
				tmp_approxMethod,
				tenorPoint,
				expiryterms_capfloor_calib,
				expiryTerms_capfloorlet,
				tenorVec,
				forwardMat_capfloorlet,
				numeraireMat_capfloorlet,
				strike_calib,
				weight,
				sign_calib,
				initValue,
				calibFlg,
				vol_calib,
				tmp_target,
				num_capfloorlet,
				num_diffterm);

			// Create constraint
			AQLQuantLibConstraint* constraint = 0;
			if (tmp_approxMethod == APPROXIMATION_ANTONOV)
			{
				constraint = new AQLMathLeastSquareSABRConstraint_Antonov(forward,
																		 initValue,
																		 calibFlg,
																		 max_alpha,
																		 min_alpha,
																		 max_beta,
																		 min_beta,
																		 max_nu,
																		 min_nu,
																		 max_rho,
																		 min_rho);
			}
			else
			{
				constraint = new AQLMathLeastSquareSABRConstraint(initValue,
																 calibFlg,
																 max_alpha,
																 min_alpha,
																 max_beta,
																 min_beta,
																 max_nu,
																 min_nu,
																 max_rho,
																 min_rho);
			}

			vector<double> x_;
			if (!calibFlg[0])
			{
				if (!calibFlg[1])
				{
					if (!calibFlg[2])
					{
						if (!calibFlg[3])
						{
							throw AQLCoreInvalidData("All calibration flags have been set to false", __FILE__, __LINE__);
						}
						else x_.push_back(rhoMat_calib[i + num_diffterm][j]);
					}
					else
					{
						x_.push_back(nuMat_calib[i + num_diffterm][j]);
						if (calibFlg[3]) x_.push_back(rhoMat_calib[i + num_diffterm][j]);
					}
				}
				else
				{
					x_.push_back(betaMat_calib[i + num_diffterm][j]);
					if (calibFlg[2])
					{
						x_.push_back(nuMat_calib[i + num_diffterm][j]);
						if (calibFlg[3]) x_.push_back(rhoMat_calib[i + num_diffterm][j]);
					}
					else
					{
						if (calibFlg[3]) x_.push_back(rhoMat_calib[i + num_diffterm][j]);
					}
				}
			}
			else
			{
				x_.push_back(alphaMat_calib[i + num_diffterm][j]);
				if (!calibFlg[1])
				{
					if (calibFlg[2])
					{
						x_.push_back(nuMat_calib[i + num_diffterm][j]);
						if (calibFlg[3]) x_.push_back(rhoMat_calib[i + num_diffterm][j]);
					}
					else
					{
						if (calibFlg[3])  x_.push_back(rhoMat_calib[i + num_diffterm][j]);
					}
				}
				else
				{
					x_.push_back(betaMat_calib[i + num_diffterm][j]);
					if (calibFlg[2])
					{
						x_.push_back(nuMat_calib[i + num_diffterm][j]);
						if (calibFlg[3]) x_.push_back(rhoMat_calib[i + num_diffterm][j]);
					}
					else
					{
						if (calibFlg[3]) x_.push_back(rhoMat_calib[i + num_diffterm][j]);
					}
				}
			}

			AQLQuantLibArray x(x_.size());
			for (size_t k = 0; k < x_.size(); ++k) x[k] = x_[k];
			AQLQuantLibProblem* opt_problem = new AQLQuantLibProblem(*costfunc, *constraint, x);

			// Set Optimization Method
			AQLQuantLibOptimizationMethod* optMethod;
			AQLString tmp_calibMethod = calibMethod;
			tmp_calibMethod.toUpper();
			if (tmp_calibMethod == CALIB_NON_LINEAR_CONJUGATE_GRADIENT_METHOD)
			{
				optMethod = new AQLQuantLibConjugateGradient();
			}
			else if (tmp_calibMethod == CALIB_STEEPEST_DESCENT_METHOD)
			{
				optMethod = new AQLQuantLibSteepestDescent();
			}
			else if (tmp_calibMethod == CALIB_SIMPLEX_METHOD)
			{
				if (tmp_target == CALIB_TARGET_VOLATILITY)
				{
					optMethod = new AQLQuantLibSimplex(0.1);
				}
				else
				{
					optMethod = new AQLQuantLibSimplex(max_alpha);
				}
			}
			else if (tmp_calibMethod == CALIB_LEVENBERG_MARQUARDT_METHOD)
			{
				double levenbergMarquardtEpsfcn = 0.0000001;       // parameters specific for Levenberg-Marquardt
				double levenbergMarquardtXtol = 0.0000001;	   //
				double levenbergMarquardtGtol = 0.0000001;       //

				optMethod = new AQLQuantLibLevenbergMarquardt(levenbergMarquardtEpsfcn,
					levenbergMarquardtXtol,
					levenbergMarquardtGtol);
			}
			else
			{
				delete optMethod;
				delete opt_problem;
				delete constraint;
				delete costfunc;

				AQ_THROW("Optimization Method: " + calibMethod + " not supported")
			}

			// Set EndCriteria
			unsigned int maxIteration = static_cast<unsigned int>(5000);
			unsigned int maxStationaryStateIteration = static_cast<unsigned int>(2500);
			double rootEpsilon = 0.0000001;
			double functionEpsilon = 0.0000001;
			double gradientNormEpsilon = 0.0000001;
			if (maxStationaryStateIteration >= maxIteration)
			{
				delete optMethod;
				delete opt_problem;
				delete constraint;
				delete costfunc;

				AQ_THROW("Solver failed to converge to a solution")
			}
			AQLQuantLibEndCriteria* endCriteria = new AQLQuantLibEndCriteria(maxIteration,
				maxStationaryStateIteration,
				rootEpsilon,
				functionEpsilon,
				gradientNormEpsilon);

			// Optimization
			msg = "";
			try
			{
				try
				{
					AQLQuantLibEndCriteria::Type endCriteriaResult = optMethod->minimize(*opt_problem, *endCriteria);
				}
				catch (std::exception& e)
				{
					msg += AQLString(e.what());
					msg += AQLString("(");
					msg += AQLString(AQLModelUtilities::n2s(i).c_str());
					msg += AQLString(",");
					msg += AQLString(AQLModelUtilities::n2s(j).c_str());
					msg += AQLString(")");
					msg += AQLString(" : ");
				}
			}
			catch (AQLCoreError& e)
			{
				for (unsigned int k = 0; k < e.getSize(); k++) msg += AQLString(e.getMsg(e.getSize() - k - 1));
				msg += AQLString("(");
				msg += AQLString(AQLModelUtilities::n2s(i).c_str());
				msg += AQLString(",");
				msg += AQLString(AQLModelUtilities::n2s(j).c_str());
				msg += AQLString(")");
				msg += AQLString(" : ");
			}

			std::shared_ptr<AQLMathLeastSquareSABRCapFloorCostFuncQlib> costfuncQlib = dynamic_pointer_cast<AQLMathLeastSquareSABRCapFloorCostFuncQlib>(costfunc->getCostFunction());
			alphaMat_calib[i + num_diffterm][j] = costfuncQlib->getAlpha_capfloor(i + num_diffterm, j);
			betaMat_calib[i + num_diffterm][j] = costfuncQlib->getBeta_capfloor(i + num_diffterm, j);
			nuMat_calib[i + num_diffterm][j] = costfuncQlib->getNu_capfloor(i + num_diffterm, j);
			rhoMat_calib[i + num_diffterm][j] = costfuncQlib->getRho_capfloor(i + num_diffterm, j);
			// add parameters of non-calibration-target terms(e.g 3M, 6M) into calibrated SABR parameters matrix
			if (i == 0 && num_diffterm != 0)
			{
				for (size_t k = 0; k < num_diffterm; k++)
				{
					alphaMat_calib[i + k][j] = costfuncQlib->getAlpha_capfloor(i + k, j);
					betaMat_calib[i + k][j] = costfuncQlib->getBeta_capfloor(i + k, j);
					nuMat_calib[i + k][j] = costfuncQlib->getNu_capfloor(i + k, j);
					rhoMat_calib[i + k][j] = costfuncQlib->getRho_capfloor(i + k, j);
				}
			}
			delete endCriteria;
			delete optMethod;
			delete opt_problem;
			delete constraint;
			delete costfunc;
		}

		alphaMat_temp.mat = alphaMat_calib;
		betaMat_temp.mat = betaMat_calib;
		nuMat_temp.mat = nuMat_calib;
		rhoMat_temp.mat = rhoMat_calib;
		alphaMat_temp.expiryVec = betaMat_temp.expiryVec = nuMat_temp.expiryVec = rhoMat_temp.expiryVec = expiryterms_calib_output;
		for (size_t i = 0; i < expiryterms_calib_output.size(); i++)
		{
			if(calibFlg[0])
				alphaMat[i][j] = alphaMat_temp.lookUpMatrix(expiryterms_output[i], tenorPoint_d);
			if (calibFlg[1])
				betaMat[i][j] = betaMat_temp.lookUpMatrix(expiryterms_output[i], tenorPoint_d);
			if (calibFlg[2])
				nuMat[i][j] = nuMat_temp.lookUpMatrix(expiryterms_output[i], tenorPoint_d);
			if (calibFlg[3])
				rhoMat[i][j] = rhoMat_temp.lookUpMatrix(expiryterms_output[i], tenorPoint_d);
		}
	}
	
	// calibrated paramameters on the same grid as the input parameters grid. ex) 1Y, 2Y ,,,
	AQLDataHolder* dh;
	AQLDataDoubleMatrix mat_att;
	dh = &dataInstance->getObjectPool().getObject(alphaID, ENCHKTYPE_NOCHECK).getData(PRICING_DATA_SWAPTIONMATRIX, ISDEFINED);
	mat_att = dynamic_cast<AQLDataDoubleMatrix &>(dh->get());
	mat_att.set(alphaMat);

	dh = &dataInstance->getObjectPool().getObject(betaID, ENCHKTYPE_NOCHECK).getData(PRICING_DATA_SWAPTIONMATRIX, ISDEFINED);
	mat_att = dynamic_cast<AQLDataDoubleMatrix &>(dh->get());
	mat_att.set(betaMat);

	dh = &dataInstance->getObjectPool().getObject(nuID, ENCHKTYPE_NOCHECK).getData(PRICING_DATA_SWAPTIONMATRIX, ISDEFINED);
	mat_att = dynamic_cast<AQLDataDoubleMatrix &>(dh->get());
	mat_att.set(nuMat);

	dh = &dataInstance->getObjectPool().getObject(rhoID, ENCHKTYPE_NOCHECK).getData(PRICING_DATA_SWAPTIONMATRIX, ISDEFINED);
	mat_att = dynamic_cast<AQLDataDoubleMatrix &>(dh->get());
	mat_att.set(rhoMat);
}

DoubleArray 
AQLMathSwaptionVolUtility::calibrateSABR
( const AQLString& approxMethod, const std::vector<bool>& calibFlg, const AQLString& calibMethod, 
  double alpha, double beta, double nu, double rho, double forward, AQLString expiryPoint,
  double numeraire, const DoubleArray& strikes, const DoubleArray& vols, 
  /*AQLStringMatrix sabrLimiter,*/ const AQLString& target, const DoubleVector& weight, const IntVector& sgn )
{
	AQLString tmp_target = target;
	tmp_target.toUpper();

	double expiry = AQLMathSwaptionVolUtility::getTenorPoint(expiryPoint);
	
	AQ_THROW_IF( vols.size() != weight.size() || vols.size() != strikes.size(), "Inconsistent Market Data: Number of Vols, Weights and Strike must match")

	forward = (std::fabs(forward) < eps_SABR) ? eps_SABR : forward;
	double max_alpha = fabs(forward) < 0.01 ? 0.01 /  fabs(forward) * 5. : 5.;
	double min_alpha = 0.0001;
	double max_beta = 1.;
	double min_beta = 0.;
	double max_nu = 5.;
	double min_nu    = 0.001;
	double max_rho = 0.98;
	double min_rho = -0.98;
	
	vector<double> initValue(4);
	initValue[0] = alpha;
	initValue[1] = beta;
	initValue[2] = nu;
	initValue[3] = rho;

	AQ_THROW_IF( alpha < min_alpha || alpha > max_alpha, "Invalid Alpha Parameter: Must be between " + AQ_TO_STRING_FROM_DOUBLE(min_alpha) +" and " +  AQ_TO_STRING_FROM_DOUBLE(max_alpha) )
	AQ_THROW_IF( beta < min_beta || beta > max_beta, "Invalid Beta Parameter: Must be between " + AQ_TO_STRING_FROM_DOUBLE(min_beta) +" and " +  AQ_TO_STRING_FROM_DOUBLE(max_beta) )
	AQ_THROW_IF( nu < min_nu || nu > max_nu,	"Invalid Alpha Parameter: Must be between " + AQ_TO_STRING_FROM_DOUBLE(min_nu) +" and " +  AQ_TO_STRING_FROM_DOUBLE(max_nu) )
	AQ_THROW_IF( rho < min_rho || rho > max_rho, "Invalid Alpha Parameter: Must be between " + AQ_TO_STRING_FROM_DOUBLE(min_rho) +" and " +  AQ_TO_STRING_FROM_DOUBLE(max_rho) )

	const double ATM_VOL = std::numeric_limits<double>::quiet_NaN();

	//calibration
	AQLMathSABR* sabr = createSABR(approxMethod, alpha, beta, nu, rho);
	AQLMathLeastSquareSABRCostFunc* costfunc =new  AQLMathLeastSquareSABRCostFunc(*sabr,
																				expiry,
																				forward,
																				numeraire, 
																				ATM_VOL,
																				strikes,
																				weight,
																				sgn,
																				initValue,
																				calibFlg,
																				vols,
																				tmp_target );
	
	// Create constraint
	AQLQuantLibConstraint* constraint;
	AQLString tmp_approxMethod = approxMethod;
	tmp_approxMethod.toUpper();
	if(approxMethod == APPROXIMATION_ANTONOV)
	{
		constraint = new AQLMathLeastSquareSABRConstraint_Antonov(forward,
															     initValue,
															     calibFlg,
															     max_alpha,
															     min_alpha,
															     max_beta,
															     min_beta,
															     max_nu,
															     min_nu,
															     max_rho,
															     min_rho );
	}	
	else
	{
		constraint = new AQLMathLeastSquareSABRConstraint(initValue,
								 					     calibFlg,
													     max_alpha,
								                         min_alpha,
								                         max_beta,
								                         min_beta,
								                         max_nu,
								                         min_nu,
								                         max_rho,
									                     min_rho );
	}

	//
	vector<double> x_;
	if(!calibFlg[0])
	{
		if(!calibFlg[1])
		{
			if(!calibFlg[2])
			{
				if(!calibFlg[3])
				{
					AQ_THROW("Invalid Calibration Data: All calibration flags have been set to false")
				}
				else x_.push_back(rho);
			}
			else
			{
				x_.push_back(nu);
				if(calibFlg[3]) x_.push_back(rho);
			}
		}
		else
		{
			x_.push_back(beta);
			if(calibFlg[2])
			{
				x_.push_back(nu);
				if(calibFlg[3]) x_.push_back(rho);
			}
			else
			{
				if(calibFlg[3]) x_.push_back(rho);
			}
		}
	}
	else
	{
		x_.push_back(alpha);
		if(!calibFlg[1])
		{
			if(calibFlg[2])
			{
				x_.push_back(nu);
				if(calibFlg[3]) x_.push_back(rho);
			}
			else
			{
				if(calibFlg[3])  x_.push_back(rho);
			}
		}
		else
		{
			 x_.push_back(beta);
			if(calibFlg[2])
			{
				x_.push_back(nu);
				if(calibFlg[3]) x_.push_back(rho);
			}
			else
			{
				if(calibFlg[3]) x_.push_back(rho);
			}
		}
	}

	AQLQuantLibArray x(x_.size());
	for(size_t k = 0; k < x_.size(); ++k) x[k] = x_[k];
	AQLQuantLibProblem* opt_problem = new AQLQuantLibProblem(*costfunc,*constraint,x);


	// Set Optimization Method
	AQLQuantLibOptimizationMethod* optMethod = 0;
	AQLString tmp_calibMethod = calibMethod;
	tmp_calibMethod.toUpper();
	if (tmp_calibMethod == CALIB_NON_LINEAR_CONJUGATE_GRADIENT_METHOD)
	{
		optMethod = new AQLQuantLibConjugateGradient();
	}
	else if (tmp_calibMethod == CALIB_STEEPEST_DESCENT_METHOD)
	{
		optMethod = new AQLQuantLibSteepestDescent();
	}
	else if (tmp_calibMethod == CALIB_SIMPLEX_METHOD)
	{
		if (tmp_target == CALIB_TARGET_VOLATILITY)
		{
			optMethod = new AQLQuantLibSimplex(0.1);
		}
		else
		{
			optMethod = new AQLQuantLibSimplex(max_alpha);
		}
	}
	else if (tmp_calibMethod == CALIB_LEVENBERG_MARQUARDT_METHOD)
	{
		double levenbergMarquardtEpsfcn = 0.0000001;       // parameters specific for Levenberg-Marquardt
		double levenbergMarquardtXtol   = 0.0000001;	   //
		double levenbergMarquardtGtol   = 0.0000001;       //

		optMethod = new AQLQuantLibLevenbergMarquardt(levenbergMarquardtEpsfcn,
			                                         levenbergMarquardtXtol, 
													 levenbergMarquardtGtol);
	}
	else
	{
		delete optMethod;
		delete opt_problem;
		delete constraint;
		delete costfunc;
		delete sabr;

		AQ_THROW(" Optimization Method: " + calibMethod + " not supported")
	}

	// Set EndCriteria
	unsigned int maxIteration                = static_cast<unsigned int>(5000); 
	unsigned int maxStationaryStateIteration = static_cast<unsigned int>(2500);
	double rootEpsilon         = 0.0000001; 
	double functionEpsilon     = 0.0000001; 
	double gradientNormEpsilon = 0.0000001;

	if (maxStationaryStateIteration >= maxIteration)
	{
		delete optMethod;
		delete opt_problem;
		delete constraint;
		delete costfunc;
		delete sabr;

		AQ_THROW("Solver Failed to Converge to a Solution")
	}

	AQLQuantLibEndCriteria* endCriteria = new AQLQuantLibEndCriteria(maxIteration, 
			                                                           maxStationaryStateIteration,
												                       rootEpsilon, 
												                       functionEpsilon, 
												                       gradientNormEpsilon);

	// Optimization
	try
	{
		try
		{
			AQLQuantLibEndCriteria::Type endCriteriaResult = optMethod->minimize(*opt_problem, *endCriteria);
		}
		catch(AQLCoreError&e)
		{
			AQLString msg = "";
			for(unsigned int k =0;k<e.getSize();k++) msg+=AQLString(e.getMsg(e.getSize()-k-1));
			throw AQLCoreError(msg.getCString(), __FILE__, __LINE__);
		}
	}
	catch(std::exception& e)
	{
		throw AQLCoreError(e.what(), __FILE__, __LINE__);
	}

	//QuantLib::Array xMinCalculated = opt_problem->currentValue();

	//set return
    DoubleArray ret(4);
    ret[0] = sabr->getAlpha();
    ret[1] = sabr->getBeta();
    ret[2] = sabr->getNu();
    ret[3] = sabr->getRho();

	delete endCriteria;
	delete optMethod;
	delete opt_problem;
	delete constraint;
	delete costfunc;
	delete sabr;
    
    return ret;
}

DoubleArray 
AQLMathSwaptionVolUtility::calibrateSABR
( double alpha, double beta, double nu, double rho, double forward, AQLString expiryPoint,
  double numeraire, const DoubleArray& strikes, const DoubleArray& vols, 
  AQLStringMatrix sabrLimiter, AQLString target, const DoubleVector& weight )
{
    upper(target);
    upper(sabrLimiter);

	double expiry = AQLMathSwaptionVolUtility::getTenorPoint(expiryPoint);
    AQLMathSABRLimiter limiter(sabrLimiter);
    size_t calibSize = vols.size();
    if( calibSize != weight.size() || calibSize != strikes.size() || limiter.getParamNum() == 0 )
	{
        AQ_THROW("Invalid Market Data: Number of Volatilities, Strikes and Parameters must match")
	}

    DoubleArray forward_calib, numeraire_calib, expiry_calib, x(limiter.getParamNum());
    AQLMathSABR_Hagan sabr;
    sabr.setSABRParam(alpha,beta,nu,rho);
    forward_calib = getConstantVector(forward,calibSize);
    numeraire_calib = getConstantVector(numeraire,calibSize);
    expiry_calib = getConstantVector(expiry,calibSize);
    //calibrate
    bool isMap = false;
    AQLMathSABRCalibrator calibrator(sabr, strikes, vols, forward_calib, expiry_calib, 
        numeraire_calib, weight, target, isMap);
    calibrator.setLimiter( limiter );
    limiter.getArgument( sabr, x, isMap );
    NL2SOL solver( calibrator );
    solver.tryToSolve( x );
    //set return
    DoubleArray ret(4);
    sabr = limiter.getSABR( sabr, x , isMap);
    ret[0] = sabr.getAlpha();
    ret[1] = sabr.getBeta();
    ret[2] = sabr.getNu();
    ret[3] = sabr.getRho();

    return ret;
}

void 
AQLMathSwaptionVolUtility::calibrateSABRATMFix
( AQLDataInstance* dataInstance, const AQLString& curveSetID, const AQLString& alphaID, const AQLString& betaID, 
  const AQLString& nuID, const AQLString& rhoID, const AQLString& swapConvID, 
  const AQLString& capConvID, const AQLString& ATMVolID, const AQLStringVector& swapVolID, 
  AQLStringMatrix sabrLimiter, const AQLString& target, const DoubleVector& weight, 
  const AQLString& forwardID,  const double forwardShiftValue )
{
	AQLPriceDataDayCount dc_act365(ACT_365_ISDA);
    AQLString tmp_target = target;
	tmp_target.toUpper();
    upper(sabrLimiter);
    const double eps = 0.000000001;
	AQLObjectPool& objPool = dataInstance->getObjectPool();

	//get ATM Vol
	vector<DoubleMatrix > marketVol, marketStk;
	for(size_t i=0; i<swapVolID.size(); i++)
    {
        matirixCheck(dataInstance, betaID, swapVolID[i], true);
        marketVol.push_back(
        dynamic_cast<const AQLDataDoubleMatrix &> (objPool.getObject( swapVolID[i], ENCHKTYPE_ISDEFINED ).get().
        getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get() );

        marketStk.push_back(
        dynamic_cast<const AQLDataDoubleMatrix &> (objPool.getObject( swapVolID[i], ENCHKTYPE_ISDEFINED ).get().
        getData(PRICING_DATA_SWAPTIONSTRIKEMATRIX,ISDEFINED).get()).get() );
    }
	matirixCheck(dataInstance, betaID, ATMVolID, true);
    DoubleMatrix& atmVol = dynamic_cast<AQLDataDoubleMatrix &> (objPool.getObject( ATMVolID, ENCHKTYPE_ISDEFINED ).get().
                getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();

	BoolMatrix isCalib;
	AQLDataHolder* dh = &objPool.getObject( ATMVolID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_CALIBRATIONFLAG, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		isCalib = dynamic_cast<const AQLDataBoolMatrix &>(dh->get()).get();
		if (isCalib.size() != atmVol.size() || isCalib[0].size() != atmVol[0].size())
		{
			AQ_THROW("Inconsistent Data: Enable Calibration table size does not match ATM Vol table size")
		}
	}

	//set alpha
    AQLObjectHolder objHolder = objPool.getObject(alphaID ,ENCHKTYPE_NOCHECK);
	if(!objHolder.isDefined())
	{
		const AQLObject& beta = dataInstance->getObjectPool().getObject( betaID, ENCHKTYPE_ISDEFINED ).get();
		AQLObject* e = beta.clone();
		e->remove( CALIBRATION_DATA_NAME );
		e->add( CALIBRATION_DATA_NAME, new AQLDataString() ).convertFromString(alphaID);
		e->remove(PRICING_DATA_SWAPTIONMATRIX);
        e->add( PRICING_DATA_SWAPTIONMATRIX, new AQLDataDoubleMatrix( atmVol ));	
		objPool.set( alphaID , e );
	}

    //alpha is not calibrated
    int row = AQLCoreUtility::findRowsNumber(sabrLimiter,"ALPHA");
    sabrLimiter[row][1] = "NO";
    AQLMathSABRLimiter limiter(sabrLimiter);

    AQ_THROW_IF( swapVolID.size() != weight.size(), "Inconsistent Data: Number of Vol Objects and Weights must match")

	matirixCheck(dataInstance, betaID, alphaID);
	matirixCheck(dataInstance, betaID, nuID);
	matirixCheck(dataInstance, betaID, rhoID);
    DoubleMatrix& betaMat = dynamic_cast<AQLDataDoubleMatrix &>
        (objPool.getObject( betaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
    const AQLStringVector& tenor = dynamic_cast<const AQLDataStrings &>
        (objPool.getObject( betaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_TENORSTRING,ISDEFINED).get()).get();
	const DoubleVector& tenor_d = dynamic_cast<const AQLDataDoubles &>
        (objPool.getObject( betaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_TENORVECTOR,ISDEFINED).get()).get();
    const DoubleVector& expiryTerms = dynamic_cast<const AQLDataDoubles &>
        (objPool.getObject( betaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_EXPIRYVECTOR,ISDEFINED).get()).get();
	const DateVector& expiryDates = dynamic_cast<const AQLDataDates &>
        (objPool.getObject( betaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_EXPIRYDATEVECTOR,ISDEFINED).get()).get();

	DoubleMatrix& alphaMat = dynamic_cast<AQLDataDoubleMatrix &>
        (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
    DoubleMatrix& nuMat = dynamic_cast<AQLDataDoubleMatrix &>
        (objPool.getObject( nuID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
    DoubleMatrix& rhoMat = dynamic_cast<AQLDataDoubleMatrix &>
        (objPool.getObject( rhoID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();

	DoubleMatrix forwardMat;
	if(forwardID != AQLString("")) 
	{
		matirixCheck(dataInstance, betaID, forwardID);
		forwardMat = dynamic_cast<AQLDataDoubleMatrix &>
			(objPool.getObject( forwardID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
	}

    double forward, numeraire;
    DoubleArray strike_calib, vol_calib, x(limiter.getParamNum());
    DateVector dates;
    AQLMathSABR_Hagan sabr;
	//AQLObject curveEntity = dataInstance->getObjectPool().getObject( curveSetID, ENCHKTYPE_ISDEFINED ).get();
	AQLString curveID,convID,foreName,dfName;
	//curveID = dynamic_cast<const AQLDataString& >(curveEntity.getData(CALIBRATION_DATA_CURVEID,ISDEFINED).get()).get();
    bool isMap = false;
	for(size_t j=0; j<tenor.size(); j++)
	{
		for(size_t i=0; i<expiryDates.size(); i++)
		{
			if (isCalib.size() !=0 && !isCalib[i][j]) continue;

			if(forwardID == AQLString(""))
			{
				//curveEntity = dataInstance->getObjectPool().getObject( curveSetID, ENCHKTYPE_ISDEFINED ).get();
				AQLObject curveEntity = dataInstance->getObjectPool().getObject( curveSetID, ENCHKTYPE_ISDEFINED ).get();
				curveID = dynamic_cast<const AQLDataString& >(curveEntity.getData(CALIBRATION_DATA_CURVEID,ISDEFINED).get()).get();

				if( tenor[j]==AQLString("1M") || tenor[j]==AQLString("3M") || tenor[j]==AQLString("6M") )
				{
					convID = capConvID;
					foreName = dynamic_cast<const AQLDataString& >(curveEntity.getData(tenor[j]+AQLString("LiborCurveName"),ISDEFINED).get()).get();
					dfName = dynamic_cast<const AQLDataString& >(curveEntity.getData(CALIBRATION_DATA_DISCOUNTCURVENAME,ISDEFINED).get()).get();
				}
				else
				{
					convID = swapConvID;
					const AQLDataHolder* dh;
					dh = &curveEntity.getData(CALIBRATION_DATA_SWAPRATELCURVENAME + AQLString("_") + tenor[j],NOCHECK);
					if(!dh->isDefined() || dh->isNull())
					{
						dh = &curveEntity.getData(CALIBRATION_DATA_SWAPRATELCURVENAME,ISDEFINED);
					}
					foreName = dynamic_cast<const AQLDataString& >(dh->get()).get();
					dfName = dynamic_cast<const AQLDataString& >(curveEntity.getData(CALIBRATION_DATA_DISCOUNTCURVENAME,ISDEFINED).get()).get();
				}	
				forward = getForward(dataInstance, expiryDates[i], tenor[j], curveID, convID, foreName, dfName);
				numeraire = getNumeraire(dataInstance, expiryDates[i], tenor[j], curveID, convID, dfName);
			}
			else
			{
				forward = forwardMat[i][j];
				numeraire = 1.;
			}

			forward += forwardShiftValue;
			forward = (std::fabs(forward) < eps_SABR) ? eps_SABR : forward;		            
			strike_calib.clear();
			vol_calib.clear();
			for(size_t k=0; k<swapVolID.size(); k++)
			{                
				if( marketVol[k][i][j] >= eps )
				{
					vol_calib.push_back(marketVol[k][i][j]);
					strike_calib.push_back(marketStk[k][i][j] + forwardShiftValue);
				}
			}

            if(vol_calib.size() < limiter.getParamNum() || atmVol[i][j] < eps) break;

            //calibration
			sabr.setSABRParam(0.01,betaMat[i][j],nuMat[i][j],rhoMat[i][j]);
			if( limiter.getParamNum() != 0 )
			{
				AQLMathSABRCalibratorATMFix calibrator( sabr, strike_calib, vol_calib, atmVol[i][j], forward, 
					expiryTerms[i], numeraire, weight, tmp_target, isMap );
				calibrator.setLimiter( limiter );
				sabr.setAlphaForATMVol( atmVol[i][j], expiryTerms[i], forward );
				limiter.getArgument( sabr, x, isMap );
				NL2SOL solver( calibrator );
				solver.tryToSolve( x );
			}
            //set param
            sabr = limiter.getSABR( sabr, x , isMap );
            sabr.setAlphaForATMVol(atmVol[i][j], expiryTerms[i], forward);
            alphaMat[i][j] = AQLMath::max(sabr.getAlpha(), alpha_low);
            betaMat[i][j] = AQLMath::max(sabr.getBeta(), beta_low);
            nuMat[i][j] = AQLMath::max(sabr.getNu(), nu_low);
            rhoMat[i][j] = AQLMath::max(sabr.getRho(), rho_low);
        }
    }
}

double 
AQLMathSwaptionVolUtility::getSABRAlpha
( AQLDataInstance* dataInstance, const AQLString& expPoint, const AQLString& tenorPoint, const AQLString& volID, const AQLString& forwardID, 
  const AQLString& alphaID, const AQLString& betaID, const AQLString& nuID, const AQLString& rhoID )
{
    AQLPriceDataDayCount dc_act365(ACT_365_ISDA);
    AQLObjectPool& objPool = dataInstance->getObjectPool();

	matirixCheck(dataInstance, alphaID, volID, true);
	matirixCheck(dataInstance, alphaID, forwardID);
	matirixCheck(dataInstance, alphaID, betaID);
	matirixCheck(dataInstance, alphaID, nuID);
	matirixCheck(dataInstance, alphaID, rhoID);

    const DoubleMatrix& alphaMat = dynamic_cast<AQLDataDoubleMatrix& >
        (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
    const DoubleVector& tenorVec = dynamic_cast<const AQLDataDoubles& >
        (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_TENORVECTOR,ISDEFINED).get()).get();
    const DoubleVector& expiryVec = dynamic_cast<const AQLDataDoubles& >
        (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_EXPIRYVECTOR,ISDEFINED).get()).get();

    const DoubleMatrix& volMat = dynamic_cast<AQLDataDoubleMatrix &>
        (objPool.getObject( volID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
    const DoubleMatrix& forwardMat = dynamic_cast<AQLDataDoubleMatrix &>
        (objPool.getObject( forwardID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
    const DoubleMatrix& betaMat = dynamic_cast<AQLDataDoubleMatrix &>
        (objPool.getObject( betaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
    const DoubleMatrix& nuMat = dynamic_cast<AQLDataDoubleMatrix &>
        (objPool.getObject( nuID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
    const DoubleMatrix& rhoMat = dynamic_cast<AQLDataDoubleMatrix &>
        (objPool.getObject( rhoID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();

    const AQLPriceDataSlidingRule& paySlr = dynamic_cast<const AQLPriceDataSlidingRule& >(objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).
		get().getData(CALIBRATION_DATA_SLIDINGRULE,ISDEFINED).get());
    const AQLPriceDataCalendar& fixCal = dynamic_cast<const AQLPriceDataCalendar& >(objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().
		getData(CALIBRATION_DATA_CALENDAR,ISDEFINED).get());
    const AQLDate& asOfDate = dynamic_cast<const AQLDataDate& >(dataInstance->getObjectPool().getObject(alphaID,ENCHKTYPE_ISDEFINED).
		get().getData(CALIBRATION_DATA_ASOFDATE,ISNOTNULL).get()).get();

	double expPoint_d = getExpiryPoint(expPoint, asOfDate, paySlr, fixCal);

    AQLMathSwaptionMatrix volMat_(volMat, expiryVec, tenorVec);
    AQLMathSwaptionMatrix forwardMat_(forwardMat, expiryVec, tenorVec);
    AQLMathSwaptionMatrix alphaMat_(alphaMat, expiryVec, tenorVec);
    AQLMathSwaptionMatrix betaMat_(betaMat, expiryVec, tenorVec);
    AQLMathSwaptionMatrix nuMat_(nuMat, expiryVec, tenorVec);
    AQLMathSwaptionMatrix rhoMat_(rhoMat, expiryVec, tenorVec);
    
    double tenorPoint_d = getTenorPoint(tenorPoint);

    double vol = volMat_.lookUpMatrix(expPoint_d, tenorPoint_d);
    double forward = forwardMat_.lookUpMatrix(expPoint_d, tenorPoint_d);
    double alpha = alphaMat_.lookUpMatrix(expPoint_d, tenorPoint_d);
    double beta = betaMat_.lookUpMatrix(expPoint_d, tenorPoint_d);
    double nu = nuMat_.lookUpMatrix(expPoint_d, tenorPoint_d);
    double rho = rhoMat_.lookUpMatrix(expPoint_d, tenorPoint_d);
    //for param check
    AQLMathSABR_Hagan sabr(alpha,beta,nu,rho);

   sabr.setAlphaForATMVol(vol, expPoint_d, forward, true);
   return sabr.getAlpha();
}

double 
AQLMathSwaptionVolUtility::getSABRAlpha2
( AQLDataInstance* dataInstance, const AQLString& expPoint, const AQLString& tenorPoint, double vol, const AQLString& alphaID,
  const AQLString& betaID, const AQLString& nuID, const AQLString& rhoID, const AQLString& curveID, const AQLString& convID,
  const AQLString& foreCurveName, const AQLString& dfCurveName)
{
    AQLPriceDataDayCount dc_act365(ACT_365_ISDA);
    AQLObjectPool& objPool = dataInstance->getObjectPool();

	matirixCheck(dataInstance, alphaID, betaID);
	matirixCheck(dataInstance, alphaID, nuID);
	matirixCheck(dataInstance, alphaID, rhoID);
    const DoubleMatrix& alphaMat = dynamic_cast<AQLDataDoubleMatrix &>
        (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
    const DoubleVector& tenorVec = dynamic_cast<const AQLDataDoubles& >
        (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_TENORVECTOR,ISDEFINED).get()).get();
    const DoubleVector& expiryVec = dynamic_cast<const AQLDataDoubles& >
        (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_EXPIRYVECTOR,ISDEFINED).get()).get();
 
    const DoubleMatrix& betaMat = dynamic_cast<AQLDataDoubleMatrix &>
        (objPool.getObject( betaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
    const DoubleMatrix& nuMat = dynamic_cast<AQLDataDoubleMatrix &>
        (objPool.getObject( nuID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
    const DoubleMatrix& rhoMat = dynamic_cast<AQLDataDoubleMatrix &>
        (objPool.getObject( rhoID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();

    const AQLObject& object = objPool.getObject( convID, ENCHKTYPE_ISDEFINED ).get();
    const AQLPriceDataSlidingRule& paySlr = 
        dynamic_cast<const AQLPriceDataSlidingRule& >(object.getData(CALIBRATION_DATA_SLIDINGRULE,ISDEFINED).get());
    const AQLPriceDataCalendar& fixCal =
        dynamic_cast<const AQLPriceDataCalendar& >(object.getData(PRICING_DATA_FIXINGCALENDAR,ISDEFINED).get());
    
	const AQLObject& curve = objPool.getObject( curveID, ENCHKTYPE_ISDEFINED ).get();
	const AQLDate& asOfDate = dynamic_cast<const AQLDataDate&>(curve.getData(CALIBRATION_DATA_ASOFDATE,ISDEFINED).get());

	double expPoint_d = getExpiryPoint(expPoint, asOfDate, paySlr, fixCal);
	AQLDate expPoint_date = getExpiryPoint2(expPoint, asOfDate, paySlr, fixCal);

    double forward = getForward(dataInstance, expPoint_date, tenorPoint, curveID, convID, foreCurveName, dfCurveName);

    AQLMathSwaptionMatrix alphaMat_(alphaMat, expiryVec, tenorVec);
    AQLMathSwaptionMatrix betaMat_(betaMat, expiryVec, tenorVec);
    AQLMathSwaptionMatrix nuMat_(nuMat, expiryVec, tenorVec);
    AQLMathSwaptionMatrix rhoMat_(rhoMat, expiryVec, tenorVec);
    
    double tenorPoint_d = getTenorPoint(tenorPoint);

    double alpha = alphaMat_.lookUpMatrix(expPoint_d, tenorPoint_d);
    double beta = betaMat_.lookUpMatrix(expPoint_d, tenorPoint_d);
    double nu = nuMat_.lookUpMatrix(expPoint_d, tenorPoint_d);
    double rho = rhoMat_.lookUpMatrix(expPoint_d, tenorPoint_d);
    //for param check
    AQLMathSABR_Hagan sabr(alpha,beta,nu,rho);

    sabr.setAlphaForATMVol(vol, expPoint_d, forward, true);
	return sabr.getAlpha();
}

double 
AQLMathSwaptionVolUtility::getSABRVol
( AQLDataInstance* dataInstance, const AQLString& expPoint, const AQLString& tenorPoint, double strike, const AQLString& fowardID, 
  const AQLString& alphaID, const AQLString& betaID, const AQLString& nuID, const AQLString& rhoID, const AQLString& approxMethod , const double shift, bool isLognormal)
{
    AQLPriceDataDayCount dc_act365(ACT_365_ISDA);
    AQLObjectPool& objPool = dataInstance->getObjectPool();

	matirixCheck(dataInstance, alphaID, fowardID);
	matirixCheck(dataInstance, alphaID, betaID);
	matirixCheck(dataInstance, alphaID, nuID);
	matirixCheck(dataInstance, alphaID, rhoID);

    const DoubleMatrix& alphaMat = dynamic_cast<AQLDataDoubleMatrix& >
        (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
    const DoubleVector& tenorVec = dynamic_cast<const AQLDataDoubles& >
        (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_TENORVECTOR,ISDEFINED).get()).get();
    const DoubleVector& expiryVec = dynamic_cast<const AQLDataDoubles& >
        (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_EXPIRYVECTOR,ISDEFINED).get()).get();
    
    const DoubleMatrix& fowardMat = dynamic_cast<AQLDataDoubleMatrix &>
        (objPool.getObject( fowardID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
    const DoubleMatrix& betaMat = dynamic_cast<AQLDataDoubleMatrix &>
        (objPool.getObject( betaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
    const DoubleMatrix& nuMat = dynamic_cast<AQLDataDoubleMatrix &>
        (objPool.getObject( nuID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
    const DoubleMatrix& rhoMat = dynamic_cast<AQLDataDoubleMatrix &>
        (objPool.getObject( rhoID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();    

    const AQLPriceDataSlidingRule& paySlr = dynamic_cast<const AQLPriceDataSlidingRule& >(objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).
		get().getData(CALIBRATION_DATA_SLIDINGRULE,ISDEFINED).get());
    const AQLPriceDataCalendar& fixCal = dynamic_cast<const AQLPriceDataCalendar& >(objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().
		getData(CALIBRATION_DATA_CALENDAR,ISDEFINED).get());
    const AQLDate& asOfDate = dynamic_cast<const AQLDataDate& >(dataInstance->getObjectPool().getObject(alphaID,ENCHKTYPE_ISDEFINED).
		get().getData(CALIBRATION_DATA_ASOFDATE,ISNOTNULL).get()).get();

    double expPoint_d = getExpiryPoint(expPoint, asOfDate, paySlr, fixCal);

    AQLMathSwaptionMatrix fowardMat_(fowardMat, expiryVec, tenorVec);
    AQLMathSwaptionMatrix alphaMat_(alphaMat, expiryVec, tenorVec);
    AQLMathSwaptionMatrix betaMat_(betaMat, expiryVec, tenorVec);
    AQLMathSwaptionMatrix nuMat_(nuMat, expiryVec, tenorVec);
    AQLMathSwaptionMatrix rhoMat_(rhoMat, expiryVec, tenorVec);
    
	double tenorPoint_d = getTenorPoint(tenorPoint);

    double forward = fowardMat_.lookUpMatrix(expPoint_d, tenorPoint_d);
    double alpha = alphaMat_.lookUpMatrix(expPoint_d, tenorPoint_d);
    double beta = betaMat_.lookUpMatrix(expPoint_d, tenorPoint_d);
    double nu = nuMat_.lookUpMatrix(expPoint_d, tenorPoint_d);
    double rho = rhoMat_.lookUpMatrix(expPoint_d, tenorPoint_d);

    //AQLMathSABR_Hagan sabr(alpha, beta, nu, rho);
    //return sabr.getSABRVol(expPoint_d, forward, strike);
	AQLMathSABR* sabr = createSABR(approxMethod, alpha, beta, nu, rho, isLognormal);

	forward = (std::fabs(forward) < eps_SABR) ? eps_SABR : forward;

	forward += shift;
	strike += shift;

	double vol = sabr->getSABRVol(expPoint_d, forward, strike);
	delete sabr;

	return vol;
}


double AQLMathSwaptionVolUtility::calcSABRVol(const double alpha, const double beta, const double nu, const double rho, const double expiryTerm, const double strike, const double forward, const double shift, const AQLString& approxMethod, bool isLognormal)
{
	//AQLMathSABR_Hagan sabr(alpha, beta, nu, rho);
	//return sabr.getSABRVol(expPoint_d, forward, strike);
	AQLMathSABR* sabr = createSABR(approxMethod, alpha, beta, nu, rho, isLognormal);

	//forward = (std::fabs(forward) < eps_SABR) ? eps_SABR : forward;

	double fwd = forward + shift;
	double stk = strike + shift;

	double vol = sabr->getSABRVol(expiryTerm, fwd, stk);
	delete sabr;

	return vol;

}

double AQLMathSwaptionVolUtility::calcSABRParam(const DoubleMatrix& paramMat, const double expiryTerm, const double tenorTerm, const DoubleVector& expiryVec, const DoubleVector& tenorVec)
{
	AQ_REQUIRE(paramMat.size() == expiryVec.size() && paramMat[0].size() == tenorVec.size(), "Size not matched.");

	AQLMathSwaptionMatrix paramMat_(paramMat, expiryVec, tenorVec);

	double value = paramMat_.lookUpMatrix(expiryTerm, tenorTerm);

	return value;
}

double 
AQLMathSwaptionVolUtility::getSABRVol2
( AQLDataInstance* dataInstance, const AQLString& expPoint, const AQLString& tenorPoint, double strike, const AQLString& alphaID,
  const AQLString& betaID, const AQLString& nuID, const AQLString& rhoID, const AQLString& curveID, 
  const AQLString& convID, AQLString foreCurveName, AQLString dfCurveName, const AQLString& approxMethod)
{
    AQLPriceDataDayCount dc_act365(ACT_365_ISDA);
    AQLObjectPool& objPool = dataInstance->getObjectPool();

	const AQLObject& object = objPool.getObject( convID, ENCHKTYPE_ISDEFINED ).get();
    const AQLPriceDataSlidingRule& paySlr = 
        dynamic_cast<const AQLPriceDataSlidingRule& >(object.getData(CALIBRATION_DATA_SLIDINGRULE,ISDEFINED).get());
    const AQLPriceDataCalendar& fixCal =
        dynamic_cast<const AQLPriceDataCalendar& >(object.getData(PRICING_DATA_FIXINGCALENDAR,ISDEFINED).get());
	const AQLObject& curve = objPool.getObject( curveID, ENCHKTYPE_ISDEFINED ).get();
	const AQLDate& asOfDate = dynamic_cast<const AQLDataDate& >(curve.getData(CALIBRATION_DATA_ASOFDATE,ISDEFINED).get());

	double expPoint_d = AQLMathSwaptionVolUtility::getExpiryPoint(expPoint, asOfDate, paySlr, fixCal);
	AQLDate expPoint_date = AQLMathSwaptionVolUtility::getExpiryPoint2(expPoint, asOfDate, paySlr, fixCal);

	double forward = getForward(dataInstance, expPoint_date, tenorPoint, curveID, convID, foreCurveName, dfCurveName);
	forward = (std::fabs(forward) < eps_SABR) ? eps_SABR : forward;
	double tenorPoint_d = getTenorPoint(tenorPoint);

	return getSABRVol3(dataInstance, expPoint_d, tenorPoint_d, forward, strike, alphaID, betaID, nuID, rhoID, approxMethod);    
}

double 
AQLMathSwaptionVolUtility::getSABRVol3
( AQLDataInstance* dataInstance, double expPoint, double tenorPoint, double forward, double strike, const AQLString& alphaID,
  const AQLString& betaID, const AQLString& nuID, const AQLString& rhoID, const AQLString& approxMethod)
{
	AQLObjectPool& objPool = dataInstance->getObjectPool();

	const DoubleMatrix& alphaMat = dynamic_cast<AQLDataDoubleMatrix& >
        (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
    const DoubleVector& tenorVec = dynamic_cast<const AQLDataDoubles& >
        (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_TENORVECTOR,ISDEFINED).get()).get();
    const DoubleVector& expiryVec = dynamic_cast<const AQLDataDoubles& >
        (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_EXPIRYVECTOR,ISDEFINED).get()).get();
    //const AQLStringVector& expiry = dynamic_cast<const AQLDataStrings& >
    //    (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_EXPIRYSTRING,ISDEFINED).get()).get();
    //const AQLStringVector& tenor = dynamic_cast<const AQLDataStrings& >
    //    (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_TENORSTRING,ISDEFINED).get()).get();

    matirixCheck(dataInstance, alphaID, betaID, true);
    const DoubleMatrix& betaMat = dynamic_cast<AQLDataDoubleMatrix& >
        (objPool.getObject( betaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
     matirixCheck(dataInstance, alphaID, nuID, true);
    const DoubleMatrix& nuMat = dynamic_cast<AQLDataDoubleMatrix& >
        (objPool.getObject( nuID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
    matirixCheck(dataInstance, alphaID, rhoID, true);
    const DoubleMatrix& rhoMat = dynamic_cast<AQLDataDoubleMatrix& >
        (objPool.getObject( rhoID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();

	AQLMathSwaptionMatrix alphaMat_(alphaMat, expiryVec, tenorVec);
    AQLMathSwaptionMatrix betaMat_(betaMat, expiryVec, tenorVec);
    AQLMathSwaptionMatrix nuMat_(nuMat, expiryVec, tenorVec);
    AQLMathSwaptionMatrix rhoMat_(rhoMat, expiryVec, tenorVec);

	double alpha = alphaMat_.lookUpMatrix(expPoint, tenorPoint);
    double beta = betaMat_.lookUpMatrix(expPoint, tenorPoint);
    double nu = nuMat_.lookUpMatrix(expPoint, tenorPoint);
    double rho = rhoMat_.lookUpMatrix(expPoint, tenorPoint);

	AQLMathSABR* sabr = createSABR(approxMethod, alpha, beta, nu, rho);

	forward = (std::fabs(forward) < eps_SABR) ? eps_SABR : forward;
	double vol = sabr->getSABRVol(expPoint, forward, strike);
	delete sabr;

	return vol;
}

double 
AQLMathSwaptionVolUtility::getSABRVol4
( AQLDataInstance* dataInstance, double expTerm, const AQLString& tenorPoint, double forward, double strike, const AQLString& alphaID,
  const AQLString& betaID, const AQLString& nuID, const AQLString& rhoID, const AQLString& curveID, 
  const AQLString& convID, AQLString foreCurveName, AQLString dfCurveName, const AQLString& approxMethod)
{
    double tenorPoint_d = getTenorPoint(tenorPoint);
	forward = (std::fabs(forward) < eps_SABR) ? eps_SABR : forward;
	return getSABRVol3(dataInstance, expTerm, tenorPoint_d, forward, strike, alphaID, betaID, nuID, rhoID, approxMethod);    
}

//++++++++ Funahashi ++++++++
double 
AQLMathSwaptionVolUtility::getSABRPrem
( AQLDataInstance* dataInstance, const AQLString& expPoint, const AQLString& tenorPoint, double strike, int sgn, const AQLString& fowardID, const AQLString& numeraireID, 
  const AQLString& alphaID, const AQLString& betaID, const AQLString& nuID, const AQLString& rhoID, const AQLString& approxMethod, double shift, bool isLognormal)
{
    AQLPriceDataDayCount dc_act365(ACT_365_ISDA);
    AQLObjectPool& objPool = dataInstance->getObjectPool();

	matirixCheck(dataInstance, alphaID, fowardID);
	matirixCheck(dataInstance, alphaID, betaID);
	matirixCheck(dataInstance, alphaID, nuID);
	matirixCheck(dataInstance, alphaID, rhoID);

    const DoubleMatrix& alphaMat = dynamic_cast<AQLDataDoubleMatrix& >
        (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
    const DoubleVector& tenorVec = dynamic_cast<const AQLDataDoubles& >
        (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_TENORVECTOR,ISDEFINED).get()).get();
    const DoubleVector& expiryVec = dynamic_cast<const AQLDataDoubles& >
        (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_EXPIRYVECTOR,ISDEFINED).get()).get();
    
    const DoubleMatrix& fowardMat = dynamic_cast<AQLDataDoubleMatrix &>
        (objPool.getObject( fowardID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
	const DoubleMatrix& numeraireMat = dynamic_cast<AQLDataDoubleMatrix &>
        (objPool.getObject( numeraireID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();

	const DoubleMatrix& betaMat = dynamic_cast<AQLDataDoubleMatrix &>
        (objPool.getObject( betaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
    const DoubleMatrix& nuMat = dynamic_cast<AQLDataDoubleMatrix &>
        (objPool.getObject( nuID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
    const DoubleMatrix& rhoMat = dynamic_cast<AQLDataDoubleMatrix &>
        (objPool.getObject( rhoID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();    

    const AQLPriceDataSlidingRule& paySlr = dynamic_cast<const AQLPriceDataSlidingRule& >(objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).
		get().getData(CALIBRATION_DATA_SLIDINGRULE,ISDEFINED).get());
    const AQLPriceDataCalendar& fixCal = dynamic_cast<const AQLPriceDataCalendar& >(objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().
		getData(CALIBRATION_DATA_CALENDAR,ISDEFINED).get());
    const AQLDate& asOfDate = dynamic_cast<const AQLDataDate& >(dataInstance->getObjectPool().getObject(alphaID,ENCHKTYPE_ISDEFINED).
		get().getData(CALIBRATION_DATA_ASOFDATE,ISNOTNULL).get()).get();

    double expPoint_d = getExpiryPoint(expPoint, asOfDate, paySlr, fixCal);

    AQLMathSwaptionMatrix fowardMat_(fowardMat, expiryVec, tenorVec);
	AQLMathSwaptionMatrix numeraireMat_(numeraireMat, expiryVec, tenorVec);
    AQLMathSwaptionMatrix alphaMat_(alphaMat, expiryVec, tenorVec);
    AQLMathSwaptionMatrix betaMat_(betaMat, expiryVec, tenorVec);
    AQLMathSwaptionMatrix nuMat_(nuMat, expiryVec, tenorVec);
    AQLMathSwaptionMatrix rhoMat_(rhoMat, expiryVec, tenorVec);
    
	double tenorPoint_d = getTenorPoint(tenorPoint);

    double forward = fowardMat_.lookUpMatrix(expPoint_d, tenorPoint_d);
	forward = (std::fabs(forward) < eps_SABR) ? eps_SABR : forward;

	double numeraire = numeraireMat_.lookUpMatrix(expPoint_d, tenorPoint_d);
    double alpha = alphaMat_.lookUpMatrix(expPoint_d, tenorPoint_d);
    double beta = betaMat_.lookUpMatrix(expPoint_d, tenorPoint_d);
    double nu = nuMat_.lookUpMatrix(expPoint_d, tenorPoint_d);
    double rho = rhoMat_.lookUpMatrix(expPoint_d, tenorPoint_d);

    //AQLMathSABR_Hagan sabr(alpha, beta, nu, rho);
    //return sabr.getSABRVol(expPoint_d, forward, strike);
	AQLMathSABR* sabr = createSABR(approxMethod, alpha, beta, nu, rho, isLognormal);

	forward += shift;
	strike += shift;

	double prem = sabr->getSABRPrem(expPoint_d, forward, strike, numeraire, sgn);
	delete sabr;

	return prem;
}

double 
AQLMathSwaptionVolUtility::getSABRPrem2
( AQLDataInstance* dataInstance, const AQLString& expPoint, const AQLString& tenorPoint, double strike, int sgn, const AQLString& alphaID,
  const AQLString& betaID, const AQLString& nuID, const AQLString& rhoID, const AQLString& curveID, 
  const AQLString& convID, AQLString foreCurveName, AQLString dfCurveName, const AQLString& approxMethod)
{
    AQLPriceDataDayCount dc_act365(ACT_365_ISDA);
    AQLObjectPool& objPool = dataInstance->getObjectPool();

	const AQLObject& object = objPool.getObject( convID, ENCHKTYPE_ISDEFINED ).get();
    const AQLPriceDataSlidingRule& paySlr = 
        dynamic_cast<const AQLPriceDataSlidingRule& >(object.getData(CALIBRATION_DATA_SLIDINGRULE,ISDEFINED).get());
    const AQLPriceDataCalendar& fixCal =
        dynamic_cast<const AQLPriceDataCalendar& >(object.getData(PRICING_DATA_FIXINGCALENDAR,ISDEFINED).get());
	const AQLObject& curve = objPool.getObject( curveID, ENCHKTYPE_ISDEFINED ).get();
	const AQLDate& asOfDate = dynamic_cast<const AQLDataDate& >(curve.getData(CALIBRATION_DATA_ASOFDATE,ISDEFINED).get());

	double expPoint_d = AQLMathSwaptionVolUtility::getExpiryPoint(expPoint, asOfDate, paySlr, fixCal);
	AQLDate expPoint_date = AQLMathSwaptionVolUtility::getExpiryPoint2(expPoint, asOfDate, paySlr, fixCal);
	double numeraire = getNumeraire(dataInstance, expPoint_date, tenorPoint, curveID, convID, dfCurveName);
	double forward = getForward(dataInstance, expPoint_date, tenorPoint, curveID, convID, foreCurveName, dfCurveName);
	double tenorPoint_d = getTenorPoint(tenorPoint);

	forward = (std::fabs(forward) < eps_SABR) ? eps_SABR : forward;

	return getSABRPrem3(dataInstance, expPoint_d, tenorPoint_d, forward, numeraire, strike, sgn, alphaID, betaID, nuID, rhoID, approxMethod);    
}

double 
AQLMathSwaptionVolUtility::getSABRPrem3
( AQLDataInstance* dataInstance, double expPoint, double tenorPoint, double forward, double numeraire, double strike, int sgn, const AQLString& alphaID,
  const AQLString& betaID, const AQLString& nuID, const AQLString& rhoID, const AQLString& approxMethod)
{
	AQLObjectPool& objPool = dataInstance->getObjectPool();

	const DoubleMatrix& alphaMat = dynamic_cast<AQLDataDoubleMatrix& >
        (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
    const DoubleVector& tenorVec = dynamic_cast<const AQLDataDoubles& >
        (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_TENORVECTOR,ISDEFINED).get()).get();
    const DoubleVector& expiryVec = dynamic_cast<const AQLDataDoubles& >
        (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_EXPIRYVECTOR,ISDEFINED).get()).get();
    const AQLStringVector& expiry = dynamic_cast<const AQLDataStrings& >
        (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_EXPIRYSTRING,ISDEFINED).get()).get();
    const AQLStringVector& tenor = dynamic_cast<const AQLDataStrings& >
        (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_TENORSTRING,ISDEFINED).get()).get();

    matirixCheck(dataInstance, alphaID, betaID);
    const DoubleMatrix& betaMat = dynamic_cast<AQLDataDoubleMatrix& >
        (objPool.getObject( betaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
    matirixCheck(dataInstance, alphaID, nuID);
    const DoubleMatrix& nuMat = dynamic_cast<AQLDataDoubleMatrix& >
        (objPool.getObject( nuID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
    matirixCheck(dataInstance, alphaID, rhoID);
    const DoubleMatrix& rhoMat = dynamic_cast<AQLDataDoubleMatrix& >
        (objPool.getObject( rhoID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();

	AQLMathSwaptionMatrix alphaMat_(alphaMat, expiryVec, tenorVec);
    AQLMathSwaptionMatrix betaMat_(betaMat, expiryVec, tenorVec);
    AQLMathSwaptionMatrix nuMat_(nuMat, expiryVec, tenorVec);
    AQLMathSwaptionMatrix rhoMat_(rhoMat, expiryVec, tenorVec);

	double alpha = alphaMat_.lookUpMatrix(expPoint, tenorPoint);
    double beta = betaMat_.lookUpMatrix(expPoint, tenorPoint);
    double nu = nuMat_.lookUpMatrix(expPoint, tenorPoint);
    double rho = rhoMat_.lookUpMatrix(expPoint, tenorPoint);

	AQLMathSABR* sabr = createSABR(approxMethod, alpha, beta, nu, rho);

	forward = (std::fabs(forward) < eps_SABR) ? eps_SABR : forward;

	double prem = sabr->getSABRPrem(expPoint, forward, strike, numeraire, sgn);
	delete sabr;

	return prem;
}

DoubleMatrix 
AQLMathSwaptionVolUtility::getSABRVolMatrix
( AQLDataInstance* dataInstance, const AQLString& strikeID, const AQLString& forwardID, const AQLString& alphaID,
  const AQLString& betaID, const AQLString& nuID, const AQLString& rhoID )
{
    AQLPriceDataDayCount dc_act365(ACT_365_ISDA);
    AQLObjectPool& objPool = dataInstance->getObjectPool();

	matirixCheck(dataInstance, alphaID, betaID);
	matirixCheck(dataInstance, alphaID, nuID);
	matirixCheck(dataInstance, alphaID, rhoID);
	matirixCheck(dataInstance, alphaID, strikeID);
	matirixCheck(dataInstance, alphaID, forwardID);

    const DoubleMatrix& alphaMat = dynamic_cast<AQLDataDoubleMatrix& >
        (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();    
    const DoubleMatrix& betaMat = dynamic_cast<AQLDataDoubleMatrix& >
        (objPool.getObject( betaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();    
    const DoubleMatrix& nuMat = dynamic_cast<AQLDataDoubleMatrix& >
        (objPool.getObject( nuID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();    
    const DoubleMatrix& rhoMat = dynamic_cast<AQLDataDoubleMatrix& >
        (objPool.getObject( rhoID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();	
    const DoubleMatrix& strikeMat = dynamic_cast<AQLDataDoubleMatrix& >
        (objPool.getObject( strikeID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();	
    const DoubleMatrix& forwardMat = dynamic_cast<AQLDataDoubleMatrix& >
        (objPool.getObject( forwardID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
	const DoubleVector& expiryVec = dynamic_cast<const AQLDataDoubles &>
        (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_EXPIRYVECTOR,ISDEFINED).get()).get();
    
	DoubleMatrix ret(alphaMat.size(),DoubleArray(alphaMat[0].size()));
	for(size_t i=0; i<alphaMat.size(); i++)
	{
		for(size_t j=0; j<alphaMat[0].size(); j++)
		{
			double forward(forwardMat[i][j]);
			forward = (std::fabs(forward) < eps_SABR) ? eps_SABR : forward;
			AQLMathSABR_Hagan sabr(alphaMat[i][j], betaMat[i][j], nuMat[i][j], rhoMat[i][j]);
			//ret[i][j] =  sabr.getSABRVol(expiryVec[i], forwardMat[i][j], strikeMat[i][j]);
			ret[i][j] =  sabr.getSABRVol(expiryVec[i], forward, strikeMat[i][j]);
		}
	}
    
    return ret;
}

void
AQLMathSwaptionVolUtility::
matirixCheck( AQLDataInstance* dataInstance, const AQLString& entityName, const AQLString& entityName2, bool isVolMat)
{
    AQLObjectPool& objPool = dataInstance->getObjectPool();
	const AQLObject& object = objPool.getObject( entityName, ENCHKTYPE_ISDEFINED ).get();
	const AQLObject& entity2 = objPool.getObject( entityName2, ENCHKTYPE_ISDEFINED ).get();

	const AQLDataHolder* dh = &object.getData(PRICING_DATA_TENORSTRING, NOCHECK);
	if ( dh->isDefined() && !dh->isNull() )
	{
		const AQLStringVector& tenor = dynamic_cast<const AQLDataStrings &>
			(object.getData(PRICING_DATA_TENORSTRING,ISDEFINED).get()).get();
		const AQLStringVector& tenor2 = dynamic_cast<const AQLDataStrings &>
			(entity2.getData(PRICING_DATA_TENORSTRING,ISDEFINED).get()).get();
		if( tenor != tenor2 ) 
			throw AQLCoreInvalidData("tenors are inconsistent!",	__FILE__,__LINE__);
	}
	else
	{
		const DoubleVector& tenor = dynamic_cast<const AQLDataDoubles &>
			(object.getData(PRICING_DATA_TENORVECTOR,ISDEFINED).get()).get();
		const DoubleVector& tenor2 = dynamic_cast<const AQLDataDoubles &>
			(entity2.getData(PRICING_DATA_TENORVECTOR,ISDEFINED).get()).get();
		if( tenor != tenor2 ) 
			throw AQLCoreInvalidData("tenors are inconsistent!",	__FILE__,__LINE__);
	}

	dh = &object.getData(PRICING_DATA_EXPIRYSTRING, NOCHECK);
	if ( dh->isDefined() && !dh->isNull() )
	{
		const AQLStringVector& expiry = dynamic_cast<const AQLDataStrings &>
			(object.getData(PRICING_DATA_EXPIRYSTRING,ISDEFINED).get()).get();
		const AQLStringVector& expiry2 = dynamic_cast<const AQLDataStrings &>
			(entity2.getData(PRICING_DATA_EXPIRYSTRING,ISDEFINED).get()).get();   
		if( expiry != expiry2 ) 
			throw AQLCoreInvalidData("expiries are inconsistent!",	__FILE__,__LINE__);
	}
	else
	{
		const DoubleVector& expiry = dynamic_cast<const AQLDataDoubles &>
			(object.getData(PRICING_DATA_EXPIRYVECTOR,ISDEFINED).get()).get();
		const DoubleVector& expiry2 = dynamic_cast<const AQLDataDoubles &>
			(entity2.getData(PRICING_DATA_EXPIRYVECTOR,ISDEFINED).get()).get();   
		if( expiry != expiry2 ) 
			throw AQLCoreInvalidData("expiries are inconsistent!",	__FILE__,__LINE__);
	}

	if(isVolMat==false)
	{
		const AQLDate& asOfDate = dynamic_cast<const AQLDataDate &>
			(object.getData(CALIBRATION_DATA_ASOFDATE,ISDEFINED).get()).get();
		const AQLDate& asOfDate2 = dynamic_cast<const AQLDataDate &>
			(entity2.getData(CALIBRATION_DATA_ASOFDATE,ISDEFINED).get()).get();   
		if( asOfDate != asOfDate2 ) 
			throw AQLCoreInvalidData("asOfDates are inconsistent!",	__FILE__,__LINE__);

		const DateVector& expiDateVec = dynamic_cast<const AQLDataDates &>
			(object.getData(PRICING_DATA_EXPIRYDATEVECTOR,ISDEFINED).get()).get();
		const DateVector& expiDateVec2 = dynamic_cast<const AQLDataDates &>
			(entity2.getData(PRICING_DATA_EXPIRYDATEVECTOR,ISDEFINED).get()).get();   
		if( expiDateVec != expiDateVec2 ) 
			throw AQLCoreInvalidData("expiry date vector is inconsistent!",	__FILE__,__LINE__);
	}
}

double 
AQLMathSwaptionVolUtility::
getForward(AQLDataInstance* dataInstance, const AQLDate& expiry, const AQLString& tenor, const AQLString& curveID, 
		   const AQLString& convID, AQLString foreCurveName, AQLString dfCurveName, bool isFWDInter)
{
	AQLObjectPool& objPool = dataInstance->getObjectPool();

	const AQLObject& object = objPool.getObject( convID, ENCHKTYPE_ISDEFINED ).get();
	const AQLDataHolder* dh;
	dh = &object.getData(IR_CALIBRATION_DATA_FREQUENCY + AQLString("_") + tenor, NOCHECK);
	if(!dh->isDefined() || dh->isNull())
	{
		dh = &object.getData(IR_CALIBRATION_DATA_FREQUENCY, ISDEFINED);
	}
	const AQLString& freq = dynamic_cast<const AQLDataString& >(dh->get()).get();
    const AQLString& spotLag = 
        dynamic_cast<const AQLDataString& >(object.getData(CURVEINPUT_SPOTLAG,ISDEFINED).get()).get();
    const AQLPriceDataDayCount& daycount =
        dynamic_cast<const AQLPriceDataDayCount& >(object.getData(IR_CALIBRATION_DATA_DAYCOUNT,ISDEFINED).get());
    const AQLPriceDataSlidingRule& paySlr = 
        dynamic_cast<const AQLPriceDataSlidingRule& >(object.getData(CALIBRATION_DATA_SLIDINGRULE,ISDEFINED).get());
    const AQLPriceDataCalendar& payCal =
        dynamic_cast<const AQLPriceDataCalendar& >(object.getData(CALIBRATION_DATA_CALENDAR,ISDEFINED).get());
    const AQLPriceDataCalendar& fixCal =
        dynamic_cast<const AQLPriceDataCalendar& >(object.getData(PRICING_DATA_FIXINGCALENDAR,ISDEFINED).get());

	AQLPriceDataSlidingRule slr_Fol; slr_Fol.convertFromString(FOL);
    AQLDate tmpDate = AQLMathDateCalculations::getDate(expiry,spotLag,slr_Fol,&fixCal,true);
    AQLDate endDate = AQLMathDateCalculations::getDate(tmpDate,tenor,paySlr,&payCal,true);
    DateVector dates;
	double rate;
	if (freq == SIMPLE)
	{
		dates.push_back(endDate);	
		dates.insert(dates.begin(),tmpDate);
		rate = AQLMathCurveFuncUtility::getParRate(dates, dataInstance, curveID, daycount.convertToString(), AQLString("SPLINE"), foreCurveName, dfCurveName, isFWDInter);
	}
	else
	{
		//AQLMathDateCalculations::generateSchedule(tmpDate, endDate, freq, true, NULL, NULL, NULL, dates, &paySlr, &payCal);
		rate = AQLMathCurveFuncUtility::getParRate(dataInstance, curveID, tmpDate, endDate, NULL, NULL, NULL, freq, daycount.convertToString(), 
												paySlr.convertToString(), payCal.convertToString(), AQLString("SPLINE"), foreCurveName, dfCurveName, isFWDInter);
	}

	return rate;
}

double 
AQLMathSwaptionVolUtility::
getNumeraire(AQLDataInstance* dataInstance, const AQLDate& expiry, const AQLString& tenor, 
			 const AQLString& curveID, const AQLString& convID, AQLString curveName)
{
	AQLPriceDataDayCount dc_act365(ACT_365_ISDA);
	AQLObjectPool& objPool = dataInstance->getObjectPool();

	const AQLObject& object = objPool.getObject( convID, ENCHKTYPE_ISDEFINED ).get();
	const AQLDataHolder* dh;
	dh = &object.getData(IR_CALIBRATION_DATA_FREQUENCY + AQLString("_") + tenor,NOCHECK);
	if(!dh->isDefined() || dh->isNull())
	{
		dh = &object.getData(IR_CALIBRATION_DATA_FREQUENCY,ISDEFINED);
	}
    const AQLString& freq = dynamic_cast<const AQLDataString& >(dh->get()).get();
    const AQLString& spotLag = 
        dynamic_cast<const AQLDataString& >(object.getData(CURVEINPUT_SPOTLAG,ISDEFINED).get()).get();
    const AQLPriceDataDayCount& daycount =
        dynamic_cast<const AQLPriceDataDayCount& >(object.getData(IR_CALIBRATION_DATA_DAYCOUNT,ISDEFINED).get());
    const AQLPriceDataSlidingRule& paySlr = 
        dynamic_cast<const AQLPriceDataSlidingRule& >(object.getData(CALIBRATION_DATA_SLIDINGRULE,ISDEFINED).get());
    const AQLPriceDataCalendar& payCal =
        dynamic_cast<const AQLPriceDataCalendar& >(object.getData(CALIBRATION_DATA_CALENDAR,ISDEFINED).get());
    const AQLPriceDataCalendar& fixCal =
        dynamic_cast<const AQLPriceDataCalendar& >(object.getData(PRICING_DATA_FIXINGCALENDAR,ISDEFINED).get());
	const AQLObject& curve = objPool.getObject( curveID, ENCHKTYPE_ISDEFINED ).get();
	const AQLDate& asOfDate = dynamic_cast<const AQLDataDate& >(curve.getData(CALIBRATION_DATA_ASOFDATE,ISDEFINED).get());

	AQLPriceDataSlidingRule slr_Fol; slr_Fol.convertFromString(FOL);
	AQLDate tmpDate = AQLMathDateCalculations::getDate(expiry,spotLag,slr_Fol,&fixCal,true);
    AQLDate endDate = AQLMathDateCalculations::getDate(tmpDate,tenor,paySlr,&payCal,true);
    const AQLDate spotDate = AQLMathDateCalculations::getDate(asOfDate, spotLag, slr_Fol, &fixCal, true);
	double adjust_term = dc_act365.getTerm(asOfDate, spotDate);
	double adjustDF = AQLMathCurveFuncUtility::getDF(adjust_term, dataInstance, curveID, dc_act365.convertToString(), 
		AQLString("SPLINE"),false,curveName);
    DateVector dates;
	if(freq==SIMPLE)
	{
		dates.push_back(endDate);	
	}
	else
	{
		AQLMathDateCalculations::generateSchedule(tmpDate, endDate, freq, true, NULL, NULL, NULL, dates, &paySlr, &payCal);
	}
    dates.insert(dates.begin(),tmpDate);

	return AQLMathCurveFuncUtility::getAnnuity(dates, dataInstance, curveID, daycount.convertToString(), AQLString("SPLINE"),
		curveName) / adjustDF;
}

double 
AQLMathSwaptionVolUtility::
getExpiryPoint(AQLString str, const AQLDate& asOfDate, const AQLPriceDataSlidingRule& slr, 
			   const AQLPriceDataCalendar& cal)
{
	AQLPriceDataDayCount dc_act365(ACT_365_ISDA);
	AQLDate date;
	double ret;
	str.toUpper();
	int y_pos=-1, m_pos=-1, d_pos=-1, dot_pos=-1;
	y_pos = str.findString("Y");
	m_pos = str.findString("M");
	d_pos = str.findString("D");

	if (y_pos == -1 && m_pos == -1 && d_pos == -1) 
	{
		date = getAQLDate(str.getCString());
		ret = dc_act365.getTerm(asOfDate, date);
	}
	else
	{
		date = AQLMathDateCalculations::getDate(asOfDate,str,slr,&cal,true);
		ret = dc_act365.getTerm(asOfDate, date);
	}

	return ret;
}

AQLDate 
AQLMathSwaptionVolUtility::
getExpiryPoint2(AQLString str, const AQLDate& asOfDate, const AQLPriceDataSlidingRule& slr, 
				const AQLPriceDataCalendar& cal)

{
	AQLDate date;
	str.toUpper();
	int y_pos=-1, m_pos=-1, d_pos=-1;
	y_pos = str.findString("Y");
	m_pos = str.findString("M");
	d_pos = str.findString("D");
	if (y_pos == -1 && m_pos == -1 && d_pos == -1) 
	{
		date = getAQLDate(str.getCString());
	}
	else
	{
		date = AQLMathDateCalculations::getDate(asOfDate,str,slr,&cal,true);
	}

	return date;
}

double 
AQLMathSwaptionVolUtility::
getTenorPoint(AQLString str)
{
	AQLDate date;
	double ret;
	str.toUpper();
	int y_pos=-1, m_pos=-1, d_pos=-1, w_pos=-1;
	y_pos = str.findString("Y");
	m_pos = str.findString("M");
	d_pos = str.findString("D");
	//if (y_pos == -1 && m_pos == -1 && d_pos == -1) 
	if (y_pos == -1 && m_pos == -1 && d_pos == -1 && w_pos == -1) 
	{
		ret = str.getDoubleValue();
	}
	else
	{
		AQLMathDateCalculations::termStrtoYMDW(str, y_pos, m_pos, d_pos, w_pos);
		ret = static_cast<double > (y_pos) + static_cast<double > (m_pos) / 12 + static_cast<double > (w_pos) / 12 / 4 + static_cast<double > (d_pos) / 365.25;
	}

	return ret;
}

AQLDate
AQLMathSwaptionVolUtility::getAQLDate(AQLString date)
{
    return AQLStringToDate(date);
	//AQLDate ret;
	//int slushCheck = date.findString("/");
	//if(slushCheck==-1)
	//{
	//	ret =  AQLMathDateUtilities::getAQLDate(date);
	//}
	//else
	//{
	//	if(slushCheck==4) date.remove(4,1);
	//	else throw AQLCoreInvalidData("Input error",__FILE__,__LINE__);

	//	slushCheck = date.findString("/");
	//	if(slushCheck==6) { date.remove(6,1);}
	//	else if(slushCheck==5)
	//	{
	//		date.remove(5,1);
	//		date.insert(4,"0");
	//	}
	//	else throw AQLCoreInvalidData("Input error",__FILE__,__LINE__);

	//	if(date.size() == 7)
	//	{
	//		date.insert(6,"0");
	//	}
	//	else if(date.size() != 8) throw AQLCoreInvalidData("Input error",__FILE__,__LINE__);
	//	AQLDate ret_(date.getCString()); ret = ret_;
	//}
	//
	//return ret;
}

AQLMathSABR*
AQLMathSwaptionVolUtility::createSABR(const AQLString& approxMethod, double alpha, double beta, double nu, double rho, bool isLognormal)
{
	AQLMathSABR* sabr;
	AQLString tmp_approxMethod = approxMethod;
	tmp_approxMethod.toUpper();

	//Only Hagan method support normal vol
	if (!isLognormal && tmp_approxMethod != APPROXIMATION_HAGAN)
	{
		AQLString msg = "Normal Vol Calibration Only Supports Hagan Approximation : Method " + approxMethod + " is not supported";
		throw AQLCoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}

	if(tmp_approxMethod == APPROXIMATION_HAGAN)
	{
	    sabr = new AQLMathSABR_Hagan(alpha, beta, nu, rho, isLognormal);
	}
	else if(tmp_approxMethod == APPROXIMATION_ANTONOV)
	{
	    sabr = new AQLMathSABR_Antonov(alpha, beta, nu, rho);
	}
	else if(tmp_approxMethod == APPROXIMATION_CHAOS_DD)
	{
	    sabr = new AQLMathSABR_Chaos_DD(alpha, beta, nu, rho);
	}
	else
	{
		AQ_THROW("Lognormal Vol Calibration Only SUpports Hagan, Antonov and Chaos Displaced Diffusion methods : Method " + approxMethod + " is not supported")
	}
	return sabr;
}


void
AQLMathSwaptionVolUtility::setUpForwardShiftValue(AQLDataInstance* dataInstance, AQLString& CurrencyID, double forwardShiftValue)
{

	upper(CurrencyID);
	AQLObjectPool& objPool = dataInstance->getObjectPool();
	AQLObject* e = NULL;
	if(!objPool.getObject(CurrencyID).isDefined())
	{
		e = new AQLObject();
		objPool.set(CurrencyID,e);
	}
	else 
	{
		objPool.getObject(CurrencyID,ENCHKTYPE_ISDEFINED).clear();
		e = &objPool.getObject(CurrencyID).get();
	}

	e->add(CurrencyID, new AQLDataDouble(forwardShiftValue)	);
}

double
AQLMathSwaptionVolUtility::getForwardShiftValue(AQLDataInstance* dataInstance, AQLString& CurrencyID )
{
	upper(CurrencyID);

	AQLObjectPool& objPool = dataInstance->getObjectPool();
	AQLObject& e = objPool.getObject(CurrencyID, ENCHKTYPE_ISDEFINED).get();
	
	AQLDataHolder* dh;

	dh = &(e.getData(CurrencyID, ISNOTNULL));
	double forwardShiftValue = dynamic_cast<const AQLDataDouble &>(dh->get()).get();
	
	return forwardShiftValue;
}