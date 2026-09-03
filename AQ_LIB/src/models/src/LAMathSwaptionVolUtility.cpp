#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include <vector>

#include <LADataInstance.h>
#include <LAFunctionUtilities.h>
#include <LAPriceDataCalendar.h>
#include <LAPriceDataSlidingRule.h>
#include <LAPriceDataCalendar.h>
#include <LADataMatrix.h>
#include <LAMathDefine.h>
#include "LANl2sol.h"

#include <LAMathYieldCurve.h>
#include <LAMathDateCalculations.h>
#include <LAPriceCashFlowGenerator.h>

#include "LAMathSwaptionVolUtility.h"
#include "LAMathDateUtilities.h"
#include "LAMathCurveFuncUtility.h"
#include "LAMathInterpolationUtilities.h"
#include "LAMathSABR.h"
#include "LAMathLeastSquareSABR.h"
#include "LAMathLeastSquareSABRQlib.h"
#include "LAMathSwaptionSABR.h"
#include "LAMathCapFloorSABR.h"
#include "LAMathSABR.h"
#include "LAModelUtilities.h"
#include "LAQuantLibConstraint.h"
#include "LAQuantLibProblem.h"
#include "LAQuantLibOptimizationMethod.h"
#include "LAQuantLibEndCriteria.h"
#include "LAQuantLibConjugateGradient.h"
#include "LAQuantLibSteepestDescent.h"
#include "LAQuantLibSimplex.h"
#include "LAQuantLibLevenbergMarquardt.h"
#include "LAAlgorithm.h"

using namespace std;

