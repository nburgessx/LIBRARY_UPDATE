#ifdef __GNUG__
#pragma implementation
#else
#pragma warning(disable:4786)
#endif

#include "AQLMathJamshidianSwaption.h"
#include "AQLObject.h"
#include "AQLDataProcedure.h"
#include "AQLDataBasics.h"
#include "AQLDataVector.h"
#include "AQLDataReference.h"
#include "AQLDataMultiReference.h"
#include "AQLDataInstance.h"
#include "AQLPriceDataManager.h"
#include "AQLObjectPool.h"
#include "AQLCoreTemplateType.h"
#include "AQLMathDefine.h"
#include "AQLPriceDataCalendar.h"
#include "AQLPriceDataSlidingRule.h"
#include "AQLPriceDataDayCount.h"
#include "AQLDataReference.h"
#include "AQLPriceDataFunction.h"
#include "AQLBasic.h"
#include "AQLAlgorithm.h"
#include "AQLMathDateCalculations.h"
#include "AQLPriceCFGenUtility.h"
#include "AQLLinearInterpolation.h"
#include "AQLSplineInterpolation.h"
#include "AQLStepInterpolation.h"
#include "AQLModelDynamicsHW1FCurve.h"

#include "AQLCoreComponentManager.h"

using namespace std;

AQLMathJamshidianSwaption::AQLMathJamshidianSwaption()
: AQLCoreValuation()
{}

AQLMathJamshidianSwaption::~AQLMathJamshidianSwaption()
{
}

/*!
    @brief Return this function type
    @return function type
*/

function_t
AQLMathJamshidianSwaption::getType() const
{
	return FN_JAMSHIDIANSWAPTION;
}


/*!
	@brief register dataValues that this class uses

	@param[in, out] dm data master 
*/
void
AQLMathJamshidianSwaption::registerData(AQLPriceDataManager& dm) const
{
	dm.setData(PRICING_DATA_CURVEID,				DATA_REFERENCE);
	dm.setData(PRICING_DATA_SDEINTEGRALGRID,		DATA_INT);
	dm.setData(PRICING_DATA_CALIBCANONICAL_T,		DATA_DOUBLES);
	dm.setData(PRICING_DATA_CALIBVOL_T,			DATA_DOUBLES);
	dm.setData(PRICING_DATA_CALIBMEANREV_T,		DATA_DOUBLES);
	dm.setData(PRICING_DATA_CALIBVARIABLES,		DATA_DOUBLES);
	dm.setData(IR_CALIBRATION_DATA_OPTIONMATURITY,			DATA_STRING);
	dm.setData(IR_CALIBRATION_DATA_SWAPTENOR,				DATA_STRING);
	dm.setData(PRICING_DATA_OPTIONTYPE,			DATA_STRING);
	dm.setData(PRICING_DATA_MODELPARAM,			DATA_REFERENCE);
	dm.setData(IR_CALIBRATION_DATA_PAYMENTCALENDAR,			DATA_CALENDAR);
	dm.setData(IR_CALIBRATION_DATA_BLACKVOLATILITY,			DATA_DOUBLE);
	dm.setData(PRICING_DATA_STRIKE,				DATA_DOUBLE);
	dm.setData(PRICING_DATA_OPTIONPREMIUM,			DATA_DOUBLE);
	dm.setData(PRICING_DATA_WEIGHT,				DATA_DOUBLE);
	dm.setData(IR_MODEL_DATA_FREQUENCY,				DATA_STRING);
	dm.setData(IR_MODEL_DATA_DAYCOUNT,				DATA_DAYCOUNT);
	dm.setData(PRICING_DATA_FIXINGCALENDAR,		DATA_CALENDAR);
	dm.setData(CALIBRATION_DATA_SLIDINGRULE,					DATA_SLIDINGRULE);
	dm.setData(PRICING_CALIBRATION_DATAOTICEPERIOD,			DATA_STRING);
	dm.setData(PRICING_DATA_ISTIMEDEPENDMEANREV,	DATA_BOOL);
}

AQLCoreFunctionBase*
AQLMathJamshidianSwaption::clone() const
{
    try 
	{
		return new AQLMathJamshidianSwaption(*this);
    }
    catch (bad_alloc & e)
	{
        throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
    }
}


/*!
	@brief value trade

	@param[in] basedate evaluate day
	@param[in,out] object trade object object(reference to AQLMathObjectValue class) 
	@param[in] att Data to hold evaluation procedure class

	@return swaption prem
	
*/
double
AQLMathJamshidianSwaption::value(const AQLDate& basedate, AQLObject& object,
					const AQLDataValuation& att) const
{
	AQLDataHolder* dh;
	AQLMathJamshidianSwaptionDataProvider* dataProvider = NULL;
	if (att.isNullDataProvider())
		dataProvider = dynamic_cast<AQLMathJamshidianSwaptionDataProvider*>(setUpDataProvider(basedate, object, att));
	else
		dataProvider = &dynamic_cast<AQLMathJamshidianSwaptionDataProvider&>(att.getDataProvider());

	//pcurve pcurve0
	AQLRatesPathElementHW1FCurveTMDPT* pcurve = dynamic_cast<AQLRatesPathElementHW1FCurveTMDPT* >(dataProvider->mpCurve);
	AQLMathPathYieldCurve* pcurve0 = dynamic_cast<AQLMathPathYieldCurve* >(dataProvider->mpCurve0);
	//clear cache
	pcurve->clear_cache();
	//set variables to curve
	dh = &(object.getData(PRICING_DATA_MODELPARAM,ISNOTNULL));
	AQLObject& models= dynamic_cast<AQLDataReference&>(dh->get()).get().get();
	dh = &(models.getData(PRICING_DATA_CALIBVARIABLES,ISNOTNULL));
	double volval = dynamic_cast<AQLDataDoubles&>(dh->get()).get()[0];
	for(unsigned int i = dataProvider->mStartpos ;i<dataProvider->mEndpos;i++)
		dataProvider->mModelVol[i] = volval;
	dynamic_cast<AQLMathHWFuncSigmaTMDPT* >(pcurve->mpHWtoolMR->getHWSigma())->set_s(dataProvider->mModelVol);
	dynamic_cast<AQLMathHWFuncSigmaTMDPT* >(pcurve->mpHWtoolVar->getHWSigma())->set_s(dataProvider->mModelVol);
	
	//find rstar
	AQLMathJamshidianRStarFinder* mprStar = dataProvider->mprStar;
	DoubleArray rStarvec(1,dataProvider->mrStar);
	dataProvider->mDataInstancefind.findRoot(*mprStar,dataProvider->mBound,rStarvec);
	dataProvider->mrStar = rStarvec[0];
	dynamic_cast<AQLRatesPathElementBase *>(pcurve)->set(dataProvider->mrStar);
	pcurve->set_t(dataProvider->mGrids[0]);
	
	//pricing swaption
	unsigned int N = dataProvider->mDelta.size();
	AnalyticAFFParam& pm = dataProvider->mAFFParam;
	double simannuity=0.0 , zboprem=0.0;
	double annuity=0.0;
	pm.Poptm= pcurve0->getP(dataProvider->mGrids[0]);
	double v = AQLMath::sqrt( dataProvider->mpGL->integrate(*pcurve->mpHWtoolVar,0.0,dataProvider->mGrids[0]) );
	double adj = pcurve->mpHWtoolMR->ExpIntegralInvMR(dataProvider->mGrids[0]);
	for(unsigned int i=0;i<N;i++)
	{
		pm.Pbondm	= pcurve0->getP(dataProvider->mGrids[i+1]);
		double b = dynamic_cast<AQLRatesPathElementHW1FCurve *>(pcurve)->B(dataProvider->mGrids[i+1]);
		pm.Vol = b*v*adj;
		pm.K = pcurve->getP(dataProvider->mGrids[i+1]);
		zboprem			= dataProvider->mAnalyticMethod->calc(pm); 
		simannuity+= zboprem * dataProvider->mDelta[i] ;
		annuity+= pm.Pbondm * dataProvider->mDelta[i];
	}
	dataProvider->mSimPrem = dataProvider->mStrike * simannuity + zboprem;
	dataProvider->mAnnuity = annuity; 
	double ret = 10000 * AQLMath::abs(dataProvider->mPrem - dataProvider->mSimPrem) * dataProvider->mWeight;
	return ret;
}