// (copied by LADate.cpp) 
static const unsigned short LOOKUP_TABLE_NUMBER_OF_DAYS_IN_A_MONTH_OR_YEAR[2][2][12] =
{ { { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
{ 0, 31, 59, 90,120,151,181,212,243,273,304,334 } },
{ { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
{ 0, 31, 60, 91,121,152,182,213,244,274,305,335 } } };

static const double OPTION_SIGN_DUMMY = -9999.;

void
LAMathSwaptionVolUtility::setUpSABRGrid( LADataInstance* dataInstance, const LAString& matID, const LAString& convID, 
									   LAStringMatrix& mat )
{
    LAString daycount(AC_365I);
    const LAObject& object = dataInstance->getObjectPool().getObject( convID, ENCHKTYPE_ISDEFINED ).get();
    const LAPriceDataCalendar& cal =
        dynamic_cast<const LAPriceDataCalendar& >(object.getData(PRICING_DATA_FIXINGCALENDAR,ISDEFINED).get());
    const LAPriceDataSlidingRule& sr = 
        dynamic_cast<const LAPriceDataSlidingRule& >(object.getData(CALIBRATION_DATA_SLIDINGRULE,ISDEFINED).get());
    const LADate asOfDate = 
        dynamic_cast<const LADataDate& >(object.getData(CALIBRATION_DATA_ASOFDATE,ISNOTNULL).get()).get();
	if(mat.size()<=1 || mat[0].size()<=1) throw LACoreInvalidData("the size of matrix is small!", __FILE__, __LINE__);

    DoubleVector tenorVec,expiryVec;
    LAStringVector tenorStr,expiryStr;
    DateVector expiDateVec;
    for(size_t i=1; i<mat.size(); i++)
    {
        expiryStr.push_back(mat[i][0]);
        expiDateVec.push_back( LAMathDateCalculations::getDate(asOfDate,mat[i][0],sr,&cal,true) );
        expiryVec.push_back( LAMathDateUtilities::getTerm(asOfDate, expiDateVec[i-1], daycount, true) );
    }

    int y,m,d,w;
    for(size_t i=1; i<mat[0].size(); i++)
    {
        tenorStr.push_back(mat[0][i]);
        LAMathDateCalculations::termStrtoYMDW(mat[0][i], y, m, d, w);
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
    LAObjectPool &objPool = dataInstance->getObjectPool();
    LAObjectHolder objHolder = objPool.getObject(matID ,ENCHKTYPE_NOCHECK);
	if(!objHolder.isDefined())
	{
		LAObject* e = new LAObject();
		e->add( CALIBRATION_DATA_NAME, new LADataString() ).convertFromString(matID);
		e->add( CALIBRATION_DATA_ASOFDATE, new LADataDate(asOfDate));
		e->add( CALIBRATION_DATA_CALENDAR, new LAPriceDataCalendar(cal));
		e->add( CALIBRATION_DATA_SLIDINGRULE, new LAPriceDataSlidingRule(sr));
        e->add( PRICING_DATA_SWAPTIONMATRIX, new LADataDoubleMatrix( mat_per ));
		e->add( PRICING_DATA_EXPIRYDATEVECTOR,	new LADataDates(expiDateVec));
		e->add( PRICING_DATA_EXPIRYVECTOR,	new LADataDoubles(expiryVec));
        e->add( PRICING_DATA_TENORVECTOR,	new LADataDoubles(tenorVec));
        e->add( PRICING_DATA_EXPIRYSTRING,	new LADataStrings(expiryStr));
        e->add( PRICING_DATA_TENORSTRING,	new LADataStrings(tenorStr));
		objPool.set( matID , e );
	}
	else if(objHolder.isDefined())
	{
        LADataHolder* dh;
		dh = &objHolder.getData(CALIBRATION_DATA_ASOFDATE,ISDEFINED);
		LADataDate& asOfDate_att = dynamic_cast<LADataDate &>(dh->get());
        asOfDate_att.set( asOfDate );	

		dh = &objHolder.getData(CALIBRATION_DATA_CALENDAR,ISDEFINED);
		LAPriceDataCalendar& cal_att = dynamic_cast<LAPriceDataCalendar &>(dh->get());
        cal_att = cal;

		dh = &objHolder.getData(CALIBRATION_DATA_SLIDINGRULE,ISDEFINED);
		LAPriceDataSlidingRule& sr_att = dynamic_cast<LAPriceDataSlidingRule &>(dh->get());
        sr_att = sr;

		dh = &objHolder.getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED);
		LADataDoubleMatrix& mat_att = dynamic_cast<LADataDoubleMatrix &>(dh->get());
        mat_att.set( mat_per );		

		dh = &objHolder.getData(PRICING_DATA_EXPIRYDATEVECTOR,ISDEFINED);
		LADataDates& expiryDateVec_att = dynamic_cast<LADataDates &>(dh->get());
		expiryDateVec_att.set(expiDateVec);

		dh = &objHolder.getData(PRICING_DATA_EXPIRYVECTOR,ISDEFINED);
		LADataDoubles& expiryVec_att = dynamic_cast<LADataDoubles &>(dh->get());
		expiryVec_att.set(expiryVec);

        dh = &objHolder.getData(PRICING_DATA_TENORVECTOR,ISDEFINED);
		LADataDoubles& tenorVec_att = dynamic_cast<LADataDoubles &>(dh->get());
		tenorVec_att.set(tenorVec);

        dh = &objHolder.getData(PRICING_DATA_EXPIRYSTRING,ISDEFINED);
		LADataStrings& expiryStr_att = dynamic_cast<LADataStrings &>(dh->get());
		expiryStr_att.set(expiryStr);

        dh = &objHolder.getData(PRICING_DATA_TENORSTRING,ISDEFINED);
		LADataStrings& tenorStr_att = dynamic_cast<LADataStrings &>(dh->get());
		tenorStr_att.set(tenorStr);        
	}
}

void
LAMathSwaptionVolUtility::setUpSwaptionVol( LADataInstance* dataInstance, const LAString& matID, LAStringMatrix& volMat, LAStringMatrix& strikeMat, LAStringMatrix& signMat)
{
	if(strikeMat.size()==0 || strikeMat[0].size()==0) throw LACoreInvalidData("the size of matrix is zero!", __FILE__, __LINE__);
	if(volMat.size()==0 || volMat[0].size()==0) throw LACoreInvalidData("the size of matrix is zero!", __FILE__, __LINE__);

    //vol matrix
    LAStringVector tenorStr_vol,expiryStr_vol;
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
    LAStringVector tenorStr_K,expiryStr_K;
    for(size_t i=1; i<strikeMat.size(); i++)
    {
        expiryStr_K.push_back(strikeMat[i][0]);
    }
    for(size_t i=1; i<strikeMat[0].size(); i++)
    {
        tenorStr_K.push_back(strikeMat[0][i]);
    }
    if(tenorStr_vol != tenorStr_K ||  expiryStr_vol != expiryStr_K)
        throw LACoreInvalidData("strike matrix and vol matrix are inconsistent!",	__FILE__,__LINE__);

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
		LAStringVector tenorStr_Sign, expiryStr_Sign;
		for (size_t i = 1; i < signMat.size(); i++)
		{
			expiryStr_Sign.push_back(signMat[i][0]);
		}
		for (size_t i = 1; i < signMat[0].size(); i++)
		{
			tenorStr_Sign.push_back(signMat[0][i]);
		}
		if (tenorStr_vol != tenorStr_Sign || expiryStr_vol != expiryStr_Sign)
			throw LACoreInvalidData("sign matrix and vol matrix are inconsistent!", __FILE__, __LINE__);

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
    LAObjectPool &objPool = dataInstance->getObjectPool();
    LAObjectHolder objHolder = objPool.getObject(matID ,ENCHKTYPE_NOCHECK);
	if(!objHolder.isDefined())
	{
		LAObject* e = new LAObject();
		e->add( CALIBRATION_DATA_NAME, new LADataString() ).convertFromString(matID);
        e->add( PRICING_DATA_SWAPTIONMATRIX, new LADataDoubleMatrix( volMat_per ));
        e->add( PRICING_DATA_SWAPTIONSTRIKEMATRIX, new LADataDoubleMatrix( strikeMat_per ));
		e->add( PRICING_DATA_SWAPTIONSIGNMATRIX, new LADataDoubleMatrix( signMat_per ));
        e->add( PRICING_DATA_EXPIRYSTRING,	new LADataStrings(expiryStr_vol));
        e->add( PRICING_DATA_TENORSTRING,	new LADataStrings(tenorStr_vol));
		objPool.set( matID , e );
	}
	else if(objHolder.isDefined())
	{
        LADataHolder* dh;

        dh = &objHolder.getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED);
		LADataDoubleMatrix& matVol_att = dynamic_cast<LADataDoubleMatrix &>(dh->get());
        matVol_att.set( volMat_per );

        dh = &objHolder.getData(PRICING_DATA_SWAPTIONSTRIKEMATRIX,ISDEFINED);
		LADataDoubleMatrix& matStri_att = dynamic_cast<LADataDoubleMatrix &>(dh->get());
        matStri_att.set( strikeMat_per );

		dh = &objHolder.getData(PRICING_DATA_SWAPTIONSIGNMATRIX, ISDEFINED);
		LADataDoubleMatrix& matSign_att = dynamic_cast<LADataDoubleMatrix &>(dh->get());
		matSign_att.set( signMat_per );

        dh = &objHolder.getData(PRICING_DATA_EXPIRYSTRING,ISDEFINED);
		LADataStrings& expiryStr_att = dynamic_cast<LADataStrings &>(dh->get());
		expiryStr_att.set(expiryStr_vol);

        dh = &objHolder.getData(PRICING_DATA_TENORSTRING,ISDEFINED);
		LADataStrings& tenorStr_att = dynamic_cast<LADataStrings &>(dh->get());
		tenorStr_att.set(tenorStr_vol);        
	}
}

double
LAMathSwaptionVolUtility::lookUpSwapGrid( LADataInstance* dataInstance, const LAString& matID, 
                                        LAString expPoint, LAString tenorPoint )
{
	LAPriceDataDayCount dc_act365(ACT_365_ISDA);

    LAObjectPool& objPool = dataInstance->getObjectPool();
    const DoubleMatrix& mat = dynamic_cast<LADataDoubleMatrix& >
        (objPool.getObject( matID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
    const DoubleVector& tenorVec = dynamic_cast<const LADataDoubles& >
        (objPool.getObject( matID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_TENORVECTOR,ISDEFINED).get()).get();
    const DoubleVector& expiryVec = dynamic_cast<const LADataDoubles& >
        (objPool.getObject( matID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_EXPIRYVECTOR,ISDEFINED).get()).get();
    const LAPriceDataSlidingRule& paySlr = dynamic_cast<const LAPriceDataSlidingRule& >(objPool.getObject( matID, ENCHKTYPE_ISDEFINED ).
		get().getData(CALIBRATION_DATA_SLIDINGRULE,ISDEFINED).get());
    const LAPriceDataCalendar& fixCal = dynamic_cast<const LAPriceDataCalendar& >(objPool.getObject( matID, ENCHKTYPE_ISDEFINED ).get().
		getData(CALIBRATION_DATA_CALENDAR,ISDEFINED).get());
    const LADate& asOfDate = dynamic_cast<const LADataDate& >(dataInstance->getObjectPool().getObject(matID,ENCHKTYPE_ISDEFINED).
		get().getData(CALIBRATION_DATA_ASOFDATE,ISNOTNULL).get()).get();

	double expPoint_d = getExpiryPoint(expPoint, asOfDate, paySlr, fixCal);
	double tenorPoint_d = getTenorPoint(tenorPoint);

    LAMathSwaptionMatrix swaptionMat(mat, expiryVec, tenorVec);
    
    return swaptionMat.lookUpMatrix(expPoint_d, tenorPoint_d);
}

double
LAMathSwaptionVolUtility::lookUpSwapGrid(LADataInstance* dataInstance, const LAString& matID, LADate expDate, LAString tenorPoint)
{
    LAPriceDataDayCount dc_act365(ACT_365_ISDA);

    LAObjectPool& objPool = dataInstance->getObjectPool();
    const DoubleMatrix& mat = dynamic_cast<LADataDoubleMatrix& >
        (objPool.getObject(matID, ENCHKTYPE_ISDEFINED).get().getData(PRICING_DATA_SWAPTIONMATRIX, ISDEFINED).get()).get();
    const DoubleVector& tenorVec = dynamic_cast<const LADataDoubles& >
        (objPool.getObject(matID, ENCHKTYPE_ISDEFINED).get().getData(PRICING_DATA_TENORVECTOR, ISDEFINED).get()).get();
    const DoubleVector& expiryVec = dynamic_cast<const LADataDoubles& >
        (objPool.getObject(matID, ENCHKTYPE_ISDEFINED).get().getData(PRICING_DATA_EXPIRYVECTOR, ISDEFINED).get()).get();
    const LAPriceDataSlidingRule& paySlr = dynamic_cast<const LAPriceDataSlidingRule& >(objPool.getObject(matID, ENCHKTYPE_ISDEFINED).
        get().getData(CALIBRATION_DATA_SLIDINGRULE, ISDEFINED).get());
    const LAPriceDataCalendar& fixCal = dynamic_cast<const LAPriceDataCalendar& >(objPool.getObject(matID, ENCHKTYPE_ISDEFINED).get().
        getData(CALIBRATION_DATA_CALENDAR, ISDEFINED).get());
    const LADate& asOfDate = dynamic_cast<const LADataDate& >(dataInstance->getObjectPool().getObject(matID, ENCHKTYPE_ISDEFINED).
        get().getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL).get()).get();

    double expPoint_d = ModelTime(asOfDate, expDate);
    double tenorPoint_d = getTenorPoint(tenorPoint);

    LAMathSwaptionMatrix swaptionMat(mat, expiryVec, tenorVec);

    return swaptionMat.lookUpMatrix(expPoint_d, tenorPoint_d);
}

void 
LAMathSwaptionVolUtility::outPutSABRGrid( LADataInstance* dataInstance, const LAString& matID, 
                                        DoubleVector& ret, size_t& row, size_t& colum )
{
    LAObjectPool& objPool = dataInstance->getObjectPool();
    const LAObject& object = objPool.getObject( matID, ENCHKTYPE_ISDEFINED ).get();

    const DoubleMatrix& mat = dynamic_cast<const LADataDoubleMatrix &>
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
LAMathSwaptionVolUtility::setUpConvention( LADataInstance* dataInstance, const LAString& convID, LAStringMatrix& convMat )
{
	LAString curveID = searchbyrow(convMat, "CurveID",1,false);
	LAString oneMCurveName = searchbyrow(convMat, "1MLName",1,false);
	LAString threeMCurveName = searchbyrow(convMat, "3MLName",1,false);
	LAString sixMCurveName = searchbyrow(convMat, "6MLName",1,false);
	LAString swapCurveName = searchbyrow(convMat, "SwapRateName",1,false);
	LAString discountCurveName = searchbyrow(convMat, "DFName",1,false);

	uppermat(convMat);
	LAString asOfDate_str = searchbyrow(convMat,CURVEINPUT_ASOFDATE,1,false);
    LAString freq = searchbyrow(convMat, CURVEINPUT_FREQUENCY,1,false);
    LAString daycount = searchbyrow(convMat, CURVEINPUT_DAYCOUNT,1,false);
    LAString paySlr = searchbyrow(convMat, CURVEINPUT_SLIDINGRULE,1,false);
    LAString spotLag = searchbyrow(convMat, CURVEINPUT_SPOTLAG,1,false);
    LAString payCal = searchbyrow(convMat, "PAYMENTCALENDAR",1,false);
    LAString fixCal = searchbyrow(convMat, "FIXINGCALENDAR",1,false);

	LAPriceDataSlidingRule paySlr_att; LAPriceDataCalendar payCal_att, fixCal_att;  LAPriceDataDayCount daycount_att;
	if(paySlr != LAString("")) { paySlr_att.convertFromString(paySlr); }
	if(payCal != LAString("")) { payCal_att.convertFromString(payCal); }
	if(fixCal != LAString("")) { fixCal_att.convertFromString(fixCal); }
	if(daycount != LAString("")) { daycount_att.convertFromString(daycount); }
	LADate asOfDate;
	if(asOfDate_str != LAString("")) 
	{
		asOfDate = LAMathDateUtilities::getLADate(asOfDate_str);
	}
	else if(curveID != LAString(""))
	{
		const LAObject& object = dataInstance->getObjectPool().getObject( curveID, ENCHKTYPE_ISDEFINED ).get();
		LADate asOfDate_curve = dynamic_cast<const LADataDate &>(object.getData(CALIBRATION_DATA_ASOFDATE,ISDEFINED).get()).get();
		if(asOfDate_str != LAString("") && asOfDate != asOfDate_curve) throw LACoreInvalidData("asOfDates are inconsistent!",	__FILE__,__LINE__);
		else asOfDate = asOfDate_curve;
	}
	else
	{
		throw LACoreInvalidData("input asOfDate!",	__FILE__,__LINE__);
	}

    LAObjectPool &objPool = dataInstance->getObjectPool();
    LAObjectHolder objHolder = objPool.getObject(convID, ENCHKTYPE_NOCHECK);
    if(!objHolder.isDefined())
	{
		LAObject* e = new LAObject();
		e->add( CALIBRATION_DATA_NAME, new LADataString()).convertFromString(convID);
		if(asOfDate_str != LAString("") || curveID != LAString("")) { e->add( CALIBRATION_DATA_ASOFDATE, new LADataDate(asOfDate) ); }
		if(curveID != LAString("")) { e->add( CALIBRATION_DATA_CURVEID, new LADataString()).convertFromString(curveID); }
		if(oneMCurveName != LAString("")) { e->add( CALIBRATION_DATA_1MLCURVENAME, new LADataString()).convertFromString(oneMCurveName); }
		if(threeMCurveName != LAString("")) { e->add( CALIBRATION_DATA_3MLCURVENAME, new LADataString()).convertFromString(threeMCurveName); }
		if(sixMCurveName != LAString("")) { e->add( CALIBRATION_DATA_6MLCURVENAME, new LADataString()).convertFromString(sixMCurveName); }
		if(swapCurveName != LAString("")) { e->add( CALIBRATION_DATA_SWAPRATELCURVENAME, new LADataString()).convertFromString(swapCurveName); }
		if(discountCurveName != LAString("")) { e->add( CALIBRATION_DATA_DISCOUNTCURVENAME, new LADataString()).convertFromString(discountCurveName); }
        if(freq != LAString("")) { e->add( IR_CALIBRATION_DATA_FREQUENCY, new LADataString(freq)); }
        if(daycount != LAString("")) { e->add( IR_CALIBRATION_DATA_DAYCOUNT, new LAPriceDataDayCount(daycount_att)); }
        if(spotLag != LAString("")) { e->add( CURVEINPUT_SPOTLAG, new LADataString(spotLag)); }
        if(paySlr != LAString("")) { e->add( CALIBRATION_DATA_SLIDINGRULE, new LAPriceDataSlidingRule(paySlr_att)); }
        if(payCal != LAString("")) { e->add( CALIBRATION_DATA_CALENDAR	, new LAPriceDataCalendar(payCal_att)); }
        if(fixCal != LAString("")) { e->add( PRICING_DATA_FIXINGCALENDAR, new LAPriceDataCalendar(fixCal_att)); }

		objPool.set( convID , e );
	}
	else if(objHolder.isDefined())
	{
        LADataHolder* dh;

		if(asOfDate_str != LAString("") || curveID != LAString(""))
		{
			dh = &objHolder.getData(CALIBRATION_DATA_ASOFDATE,ISDEFINED);
			LADataDate& asOfDate_att = dynamic_cast<LADataDate &>(dh->get());
			asOfDate_att.set(asOfDate);
		}

		if(curveID != LAString(""))
		{
			dh = &objHolder.getData(CALIBRATION_DATA_CURVEID,ISDEFINED);
			LADataString& curveid_att = dynamic_cast<LADataString &>(dh->get());
			curveid_att.set(curveID);
		}

		if(oneMCurveName != LAString(""))
		{
			dh = &objHolder.getData(CALIBRATION_DATA_1MLCURVENAME,ISDEFINED);
			LADataString& curveid_att = dynamic_cast<LADataString &>(dh->get());
			curveid_att.set(oneMCurveName);
		}

		if(threeMCurveName != LAString(""))
		{
			dh = &objHolder.getData(CALIBRATION_DATA_3MLCURVENAME,ISDEFINED);
			LADataString& curveid_att = dynamic_cast<LADataString &>(dh->get());
			curveid_att.set(threeMCurveName);
		}

		if(sixMCurveName != LAString(""))
		{
			dh = &objHolder.getData(CALIBRATION_DATA_6MLCURVENAME,ISDEFINED);
			LADataString& curveid_att = dynamic_cast<LADataString &>(dh->get());
			curveid_att.set(sixMCurveName);
		}

		if(swapCurveName != LAString(""))
		{
			dh = &objHolder.getData(CALIBRATION_DATA_SWAPRATELCURVENAME,ISDEFINED);
			LADataString& curveid_att = dynamic_cast<LADataString &>(dh->get());
			curveid_att.set(swapCurveName);
		}

		if(discountCurveName != LAString(""))
		{
			dh = &objHolder.getData(CALIBRATION_DATA_DISCOUNTCURVENAME,ISDEFINED);
			LADataString& curveid_att = dynamic_cast<LADataString &>(dh->get());
			curveid_att.set(discountCurveName);
		}

		if(freq != LAString(""))
		{
			dh = &objHolder.getData(IR_CALIBRATION_DATA_FREQUENCY,ISDEFINED);
			LADataString& freq_ = dynamic_cast<LADataString &>(dh->get());
			freq_.set(freq);
		}

		if(daycount != LAString(""))
		{
			dh = &objHolder.getData(IR_CALIBRATION_DATA_DAYCOUNT,ISDEFINED);
			LAPriceDataDayCount& daycount_att_ = dynamic_cast<LAPriceDataDayCount &>(dh->get());
			daycount_att_ = daycount_att;      
		}

		if(spotLag != LAString(""))
		{
			dh = &objHolder.getData(CURVEINPUT_SPOTLAG,ISDEFINED);
			LADataString& spotLag_ = dynamic_cast<LADataString& >(dh->get());
			spotLag_.set(spotLag);
		}

		if(paySlr != LAString("")) 
		{
			dh = &objHolder.getData(CALIBRATION_DATA_SLIDINGRULE,ISDEFINED);
			LAPriceDataSlidingRule& paySlr_att_ = dynamic_cast<LAPriceDataSlidingRule& >(dh->get());
			paySlr_att_ = paySlr_att;
		}

		if(payCal != LAString(""))
		{
			dh = &objHolder.getData(CALIBRATION_DATA_CALENDAR,ISDEFINED);
			LAPriceDataCalendar& payCal_att_ = dynamic_cast<LAPriceDataCalendar& >(dh->get());
			payCal_att_ = payCal_att;
		}

		if(fixCal != LAString(""))
		{
			dh = &objHolder.getData(PRICING_DATA_FIXINGCALENDAR,ISDEFINED);
			LAPriceDataCalendar& fixCal_att_ = dynamic_cast<LAPriceDataCalendar& >(dh->get());
			fixCal_att_ = fixCal_att;
		}
	}
}

void 
LAMathSwaptionVolUtility::setCurveID(LADataInstance* dataInstance, const LAString& name, const LAString& swapConvID, const LAString& capConvID)
{
	const LAObject& object = dataInstance->getObjectPool().getObject( swapConvID, ENCHKTYPE_ISDEFINED ).get();
	LAString curveID = dynamic_cast<const LADataString& >(object.getData(CALIBRATION_DATA_CURVEID,ISDEFINED).get()).get();
	LAString oneMCurveName(""),threeMCurveName(""),sixMCurveName(""),swapCurveName(""),discountCurveName("");
	if(capConvID != LAString(""))
	{
		const LAObject& entity_cap = dataInstance->getObjectPool().getObject(capConvID, ENCHKTYPE_ISDEFINED).get();
		oneMCurveName = dynamic_cast<const LADataString& >(entity_cap.getData(CALIBRATION_DATA_1MLCURVENAME,ISDEFINED).get()).get();
		threeMCurveName = dynamic_cast<const LADataString& >(entity_cap.getData(CALIBRATION_DATA_3MLCURVENAME,ISDEFINED).get()).get();
		sixMCurveName = dynamic_cast<const LADataString& >(entity_cap.getData(CALIBRATION_DATA_6MLCURVENAME,ISDEFINED).get()).get();
		swapCurveName = dynamic_cast<const LADataString& >(entity_cap.getData(CALIBRATION_DATA_SWAPRATELCURVENAME,ISDEFINED).get()).get();
		discountCurveName = dynamic_cast<const LADataString& >(entity_cap.getData(CALIBRATION_DATA_DISCOUNTCURVENAME,ISDEFINED).get()).get();
	}
	LAObjectPool &objPool = dataInstance->getObjectPool();
    LAObjectHolder objHolder = objPool.getObject(name ,ENCHKTYPE_NOCHECK);
    if(!objHolder.isDefined())
	{
		LAObject* e = new LAObject();
		e->add( CALIBRATION_DATA_NAME, new LADataString()).convertFromString(name);
		e->add( CALIBRATION_DATA_CURVEID, new LADataString()).convertFromString(curveID); 

		if(oneMCurveName != LAString("")) 
		{ 
			e->add( CALIBRATION_DATA_1MLCURVENAME, new LADataString()).convertFromString(oneMCurveName); 
		}
		else
		{
			e->add( CALIBRATION_DATA_1MLCURVENAME, new LADataString()).convertFromString(STD); 
		}

		if(threeMCurveName != LAString("")) 
		{
			e->add( CALIBRATION_DATA_3MLCURVENAME, new LADataString()).convertFromString(threeMCurveName); 
		}
		else
		{
			e->add( CALIBRATION_DATA_3MLCURVENAME, new LADataString()).convertFromString(STD); 
		}

		if(sixMCurveName != LAString("")) 
		{ 
			e->add( CALIBRATION_DATA_6MLCURVENAME, new LADataString()).convertFromString(sixMCurveName); 
		}
		else
		{
			e->add( CALIBRATION_DATA_6MLCURVENAME, new LADataString()).convertFromString(STD); 
		}

		if(swapCurveName != LAString("")) 
		{ 
			e->add( CALIBRATION_DATA_SWAPRATELCURVENAME	, new LADataString()).convertFromString(swapCurveName); 
		}
		else
		{
			e->add( CALIBRATION_DATA_SWAPRATELCURVENAME	, new LADataString()).convertFromString(STD); 
		}

		if(discountCurveName != LAString("")) 
		{ 
			e->add( CALIBRATION_DATA_DISCOUNTCURVENAME	, new LADataString()).convertFromString(discountCurveName); 
		}
		else
		{
			e->add( CALIBRATION_DATA_DISCOUNTCURVENAME	, new LADataString()).convertFromString(STD); 
		}
      
		objPool.set( name , e );
	}
	else if(objHolder.isDefined())
	{
        LADataHolder* dh;

		dh = &objHolder.getData(CALIBRATION_DATA_CURVEID,ISDEFINED);
		LADataString& curveid_att = dynamic_cast<LADataString &>(dh->get());
		curveid_att.set(curveID);

		if(oneMCurveName != LAString(""))
		{
			dh = &objHolder.getData(CALIBRATION_DATA_1MLCURVENAME,ISDEFINED);
			LADataString& curveid_att = dynamic_cast<LADataString &>(dh->get());
			curveid_att.set(oneMCurveName);
		}
		else
		{
			dh = &objHolder.getData(CALIBRATION_DATA_1MLCURVENAME,ISDEFINED);
			LADataString& curveid_att = dynamic_cast<LADataString &>(dh->get());
			curveid_att.set(STD);
		}

		if(threeMCurveName != LAString(""))
		{
			dh = &objHolder.getData(CALIBRATION_DATA_3MLCURVENAME,ISDEFINED);
			LADataString& curveid_att = dynamic_cast<LADataString &>(dh->get());
			curveid_att.set(threeMCurveName);
		}
		else
		{
			dh = &objHolder.getData(CALIBRATION_DATA_3MLCURVENAME,ISDEFINED);
			LADataString& curveid_att = dynamic_cast<LADataString &>(dh->get());
			curveid_att.set(STD);
		}

		if(sixMCurveName != LAString(""))
		{
			dh = &objHolder.getData(CALIBRATION_DATA_6MLCURVENAME,ISDEFINED);
			LADataString& curveid_att = dynamic_cast<LADataString &>(dh->get());
			curveid_att.set(sixMCurveName);
		}
		else
		{
			dh = &objHolder.getData(CALIBRATION_DATA_6MLCURVENAME,ISDEFINED);
			LADataString& curveid_att = dynamic_cast<LADataString &>(dh->get());
			curveid_att.set(STD);
		}

		if(swapCurveName != LAString(""))
		{
			dh = &objHolder.getData(CALIBRATION_DATA_SWAPRATELCURVENAME	,NOCHECK);
			LADataString& curveid_att = dynamic_cast<LADataString &>(dh->get());
			curveid_att.set(swapCurveName);
		}
		else
		{
			dh = &objHolder.getData(CALIBRATION_DATA_SWAPRATELCURVENAME	,NOCHECK);
			LADataString& curveid_att = dynamic_cast<LADataString &>(dh->get());
			curveid_att.set(STD);
		}

		if(discountCurveName != LAString(""))
		{
			dh = &objHolder.getData(CALIBRATION_DATA_DISCOUNTCURVENAME	,NOCHECK);
			LADataString& curveid_att = dynamic_cast<LADataString &>(dh->get());
			curveid_att.set(discountCurveName);
		}
		else
		{
			dh = &objHolder.getData(CALIBRATION_DATA_DISCOUNTCURVENAME	,NOCHECK);
			LADataString& curveid_att = dynamic_cast<LADataString &>(dh->get());
			curveid_att.set(STD);
		}
	}
}

void 
LAMathSwaptionVolUtility::setCurveID2(LADataInstance* dataInstance, const LAString& name, LAStringMatrix& curveMat)
{
	LAString curveID = searchbyrow(curveMat, "CurveID",1,true);
	LAString oneMCurveName = searchbyrow(curveMat, "1MLName",1,true);
	LAString threeMCurveName = searchbyrow(curveMat, "3MLName",1,true);
	LAString sixMCurveName = searchbyrow(curveMat, "6MLName",1,true);
	LAString swapCurveName = searchbyrow(curveMat, "SwapRateName",1,true);
	LAString discountCurveName = searchbyrow(curveMat, "DFName",1,true);

	LAObjectPool &objPool = dataInstance->getObjectPool();
    LAObjectHolder objHolder = objPool.getObject(name ,ENCHKTYPE_NOCHECK);
    if(!objHolder.isDefined())
	{
		LAObject* e = new LAObject();
		e->add( CALIBRATION_DATA_NAME, new LADataString()).convertFromString(name);
		if(curveID != LAString("")) 
		{ 
			e->add( CALIBRATION_DATA_CURVEID, new LADataString()).convertFromString(curveID); 
		}
		else
		{
			throw LACoreInvalidData("input curve ID!", __FILE__, __LINE__);
		}

		if(oneMCurveName != LAString("")) 
		{ 
			e->add( CALIBRATION_DATA_1MLCURVENAME, new LADataString()).convertFromString(oneMCurveName); 
		}
		else
		{
			e->add( CALIBRATION_DATA_1MLCURVENAME, new LADataString()).convertFromString(STD); 
		}

		if(threeMCurveName != LAString("")) 
		{
			e->add( CALIBRATION_DATA_3MLCURVENAME, new LADataString()).convertFromString(threeMCurveName); 
		}
		else
		{
			e->add( CALIBRATION_DATA_3MLCURVENAME, new LADataString()).convertFromString(STD); 
		}

		if(sixMCurveName != LAString("")) 
		{ 
			e->add( CALIBRATION_DATA_6MLCURVENAME, new LADataString()).convertFromString(sixMCurveName); 
		}
		else
		{
			e->add( CALIBRATION_DATA_6MLCURVENAME, new LADataString()).convertFromString(STD); 
		}

		if(swapCurveName != LAString("")) 
		{ 
			e->add( CALIBRATION_DATA_SWAPRATELCURVENAME	, new LADataString()).convertFromString(swapCurveName); 
		}
		else
		{
			e->add( CALIBRATION_DATA_SWAPRATELCURVENAME	, new LADataString()).convertFromString(STD); 
		}

		if(discountCurveName != LAString("")) 
		{ 
			e->add( CALIBRATION_DATA_DISCOUNTCURVENAME	, new LADataString()).convertFromString(discountCurveName); 
		}
		else
		{
			e->add( CALIBRATION_DATA_DISCOUNTCURVENAME	, new LADataString()).convertFromString(STD); 
		}
      
		objPool.set( name , e );
	}
	else if(objHolder.isDefined())
	{
        LADataHolder* dh;

		if(curveID != LAString(""))
		{
			dh = &objHolder.getData(CALIBRATION_DATA_CURVEID,NOCHECK);
			LADataString& curveid_att = dynamic_cast<LADataString &>(dh->get());
			curveid_att.set(curveID);
		}
		else
		{
			throw LACoreInvalidData("input curve ID!", __FILE__, __LINE__);
		}

		if(oneMCurveName != LAString(""))
		{
			dh = &objHolder.getData(CALIBRATION_DATA_1MLCURVENAME,ISDEFINED);
			LADataString& curveid_att = dynamic_cast<LADataString &>(dh->get());
			curveid_att.set(oneMCurveName);
		}
		else
		{
			dh = &objHolder.getData(CALIBRATION_DATA_1MLCURVENAME,ISDEFINED);
			LADataString& curveid_att = dynamic_cast<LADataString &>(dh->get());
			curveid_att.set(STD);
		}

		if(threeMCurveName != LAString(""))
		{
			dh = &objHolder.getData(CALIBRATION_DATA_3MLCURVENAME,ISDEFINED);
			LADataString& curveid_att = dynamic_cast<LADataString &>(dh->get());
			curveid_att.set(threeMCurveName);
		}
		else
		{
			dh = &objHolder.getData(CALIBRATION_DATA_3MLCURVENAME,ISDEFINED);
			LADataString& curveid_att = dynamic_cast<LADataString &>(dh->get());
			curveid_att.set(STD);
		}

		if(sixMCurveName != LAString(""))
		{
			dh = &objHolder.getData(CALIBRATION_DATA_6MLCURVENAME,ISDEFINED);
			LADataString& curveid_att = dynamic_cast<LADataString &>(dh->get());
			curveid_att.set(sixMCurveName);
		}
		else
		{
			dh = &objHolder.getData(CALIBRATION_DATA_6MLCURVENAME,ISDEFINED);
			LADataString& curveid_att = dynamic_cast<LADataString &>(dh->get());
			curveid_att.set(STD);
		}

		if(swapCurveName != LAString(""))
		{
			dh = &objHolder.getData(CALIBRATION_DATA_SWAPRATELCURVENAME	,NOCHECK);
			LADataString& curveid_att = dynamic_cast<LADataString &>(dh->get());
			curveid_att.set(swapCurveName);
		}
		else
		{
			dh = &objHolder.getData(CALIBRATION_DATA_SWAPRATELCURVENAME	,NOCHECK);
			LADataString& curveid_att = dynamic_cast<LADataString &>(dh->get());
			curveid_att.set(STD);
		}

		if(discountCurveName != LAString(""))
		{
			dh = &objHolder.getData(CALIBRATION_DATA_DISCOUNTCURVENAME	,NOCHECK);
			LADataString& curveid_att = dynamic_cast<LADataString &>(dh->get());
			curveid_att.set(discountCurveName);
		}
		else
		{
			dh = &objHolder.getData(CALIBRATION_DATA_DISCOUNTCURVENAME	,NOCHECK);
			LADataString& curveid_att = dynamic_cast<LADataString &>(dh->get());
			curveid_att.set(STD);
		}
	}
}

void 
LAMathSwaptionVolUtility::setCurveID2(LADataInstance* dataInstance, const LAString& name, const LAStringVector& sTenor, LAStringMatrix& curveMat)
{
	LAMathSwaptionVolUtility::setCurveID2(dataInstance, name, curveMat);

	LAObjectPool &objPool = dataInstance->getObjectPool();
    LAObjectHolder objHolder = objPool.getObject(name,ENCHKTYPE_NOCHECK);

	for(size_t i = 0; i < sTenor.size(); i++)
	{
		LAString swapCurveName = searchbyrow(curveMat, LAString("SwapRateName_") + sTenor[i],1,false);

		LADataHolder* dh;
		dh = &objHolder.getData(CALIBRATION_DATA_SWAPRATELCURVENAME + LAString("_") + sTenor[i], NOCHECK);		
		if(dh->isDefined() && !dh->isNull())
		{
			if(swapCurveName != LAString(""))
			{
				LADataString& curveid_att = dynamic_cast<LADataString &>(dh->get());
				curveid_att.set(swapCurveName);
			}
		}
		else
		{
			if(swapCurveName != LAString(""))
				objHolder.get().add( CALIBRATION_DATA_SWAPRATELCURVENAME + LAString("_") + sTenor[i], new LADataString()).convertFromString(swapCurveName); 
		}
	}
}

void 
LAMathSwaptionVolUtility::calibrateSABRMatrix
( LADataInstance* dataInstance, const LAString& approxMethod, const std::vector<bool>& calibFlg, const LAString& calibMethod, const LAString& curveSetID, const LAString& alphaID, const LAString& betaID, 
  const LAString& nuID, const LAString& rhoID, const LAString& swapConvID, 
  const LAString& capConvID, const LAStringVector& swapVolID, /*LAStringMatrix sabrLimiter,*/
  const LAString& target, const DoubleVector& weight, const IntVector& sgn, const LAString& forwardID, const double forwardShiftValue, 
  const LAString& numeraireID, LAString& msg, const BoolMatrix *calibFlgMtx, bool isLognormal)
{
    LAPriceDataDayCount dc_act365(ACT_365_ISDA);
	LAString tmp_target = target;
	tmp_target.toUpper();
    //upper(sabrLimiter);
    //const double eps = 0.000000001;

    //LAMathSABRLimiter limiter(sabrLimiter);

    if( swapVolID.size() != weight.size() )
        throw LACoreInvalidData("sizes are inconsistent!",	__FILE__,__LINE__);

	if( swapVolID.size() != sgn.size() )
        throw LACoreInvalidData("sizes are inconsistent!",	__FILE__,__LINE__);

	matirixCheck(dataInstance, alphaID, betaID);
	matirixCheck(dataInstance, alphaID, nuID);
	matirixCheck(dataInstance, alphaID, rhoID);

    LAObjectPool& objPool = dataInstance->getObjectPool();
    DoubleMatrix& alphaMat = dynamic_cast<LADataDoubleMatrix &>
        (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
    const LAStringVector& expiry = dynamic_cast<const LADataStrings &>
        (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_EXPIRYSTRING,ISDEFINED).get()).get();
    const LAStringVector& tenor = dynamic_cast<const LADataStrings &>
        (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_TENORSTRING,ISDEFINED).get()).get();
	const DoubleVector& expiryTerms = dynamic_cast<const LADataDoubles &>
        (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_EXPIRYVECTOR,ISDEFINED).get()).get();
	const DateVector& expiryDates = dynamic_cast<const LADataDates &>
        (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_EXPIRYDATEVECTOR,ISDEFINED).get()).get();
    DoubleMatrix& betaMat = dynamic_cast<LADataDoubleMatrix &>
        (objPool.getObject( betaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
    DoubleMatrix& nuMat = dynamic_cast<LADataDoubleMatrix &>
        (objPool.getObject( nuID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
    DoubleMatrix& rhoMat = dynamic_cast<LADataDoubleMatrix &>
        (objPool.getObject( rhoID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();

    LADataHolder* dh = &((objPool.getObject( alphaID, ENCHKTYPE_NOCHECK ).get()).getData(PRICING_DATA_GRIDAPPROXMETHOD,NOCHECK));
	LAString tmp_approxMethod = approxMethod;
	LAStringVector stmpvec(tenor.size(),tmp_approxMethod.toUpper());
	LAStringMatrix gridApproxMethodMtx(expiry.size(),stmpvec);

	if(dh->isDefined() && !dh->isNull())
	{
		gridApproxMethodMtx = dynamic_cast<const LADataStringMatrix&>(dh->get()).get();
		if (gridApproxMethodMtx.size() != expiry.size() || (gridApproxMethodMtx)[0].size() != tenor.size())
		{
			throw LACoreInvalidData("Grid Approximation Method matrix does not match swaption volatility matrix!",	__FILE__,__LINE__);
		}
	}
	
	if(calibFlgMtx)
	{
		if (calibFlgMtx->size() != expiry.size() || (*calibFlgMtx)[0].size() != tenor.size())
		{
			throw LACoreInvalidData("Calibration flag matrix does not match swaption volatility matrix!",	__FILE__,__LINE__);
		}
	}

	DoubleMatrix forwardMat, numeraireMat;
	if(forwardID != LAString("")) 
	{
		matirixCheck(dataInstance, alphaID, forwardID);
		forwardMat = dynamic_cast<LADataDoubleMatrix &>
			(objPool.getObject( forwardID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();

		numeraireMat = dynamic_cast<LADataDoubleMatrix &>
			(objPool.getObject( numeraireID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
	}

    vector<DoubleMatrix > marketVol, marketStk;
    for(size_t i=0; i<swapVolID.size(); i++)
    {
        matirixCheck(dataInstance, alphaID, swapVolID[i], true);
        marketVol.push_back(
        dynamic_cast<const LADataDoubleMatrix &> (objPool.getObject( swapVolID[i], ENCHKTYPE_ISDEFINED ).get().
        getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get() );

        marketStk.push_back(
        dynamic_cast<const LADataDoubleMatrix &> (objPool.getObject( swapVolID[i], ENCHKTYPE_ISDEFINED ).get().
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
//    LAMathSABR_Hagan sabr;
	LAString curveID,convID,foreName,dfName;
	LAObject curveEntity;
	
	if(forwardID == LAString(""))
	{
		curveEntity = dataInstance->getObjectPool().getObject( curveSetID, ENCHKTYPE_ISDEFINED ).get();
		curveID = dynamic_cast<const LADataString& >(curveEntity.getData(CALIBRATION_DATA_CURVEID,ISDEFINED).get()).get();
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
			if(forwardID == LAString(""))
			{
				if( tenor[j]==LAString("1M") || tenor[j]==LAString("3M") || tenor[j]==LAString("6M") )
				{
					convID = capConvID;
					foreName = dynamic_cast<const LADataString& >(curveEntity.getData(tenor[j]+LAString("LiborCurveName"),ISDEFINED).get()).get();
					dfName = dynamic_cast<const LADataString& >(curveEntity.getData(CALIBRATION_DATA_DISCOUNTCURVENAME,ISDEFINED).get()).get();
				}
				else
				{
					convID = swapConvID;
					foreName = dynamic_cast<const LADataString& >(curveEntity.getData(LAString("SwapRateCurveName"),ISDEFINED).get()).get();
					dfName = dynamic_cast<const LADataString& >(curveEntity.getData(CALIBRATION_DATA_DISCOUNTCURVENAME,ISDEFINED).get()).get();
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


			msg = LAString("alpha is out of range.");
			if(initValue[0] < min_alpha || initValue[0] > max_alpha) throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);

			msg = LAString("beta is out of range.");
			if(initValue[1] < min_beta || initValue[1] > max_beta) throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);

			msg = LAString("nu is out of range.");
			if(initValue[2] < min_nu || initValue[2] > max_nu) throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);

			msg = LAString("rho is out of range.");
			if(initValue[3] < min_rho || initValue[3] > max_rho) throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);

            //calibration
			LAMathSABR* sabr = createSABR(gridApproxMethodMtx[i][j], alphaMat[i][j], betaMat[i][j], nuMat[i][j], rhoMat[i][j], isLognormal);

			LAMathLeastSquareSABRCostFunc* costfunc = new LAMathLeastSquareSABRCostFunc(*sabr,
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
			LAQuantLibConstraint* constraint = 0;
			LAString tmp_approxMethod = gridApproxMethodMtx[i][j];
			tmp_approxMethod.toUpper();
			if(tmp_approxMethod == APPROXIMATION_ANTONOV)
			{
				constraint = new LAMathLeastSquareSABRConstraint_Antonov(forward,
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
				constraint = new LAMathLeastSquareSABRConstraint(initValue,
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
							throw LACoreInvalidData("All element of flg is false : LAMathLeastSquareSABRCostFunc::set_params", __FILE__, __LINE__);
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

			LAQuantLibArray x(x_.size());
			for(size_t k = 0; k < x_.size(); ++k) x[k] = x_[k];
			LAQuantLibProblem* opt_problem = new LAQuantLibProblem(*costfunc, *constraint, x);


			// Set Optimization Method
			LAQuantLibOptimizationMethod* optMethod;
			LAString tmp_calibMethod = calibMethod;
			tmp_calibMethod.toUpper();
			if (tmp_calibMethod == CALIB_NON_LINEAR_CONJUGATE_GRADIENT_METHOD)
			{
				optMethod = new LAQuantLibConjugateGradient();
			}
			else if (tmp_calibMethod == CALIB_STEEPEST_DESCENT_METHOD)
			{
				optMethod = new LAQuantLibSteepestDescent();
			}
			else if (tmp_calibMethod == CALIB_SIMPLEX_METHOD)
			{
				if (tmp_target == CALIB_TARGET_VOLATILITY)
				{
					optMethod = new LAQuantLibSimplex(0.1);
				}
				else
				{
					optMethod = new LAQuantLibSimplex(max_alpha);
				}
			}
			else if (tmp_calibMethod == CALIB_LEVENBERG_MARQUARDT_METHOD)
			{
				double levenbergMarquardtEpsfcn = 0.0000001;       // parameters specific for Levenberg-Marquardt
				double levenbergMarquardtXtol   = 0.0000001;	   //
				double levenbergMarquardtGtol   = 0.0000001;       //

				optMethod = new LAQuantLibLevenbergMarquardt(levenbergMarquardtEpsfcn,
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


				LAString msg = LAString("Optimization Method: ") + calibMethod + " not support";
				throw LACoreInvalidData(msg.getCString(),__FILE__,__LINE__);
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

				LAString msg = LAString("maxStationaryStateIteration must be smaller than maxIterration.");
				throw LACoreInvalidData(msg.getCString(),__FILE__,__LINE__);
			}

			LAQuantLibEndCriteria* endCriteria = new LAQuantLibEndCriteria(maxIteration, 
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
					LAQuantLibEndCriteria::Type endCriteriaResult = optMethod->minimize(*opt_problem, *endCriteria);
				}
				catch(std::exception& e)
				{
					msg += LAString(e.what());
					msg += LAString("(");
					msg += LAString(LAModelUtilities::n2s(i).c_str());
					msg += LAString(",");
					msg += LAString(LAModelUtilities::n2s(j).c_str());
					msg += LAString(")");
					msg += LAString(" : ");

				}
			}
			catch(LACoreError&e)
			{
				for(unsigned int k =0;k<e.getSize();k++) msg+=LAString(e.getMsg(e.getSize()-k-1));
				msg += LAString("(");
				msg += LAString(LAModelUtilities::n2s(i).c_str());
				msg += LAString(",");
				msg += LAString(LAModelUtilities::n2s(j).c_str());
				msg += LAString(")");
				msg += LAString(" : ");
			}

			//QuantLib::Array xMinCalculated = opt_problem->currentValue();

			std::shared_ptr<LAMathLeastSquareSABRCostFuncQlib> costfuncQlib = dynamic_pointer_cast<LAMathLeastSquareSABRCostFuncQlib>(costfunc->getCostFunction());
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

    LADataDoubleMatrix mat_att;
    dh = &dataInstance->getObjectPool().getObject(alphaID ,ENCHKTYPE_NOCHECK).getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED);   
    mat_att = dynamic_cast<LADataDoubleMatrix &>(dh->get());
    mat_att.set( alphaMat );

    dh = &dataInstance->getObjectPool().getObject(betaID ,ENCHKTYPE_NOCHECK).getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED);   
    mat_att = dynamic_cast<LADataDoubleMatrix &>(dh->get());
    mat_att.set( betaMat );

    dh = &dataInstance->getObjectPool().getObject(nuID ,ENCHKTYPE_NOCHECK).getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED);   
    mat_att = dynamic_cast<LADataDoubleMatrix &>(dh->get());
    mat_att.set( nuMat );

    dh = &dataInstance->getObjectPool().getObject(rhoID ,ENCHKTYPE_NOCHECK).getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED);   
    mat_att = dynamic_cast<LADataDoubleMatrix &>(dh->get());
    mat_att.set( rhoMat );
}


void
LAMathSwaptionVolUtility::calibrateSABRMatrix(DoubleMatrix& alphaMat, DoubleMatrix& betaMat, DoubleMatrix& nuMat, DoubleMatrix& rhoMat,
											const LAString& approxMethod, const std::vector<bool>& calibFlg, const LAString& calibMethod,
											const std::vector<DoubleMatrix >& marketVol, const std::vector<DoubleMatrix >& marketStrike, const DoubleVector& expiryTerm,
											const LAString& target, const DoubleVector& weight, const IntVector& sgn, const double solverEpsilon,
											const DoubleMatrix& forwardMat, const double forwardShiftValue, const DoubleMatrix& numeraireMat,
											const DoubleMatrix& atmMarketVol, const bool alphaFromAtmVol,
											LAString& msg, const BoolMatrix *calibFlgMtx, const bool isLognormal)
{

	LAString tmp_target = target;
	tmp_target.toUpper();

	//vol size is the row size of marketVol
	auto swapVolSize = marketVol.size();

	//tenor size is the column size of marketVol
	AQ_REQUIRE(marketVol.size() > 0 && marketVol[0].size() > 0, "Market Vol Matrix cannot be empty");
	auto tenorSize = marketVol[0][0].size();

	auto expirySize = expiryTerm.size();

	AQ_REQUIRE(swapVolSize == weight.size(), "Inconsistent Market Data: The number of Swap Vol Objects and Weights must match")

	AQ_REQUIRE(swapVolSize == sgn.size(), "Inconsistent Market Data: The number of Swap Vol Objects and Call/Put Sign Indicators must match")
	
	LAString tmp_approxMethod = approxMethod;
	LAStringVector stmpvec(tenorSize, tmp_approxMethod.toUpper());
	LAStringMatrix gridApproxMethodMtx(expiryTerm.size(), stmpvec);

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
			LAMathSABR* sabr = createSABR(gridApproxMethodMtx[i][j], alphaMat[i][j], betaMat[i][j], nuMat[i][j], rhoMat[i][j], isLognormal);

			double atmVol = alphaFromAtmVol ? atmMarketVol[i][j] : std::numeric_limits<double>::quiet_NaN();

			LAMathLeastSquareSABRCostFunc* costfunc = new LAMathLeastSquareSABRCostFunc(*sabr,
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
			LAQuantLibConstraint* constraint = 0;
			LAString tmp_approxMethod = gridApproxMethodMtx[i][j];
			tmp_approxMethod.toUpper();
			if (tmp_approxMethod == APPROXIMATION_ANTONOV)
			{
				constraint = new LAMathLeastSquareSABRConstraint_Antonov(forward,
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
				constraint = new LAMathLeastSquareSABRConstraint(initValue,
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

			LAQuantLibArray x(x_.size());
			for (size_t k = 0; k < x_.size(); ++k) x[k] = x_[k];
			LAQuantLibProblem* opt_problem = new LAQuantLibProblem(*costfunc, *constraint, x);


			// Set Optimization Method
			LAQuantLibOptimizationMethod* optMethod;
			LAString tmp_calibMethod = calibMethod;
			tmp_calibMethod.toUpper();
			if (tmp_calibMethod == CALIB_NON_LINEAR_CONJUGATE_GRADIENT_METHOD)
			{
				optMethod = new LAQuantLibConjugateGradient();
			}
			else if (tmp_calibMethod == CALIB_STEEPEST_DESCENT_METHOD)
			{
				optMethod = new LAQuantLibSteepestDescent();
			}
			else if (tmp_calibMethod == CALIB_SIMPLEX_METHOD)
			{
				if (tmp_target == CALIB_TARGET_VOLATILITY)
				{
					optMethod = new LAQuantLibSimplex(0.1);
				}
				else
				{
					optMethod = new LAQuantLibSimplex(max_alpha);
				}
			}
			else if (tmp_calibMethod == CALIB_LEVENBERG_MARQUARDT_METHOD)
			{
				double levenbergMarquardtEpsfcn = 0.0000001;       // parameters specific for Levenberg-Marquardt
				double levenbergMarquardtXtol = 0.0000001;	   //
				double levenbergMarquardtGtol = 0.0000001;       //

				optMethod = new LAQuantLibLevenbergMarquardt(levenbergMarquardtEpsfcn,
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

			LAQuantLibEndCriteria* endCriteria = new LAQuantLibEndCriteria(maxIteration,
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
					LAQuantLibEndCriteria::Type endCriteriaResult = optMethod->minimize(*opt_problem, *endCriteria);
				}
				catch (std::exception& e)
				{
					msg += LAString(e.what());
					msg += LAString("(");
					msg += LAString(LAModelUtilities::n2s(i).c_str());
					msg += LAString(",");
					msg += LAString(LAModelUtilities::n2s(j).c_str());
					msg += LAString(")");
					msg += LAString(" : ");

				}
			}
			catch (LACoreError&e)
			{
				for (unsigned int k = 0; k < e.getSize(); k++) msg += LAString(e.getMsg(e.getSize() - k - 1));
				msg += LAString("(");
				msg += LAString(LAModelUtilities::n2s(i).c_str());
				msg += LAString(",");
				msg += LAString(LAModelUtilities::n2s(j).c_str());
				msg += LAString(")");
				msg += LAString(" : ");
			}

			//QuantLib::Array xMinCalculated = opt_problem->currentValue();

			std::shared_ptr<LAMathLeastSquareSABRCostFuncQlib> costfuncQlib = dynamic_pointer_cast<LAMathLeastSquareSABRCostFuncQlib>(costfunc->getCostFunction());
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
LAMathSwaptionVolUtility::calibrateSABRMatrix
( LADataInstance* dataInstance, const LAString& curveSetID, const LAString& alphaID, const LAString& betaID, 
  const LAString& nuID, const LAString& rhoID, const LAString& swapConvID, 
  const LAString& capConvID, const LAStringVector& swapVolID, LAStringMatrix sabrLimiter, 
  LAString target, const DoubleVector& weight, const LAString& forwardID, const double forwardShiftValue )
{
    LAPriceDataDayCount dc_act365(ACT_365_ISDA);
    upper(target);
    upper(sabrLimiter);
    const double eps = 0.000000001;

    LAMathSABRLimiter limiter(sabrLimiter);

    AQ_THROW_IF(swapVolID.size() != weight.size(), "Invalid Market Data: Number of Swap Vol Objects and Weights does not match")

	matirixCheck(dataInstance, alphaID, betaID);
	matirixCheck(dataInstance, alphaID, nuID);
	matirixCheck(dataInstance, alphaID, rhoID);

    LAObjectPool& objPool = dataInstance->getObjectPool();
    DoubleMatrix& alphaMat = dynamic_cast<LADataDoubleMatrix &>
        (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
    const LAStringVector& expiry = dynamic_cast<const LADataStrings &>
        (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_EXPIRYSTRING,ISDEFINED).get()).get();
    const LAStringVector& tenor = dynamic_cast<const LADataStrings &>
        (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_TENORSTRING,ISDEFINED).get()).get();
	const DoubleVector& expiryTerms = dynamic_cast<const LADataDoubles &>
        (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_EXPIRYVECTOR,ISDEFINED).get()).get();
	const DateVector& expiryDates = dynamic_cast<const LADataDates &>
        (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_EXPIRYDATEVECTOR,ISDEFINED).get()).get();
    DoubleMatrix& betaMat = dynamic_cast<LADataDoubleMatrix &>
        (objPool.getObject( betaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
    DoubleMatrix& nuMat = dynamic_cast<LADataDoubleMatrix &>
        (objPool.getObject( nuID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
    DoubleMatrix& rhoMat = dynamic_cast<LADataDoubleMatrix &>
        (objPool.getObject( rhoID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
	
	DoubleMatrix forwardMat;
	if(forwardID != LAString("")) 
	{
		matirixCheck(dataInstance, alphaID, forwardID);
		forwardMat = dynamic_cast<LADataDoubleMatrix &>
			(objPool.getObject( forwardID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
	}

    vector<DoubleMatrix > marketVol, marketStk;
    for(size_t i=0; i<swapVolID.size(); i++)
    {
        matirixCheck(dataInstance, alphaID, swapVolID[i], true);
        marketVol.push_back(
        dynamic_cast<const LADataDoubleMatrix &> (objPool.getObject( swapVolID[i], ENCHKTYPE_ISDEFINED ).get().
        getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get() );

        marketStk.push_back(
        dynamic_cast<const LADataDoubleMatrix &> (objPool.getObject( swapVolID[i], ENCHKTYPE_ISDEFINED ).get().
        getData("SwaptionStrikeMatrix",ISDEFINED).get()).get() );
    }

    double forward, numeraire;
    DoubleArray forward_calib, numeraire_calib, strike_calib, vol_calib, expiry_calib, x(limiter.getParamNum());
    DateVector dates;
    LAMathSABR_Hagan sabr;
	LAObject curveEntity = dataInstance->getObjectPool().getObject( curveSetID, ENCHKTYPE_ISDEFINED ).get();
	LAString curveID,convID,foreName,dfName;
	curveID = dynamic_cast<const LADataString& >(curveEntity.getData(CALIBRATION_DATA_CURVEID,ISDEFINED).get()).get();
    bool isMap = false;
	for(size_t j=0; j<tenor.size(); j++)
    {
		for(size_t i=0; i<expiry.size(); i++)
		{  
			if(forwardID == LAString(""))
			{
				if( tenor[j]==LAString("1M") || tenor[j]==LAString("3M") || tenor[j]==LAString("6M") )
				{
					convID = capConvID;
					foreName = dynamic_cast<const LADataString& >(curveEntity.getData(tenor[j]+LAString("LiborCurveName"),ISDEFINED).get()).get();
					dfName = dynamic_cast<const LADataString& >(curveEntity.getData(CALIBRATION_DATA_DISCOUNTCURVENAME,ISDEFINED).get()).get();
				}
				else
				{
					convID = swapConvID;
					foreName = dynamic_cast<const LADataString& >(curveEntity.getData(LAString("SwapRateCurveName"),ISDEFINED).get()).get();
					dfName = dynamic_cast<const LADataString& >(curveEntity.getData(CALIBRATION_DATA_DISCOUNTCURVENAME,ISDEFINED).get()).get();
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
            LAMathSABRCalibrator calibrator(sabr, strike_calib, vol_calib, forward_calib, expiry_calib, 
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

    LADataHolder* dh;
    LADataDoubleMatrix mat_att;
    dh = &dataInstance->getObjectPool().getObject(alphaID ,ENCHKTYPE_NOCHECK).getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED);   
    mat_att = dynamic_cast<LADataDoubleMatrix &>(dh->get());
    mat_att.set( alphaMat );

    dh = &dataInstance->getObjectPool().getObject(betaID ,ENCHKTYPE_NOCHECK).getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED);   
    mat_att = dynamic_cast<LADataDoubleMatrix &>(dh->get());
    mat_att.set( betaMat );

    dh = &dataInstance->getObjectPool().getObject(nuID ,ENCHKTYPE_NOCHECK).getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED);   
    mat_att = dynamic_cast<LADataDoubleMatrix &>(dh->get());
    mat_att.set( nuMat );

    dh = &dataInstance->getObjectPool().getObject(rhoID ,ENCHKTYPE_NOCHECK).getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED);   
    mat_att = dynamic_cast<LADataDoubleMatrix &>(dh->get());
    mat_att.set( rhoMat );
}

void
LAMathSwaptionVolUtility::calibrateSABRMatrixCapFloor
(LADataInstance* dataInstance, const LAString& approxMethod, const std::vector<bool>& calibFlg, const LAString& calibMethod, const LAString& curveSetID, const LAString& alphaID, const LAString& betaID,
	const LAString& nuID, const LAString& rhoID, const LAString& swapConvID,
	const LAString& capConvID, const LAStringVector& swapVolID, /*LAStringMatrix sabrLimiter,*/
	const LAString& target, const DoubleVector& weight, const IntVector& sgn_in, const double forwardShiftValue,
    LAString& msg, const BoolMatrix *calibFlgMtx)
{
	LAString tmp_target = target;
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

	LAObjectPool& objPool = dataInstance->getObjectPool();
	DoubleMatrix& alphaMat = dynamic_cast<LADataDoubleMatrix &>
		(objPool.getObject(alphaID, ENCHKTYPE_ISDEFINED).get().getData(PRICING_DATA_SWAPTIONMATRIX, ISDEFINED).get()).get();
	const LAStringVector& expiry = dynamic_cast<const LADataStrings &>
		(objPool.getObject(alphaID, ENCHKTYPE_ISDEFINED).get().getData(PRICING_DATA_EXPIRYSTRING, ISDEFINED).get()).get();
	const LAStringVector& tenor = dynamic_cast<const LADataStrings &>
		(objPool.getObject(alphaID, ENCHKTYPE_ISDEFINED).get().getData(PRICING_DATA_TENORSTRING, ISDEFINED).get()).get();
	DoubleMatrix& betaMat = dynamic_cast<LADataDoubleMatrix &>
		(objPool.getObject(betaID, ENCHKTYPE_ISDEFINED).get().getData(PRICING_DATA_SWAPTIONMATRIX, ISDEFINED).get()).get();
	DoubleMatrix& nuMat = dynamic_cast<LADataDoubleMatrix &>
		(objPool.getObject(nuID, ENCHKTYPE_ISDEFINED).get().getData(PRICING_DATA_SWAPTIONMATRIX, ISDEFINED).get()).get();
	DoubleMatrix& rhoMat = dynamic_cast<LADataDoubleMatrix &>
		(objPool.getObject(rhoID, ENCHKTYPE_ISDEFINED).get().getData(PRICING_DATA_SWAPTIONMATRIX, ISDEFINED).get()).get();
	const LAObject& object = dataInstance->getObjectPool().getObject(capConvID, ENCHKTYPE_ISDEFINED).get();
	const LAPriceDataCalendar& cal =	dynamic_cast<const LAPriceDataCalendar& >(object.getData(CALIBRATION_DATA_CALENDAR, ISDEFINED).get());
	const LAPriceDataCalendar& cal2 = dynamic_cast<const LAPriceDataCalendar& >(object.getData(PRICING_DATA_FIXINGCALENDAR, ISDEFINED).get());
	const LAPriceDataSlidingRule& sr = dynamic_cast<const LAPriceDataSlidingRule& >(object.getData(CALIBRATION_DATA_SLIDINGRULE, ISDEFINED).get());
	const LADate& asOfDate =	dynamic_cast<const LADataDate& >(object.getData(CALIBRATION_DATA_ASOFDATE, ISNOTNULL).get()).get();
	const LAString& spotlag = dynamic_cast<const LADataString& >(object.getData(CURVEINPUT_SPOTLAG, ISDEFINED).get()).get();
	const LAPriceDataDayCount& daycount = dynamic_cast<const LAPriceDataDayCount& >(object.getData(IR_CALIBRATION_DATA_DAYCOUNT, ISDEFINED).get());

	if (calibFlgMtx)
	{
		if (calibFlgMtx->size() != expiry.size() || (*calibFlgMtx)[0].size() != tenor.size())
		{
			throw LACoreInvalidData("Calibration flag matrix does not match capfloor premium matrix!", __FILE__, __LINE__);
		}
	}

	vector<DoubleMatrix > marketVol, marketStk, marketSign;
	vector<LAStringVector > premiumTerms;
	for (size_t i = 0; i<swapVolID.size(); i++)
	{
		marketVol.push_back(
			dynamic_cast<const LADataDoubleMatrix &> (objPool.getObject(swapVolID[i], ENCHKTYPE_ISDEFINED).get().
				getData(PRICING_DATA_SWAPTIONMATRIX, ISDEFINED).get()).get());
		marketStk.push_back(
			dynamic_cast<const LADataDoubleMatrix &> (objPool.getObject(swapVolID[i], ENCHKTYPE_ISDEFINED).get().
				getData(PRICING_DATA_SWAPTIONSTRIKEMATRIX, ISDEFINED).get()).get());
		marketSign.push_back(
			dynamic_cast<const LADataDoubleMatrix &> (objPool.getObject(swapVolID[i], ENCHKTYPE_ISDEFINED).get().
				getData(PRICING_DATA_SWAPTIONSIGNMATRIX, ISDEFINED).get()).get());
		premiumTerms.push_back(
			dynamic_cast<const LADataStrings &> (objPool.getObject(swapVolID[i], ENCHKTYPE_ISDEFINED).get().
				getData(PRICING_DATA_EXPIRYSTRING, ISDEFINED).get()).get());
	}

	double max_alpha = 3.;
	const double min_alpha = 0.0001, max_beta = 1., min_beta = 0., max_nu = 5., min_nu = 0.0001, max_rho = 0.98, min_rho = -0.98;

	DoubleMatrix alphaMat_calib(alphaMat), betaMat_calib(betaMat), nuMat_calib(nuMat), rhoMat_calib(rhoMat);

	int y, m, d, w;
	DoubleVector tenorVec;
	for (size_t j = 0; j < tenor.size(); ++j)
	{
		LAMathDateCalculations::termStrtoYMDW(tenor[j], y, m, d, w);
		tenorVec.push_back(y + m / 12.);
	}
		
	for (size_t j = 0; j<tenor.size(); j++)
	{
		// get caplet/floorlet tenor
		const LAString tenorPoint = tenor[j];
		const double tenorPoint_d = LAMathSwaptionVolUtility::getTenorPoint(tenorPoint);
		LAString CapFloorletFrequency;
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

		// Set up LAMathYieldCurve for fwdrate, numeraire
		LAObject curveEntity;
		LAString curveID, convID, foreName, dfName;
		curveEntity = dataInstance->getObjectPool().getObject(curveSetID, ENCHKTYPE_ISDEFINED).get();
		const LAString curveid = dynamic_cast<const LADataString&>(curveEntity.getData(CALIBRATION_DATA_CURVEID, ISDEFINED).get()).get();
		LAMathYieldCurve& yc = LAMathCurveFuncUtility::getYieldCurveForCurveID(dataInstance, curveid);
		yc.setInterpolation(FN_SPLINEINTERPOLATION_STR);
		yc.getDayCount().setDayCount(daycount.convertToString());
		yc.getSlidingRule().convertFromString(NO_CH);
		LAMathCurveFuncUtility::setCalendarForCurveID(yc, cal.convertToString());
		yc.getFrequency().convertFromString(CapFloorletFrequency);
		LAPriceDataConvention conv(ACT_ACT, CONT), conv2(yc.getDayCount().getDayCount(), CONT);
		yc.getFrequency().set("SIMPLE");
		// set up slidingrule and daycount for numeraire
		LAMathYieldCurve& yc_nu = yc;
		yc_nu.getSlidingRule() = sr;
		yc_nu.getDayCount().setDayCount(ACT_ACT);

		// get curvename
		if (tenor[j] == LAString("1M") || tenor[j] == LAString("3M") || tenor[j] == LAString("6M"))
		{
			convID = capConvID;
			foreName = dynamic_cast<const LADataString&>(curveEntity.getData(tenor[j] + LAString("LiborCurveName"), ISDEFINED).get()).get();
			dfName = dynamic_cast<const LADataString&>(curveEntity.getData(CALIBRATION_DATA_DISCOUNTCURVENAME, ISDEFINED).get()).get();
		}
		else
		{
			convID = swapConvID;
			foreName = dynamic_cast<const LADataString&>(curveEntity.getData(LAString("SwapRateCurveName"), ISDEFINED).get()).get();
			dfName = dynamic_cast<const LADataString&>(curveEntity.getData(CALIBRATION_DATA_DISCOUNTCURVENAME, ISDEFINED).get()).get();
		}

		// check SABR parameters term
		const size_t num_diffterm = expiry.size() - premiumTerms[0].size();; // this parameter is used for calibrating some parameter grids simultaneously which do not exist in premium grid. ex) premium grid = 1Y, parameter grids = 3M, 6M, 1Y  
		LAStringVector paramsterm_month, paramsterm_month_calib; //  the second parmeter is used for changing expiry of SABR parameters into "expiry - tenor";
		int paramsterm, paramsterm_calib;
		for (size_t i = 0; i < expiry.size(); i++)
		{
			LAMathDateCalculations::termStrtoYMDW(expiry[i], y, m, d, w);
			paramsterm = 12 * y + m;
			paramsterm_month.push_back(LAString(paramsterm) + "M");
			
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
			paramsterm_month_calib.push_back(LAString(paramsterm_calib) + "M");
		}

		// check whether expiry of SABR parameters include all of the "premiumTerms - tenor"
		// Also, we can input expiry of SABR parameters which is shorter than the first "premiumTerms - tenor".
		
		for (size_t i = 0; i < swapVolID.size(); i++)
		{
			for (size_t k = 0; k < premiumTerms[i].size() - 1; k++)
			{
				LAMathDateCalculations::termStrtoYMDW(premiumTerms[i][k], y, m, d, w);
				int premiumterm = 12 * y + m;
				const LAString premiumterm_month = LAString(premiumterm) + "M";
				if (num_diffterm >= 0)
				{
					if (premiumterm_month != paramsterm_month[k + num_diffterm])
						throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
				}
				else
				{
					AQ_THROW("Inconsistent Data: SABR terms data and premium data are inconsistent")
				}
			}
		}

		// These objects are used for converting term. ex) 1Y, 2Y,... <-> 0.75Y, 1.75Y, ...
		LAMathSwaptionMatrix alphaMat_temp({ {1} ,{1} }, {1}, tenorVec);
		LAMathSwaptionMatrix betaMat_temp({ {1} ,{1} }, {1}, tenorVec);
		LAMathSwaptionMatrix nuMat_temp({ {1} ,{1} }, {1}, tenorVec);
		LAMathSwaptionMatrix rhoMat_temp({ {1} ,{1} }, {1}, tenorVec);

		// create expiry of calibration target
		LAStringVector target_expiry;
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
			const LAString roll_conv = "TRUE";
			const LADate tmpstart = LAMathDateCalculations::getDate(asOfDate, spotlag, sr, &cal, true, &roll_conv);
			LADate endpay(tmpstart);
			int y, m, d, w;
			LAMathDateCalculations::termStrtoYMDW(premiumTerms[0][i], y, m, d, w);
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
			LAMathDateCalculations::generateSchedule(tmpstart, endpay, CapFloorletFrequency, true, NULL, NULL, &roll, payVec, &sr, &cal);
			//slidingrule 
			LAPriceDataSlidingRule sr2;
			sr2.convertFromString(PRE);
			//fixingVector
			const size_t N = payVec.size();
			DateVector fixVec(N);
			for (int k = 0; k < N; k++)
			{
				fixVec[k] = LAMathDateCalculations::getDate(payVec[k], spotlag, sr2, &cal2, false);
			}

			unsigned int pos = 0;
			LAAlgorithm::locate<DateVector, LADate>(fixVec, asOfDate, N - 1, pos);
			if (N - 1 == static_cast<int>(pos))
			{
				LAString msg = "valuedate : " + asOfDate.stringWithFormat("YYYYMMDD") + " is not support for this option pricing";
				throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
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
			LADate expiryDate;
			LAString daycount_expiry(AC_365I);
			for (size_t k = 0; k < i + num_diffterm + 1; k++)
			{
				expiryDate = LAMathDateCalculations::getDate(asOfDate, expiry[k], sr, &cal2, true);
				expiryterms_capfloor.push_back(LAMathDateUtilities::getTerm(asOfDate, expiryDate, daycount_expiry, true));
			}
			// create expiry terms for calibration target cap/floor
			DoubleVector expiryterms_capfloor_calib;
			for (size_t k = 0; k < num_diffterm; k++)
			{
				expiryterms_capfloor_calib.push_back(expiryterms_capfloor[k]);
			}
			for (size_t k = num_diffterm; k < i + num_diffterm + 1; k++)
			{
				LAMathDateCalculations::termStrtoYMDW(expiry[k], y, m, d, w);
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

			LAMathDateCalculations::termStrtoYMDW(target_expiry[i], y, m, d, w);
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
			msg = LAString("alpha is out of range.");
			if (initValue[0] < min_alpha || initValue[0] > max_alpha) throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			msg = LAString("beta is out of range.");
			if (initValue[1] < min_beta || initValue[1] > max_beta) throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			msg = LAString("nu is out of range.");
			if (initValue[2] < min_nu || initValue[2] > max_nu) throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
			msg = LAString("rho is out of range.");
			if (initValue[3] < min_rho || initValue[3] > max_rho) throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);

			// Create CostFunc
			LAString tmp_approxMethod = approxMethod;
			tmp_approxMethod.toUpper();
			IntVector param_pos(2);
			param_pos[0] = i + num_diffterm;
			param_pos[1] = j;
			LAMathLeastSquareSABRCapFloorCostFunc* costfunc = new LAMathLeastSquareSABRCapFloorCostFunc(
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
			LAQuantLibConstraint* constraint = 0;
			if (tmp_approxMethod == APPROXIMATION_ANTONOV)
			{
				constraint = new LAMathLeastSquareSABRConstraint_Antonov(forward,
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
				constraint = new LAMathLeastSquareSABRConstraint(initValue,
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
							throw LACoreInvalidData("All calibration flags have been set to false", __FILE__, __LINE__);
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

			LAQuantLibArray x(x_.size());
			for (size_t k = 0; k < x_.size(); ++k) x[k] = x_[k];
			LAQuantLibProblem* opt_problem = new LAQuantLibProblem(*costfunc, *constraint, x);

			// Set Optimization Method
			LAQuantLibOptimizationMethod* optMethod;
			LAString tmp_calibMethod = calibMethod;
			tmp_calibMethod.toUpper();
			if (tmp_calibMethod == CALIB_NON_LINEAR_CONJUGATE_GRADIENT_METHOD)
			{
				optMethod = new LAQuantLibConjugateGradient();
			}
			else if (tmp_calibMethod == CALIB_STEEPEST_DESCENT_METHOD)
			{
				optMethod = new LAQuantLibSteepestDescent();
			}
			else if (tmp_calibMethod == CALIB_SIMPLEX_METHOD)
			{
				if (tmp_target == CALIB_TARGET_VOLATILITY)
				{
					optMethod = new LAQuantLibSimplex(0.1);
				}
				else
				{
					optMethod = new LAQuantLibSimplex(max_alpha);
				}
			}
			else if (tmp_calibMethod == CALIB_LEVENBERG_MARQUARDT_METHOD)
			{
				double levenbergMarquardtEpsfcn = 0.0000001;       // parameters specific for Levenberg-Marquardt
				double levenbergMarquardtXtol = 0.0000001;	   //
				double levenbergMarquardtGtol = 0.0000001;       //

				optMethod = new LAQuantLibLevenbergMarquardt(levenbergMarquardtEpsfcn,
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
			LAQuantLibEndCriteria* endCriteria = new LAQuantLibEndCriteria(maxIteration,
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
					LAQuantLibEndCriteria::Type endCriteriaResult = optMethod->minimize(*opt_problem, *endCriteria);
				}
				catch (std::exception& e)
				{
					msg += LAString(e.what());
					msg += LAString("(");
					msg += LAString(LAModelUtilities::n2s(i).c_str());
					msg += LAString(",");
					msg += LAString(LAModelUtilities::n2s(j).c_str());
					msg += LAString(")");
					msg += LAString(" : ");
				}
			}
			catch (LACoreError& e)
			{
				for (unsigned int k = 0; k < e.getSize(); k++) msg += LAString(e.getMsg(e.getSize() - k - 1));
				msg += LAString("(");
				msg += LAString(LAModelUtilities::n2s(i).c_str());
				msg += LAString(",");
				msg += LAString(LAModelUtilities::n2s(j).c_str());
				msg += LAString(")");
				msg += LAString(" : ");
			}

			std::shared_ptr<LAMathLeastSquareSABRCapFloorCostFuncQlib> costfuncQlib = dynamic_pointer_cast<LAMathLeastSquareSABRCapFloorCostFuncQlib>(costfunc->getCostFunction());
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
	LADataHolder* dh;
	LADataDoubleMatrix mat_att;
	dh = &dataInstance->getObjectPool().getObject(alphaID, ENCHKTYPE_NOCHECK).getData(PRICING_DATA_SWAPTIONMATRIX, ISDEFINED);
	mat_att = dynamic_cast<LADataDoubleMatrix &>(dh->get());
	mat_att.set(alphaMat);

	dh = &dataInstance->getObjectPool().getObject(betaID, ENCHKTYPE_NOCHECK).getData(PRICING_DATA_SWAPTIONMATRIX, ISDEFINED);
	mat_att = dynamic_cast<LADataDoubleMatrix &>(dh->get());
	mat_att.set(betaMat);

	dh = &dataInstance->getObjectPool().getObject(nuID, ENCHKTYPE_NOCHECK).getData(PRICING_DATA_SWAPTIONMATRIX, ISDEFINED);
	mat_att = dynamic_cast<LADataDoubleMatrix &>(dh->get());
	mat_att.set(nuMat);

	dh = &dataInstance->getObjectPool().getObject(rhoID, ENCHKTYPE_NOCHECK).getData(PRICING_DATA_SWAPTIONMATRIX, ISDEFINED);
	mat_att = dynamic_cast<LADataDoubleMatrix &>(dh->get());
	mat_att.set(rhoMat);
}

DoubleArray 
LAMathSwaptionVolUtility::calibrateSABR
( const LAString& approxMethod, const std::vector<bool>& calibFlg, const LAString& calibMethod, 
  double alpha, double beta, double nu, double rho, double forward, LAString expiryPoint,
  double numeraire, const DoubleArray& strikes, const DoubleArray& vols, 
  /*LAStringMatrix sabrLimiter,*/ const LAString& target, const DoubleVector& weight, const IntVector& sgn )
{
	LAString tmp_target = target;
	tmp_target.toUpper();

	double expiry = LAMathSwaptionVolUtility::getTenorPoint(expiryPoint);
	
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
	LAMathSABR* sabr = createSABR(approxMethod, alpha, beta, nu, rho);
	LAMathLeastSquareSABRCostFunc* costfunc =new  LAMathLeastSquareSABRCostFunc(*sabr,
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
	LAQuantLibConstraint* constraint;
	LAString tmp_approxMethod = approxMethod;
	tmp_approxMethod.toUpper();
	if(approxMethod == APPROXIMATION_ANTONOV)
	{
		constraint = new LAMathLeastSquareSABRConstraint_Antonov(forward,
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
		constraint = new LAMathLeastSquareSABRConstraint(initValue,
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

	LAQuantLibArray x(x_.size());
	for(size_t k = 0; k < x_.size(); ++k) x[k] = x_[k];
	LAQuantLibProblem* opt_problem = new LAQuantLibProblem(*costfunc,*constraint,x);


	// Set Optimization Method
	LAQuantLibOptimizationMethod* optMethod = 0;
	LAString tmp_calibMethod = calibMethod;
	tmp_calibMethod.toUpper();
	if (tmp_calibMethod == CALIB_NON_LINEAR_CONJUGATE_GRADIENT_METHOD)
	{
		optMethod = new LAQuantLibConjugateGradient();
	}
	else if (tmp_calibMethod == CALIB_STEEPEST_DESCENT_METHOD)
	{
		optMethod = new LAQuantLibSteepestDescent();
	}
	else if (tmp_calibMethod == CALIB_SIMPLEX_METHOD)
	{
		if (tmp_target == CALIB_TARGET_VOLATILITY)
		{
			optMethod = new LAQuantLibSimplex(0.1);
		}
		else
		{
			optMethod = new LAQuantLibSimplex(max_alpha);
		}
	}
	else if (tmp_calibMethod == CALIB_LEVENBERG_MARQUARDT_METHOD)
	{
		double levenbergMarquardtEpsfcn = 0.0000001;       // parameters specific for Levenberg-Marquardt
		double levenbergMarquardtXtol   = 0.0000001;	   //
		double levenbergMarquardtGtol   = 0.0000001;       //

		optMethod = new LAQuantLibLevenbergMarquardt(levenbergMarquardtEpsfcn,
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

	LAQuantLibEndCriteria* endCriteria = new LAQuantLibEndCriteria(maxIteration, 
			                                                           maxStationaryStateIteration,
												                       rootEpsilon, 
												                       functionEpsilon, 
												                       gradientNormEpsilon);

	// Optimization
	try
	{
		try
		{
			LAQuantLibEndCriteria::Type endCriteriaResult = optMethod->minimize(*opt_problem, *endCriteria);
		}
		catch(LACoreError&e)
		{
			LAString msg = "";
			for(unsigned int k =0;k<e.getSize();k++) msg+=LAString(e.getMsg(e.getSize()-k-1));
			throw LACoreError(msg.getCString(), __FILE__, __LINE__);
		}
	}
	catch(std::exception& e)
	{
		throw LACoreError(e.what(), __FILE__, __LINE__);
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
LAMathSwaptionVolUtility::calibrateSABR
( double alpha, double beta, double nu, double rho, double forward, LAString expiryPoint,
  double numeraire, const DoubleArray& strikes, const DoubleArray& vols, 
  LAStringMatrix sabrLimiter, LAString target, const DoubleVector& weight )
{
    upper(target);
    upper(sabrLimiter);

	double expiry = LAMathSwaptionVolUtility::getTenorPoint(expiryPoint);
    LAMathSABRLimiter limiter(sabrLimiter);
    size_t calibSize = vols.size();
    if( calibSize != weight.size() || calibSize != strikes.size() || limiter.getParamNum() == 0 )
	{
        AQ_THROW("Invalid Market Data: Number of Volatilities, Strikes and Parameters must match")
	}

    DoubleArray forward_calib, numeraire_calib, expiry_calib, x(limiter.getParamNum());
    LAMathSABR_Hagan sabr;
    sabr.setSABRParam(alpha,beta,nu,rho);
    forward_calib = getConstantVector(forward,calibSize);
    numeraire_calib = getConstantVector(numeraire,calibSize);
    expiry_calib = getConstantVector(expiry,calibSize);
    //calibrate
    bool isMap = false;
    LAMathSABRCalibrator calibrator(sabr, strikes, vols, forward_calib, expiry_calib, 
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
LAMathSwaptionVolUtility::calibrateSABRATMFix
( LADataInstance* dataInstance, const LAString& curveSetID, const LAString& alphaID, const LAString& betaID, 
  const LAString& nuID, const LAString& rhoID, const LAString& swapConvID, 
  const LAString& capConvID, const LAString& ATMVolID, const LAStringVector& swapVolID, 
  LAStringMatrix sabrLimiter, const LAString& target, const DoubleVector& weight, 
  const LAString& forwardID,  const double forwardShiftValue )
{
	LAPriceDataDayCount dc_act365(ACT_365_ISDA);
    LAString tmp_target = target;
	tmp_target.toUpper();
    upper(sabrLimiter);
    const double eps = 0.000000001;
	LAObjectPool& objPool = dataInstance->getObjectPool();

	//get ATM Vol
	vector<DoubleMatrix > marketVol, marketStk;
	for(size_t i=0; i<swapVolID.size(); i++)
    {
        matirixCheck(dataInstance, betaID, swapVolID[i], true);
        marketVol.push_back(
        dynamic_cast<const LADataDoubleMatrix &> (objPool.getObject( swapVolID[i], ENCHKTYPE_ISDEFINED ).get().
        getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get() );

        marketStk.push_back(
        dynamic_cast<const LADataDoubleMatrix &> (objPool.getObject( swapVolID[i], ENCHKTYPE_ISDEFINED ).get().
        getData(PRICING_DATA_SWAPTIONSTRIKEMATRIX,ISDEFINED).get()).get() );
    }
	matirixCheck(dataInstance, betaID, ATMVolID, true);
    DoubleMatrix& atmVol = dynamic_cast<LADataDoubleMatrix &> (objPool.getObject( ATMVolID, ENCHKTYPE_ISDEFINED ).get().
                getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();

	BoolMatrix isCalib;
	LADataHolder* dh = &objPool.getObject( ATMVolID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_CALIBRATIONFLAG, NOCHECK);
	if (dh->isDefined() && !dh->isNull())
	{
		isCalib = dynamic_cast<const LADataBoolMatrix &>(dh->get()).get();
		if (isCalib.size() != atmVol.size() || isCalib[0].size() != atmVol[0].size())
		{
			AQ_THROW("Inconsistent Data: Enable Calibration table size does not match ATM Vol table size")
		}
	}

	//set alpha
    LAObjectHolder objHolder = objPool.getObject(alphaID ,ENCHKTYPE_NOCHECK);
	if(!objHolder.isDefined())
	{
		const LAObject& beta = dataInstance->getObjectPool().getObject( betaID, ENCHKTYPE_ISDEFINED ).get();
		LAObject* e = beta.clone();
		e->remove( CALIBRATION_DATA_NAME );
		e->add( CALIBRATION_DATA_NAME, new LADataString() ).convertFromString(alphaID);
		e->remove(PRICING_DATA_SWAPTIONMATRIX);
        e->add( PRICING_DATA_SWAPTIONMATRIX, new LADataDoubleMatrix( atmVol ));	
		objPool.set( alphaID , e );
	}

    //alpha is not calibrated
    int row = LACoreUtility::findRowsNumber(sabrLimiter,"ALPHA");
    sabrLimiter[row][1] = "NO";
    LAMathSABRLimiter limiter(sabrLimiter);

    AQ_THROW_IF( swapVolID.size() != weight.size(), "Inconsistent Data: Number of Vol Objects and Weights must match")

	matirixCheck(dataInstance, betaID, alphaID);
	matirixCheck(dataInstance, betaID, nuID);
	matirixCheck(dataInstance, betaID, rhoID);
    DoubleMatrix& betaMat = dynamic_cast<LADataDoubleMatrix &>
        (objPool.getObject( betaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
    const LAStringVector& tenor = dynamic_cast<const LADataStrings &>
        (objPool.getObject( betaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_TENORSTRING,ISDEFINED).get()).get();
	const DoubleVector& tenor_d = dynamic_cast<const LADataDoubles &>
        (objPool.getObject( betaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_TENORVECTOR,ISDEFINED).get()).get();
    const DoubleVector& expiryTerms = dynamic_cast<const LADataDoubles &>
        (objPool.getObject( betaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_EXPIRYVECTOR,ISDEFINED).get()).get();
	const DateVector& expiryDates = dynamic_cast<const LADataDates &>
        (objPool.getObject( betaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_EXPIRYDATEVECTOR,ISDEFINED).get()).get();

	DoubleMatrix& alphaMat = dynamic_cast<LADataDoubleMatrix &>
        (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
    DoubleMatrix& nuMat = dynamic_cast<LADataDoubleMatrix &>
        (objPool.getObject( nuID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
    DoubleMatrix& rhoMat = dynamic_cast<LADataDoubleMatrix &>
        (objPool.getObject( rhoID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();

	DoubleMatrix forwardMat;
	if(forwardID != LAString("")) 
	{
		matirixCheck(dataInstance, betaID, forwardID);
		forwardMat = dynamic_cast<LADataDoubleMatrix &>
			(objPool.getObject( forwardID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
	}

    double forward, numeraire;
    DoubleArray strike_calib, vol_calib, x(limiter.getParamNum());
    DateVector dates;
    LAMathSABR_Hagan sabr;
	//LAObject curveEntity = dataInstance->getObjectPool().getObject( curveSetID, ENCHKTYPE_ISDEFINED ).get();
	LAString curveID,convID,foreName,dfName;
	//curveID = dynamic_cast<const LADataString& >(curveEntity.getData(CALIBRATION_DATA_CURVEID,ISDEFINED).get()).get();
    bool isMap = false;
	for(size_t j=0; j<tenor.size(); j++)
	{
		for(size_t i=0; i<expiryDates.size(); i++)
		{
			if (isCalib.size() !=0 && !isCalib[i][j]) continue;

			if(forwardID == LAString(""))
			{
				//curveEntity = dataInstance->getObjectPool().getObject( curveSetID, ENCHKTYPE_ISDEFINED ).get();
				LAObject curveEntity = dataInstance->getObjectPool().getObject( curveSetID, ENCHKTYPE_ISDEFINED ).get();
				curveID = dynamic_cast<const LADataString& >(curveEntity.getData(CALIBRATION_DATA_CURVEID,ISDEFINED).get()).get();

				if( tenor[j]==LAString("1M") || tenor[j]==LAString("3M") || tenor[j]==LAString("6M") )
				{
					convID = capConvID;
					foreName = dynamic_cast<const LADataString& >(curveEntity.getData(tenor[j]+LAString("LiborCurveName"),ISDEFINED).get()).get();
					dfName = dynamic_cast<const LADataString& >(curveEntity.getData(CALIBRATION_DATA_DISCOUNTCURVENAME,ISDEFINED).get()).get();
				}
				else
				{
					convID = swapConvID;
					const LADataHolder* dh;
					dh = &curveEntity.getData(CALIBRATION_DATA_SWAPRATELCURVENAME + LAString("_") + tenor[j],NOCHECK);
					if(!dh->isDefined() || dh->isNull())
					{
						dh = &curveEntity.getData(CALIBRATION_DATA_SWAPRATELCURVENAME,ISDEFINED);
					}
					foreName = dynamic_cast<const LADataString& >(dh->get()).get();
					dfName = dynamic_cast<const LADataString& >(curveEntity.getData(CALIBRATION_DATA_DISCOUNTCURVENAME,ISDEFINED).get()).get();
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
				LAMathSABRCalibratorATMFix calibrator( sabr, strike_calib, vol_calib, atmVol[i][j], forward, 
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
            alphaMat[i][j] = LAMath::max(sabr.getAlpha(), alpha_low);
            betaMat[i][j] = LAMath::max(sabr.getBeta(), beta_low);
            nuMat[i][j] = LAMath::max(sabr.getNu(), nu_low);
            rhoMat[i][j] = LAMath::max(sabr.getRho(), rho_low);
        }
    }
}

double 
LAMathSwaptionVolUtility::getSABRAlpha
( LADataInstance* dataInstance, const LAString& expPoint, const LAString& tenorPoint, const LAString& volID, const LAString& forwardID, 
  const LAString& alphaID, const LAString& betaID, const LAString& nuID, const LAString& rhoID )
{
    LAPriceDataDayCount dc_act365(ACT_365_ISDA);
    LAObjectPool& objPool = dataInstance->getObjectPool();

	matirixCheck(dataInstance, alphaID, volID, true);
	matirixCheck(dataInstance, alphaID, forwardID);
	matirixCheck(dataInstance, alphaID, betaID);
	matirixCheck(dataInstance, alphaID, nuID);
	matirixCheck(dataInstance, alphaID, rhoID);

    const DoubleMatrix& alphaMat = dynamic_cast<LADataDoubleMatrix& >
        (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
    const DoubleVector& tenorVec = dynamic_cast<const LADataDoubles& >
        (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_TENORVECTOR,ISDEFINED).get()).get();
    const DoubleVector& expiryVec = dynamic_cast<const LADataDoubles& >
        (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_EXPIRYVECTOR,ISDEFINED).get()).get();

    const DoubleMatrix& volMat = dynamic_cast<LADataDoubleMatrix &>
        (objPool.getObject( volID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
    const DoubleMatrix& forwardMat = dynamic_cast<LADataDoubleMatrix &>
        (objPool.getObject( forwardID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
    const DoubleMatrix& betaMat = dynamic_cast<LADataDoubleMatrix &>
        (objPool.getObject( betaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
    const DoubleMatrix& nuMat = dynamic_cast<LADataDoubleMatrix &>
        (objPool.getObject( nuID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
    const DoubleMatrix& rhoMat = dynamic_cast<LADataDoubleMatrix &>
        (objPool.getObject( rhoID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();

    const LAPriceDataSlidingRule& paySlr = dynamic_cast<const LAPriceDataSlidingRule& >(objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).
		get().getData(CALIBRATION_DATA_SLIDINGRULE,ISDEFINED).get());
    const LAPriceDataCalendar& fixCal = dynamic_cast<const LAPriceDataCalendar& >(objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().
		getData(CALIBRATION_DATA_CALENDAR,ISDEFINED).get());
    const LADate& asOfDate = dynamic_cast<const LADataDate& >(dataInstance->getObjectPool().getObject(alphaID,ENCHKTYPE_ISDEFINED).
		get().getData(CALIBRATION_DATA_ASOFDATE,ISNOTNULL).get()).get();

	double expPoint_d = getExpiryPoint(expPoint, asOfDate, paySlr, fixCal);

    LAMathSwaptionMatrix volMat_(volMat, expiryVec, tenorVec);
    LAMathSwaptionMatrix forwardMat_(forwardMat, expiryVec, tenorVec);
    LAMathSwaptionMatrix alphaMat_(alphaMat, expiryVec, tenorVec);
    LAMathSwaptionMatrix betaMat_(betaMat, expiryVec, tenorVec);
    LAMathSwaptionMatrix nuMat_(nuMat, expiryVec, tenorVec);
    LAMathSwaptionMatrix rhoMat_(rhoMat, expiryVec, tenorVec);
    
    double tenorPoint_d = getTenorPoint(tenorPoint);

    double vol = volMat_.lookUpMatrix(expPoint_d, tenorPoint_d);
    double forward = forwardMat_.lookUpMatrix(expPoint_d, tenorPoint_d);
    double alpha = alphaMat_.lookUpMatrix(expPoint_d, tenorPoint_d);
    double beta = betaMat_.lookUpMatrix(expPoint_d, tenorPoint_d);
    double nu = nuMat_.lookUpMatrix(expPoint_d, tenorPoint_d);
    double rho = rhoMat_.lookUpMatrix(expPoint_d, tenorPoint_d);
    //for param check
    LAMathSABR_Hagan sabr(alpha,beta,nu,rho);

   sabr.setAlphaForATMVol(vol, expPoint_d, forward, true);
   return sabr.getAlpha();
}

double 
LAMathSwaptionVolUtility::getSABRAlpha2
( LADataInstance* dataInstance, const LAString& expPoint, const LAString& tenorPoint, double vol, const LAString& alphaID,
  const LAString& betaID, const LAString& nuID, const LAString& rhoID, const LAString& curveID, const LAString& convID,
  const LAString& foreCurveName, const LAString& dfCurveName)
{
    LAPriceDataDayCount dc_act365(ACT_365_ISDA);
    LAObjectPool& objPool = dataInstance->getObjectPool();

	matirixCheck(dataInstance, alphaID, betaID);
	matirixCheck(dataInstance, alphaID, nuID);
	matirixCheck(dataInstance, alphaID, rhoID);
    const DoubleMatrix& alphaMat = dynamic_cast<LADataDoubleMatrix &>
        (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
    const DoubleVector& tenorVec = dynamic_cast<const LADataDoubles& >
        (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_TENORVECTOR,ISDEFINED).get()).get();
    const DoubleVector& expiryVec = dynamic_cast<const LADataDoubles& >
        (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_EXPIRYVECTOR,ISDEFINED).get()).get();
 
    const DoubleMatrix& betaMat = dynamic_cast<LADataDoubleMatrix &>
        (objPool.getObject( betaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
    const DoubleMatrix& nuMat = dynamic_cast<LADataDoubleMatrix &>
        (objPool.getObject( nuID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
    const DoubleMatrix& rhoMat = dynamic_cast<LADataDoubleMatrix &>
        (objPool.getObject( rhoID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();

    const LAObject& object = objPool.getObject( convID, ENCHKTYPE_ISDEFINED ).get();
    const LAPriceDataSlidingRule& paySlr = 
        dynamic_cast<const LAPriceDataSlidingRule& >(object.getData(CALIBRATION_DATA_SLIDINGRULE,ISDEFINED).get());
    const LAPriceDataCalendar& fixCal =
        dynamic_cast<const LAPriceDataCalendar& >(object.getData(PRICING_DATA_FIXINGCALENDAR,ISDEFINED).get());
    
	const LAObject& curve = objPool.getObject( curveID, ENCHKTYPE_ISDEFINED ).get();
	const LADate& asOfDate = dynamic_cast<const LADataDate&>(curve.getData(CALIBRATION_DATA_ASOFDATE,ISDEFINED).get());

	double expPoint_d = getExpiryPoint(expPoint, asOfDate, paySlr, fixCal);
	LADate expPoint_date = getExpiryPoint2(expPoint, asOfDate, paySlr, fixCal);

    double forward = getForward(dataInstance, expPoint_date, tenorPoint, curveID, convID, foreCurveName, dfCurveName);

    LAMathSwaptionMatrix alphaMat_(alphaMat, expiryVec, tenorVec);
    LAMathSwaptionMatrix betaMat_(betaMat, expiryVec, tenorVec);
    LAMathSwaptionMatrix nuMat_(nuMat, expiryVec, tenorVec);
    LAMathSwaptionMatrix rhoMat_(rhoMat, expiryVec, tenorVec);
    
    double tenorPoint_d = getTenorPoint(tenorPoint);

    double alpha = alphaMat_.lookUpMatrix(expPoint_d, tenorPoint_d);
    double beta = betaMat_.lookUpMatrix(expPoint_d, tenorPoint_d);
    double nu = nuMat_.lookUpMatrix(expPoint_d, tenorPoint_d);
    double rho = rhoMat_.lookUpMatrix(expPoint_d, tenorPoint_d);
    //for param check
    LAMathSABR_Hagan sabr(alpha,beta,nu,rho);

    sabr.setAlphaForATMVol(vol, expPoint_d, forward, true);
	return sabr.getAlpha();
}

double 
LAMathSwaptionVolUtility::getSABRVol
( LADataInstance* dataInstance, const LAString& expPoint, const LAString& tenorPoint, double strike, const LAString& fowardID, 
  const LAString& alphaID, const LAString& betaID, const LAString& nuID, const LAString& rhoID, const LAString& approxMethod , const double shift, bool isLognormal)
{
    LAPriceDataDayCount dc_act365(ACT_365_ISDA);
    LAObjectPool& objPool = dataInstance->getObjectPool();

	matirixCheck(dataInstance, alphaID, fowardID);
	matirixCheck(dataInstance, alphaID, betaID);
	matirixCheck(dataInstance, alphaID, nuID);
	matirixCheck(dataInstance, alphaID, rhoID);

    const DoubleMatrix& alphaMat = dynamic_cast<LADataDoubleMatrix& >
        (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
    const DoubleVector& tenorVec = dynamic_cast<const LADataDoubles& >
        (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_TENORVECTOR,ISDEFINED).get()).get();
    const DoubleVector& expiryVec = dynamic_cast<const LADataDoubles& >
        (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_EXPIRYVECTOR,ISDEFINED).get()).get();
    
    const DoubleMatrix& fowardMat = dynamic_cast<LADataDoubleMatrix &>
        (objPool.getObject( fowardID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
    const DoubleMatrix& betaMat = dynamic_cast<LADataDoubleMatrix &>
        (objPool.getObject( betaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
    const DoubleMatrix& nuMat = dynamic_cast<LADataDoubleMatrix &>
        (objPool.getObject( nuID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
    const DoubleMatrix& rhoMat = dynamic_cast<LADataDoubleMatrix &>
        (objPool.getObject( rhoID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();    

    const LAPriceDataSlidingRule& paySlr = dynamic_cast<const LAPriceDataSlidingRule& >(objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).
		get().getData(CALIBRATION_DATA_SLIDINGRULE,ISDEFINED).get());
    const LAPriceDataCalendar& fixCal = dynamic_cast<const LAPriceDataCalendar& >(objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().
		getData(CALIBRATION_DATA_CALENDAR,ISDEFINED).get());
    const LADate& asOfDate = dynamic_cast<const LADataDate& >(dataInstance->getObjectPool().getObject(alphaID,ENCHKTYPE_ISDEFINED).
		get().getData(CALIBRATION_DATA_ASOFDATE,ISNOTNULL).get()).get();

    double expPoint_d = getExpiryPoint(expPoint, asOfDate, paySlr, fixCal);

    LAMathSwaptionMatrix fowardMat_(fowardMat, expiryVec, tenorVec);
    LAMathSwaptionMatrix alphaMat_(alphaMat, expiryVec, tenorVec);
    LAMathSwaptionMatrix betaMat_(betaMat, expiryVec, tenorVec);
    LAMathSwaptionMatrix nuMat_(nuMat, expiryVec, tenorVec);
    LAMathSwaptionMatrix rhoMat_(rhoMat, expiryVec, tenorVec);
    
	double tenorPoint_d = getTenorPoint(tenorPoint);

    double forward = fowardMat_.lookUpMatrix(expPoint_d, tenorPoint_d);
    double alpha = alphaMat_.lookUpMatrix(expPoint_d, tenorPoint_d);
    double beta = betaMat_.lookUpMatrix(expPoint_d, tenorPoint_d);
    double nu = nuMat_.lookUpMatrix(expPoint_d, tenorPoint_d);
    double rho = rhoMat_.lookUpMatrix(expPoint_d, tenorPoint_d);

    //LAMathSABR_Hagan sabr(alpha, beta, nu, rho);
    //return sabr.getSABRVol(expPoint_d, forward, strike);
	LAMathSABR* sabr = createSABR(approxMethod, alpha, beta, nu, rho, isLognormal);

	forward = (std::fabs(forward) < eps_SABR) ? eps_SABR : forward;

	forward += shift;
	strike += shift;

	double vol = sabr->getSABRVol(expPoint_d, forward, strike);
	delete sabr;

	return vol;
}


double LAMathSwaptionVolUtility::calcSABRVol(const double alpha, const double beta, const double nu, const double rho, const double expiryTerm, const double strike, const double forward, const double shift, const LAString& approxMethod, bool isLognormal)
{
	//LAMathSABR_Hagan sabr(alpha, beta, nu, rho);
	//return sabr.getSABRVol(expPoint_d, forward, strike);
	LAMathSABR* sabr = createSABR(approxMethod, alpha, beta, nu, rho, isLognormal);

	//forward = (std::fabs(forward) < eps_SABR) ? eps_SABR : forward;

	double fwd = forward + shift;
	double stk = strike + shift;

	double vol = sabr->getSABRVol(expiryTerm, fwd, stk);
	delete sabr;

	return vol;

}

double LAMathSwaptionVolUtility::calcSABRParam(const DoubleMatrix& paramMat, const double expiryTerm, const double tenorTerm, const DoubleVector& expiryVec, const DoubleVector& tenorVec)
{
	AQ_REQUIRE(paramMat.size() == expiryVec.size() && paramMat[0].size() == tenorVec.size(), "Size not matched.");

	LAMathSwaptionMatrix paramMat_(paramMat, expiryVec, tenorVec);

	double value = paramMat_.lookUpMatrix(expiryTerm, tenorTerm);

	return value;
}

double 
LAMathSwaptionVolUtility::getSABRVol2
( LADataInstance* dataInstance, const LAString& expPoint, const LAString& tenorPoint, double strike, const LAString& alphaID,
  const LAString& betaID, const LAString& nuID, const LAString& rhoID, const LAString& curveID, 
  const LAString& convID, LAString foreCurveName, LAString dfCurveName, const LAString& approxMethod)
{
    LAPriceDataDayCount dc_act365(ACT_365_ISDA);
    LAObjectPool& objPool = dataInstance->getObjectPool();

	const LAObject& object = objPool.getObject( convID, ENCHKTYPE_ISDEFINED ).get();
    const LAPriceDataSlidingRule& paySlr = 
        dynamic_cast<const LAPriceDataSlidingRule& >(object.getData(CALIBRATION_DATA_SLIDINGRULE,ISDEFINED).get());
    const LAPriceDataCalendar& fixCal =
        dynamic_cast<const LAPriceDataCalendar& >(object.getData(PRICING_DATA_FIXINGCALENDAR,ISDEFINED).get());
	const LAObject& curve = objPool.getObject( curveID, ENCHKTYPE_ISDEFINED ).get();
	const LADate& asOfDate = dynamic_cast<const LADataDate& >(curve.getData(CALIBRATION_DATA_ASOFDATE,ISDEFINED).get());

	double expPoint_d = LAMathSwaptionVolUtility::getExpiryPoint(expPoint, asOfDate, paySlr, fixCal);
	LADate expPoint_date = LAMathSwaptionVolUtility::getExpiryPoint2(expPoint, asOfDate, paySlr, fixCal);

	double forward = getForward(dataInstance, expPoint_date, tenorPoint, curveID, convID, foreCurveName, dfCurveName);
	forward = (std::fabs(forward) < eps_SABR) ? eps_SABR : forward;
	double tenorPoint_d = getTenorPoint(tenorPoint);

	return getSABRVol3(dataInstance, expPoint_d, tenorPoint_d, forward, strike, alphaID, betaID, nuID, rhoID, approxMethod);    
}

double 
LAMathSwaptionVolUtility::getSABRVol3
( LADataInstance* dataInstance, double expPoint, double tenorPoint, double forward, double strike, const LAString& alphaID,
  const LAString& betaID, const LAString& nuID, const LAString& rhoID, const LAString& approxMethod)
{
	LAObjectPool& objPool = dataInstance->getObjectPool();

	const DoubleMatrix& alphaMat = dynamic_cast<LADataDoubleMatrix& >
        (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
    const DoubleVector& tenorVec = dynamic_cast<const LADataDoubles& >
        (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_TENORVECTOR,ISDEFINED).get()).get();
    const DoubleVector& expiryVec = dynamic_cast<const LADataDoubles& >
        (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_EXPIRYVECTOR,ISDEFINED).get()).get();
    //const LAStringVector& expiry = dynamic_cast<const LADataStrings& >
    //    (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_EXPIRYSTRING,ISDEFINED).get()).get();
    //const LAStringVector& tenor = dynamic_cast<const LADataStrings& >
    //    (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_TENORSTRING,ISDEFINED).get()).get();

    matirixCheck(dataInstance, alphaID, betaID, true);
    const DoubleMatrix& betaMat = dynamic_cast<LADataDoubleMatrix& >
        (objPool.getObject( betaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
     matirixCheck(dataInstance, alphaID, nuID, true);
    const DoubleMatrix& nuMat = dynamic_cast<LADataDoubleMatrix& >
        (objPool.getObject( nuID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
    matirixCheck(dataInstance, alphaID, rhoID, true);
    const DoubleMatrix& rhoMat = dynamic_cast<LADataDoubleMatrix& >
        (objPool.getObject( rhoID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();

	LAMathSwaptionMatrix alphaMat_(alphaMat, expiryVec, tenorVec);
    LAMathSwaptionMatrix betaMat_(betaMat, expiryVec, tenorVec);
    LAMathSwaptionMatrix nuMat_(nuMat, expiryVec, tenorVec);
    LAMathSwaptionMatrix rhoMat_(rhoMat, expiryVec, tenorVec);

	double alpha = alphaMat_.lookUpMatrix(expPoint, tenorPoint);
    double beta = betaMat_.lookUpMatrix(expPoint, tenorPoint);
    double nu = nuMat_.lookUpMatrix(expPoint, tenorPoint);
    double rho = rhoMat_.lookUpMatrix(expPoint, tenorPoint);

	LAMathSABR* sabr = createSABR(approxMethod, alpha, beta, nu, rho);

	forward = (std::fabs(forward) < eps_SABR) ? eps_SABR : forward;
	double vol = sabr->getSABRVol(expPoint, forward, strike);
	delete sabr;

	return vol;
}

double 
LAMathSwaptionVolUtility::getSABRVol4
( LADataInstance* dataInstance, double expTerm, const LAString& tenorPoint, double forward, double strike, const LAString& alphaID,
  const LAString& betaID, const LAString& nuID, const LAString& rhoID, const LAString& curveID, 
  const LAString& convID, LAString foreCurveName, LAString dfCurveName, const LAString& approxMethod)
{
    double tenorPoint_d = getTenorPoint(tenorPoint);
	forward = (std::fabs(forward) < eps_SABR) ? eps_SABR : forward;
	return getSABRVol3(dataInstance, expTerm, tenorPoint_d, forward, strike, alphaID, betaID, nuID, rhoID, approxMethod);    
}

//++++++++ Funahashi ++++++++
double 
LAMathSwaptionVolUtility::getSABRPrem
( LADataInstance* dataInstance, const LAString& expPoint, const LAString& tenorPoint, double strike, int sgn, const LAString& fowardID, const LAString& numeraireID, 
  const LAString& alphaID, const LAString& betaID, const LAString& nuID, const LAString& rhoID, const LAString& approxMethod, double shift, bool isLognormal)
{
    LAPriceDataDayCount dc_act365(ACT_365_ISDA);
    LAObjectPool& objPool = dataInstance->getObjectPool();

	matirixCheck(dataInstance, alphaID, fowardID);
	matirixCheck(dataInstance, alphaID, betaID);
	matirixCheck(dataInstance, alphaID, nuID);
	matirixCheck(dataInstance, alphaID, rhoID);

    const DoubleMatrix& alphaMat = dynamic_cast<LADataDoubleMatrix& >
        (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
    const DoubleVector& tenorVec = dynamic_cast<const LADataDoubles& >
        (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_TENORVECTOR,ISDEFINED).get()).get();
    const DoubleVector& expiryVec = dynamic_cast<const LADataDoubles& >
        (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_EXPIRYVECTOR,ISDEFINED).get()).get();
    
    const DoubleMatrix& fowardMat = dynamic_cast<LADataDoubleMatrix &>
        (objPool.getObject( fowardID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
	const DoubleMatrix& numeraireMat = dynamic_cast<LADataDoubleMatrix &>
        (objPool.getObject( numeraireID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();

	const DoubleMatrix& betaMat = dynamic_cast<LADataDoubleMatrix &>
        (objPool.getObject( betaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
    const DoubleMatrix& nuMat = dynamic_cast<LADataDoubleMatrix &>
        (objPool.getObject( nuID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
    const DoubleMatrix& rhoMat = dynamic_cast<LADataDoubleMatrix &>
        (objPool.getObject( rhoID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();    

    const LAPriceDataSlidingRule& paySlr = dynamic_cast<const LAPriceDataSlidingRule& >(objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).
		get().getData(CALIBRATION_DATA_SLIDINGRULE,ISDEFINED).get());
    const LAPriceDataCalendar& fixCal = dynamic_cast<const LAPriceDataCalendar& >(objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().
		getData(CALIBRATION_DATA_CALENDAR,ISDEFINED).get());
    const LADate& asOfDate = dynamic_cast<const LADataDate& >(dataInstance->getObjectPool().getObject(alphaID,ENCHKTYPE_ISDEFINED).
		get().getData(CALIBRATION_DATA_ASOFDATE,ISNOTNULL).get()).get();

    double expPoint_d = getExpiryPoint(expPoint, asOfDate, paySlr, fixCal);

    LAMathSwaptionMatrix fowardMat_(fowardMat, expiryVec, tenorVec);
	LAMathSwaptionMatrix numeraireMat_(numeraireMat, expiryVec, tenorVec);
    LAMathSwaptionMatrix alphaMat_(alphaMat, expiryVec, tenorVec);
    LAMathSwaptionMatrix betaMat_(betaMat, expiryVec, tenorVec);
    LAMathSwaptionMatrix nuMat_(nuMat, expiryVec, tenorVec);
    LAMathSwaptionMatrix rhoMat_(rhoMat, expiryVec, tenorVec);
    
	double tenorPoint_d = getTenorPoint(tenorPoint);

    double forward = fowardMat_.lookUpMatrix(expPoint_d, tenorPoint_d);
	forward = (std::fabs(forward) < eps_SABR) ? eps_SABR : forward;

	double numeraire = numeraireMat_.lookUpMatrix(expPoint_d, tenorPoint_d);
    double alpha = alphaMat_.lookUpMatrix(expPoint_d, tenorPoint_d);
    double beta = betaMat_.lookUpMatrix(expPoint_d, tenorPoint_d);
    double nu = nuMat_.lookUpMatrix(expPoint_d, tenorPoint_d);
    double rho = rhoMat_.lookUpMatrix(expPoint_d, tenorPoint_d);

    //LAMathSABR_Hagan sabr(alpha, beta, nu, rho);
    //return sabr.getSABRVol(expPoint_d, forward, strike);
	LAMathSABR* sabr = createSABR(approxMethod, alpha, beta, nu, rho, isLognormal);

	forward += shift;
	strike += shift;

	double prem = sabr->getSABRPrem(expPoint_d, forward, strike, numeraire, sgn);
	delete sabr;

	return prem;
}

double 
LAMathSwaptionVolUtility::getSABRPrem2
( LADataInstance* dataInstance, const LAString& expPoint, const LAString& tenorPoint, double strike, int sgn, const LAString& alphaID,
  const LAString& betaID, const LAString& nuID, const LAString& rhoID, const LAString& curveID, 
  const LAString& convID, LAString foreCurveName, LAString dfCurveName, const LAString& approxMethod)
{
    LAPriceDataDayCount dc_act365(ACT_365_ISDA);
    LAObjectPool& objPool = dataInstance->getObjectPool();

	const LAObject& object = objPool.getObject( convID, ENCHKTYPE_ISDEFINED ).get();
    const LAPriceDataSlidingRule& paySlr = 
        dynamic_cast<const LAPriceDataSlidingRule& >(object.getData(CALIBRATION_DATA_SLIDINGRULE,ISDEFINED).get());
    const LAPriceDataCalendar& fixCal =
        dynamic_cast<const LAPriceDataCalendar& >(object.getData(PRICING_DATA_FIXINGCALENDAR,ISDEFINED).get());
	const LAObject& curve = objPool.getObject( curveID, ENCHKTYPE_ISDEFINED ).get();
	const LADate& asOfDate = dynamic_cast<const LADataDate& >(curve.getData(CALIBRATION_DATA_ASOFDATE,ISDEFINED).get());

	double expPoint_d = LAMathSwaptionVolUtility::getExpiryPoint(expPoint, asOfDate, paySlr, fixCal);
	LADate expPoint_date = LAMathSwaptionVolUtility::getExpiryPoint2(expPoint, asOfDate, paySlr, fixCal);
	double numeraire = getNumeraire(dataInstance, expPoint_date, tenorPoint, curveID, convID, dfCurveName);
	double forward = getForward(dataInstance, expPoint_date, tenorPoint, curveID, convID, foreCurveName, dfCurveName);
	double tenorPoint_d = getTenorPoint(tenorPoint);

	forward = (std::fabs(forward) < eps_SABR) ? eps_SABR : forward;

	return getSABRPrem3(dataInstance, expPoint_d, tenorPoint_d, forward, numeraire, strike, sgn, alphaID, betaID, nuID, rhoID, approxMethod);    
}

double 
LAMathSwaptionVolUtility::getSABRPrem3
( LADataInstance* dataInstance, double expPoint, double tenorPoint, double forward, double numeraire, double strike, int sgn, const LAString& alphaID,
  const LAString& betaID, const LAString& nuID, const LAString& rhoID, const LAString& approxMethod)
{
	LAObjectPool& objPool = dataInstance->getObjectPool();

	const DoubleMatrix& alphaMat = dynamic_cast<LADataDoubleMatrix& >
        (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
    const DoubleVector& tenorVec = dynamic_cast<const LADataDoubles& >
        (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_TENORVECTOR,ISDEFINED).get()).get();
    const DoubleVector& expiryVec = dynamic_cast<const LADataDoubles& >
        (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_EXPIRYVECTOR,ISDEFINED).get()).get();
    const LAStringVector& expiry = dynamic_cast<const LADataStrings& >
        (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_EXPIRYSTRING,ISDEFINED).get()).get();
    const LAStringVector& tenor = dynamic_cast<const LADataStrings& >
        (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_TENORSTRING,ISDEFINED).get()).get();

    matirixCheck(dataInstance, alphaID, betaID);
    const DoubleMatrix& betaMat = dynamic_cast<LADataDoubleMatrix& >
        (objPool.getObject( betaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
    matirixCheck(dataInstance, alphaID, nuID);
    const DoubleMatrix& nuMat = dynamic_cast<LADataDoubleMatrix& >
        (objPool.getObject( nuID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
    matirixCheck(dataInstance, alphaID, rhoID);
    const DoubleMatrix& rhoMat = dynamic_cast<LADataDoubleMatrix& >
        (objPool.getObject( rhoID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();

	LAMathSwaptionMatrix alphaMat_(alphaMat, expiryVec, tenorVec);
    LAMathSwaptionMatrix betaMat_(betaMat, expiryVec, tenorVec);
    LAMathSwaptionMatrix nuMat_(nuMat, expiryVec, tenorVec);
    LAMathSwaptionMatrix rhoMat_(rhoMat, expiryVec, tenorVec);

	double alpha = alphaMat_.lookUpMatrix(expPoint, tenorPoint);
    double beta = betaMat_.lookUpMatrix(expPoint, tenorPoint);
    double nu = nuMat_.lookUpMatrix(expPoint, tenorPoint);
    double rho = rhoMat_.lookUpMatrix(expPoint, tenorPoint);

	LAMathSABR* sabr = createSABR(approxMethod, alpha, beta, nu, rho);

	forward = (std::fabs(forward) < eps_SABR) ? eps_SABR : forward;

	double prem = sabr->getSABRPrem(expPoint, forward, strike, numeraire, sgn);
	delete sabr;

	return prem;
}

DoubleMatrix 
LAMathSwaptionVolUtility::getSABRVolMatrix
( LADataInstance* dataInstance, const LAString& strikeID, const LAString& forwardID, const LAString& alphaID,
  const LAString& betaID, const LAString& nuID, const LAString& rhoID )
{
    LAPriceDataDayCount dc_act365(ACT_365_ISDA);
    LAObjectPool& objPool = dataInstance->getObjectPool();

	matirixCheck(dataInstance, alphaID, betaID);
	matirixCheck(dataInstance, alphaID, nuID);
	matirixCheck(dataInstance, alphaID, rhoID);
	matirixCheck(dataInstance, alphaID, strikeID);
	matirixCheck(dataInstance, alphaID, forwardID);

    const DoubleMatrix& alphaMat = dynamic_cast<LADataDoubleMatrix& >
        (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();    
    const DoubleMatrix& betaMat = dynamic_cast<LADataDoubleMatrix& >
        (objPool.getObject( betaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();    
    const DoubleMatrix& nuMat = dynamic_cast<LADataDoubleMatrix& >
        (objPool.getObject( nuID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();    
    const DoubleMatrix& rhoMat = dynamic_cast<LADataDoubleMatrix& >
        (objPool.getObject( rhoID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();	
    const DoubleMatrix& strikeMat = dynamic_cast<LADataDoubleMatrix& >
        (objPool.getObject( strikeID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();	
    const DoubleMatrix& forwardMat = dynamic_cast<LADataDoubleMatrix& >
        (objPool.getObject( forwardID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_SWAPTIONMATRIX,ISDEFINED).get()).get();
	const DoubleVector& expiryVec = dynamic_cast<const LADataDoubles &>
        (objPool.getObject( alphaID, ENCHKTYPE_ISDEFINED ).get().getData(PRICING_DATA_EXPIRYVECTOR,ISDEFINED).get()).get();
    
	DoubleMatrix ret(alphaMat.size(),DoubleArray(alphaMat[0].size()));
	for(size_t i=0; i<alphaMat.size(); i++)
	{
		for(size_t j=0; j<alphaMat[0].size(); j++)
		{
			double forward(forwardMat[i][j]);
			forward = (std::fabs(forward) < eps_SABR) ? eps_SABR : forward;
			LAMathSABR_Hagan sabr(alphaMat[i][j], betaMat[i][j], nuMat[i][j], rhoMat[i][j]);
			//ret[i][j] =  sabr.getSABRVol(expiryVec[i], forwardMat[i][j], strikeMat[i][j]);
			ret[i][j] =  sabr.getSABRVol(expiryVec[i], forward, strikeMat[i][j]);
		}
	}
    
    return ret;
}

void
LAMathSwaptionVolUtility::
matirixCheck( LADataInstance* dataInstance, const LAString& entityName, const LAString& entityName2, bool isVolMat)
{
    LAObjectPool& objPool = dataInstance->getObjectPool();
	const LAObject& object = objPool.getObject( entityName, ENCHKTYPE_ISDEFINED ).get();
	const LAObject& entity2 = objPool.getObject( entityName2, ENCHKTYPE_ISDEFINED ).get();

	const LADataHolder* dh = &object.getData(PRICING_DATA_TENORSTRING, NOCHECK);
	if ( dh->isDefined() && !dh->isNull() )
	{
		const LAStringVector& tenor = dynamic_cast<const LADataStrings &>
			(object.getData(PRICING_DATA_TENORSTRING,ISDEFINED).get()).get();
		const LAStringVector& tenor2 = dynamic_cast<const LADataStrings &>
			(entity2.getData(PRICING_DATA_TENORSTRING,ISDEFINED).get()).get();
		if( tenor != tenor2 ) 
			throw LACoreInvalidData("tenors are inconsistent!",	__FILE__,__LINE__);
	}
	else
	{
		const DoubleVector& tenor = dynamic_cast<const LADataDoubles &>
			(object.getData(PRICING_DATA_TENORVECTOR,ISDEFINED).get()).get();
		const DoubleVector& tenor2 = dynamic_cast<const LADataDoubles &>
			(entity2.getData(PRICING_DATA_TENORVECTOR,ISDEFINED).get()).get();
		if( tenor != tenor2 ) 
			throw LACoreInvalidData("tenors are inconsistent!",	__FILE__,__LINE__);
	}

	dh = &object.getData(PRICING_DATA_EXPIRYSTRING, NOCHECK);
	if ( dh->isDefined() && !dh->isNull() )
	{
		const LAStringVector& expiry = dynamic_cast<const LADataStrings &>
			(object.getData(PRICING_DATA_EXPIRYSTRING,ISDEFINED).get()).get();
		const LAStringVector& expiry2 = dynamic_cast<const LADataStrings &>
			(entity2.getData(PRICING_DATA_EXPIRYSTRING,ISDEFINED).get()).get();   
		if( expiry != expiry2 ) 
			throw LACoreInvalidData("expiries are inconsistent!",	__FILE__,__LINE__);
	}
	else
	{
		const DoubleVector& expiry = dynamic_cast<const LADataDoubles &>
			(object.getData(PRICING_DATA_EXPIRYVECTOR,ISDEFINED).get()).get();
		const DoubleVector& expiry2 = dynamic_cast<const LADataDoubles &>
			(entity2.getData(PRICING_DATA_EXPIRYVECTOR,ISDEFINED).get()).get();   
		if( expiry != expiry2 ) 
			throw LACoreInvalidData("expiries are inconsistent!",	__FILE__,__LINE__);
	}

	if(isVolMat==false)
	{
		const LADate& asOfDate = dynamic_cast<const LADataDate &>
			(object.getData(CALIBRATION_DATA_ASOFDATE,ISDEFINED).get()).get();
		const LADate& asOfDate2 = dynamic_cast<const LADataDate &>
			(entity2.getData(CALIBRATION_DATA_ASOFDATE,ISDEFINED).get()).get();   
		if( asOfDate != asOfDate2 ) 
			throw LACoreInvalidData("asOfDates are inconsistent!",	__FILE__,__LINE__);

		const DateVector& expiDateVec = dynamic_cast<const LADataDates &>
			(object.getData(PRICING_DATA_EXPIRYDATEVECTOR,ISDEFINED).get()).get();
		const DateVector& expiDateVec2 = dynamic_cast<const LADataDates &>
			(entity2.getData(PRICING_DATA_EXPIRYDATEVECTOR,ISDEFINED).get()).get();   
		if( expiDateVec != expiDateVec2 ) 
			throw LACoreInvalidData("expiry date vector is inconsistent!",	__FILE__,__LINE__);
	}
}

double 
LAMathSwaptionVolUtility::
getForward(LADataInstance* dataInstance, const LADate& expiry, const LAString& tenor, const LAString& curveID, 
		   const LAString& convID, LAString foreCurveName, LAString dfCurveName, bool isFWDInter)
{
	LAObjectPool& objPool = dataInstance->getObjectPool();

	const LAObject& object = objPool.getObject( convID, ENCHKTYPE_ISDEFINED ).get();
	const LADataHolder* dh;
	dh = &object.getData(IR_CALIBRATION_DATA_FREQUENCY + LAString("_") + tenor, NOCHECK);
	if(!dh->isDefined() || dh->isNull())
	{
		dh = &object.getData(IR_CALIBRATION_DATA_FREQUENCY, ISDEFINED);
	}
	const LAString& freq = dynamic_cast<const LADataString& >(dh->get()).get();
    const LAString& spotLag = 
        dynamic_cast<const LADataString& >(object.getData(CURVEINPUT_SPOTLAG,ISDEFINED).get()).get();
    const LAPriceDataDayCount& daycount =
        dynamic_cast<const LAPriceDataDayCount& >(object.getData(IR_CALIBRATION_DATA_DAYCOUNT,ISDEFINED).get());
    const LAPriceDataSlidingRule& paySlr = 
        dynamic_cast<const LAPriceDataSlidingRule& >(object.getData(CALIBRATION_DATA_SLIDINGRULE,ISDEFINED).get());
    const LAPriceDataCalendar& payCal =
        dynamic_cast<const LAPriceDataCalendar& >(object.getData(CALIBRATION_DATA_CALENDAR,ISDEFINED).get());
    const LAPriceDataCalendar& fixCal =
        dynamic_cast<const LAPriceDataCalendar& >(object.getData(PRICING_DATA_FIXINGCALENDAR,ISDEFINED).get());

	LAPriceDataSlidingRule slr_Fol; slr_Fol.convertFromString(FOL);
    LADate tmpDate = LAMathDateCalculations::getDate(expiry,spotLag,slr_Fol,&fixCal,true);
    LADate endDate = LAMathDateCalculations::getDate(tmpDate,tenor,paySlr,&payCal,true);
    DateVector dates;
	double rate;
	if (freq == SIMPLE)
	{
		dates.push_back(endDate);	
		dates.insert(dates.begin(),tmpDate);
		rate = LAMathCurveFuncUtility::getParRate(dates, dataInstance, curveID, daycount.convertToString(), LAString("SPLINE"), foreCurveName, dfCurveName, isFWDInter);
	}
	else
	{
		//LAMathDateCalculations::generateSchedule(tmpDate, endDate, freq, true, NULL, NULL, NULL, dates, &paySlr, &payCal);
		rate = LAMathCurveFuncUtility::getParRate(dataInstance, curveID, tmpDate, endDate, NULL, NULL, NULL, freq, daycount.convertToString(), 
												paySlr.convertToString(), payCal.convertToString(), LAString("SPLINE"), foreCurveName, dfCurveName, isFWDInter);
	}

	return rate;
}

double 
LAMathSwaptionVolUtility::
getNumeraire(LADataInstance* dataInstance, const LADate& expiry, const LAString& tenor, 
			 const LAString& curveID, const LAString& convID, LAString curveName)
{
	LAPriceDataDayCount dc_act365(ACT_365_ISDA);
	LAObjectPool& objPool = dataInstance->getObjectPool();

	const LAObject& object = objPool.getObject( convID, ENCHKTYPE_ISDEFINED ).get();
	const LADataHolder* dh;
	dh = &object.getData(IR_CALIBRATION_DATA_FREQUENCY + LAString("_") + tenor,NOCHECK);
	if(!dh->isDefined() || dh->isNull())
	{
		dh = &object.getData(IR_CALIBRATION_DATA_FREQUENCY,ISDEFINED);
	}
    const LAString& freq = dynamic_cast<const LADataString& >(dh->get()).get();
    const LAString& spotLag = 
        dynamic_cast<const LADataString& >(object.getData(CURVEINPUT_SPOTLAG,ISDEFINED).get()).get();
    const LAPriceDataDayCount& daycount =
        dynamic_cast<const LAPriceDataDayCount& >(object.getData(IR_CALIBRATION_DATA_DAYCOUNT,ISDEFINED).get());
    const LAPriceDataSlidingRule& paySlr = 
        dynamic_cast<const LAPriceDataSlidingRule& >(object.getData(CALIBRATION_DATA_SLIDINGRULE,ISDEFINED).get());
    const LAPriceDataCalendar& payCal =
        dynamic_cast<const LAPriceDataCalendar& >(object.getData(CALIBRATION_DATA_CALENDAR,ISDEFINED).get());
    const LAPriceDataCalendar& fixCal =
        dynamic_cast<const LAPriceDataCalendar& >(object.getData(PRICING_DATA_FIXINGCALENDAR,ISDEFINED).get());
	const LAObject& curve = objPool.getObject( curveID, ENCHKTYPE_ISDEFINED ).get();
	const LADate& asOfDate = dynamic_cast<const LADataDate& >(curve.getData(CALIBRATION_DATA_ASOFDATE,ISDEFINED).get());

	LAPriceDataSlidingRule slr_Fol; slr_Fol.convertFromString(FOL);
	LADate tmpDate = LAMathDateCalculations::getDate(expiry,spotLag,slr_Fol,&fixCal,true);
    LADate endDate = LAMathDateCalculations::getDate(tmpDate,tenor,paySlr,&payCal,true);
    const LADate spotDate = LAMathDateCalculations::getDate(asOfDate, spotLag, slr_Fol, &fixCal, true);
	double adjust_term = dc_act365.getTerm(asOfDate, spotDate);
	double adjustDF = LAMathCurveFuncUtility::getDF(adjust_term, dataInstance, curveID, dc_act365.convertToString(), 
		LAString("SPLINE"),false,curveName);
    DateVector dates;
	if(freq==SIMPLE)
	{
		dates.push_back(endDate);	
	}
	else
	{
		LAMathDateCalculations::generateSchedule(tmpDate, endDate, freq, true, NULL, NULL, NULL, dates, &paySlr, &payCal);
	}
    dates.insert(dates.begin(),tmpDate);

	return LAMathCurveFuncUtility::getAnnuity(dates, dataInstance, curveID, daycount.convertToString(), LAString("SPLINE"),
		curveName) / adjustDF;
}

double 
LAMathSwaptionVolUtility::
getExpiryPoint(LAString str, const LADate& asOfDate, const LAPriceDataSlidingRule& slr, 
			   const LAPriceDataCalendar& cal)
{
	LAPriceDataDayCount dc_act365(ACT_365_ISDA);
	LADate date;
	double ret;
	str.toUpper();
	int y_pos=-1, m_pos=-1, d_pos=-1, dot_pos=-1;
	y_pos = str.findString("Y");
	m_pos = str.findString("M");
	d_pos = str.findString("D");

	if (y_pos == -1 && m_pos == -1 && d_pos == -1) 
	{
		date = getLADate(str.getCString());
		ret = dc_act365.getTerm(asOfDate, date);
	}
	else
	{
		date = LAMathDateCalculations::getDate(asOfDate,str,slr,&cal,true);
		ret = dc_act365.getTerm(asOfDate, date);
	}

	return ret;
}

LADate 
LAMathSwaptionVolUtility::
getExpiryPoint2(LAString str, const LADate& asOfDate, const LAPriceDataSlidingRule& slr, 
				const LAPriceDataCalendar& cal)

{
	LADate date;
	str.toUpper();
	int y_pos=-1, m_pos=-1, d_pos=-1;
	y_pos = str.findString("Y");
	m_pos = str.findString("M");
	d_pos = str.findString("D");
	if (y_pos == -1 && m_pos == -1 && d_pos == -1) 
	{
		date = getLADate(str.getCString());
	}
	else
	{
		date = LAMathDateCalculations::getDate(asOfDate,str,slr,&cal,true);
	}

	return date;
}

double 
LAMathSwaptionVolUtility::
getTenorPoint(LAString str)
{
	LADate date;
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
		LAMathDateCalculations::termStrtoYMDW(str, y_pos, m_pos, d_pos, w_pos);
		ret = static_cast<double > (y_pos) + static_cast<double > (m_pos) / 12 + static_cast<double > (w_pos) / 12 / 4 + static_cast<double > (d_pos) / 365.25;
	}

	return ret;
}

LADate
LAMathSwaptionVolUtility::getLADate(LAString date)
{
    return LAStringToDate(date);
	//LADate ret;
	//int slushCheck = date.findString("/");
	//if(slushCheck==-1)
	//{
	//	ret =  LAMathDateUtilities::getLADate(date);
	//}
	//else
	//{
	//	if(slushCheck==4) date.remove(4,1);
	//	else throw LACoreInvalidData("Input error",__FILE__,__LINE__);

	//	slushCheck = date.findString("/");
	//	if(slushCheck==6) { date.remove(6,1);}
	//	else if(slushCheck==5)
	//	{
	//		date.remove(5,1);
	//		date.insert(4,"0");
	//	}
	//	else throw LACoreInvalidData("Input error",__FILE__,__LINE__);

	//	if(date.size() == 7)
	//	{
	//		date.insert(6,"0");
	//	}
	//	else if(date.size() != 8) throw LACoreInvalidData("Input error",__FILE__,__LINE__);
	//	LADate ret_(date.getCString()); ret = ret_;
	//}
	//
	//return ret;
}

LAMathSABR*
LAMathSwaptionVolUtility::createSABR(const LAString& approxMethod, double alpha, double beta, double nu, double rho, bool isLognormal)
{
	LAMathSABR* sabr;
	LAString tmp_approxMethod = approxMethod;
	tmp_approxMethod.toUpper();

	//Only Hagan method support normal vol
	if (!isLognormal && tmp_approxMethod != APPROXIMATION_HAGAN)
	{
		LAString msg = "Normal Vol Calibration Only Supports Hagan Approximation : Method " + approxMethod + " is not supported";
		throw LACoreInvalidData(msg.getCString(), __FILE__, __LINE__);
	}

	if(tmp_approxMethod == APPROXIMATION_HAGAN)
	{
	    sabr = new LAMathSABR_Hagan(alpha, beta, nu, rho, isLognormal);
	}
	else if(tmp_approxMethod == APPROXIMATION_ANTONOV)
	{
	    sabr = new LAMathSABR_Antonov(alpha, beta, nu, rho);
	}
	else if(tmp_approxMethod == APPROXIMATION_CHAOS_DD)
	{
	    sabr = new LAMathSABR_Chaos_DD(alpha, beta, nu, rho);
	}
	else
	{
		AQ_THROW("Lognormal Vol Calibration Only SUpports Hagan, Antonov and Chaos Displaced Diffusion methods : Method " + approxMethod + " is not supported")
	}
	return sabr;
}


void
LAMathSwaptionVolUtility::setUpForwardShiftValue(LADataInstance* dataInstance, LAString& CurrencyID, double forwardShiftValue)
{

	upper(CurrencyID);
	LAObjectPool& objPool = dataInstance->getObjectPool();
	LAObject* e = NULL;
	if(!objPool.getObject(CurrencyID).isDefined())
	{
		e = new LAObject();
		objPool.set(CurrencyID,e);
	}
	else 
	{
		objPool.getObject(CurrencyID,ENCHKTYPE_ISDEFINED).clear();
		e = &objPool.getObject(CurrencyID).get();
	}

	e->add(CurrencyID, new LADataDouble(forwardShiftValue)	);
}

double
LAMathSwaptionVolUtility::getForwardShiftValue(LADataInstance* dataInstance, LAString& CurrencyID )
{
	upper(CurrencyID);

	LAObjectPool& objPool = dataInstance->getObjectPool();
	LAObject& e = objPool.getObject(CurrencyID, ENCHKTYPE_ISDEFINED).get();
	
	LADataHolder* dh;

	dh = &(e.getData(CurrencyID, ISNOTNULL));
	double forwardShiftValue = dynamic_cast<const LADataDouble &>(dh->get()).get();
	
	return forwardShiftValue;
}