/*!
	@brief setup cashe class

	@param[in] basedate basedate of valuation
	@param[in] object trade
	@param[in] att AQLDataValuation class that this valuation class is setted

	@return cashe class
	
*/
AQLDataProvider*					
AQLMathJamshidianSwaption::setUpDataProvider(const AQLDate& basedate, AQLObject& object, 
											const AQLDataValuation& att) const
{
	//temporary
	AQLDate asof = basedate;

	AQLDataHolder* dh;
	AQLMathJamshidianSwaptionDataProvider* dataProvider = dynamic_cast<AQLMathJamshidianSwaptionDataProvider*>(createNewDataProvider());
	att.setDataProvider(dataProvider);

	//mStrike
	dh = &(object.getData(PRICING_DATA_STRIKE,ISNOTNULL));
	double strike = dynamic_cast<AQLDataDouble &>(dh->get()).get();
	dataProvider->mStrike = strike;

	dh = &(object.getData(IR_CALIBRATION_DATA_BLACKVOLATILITY,NOCHECK));
	double vol = 0.0;
	if(dh->isDefined() && !dh->isNull())
		vol = dynamic_cast<AQLDataDouble &>(dh->get()).get();
	dataProvider->mVol = vol;
	
	//mPrem
	dh = &(object.getData(PRICING_DATA_OPTIONPREMIUM,ISNOTNULL));
	double premium = dynamic_cast<AQLDataDouble &>(dh->get()).get();
	dataProvider->mPrem = premium;

	//mWeight
	dh = &(object.getData(PRICING_DATA_WEIGHT,ISNOTNULL));
	double weight = dynamic_cast<AQLDataDouble &>(dh->get()).get();
	dataProvider->mWeight = weight;

	//mrStar
	dh = &(object.getData(PRICING_DATA_INITIALRSTAR,ISNOTNULL));
	double rstar = dynamic_cast<AQLDataDouble&>(dh->get()).get();
	dataProvider->mrStar = rstar;

	//mAnalyticMethod
	dh = &(object.getData(PRICING_DATA_OPTIONTYPE,ISNOTNULL));
	AQLString optiontype = dynamic_cast<AQLDataString &>(dh->get()).get();
	optiontype.toUpper();
	if("PAYERS" == optiontype)
		optiontype= "PUT";
	else if("RECEIVERS" == optiontype)
		optiontype = "CALL";
	else
		throw AQLCoreInvalidData("Error or OptonType",__FILE__,__LINE__);
	AQLString bscomponent = AQLString(AFF) + AQLString(PREM)  + optiontype;
	std::map<AQLString, AQLBlackScholesBase*> &var = AQLCoreComponentManager::getBlackComponentMap();
	std::map<AQLString, AQLBlackScholesBase*> ::iterator it = var.find(bscomponent);
	if(it==var.end())
		throw AQLCoreInvalidData("Option type is not supported",__FILE__,__LINE__);
	dataProvider->mAnalyticMethod = it->second;

	//mGridMat mDeltaMat
	dh = &(object.getData(IR_CALIBRATION_DATA_OPTIONMATURITY,ISNOTNULL));
	AQLString optionmatu = dynamic_cast<AQLDataString &>(dh->get()).get();

	dh = &(object.getData(IR_CALIBRATION_DATA_SWAPTENOR,ISNOTNULL));
	AQLString swapterm = dynamic_cast<AQLDataString &>(dh->get()).get();

	dh = &(object.getData(CALIBRATION_DATA_SLIDINGRULE,ISNOTNULL));
	const AQLPriceDataSlidingRule& sr = dynamic_cast<const AQLPriceDataSlidingRule &>(dh->get());

	dh = &(object.getData(PRICING_DATA_FIXINGCALENDAR,ISNOTNULL));
	const AQLPriceDataCalendar& fixcal = dynamic_cast<const AQLPriceDataCalendar &>(dh->get());

	dh = &(object.getData(IR_CALIBRATION_DATA_PAYMENTCALENDAR,ISNOTNULL));
	const AQLPriceDataCalendar& paycal = dynamic_cast<const AQLPriceDataCalendar &>(dh->get());

	dh = &(object.getData(IR_MODEL_DATA_DAYCOUNT,ISNOTNULL));
	const AQLPriceDataDayCount& daycount = dynamic_cast<const AQLPriceDataDayCount &>(dh->get());
	
	dh = &(object.getData(PRICING_CALIBRATION_DATAOTICEPERIOD,ISNOTNULL));
	AQLString spotlag = dynamic_cast<AQLDataString &>(dh->get()).get();
	
	dh = &(object.getData(IR_MODEL_DATA_FREQUENCY,ISNOTNULL));
	AQLString frequency = dynamic_cast<AQLDataString &>(dh->get()).get();

	AQLDate expdate = AQLMathDateCalculations::getDate(asof,optionmatu,sr,&fixcal,true);
	AQLDate valuedate = AQLMathDateCalculations::getDate(asof,spotlag,sr,&fixcal,true);
	AQLDate tmpdate = AQLMathDateCalculations::getDate(expdate,spotlag,sr,&fixcal,true);
	
	AQLPriceDataDayCount dc(ACT_365_ISDA);
	AQLDate swapenddate = AQLMathDateCalculations::getDate(tmpdate,swapterm,sr,&paycal,true);
	DateVector datevec;
	AQLMathDateCalculations::generateSchedule(expdate,swapenddate,frequency,true,NULL,NULL,NULL,datevec,&sr,&paycal);
	
	datevec.insert(datevec.begin(),expdate);
	DoubleArray gridvec(datevec.size(),0.0);
	DoubleArray testgridvec(datevec.size(),0.0);
	DoubleArray deltavec(datevec.size()-1,0.0);
	DoubleArray testdeltavec(datevec.size()-1,0.0);
	for(unsigned int j = 0;j < gridvec.size();j++)
		gridvec[j] = dc.getTerm(asof,datevec[j],false);
	for(unsigned int j = 0;j < gridvec.size()-1;j++)
		deltavec[j] = daycount.getTerm(datevec[j],datevec[j+1],false);
	dataProvider->mGrids = gridvec;
	dataProvider->mDelta = deltavec;

	//mModelGrid mModelVol
	dh = &(object.getData(PRICING_DATA_MODELPARAM,ISNOTNULL));
	AQLObject& models= dynamic_cast<AQLDataReference&>(dh->get()).get().get();
	dh = &(models.getData(PRICING_DATA_CALIBCANONICAL_T,ISNOTNULL));
	DoubleVector calib_T = dynamic_cast<AQLDataDoubles&>(dh->get()).get();
	dh = &(models.getData(PRICING_DATA_CALIBVOL_T,ISNOTNULL));
	DoubleVector vol_T = dynamic_cast<AQLDataDoubles&>(dh->get()).get();
	dataProvider->mModelGrid = calib_T;
	dataProvider->mModelVol = vol_T;

	//mStartpos
	unsigned int spos = 0;
	dh = &(object.getData(PRICING_DATA_STARTPOSITION,ISNOTNULL));
	spos = dynamic_cast<AQLDataInt&>(dh->get()).get();
	dataProvider->mStartpos = spos;
	
	//mEndpos
	unsigned int epos = 0;
	DoubleArray::iterator its;
	AQLAlgorithm::locate(calib_T,gridvec[0],calib_T.size(),epos);
	if (epos == calib_T.size())
	{
		--epos;
	}
	else
	{
		double diff = AQLMath::abs(calib_T[epos] - gridvec[0]); 
		if (diff > AQLMath::abs(gridvec[0] - calib_T[epos - 1]))
		{
			--epos;
		}
	}
	//check whether HWVolterm is longer than Swaption maturity;
	if(gridvec[0] > calib_T.back())
		throw AQLCoreInvalidData("HWVolTerm must be longer than Swaption maturity",__FILE__,__LINE__); 
	dataProvider->mEndpos = epos;
	
	//mpGL
	delete dataProvider->mpGL;
	dataProvider->mpGL = new AQLGaussLegendre(dataProvider->mEndpos*2);
	dataProvider->mBound.resize(1);
	//dataProvider->mBound[0].first  = -1.0;
	//dataProvider->mBound[0].second = 0.1;
	dataProvider->mBound[0].first  = -2.0;
	dataProvider->mBound[0].second = 0.2;
	//mpCurve0
	dh = &(models.getData(PRICING_DATA_CURVEID,ISNOTNULL));
	AQLObject& yldentity = dynamic_cast<AQLDataReference &>(dh->get()).get().get();
	
	dh = &(yldentity.getData(CALIBRATION_DATA_NAME,ISNOTNULL));
	AQLString yldname = dynamic_cast<AQLDataString &>(dh->get()).get();

	// create tmp curve
	delete dataProvider->mpTmpCurve;
	AQLDataInstance* pDataInstance = object.getDataInstance();
	dataProvider->mpTmpCurve = new AQLMathYieldCurve(pDataInstance);
	dataProvider->mpTmpCurve->getData(IR_CALIBRATION_DATA_YIELDDATA,ISDEFINED).convertFromString(yldname);
	dataProvider->mpTmpCurve->setInterpolation(FN_SPLINEINTERPOLATION_STR);
	AQLMathPathYieldCurve* curve0 = new AQLMathPathYieldCurve(dataProvider->mpTmpCurve,0.0,ACT_365_ISDA);

	delete dataProvider->mpCurve0;
	dataProvider->mpCurve0 = curve0;
	
	//mpCurve
	dh = &(models.getData(PRICING_DATA_CALIBMEANREV_T,ISNOTNULL));
	const DoubleArray& a_T = dynamic_cast<const AQLDataDoubles &>(dh->get()).get();
	double a0 = dynamic_cast<AQLDataDoubles &>(dh->get()).get()[0];
	dh = &(models.getData(PRICING_DATA_SDEINTEGRALGRID,ISNOTNULL));
	int integralgrid = dynamic_cast<AQLDataInt &>(dh->get()).get();

	dh = &(models.getData(PRICING_DATA_ISTIMEDEPENDMEANREV,ISNOTNULL));
	bool isTMREV = dynamic_cast<const AQLDataBool &>(dh->get()).get();
	AQLMathHWFuncMR* HW_a = 0;
	if (isTMREV)
	{
		HW_a = new AQLMathHWFuncMRTMDPT(calib_T,a_T,*(new AQLStepInterpolation()));
	}
	else
	{
		HW_a = new AQLMathHWFuncMR(a0);
	}
	AQLMathHWFuncSigmaTMDPT* HW_s = new AQLMathHWFuncSigmaTMDPT(calib_T,vol_T,*(new AQLStepInterpolation()));
	double r0 = curve0->getF(0.0001,0.0001);
	
	AQLRatesPathElementCurve* curve0_HW = new AQLRatesPathElementHW1FCurveTMDPT(0.0,r0,*HW_a,*HW_s, *curve0);
	delete dataProvider->mpCurve;
	dataProvider->mpCurve = curve0_HW;
	//mprStar
	delete dataProvider->mprStar;
	dataProvider->mprStar = new AQLMathJamshidianRStarFinder(dataProvider->mStrike,dataProvider->mPrem,dataProvider->mGrids,dataProvider->mDelta,dataProvider->mpCurve);

	//set curve to modelVol
	AQLRatesPathElementHW1FCurveTMDPT* ptmp = dynamic_cast<AQLRatesPathElementHW1FCurveTMDPT*>(dataProvider->mpCurve);
	dynamic_cast<AQLMathHWFuncSigmaTMDPT* >(ptmp->mpHWtoolMR->getHWSigma())->set_s(dataProvider->mModelVol);
	dynamic_cast<AQLMathHWFuncSigmaTMDPT* >(ptmp->mpHWtoolVar->getHWSigma())->set_s(dataProvider->mModelVol);

	//adjterm and adjdf
	double adjterm = dc.getTerm(asof,valuedate,false);
	dataProvider->mAdjTerm = adjterm;
	double adjdf = dataProvider->mpTmpCurve->getDF(asof,valuedate);
	dataProvider->mAdjDF	= adjdf;
	return dataProvider;
}

DoubleVector	
AQLMathJamshidianSwaption::getVolatilityResult(const AQLDataValuation& att) const 
{
	AQLMathJamshidianSwaptionDataProvider* dataProvider = NULL;
	if (att.isNullDataProvider())
		throw AQLCoreInvalidData("JamshidianSwaptionDataProvider does not exist",__FILE__, __LINE__);
	else
		dataProvider = &dynamic_cast<AQLMathJamshidianSwaptionDataProvider&>(att.getDataProvider());

	return dataProvider->mModelVol;

}
unsigned int				
AQLMathJamshidianSwaption::getNextPos(const AQLDataValuation& att) const
{
	AQLMathJamshidianSwaptionDataProvider* dataProvider = NULL;
	if (att.isNullDataProvider())
		throw AQLCoreInvalidData("JamshidianSwaptionDataProvider does not exist",__FILE__, __LINE__);
	else
		dataProvider = &dynamic_cast<AQLMathJamshidianSwaptionDataProvider&>(att.getDataProvider());

	return dataProvider->mEndpos;
}

double						
AQLMathJamshidianSwaption::getNextRstar(const AQLDataValuation& att) const
{
	AQLMathJamshidianSwaptionDataProvider* dataProvider = NULL;
	if (att.isNullDataProvider())
		throw AQLCoreInvalidData("JamshidianSwaptionDataProvider does not exist",__FILE__, __LINE__);
	else
		dataProvider = &dynamic_cast<AQLMathJamshidianSwaptionDataProvider&>(att.getDataProvider());

	return dataProvider->mrStar;
}

double						
AQLMathJamshidianSwaption::getSimPrem(const AQLDataValuation& att) const
{
	AQLMathJamshidianSwaptionDataProvider* dataProvider = NULL;
	if (att.isNullDataProvider())
		throw AQLCoreInvalidData("JamshidianSwaptionDataProvider does not exist",__FILE__, __LINE__);
	else
		dataProvider = &dynamic_cast<AQLMathJamshidianSwaptionDataProvider&>(att.getDataProvider());

	return dataProvider->mSimPrem;
}

/*!
	@brief create new cache class
	@return cache class
*/
AQLDataProvider*
AQLMathJamshidianSwaption::createNewDataProvider() const
{
	AQLMathJamshidianSwaptionDataProvider* dataProvider = NULL;
	try 
	{
		dataProvider = new AQLMathJamshidianSwaptionDataProvider();
	}
	catch (bad_alloc & e)
	{
		throw AQLCoreSystemError(e.what(), __FILE__, __LINE__);
	}
	return dataProvider;
}

AQLMathJamshidianSwaption::AQLMathJamshidianSwaptionDataProvider::AQLMathJamshidianSwaptionDataProvider()
: mStrike(0.0),mPrem(0.0),mGrids(DoubleVector()),mDelta(DoubleVector()),
mModelGrid(DoubleVector()),mModelVol(DoubleVector()),mStartpos(0),mEndpos(0),mrStar(0.0),
mprStar(0),mpCurve(0),mpCurve0(0),mpTmpCurve(0),mAnalyticMethod(0),mAFFParam(),mpGL(0),mDataInstancefind(),mBound()
,mSimPrem(0.0),mVol(0.0),mAnnuity(0.0),mAdjTerm(0.0),mAdjDF(1.0)
{	
}

AQLMathJamshidianSwaption::AQLMathJamshidianSwaptionDataProvider::~AQLMathJamshidianSwaptionDataProvider()
{
	if(mprStar)
		delete	mprStar;
	if(mpCurve)
		delete	mpCurve;
	if(mpCurve0)
		delete	mpCurve0;
	if(mpTmpCurve)
		delete mpTmpCurve;
	if(mpGL)
		delete mpGL;
}
